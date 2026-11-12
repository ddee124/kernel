#include "lib/io.h"
#include "driver/ps2/ps2.h"
#include "driver/apic/apic.h"
#include "interrupt/interrupt.h"
#include "kernel/printk.h"
#include "arch/x86_64/ptrace.h"

static void (*keyboard_handler)(uint8_t byte)=0;
static void (*aux_handler)(uint8_t byte)=0;
static struct hw_int_controller ps2_int_controller={
	.enable=IOAPIC_enable,
	.disable=IOAPIC_disable,
	.install=IOAPIC_install,
	.uninstall=IOAPIC_uninstall,
	.ack=IOAPIC_edge_ack,
};
/* throw away whatever the controller still holds */
static void ps2_flush(){
	uint64_t i=0;
	while((io_in8(PS2_STATUS_PORT)&PS2_ST_OBF)&&i++<100000)	io_in8(PS2_DATA_PORT);
}
static uint8_t ps2_read_cmd_byte(){
	PS2_WAIT_WRITE();
	io_out8(PS2_CMD_PORT,0x20);		/* read command byte */
	PS2_WAIT_READ();
	return io_in8(PS2_DATA_PORT);
}
static void ps2_write_cmd_byte(uint8_t v){
	PS2_WAIT_WRITE();
	io_out8(PS2_CMD_PORT,0x60);		/* write command byte */
	PS2_WAIT_WRITE();
	io_out8(PS2_DATA_PORT,v);
}
void ps2_register_keyboard(void (*handler)(uint8_t byte)){
	keyboard_handler=handler;
}
void ps2_register_aux(void (*handler)(uint8_t byte)){
	aux_handler=handler;
}
/* send a command to the auxiliary device (0xd4 prefix) and return its ack */
int ps2_write_aux(uint8_t data,uint8_t* ack){
	uint64_t spins=1000000;
	PS2_WAIT_WRITE();
	io_out8(PS2_CMD_PORT,0xd4);
	PS2_WAIT_WRITE();
	io_out8(PS2_DATA_PORT,data);
	while(spins--){
		if(io_in8(PS2_STATUS_PORT)&PS2_ST_OBF){
			*ack=io_in8(PS2_DATA_PORT);
			return 1;
		}
	}
	return 0;
}
/* one handler for both irq1 and irq12: the status byte decides the source,
   because the output buffer is shared between the two devices */
static void ps2_irq(uint64_t nr,uint64_t parameter,struct pt_regs* regs){
	uint8_t status,data;
	status=io_in8(PS2_STATUS_PORT);
	if(!(status&PS2_ST_OBF))	return;
	data=io_in8(PS2_DATA_PORT);
	if(status&PS2_ST_AUX){
		if(aux_handler)	aux_handler(data);	/* always drained above */
		return;
	}
	if(keyboard_handler)	keyboard_handler(data);
}
static void ps2_install_irq(uint64_t irq){
	struct IO_APIC_RET_entry entry;
	entry.vector=irq;
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
	register_irq(irq,&entry,&ps2_irq,0,&ps2_int_controller,"ps/2");
}
void ps2_init(){
	uint8_t ctr;
	ps2_flush();
	ctr=ps2_read_cmd_byte();
	ctr|=(1<<0)|(1<<1);		/* irq1 (keyboard) + irq12 (mouse) */
	ctr&=~((1<<4)|(1<<5));		/* both device clocks enabled */
	ctr|=(1<<6);			/* translation on -> set 1 scancodes */
	ps2_write_cmd_byte(ctr);
	PS2_WAIT_WRITE();
	io_out8(PS2_CMD_PORT,0xa8);	/* enable the aux port */
	ps2_install_irq(0x21);
	ps2_install_irq(0x2c);
}
