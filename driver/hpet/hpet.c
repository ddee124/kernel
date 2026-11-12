#include "driver/hpet/hpet.h"
#include "driver/acpi/acpi.h"
#include "driver/apic/apic.h"
#include "mem/pagetable.h"
#include "kernel/printk.h"
#include "lib/lib.h"
#include "lib/io.h"
#include "interrupt/interrupt.h"
#include "arch/x86_64/ptrace.h"

/* register offsets inside the hpet memory block */
#define HPET_GCAP	0x000	/* general capabilities, read only */
#define HPET_GCONF	0x010	/* general configuration */
#define HPET_GISR	0x020	/* general interrupt status, write 1 to clear */
#define HPET_MCV	0x0f0	/* main counter value */
#define HPET_TN_CFG	0x100	/* timer0 configuration */
#define HPET_TN_CMP	0x108	/* timer0 comparator */

/* general configuration bits */
#define HPET_CFG_ENABLE		0x0001
#define HPET_CFG_LEGACY		0x0002

/* timer0 configuration bits, IA-PC HPET spec 1.0a */
#define HPET_TN_LEVEL		0x0002	/* 0 = edge, 1 = level */
#define HPET_TN_ENABLE		0x0004
#define HPET_TN_PERIODIC	0x0008
#define HPET_TN_SETVAL		0x0040
#define HPET_TN_32BIT		0x0100
#define HPET_TN_ROUTE		0x3e00
#define HPET_TN_FSB		0x4000

#define HPET_IRQ	0x22		/* legacy replacement -> irq0 -> gsi2 */
#define HPET_TRACE	1

static volatile uint64_t* hpet=0;
static uint64_t hpet_period_fs=0;
static uint64_t hpet_interval=0;	/* main counter ticks per 1ms */
static uint64_t hpet_ticks=0;		/* 1ms interrupts since init */

static struct hw_int_controller hpet_int_controller={
	.enable=IOAPIC_enable,
	.disable=IOAPIC_disable,
	.install=IOAPIC_install,
	.uninstall=IOAPIC_uninstall,
	.ack=IOAPIC_edge_ack,
};
static inline uint64_t hpet_read(uint32_t reg){return hpet[reg>>3];}
static inline void hpet_write(uint32_t reg,uint64_t val){hpet[reg>>3]=val;}

static void hpet_handler(uint64_t nr,uint64_t parameter,struct pt_regs* regs){
	hpet_ticks++;
	hpet_write(HPET_GISR,1);	/* clear timer0 status */
#if HPET_TRACE
	if(!(hpet_ticks%1000))
		color_printk(CYAN,BLACK,"[hpet] %lu s\n",hpet_ticks/1000);
#endif
}
uint64_t HPET_frequency(){
	if(hpet_period_fs==0)	return 0;
	return 1000000000000000ul/hpet_period_fs;
}
uint64_t HPET_ticks(){return hpet_ticks;}
void HPET_init(){
	if(hpet_table==0){
		color_printk(RED,BLACK,"HPET: no ACPI table\n");
		return;
	}
	if(hpet_table->address_space_id!=0){
		color_printk(RED,BLACK,"HPET: not memory mapped,space id:%d\n",hpet_table->address_space_id);
		return;
	}
	uint64_t phy=hpet_table->address&(~0xffful);
	map_4K_page(phy,PAGE_PCD);
	hpet=(volatile uint64_t*)Phy_To_Virt(phy);
	uint64_t caps=hpet_read(HPET_GCAP);
	uint64_t gconf=hpet_read(HPET_GCONF);
	if(caps==0||caps==~0ul||gconf==~0ul){
		color_printk(RED,BLACK,"HPET: no device at %#018lx\n",phy);
		hpet=0;
		return;
	}
	hpet_period_fs=caps>>32;
	if(hpet_period_fs<1000000||hpet_period_fs>100000000){
		color_printk(RED,BLACK,"HPET: bad period %lufs\n",hpet_period_fs);
		hpet=0;
		return;
	}
	uint64_t timers=((caps>>8)&0x1f)+1;
	hpet_interval=1000000000000ul/hpet_period_fs;	/* 1ms */
	if(hpet_interval==0)	hpet_interval=1;
	color_printk(GREEN,BLACK,"HPET: rev:%lu timers:%lu period:%lufs,cap:%#018lx\n",
		caps&0xff,timers,hpet_period_fs,caps);
	color_printk(GREEN,BLACK,"HPET: gconf:%#lx,legacy capable:%lu,%lu ticks/ms\n",
		gconf,(caps>>15)&1,hpet_interval);
	if(!((caps>>15)&1)){
		color_printk(RED,BLACK,"HPET: no legacy replacement route\n");
		hpet=0;
		return;
	}
	/* mask the 8259s,then let legacy replacement routing take irq0 away
	   from the 8254 (the chipset gates the pit output in that mode) */
	io_out8(0x21,0xff);
	io_out8(0xa1,0xff);
	/* stop the counter and clear the pending status */
	hpet_write(HPET_GCONF,hpet_read(HPET_GCONF)&(~(HPET_CFG_ENABLE|HPET_CFG_LEGACY)));
	hpet_write(HPET_MCV,0);
	hpet_write(HPET_GISR,1);
	/* edge triggered (HPET_TN_LEVEL clear),periodic,32 bit,like linux */
	uint64_t cfg=hpet_read(HPET_TN_CFG);
	cfg&=~(HPET_TN_ENABLE|HPET_TN_LEVEL|HPET_TN_FSB|HPET_TN_PERIODIC|HPET_TN_SETVAL|HPET_TN_32BIT|HPET_TN_ROUTE);
	cfg|=HPET_TN_ENABLE|HPET_TN_PERIODIC|HPET_TN_SETVAL|HPET_TN_32BIT;
	hpet_write(HPET_TN_CFG,cfg);
	hpet_write(HPET_TN_CMP,hpet_interval);		/* first write: loads the accumulator */
	{
		uint64_t i;
		for(i=0;i<100000;i++)	nop();		/* ~1us,spec wants a delay here */
	}
	hpet_write(HPET_TN_CMP,hpet_interval);		/* second write: sets the period */
	struct IO_APIC_RET_entry entry;
	entry.vector=HPET_IRQ;
	entry.deliver_mode=APIC_ICR_IOAPIC_Fixed;
	entry.dest_mode=ICR_IOAPIC_DELV_PHYSICAL;
	entry.deliver_status=APIC_ICR_IOAPIC_Idle;
	entry.polarity=APIC_IOAPIC_POLARITY_HIGH;
	entry.irr=APIC_IOAPIC_IRR_RESET;
	entry.trigger=APIC_ICR_IOAPIC_Edge;
	entry.mask=APIC_ICR_IOAPIC_Masked;
	entry.reserved=0;
	entry.destination.physical.reserved_1=0;
	entry.destination.physical.phy_dest=0;
	entry.destination.physical.reserved_2=0;
	register_irq(HPET_IRQ,&entry,&hpet_handler,0,&hpet_int_controller,"HPET timer0");
	/* start the counter and enable legacy routing: timer0 -> irq0 -> gsi2 */
	hpet_write(HPET_GCONF,hpet_read(HPET_GCONF)|HPET_CFG_ENABLE|HPET_CFG_LEGACY);
}
void HPET_dump(){
	uint64_t i;
	if(hpet==0){
		color_printk(RED,BLACK,"HPET: not initialised\n");
		return;
	}
	uint64_t c1=hpet_read(HPET_MCV);
	for(i=0;i<200000;i++)	nop();
	uint64_t c2=hpet_read(HPET_MCV);
	color_printk(CYAN,BLACK,"HPET: gconf:%#lx tcfg:%#lx cmp:%lu isr:%#lx\n",
		hpet_read(HPET_GCONF),hpet_read(HPET_TN_CFG),hpet_read(HPET_TN_CMP),hpet_read(HPET_GISR));
	color_printk(CYAN,BLACK,"HPET: mcv %lu -> %lu,ticks:%lu,rte2:%#018lx\n",
		c1,c2,hpet_ticks,ioapic_rte_read(2));
}
