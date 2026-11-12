#include "driver/acpi/acpi.h"
#include "kernel/printk.h"
#include "lib/multiboot2.h"
#include "mem/pagetable.h"
#include "lib/io.h"
#include "lib/lib.h"
struct acpi_madt* madt_table;
struct acpi_hpet* hpet_table;
struct acpi_rsdp* rsdp_table;
struct acpi_xsdt* xsdt_table;
struct acpi_gas acpi_reset_reg;
uint8_t acpi_reset_value=0;
uint64_t rsdp_broken(struct acpi_rsdp* rsdp){
	if(rsdp->signature[0]!='R')	return 1;
	if(rsdp->signature[1]!='S')	return 1;
	if(rsdp->signature[2]!='D')	return 1;
	if(rsdp->signature[3]!=' ')	return 1;
	if(rsdp->signature[4]!='P')	return 1;
	if(rsdp->signature[5]!='T')	return 1;
	if(rsdp->signature[6]!='R')	return 1;
	uint8_t sum=0;
	for(uint64_t i=0;i<rsdp->length;i++)	sum+=*((uint8_t*)rsdp+i);
	if(sum)	return 1;
	return 0;
}
uint64_t other_broken(struct acpi_table_header* hd){
	uint8_t sum=0;
	for(uint64_t i=0;i<hd->length;i++)	sum+=*((uint8_t*)hd+i);
	if(sum)	return 1;
	return 0;
}
static void acpi_parse_fadt(struct acpi_table_header* hd);
void ACPI_init(struct multiboot2_tag_acpi_new* acpi_tag){
	uint64_t i;
	rsdp_table=(struct acpi_rsdp*)acpi_tag->rsdp;
	if(rsdp_broken(rsdp_table)){
		//color_printk(RED,BLACK,"RSDP Table broken!");
		while(1);
	}
	xsdt_table=(struct acpi_xsdt*)Phy_To_Virt(rsdp_table->xsdt_address);
	//color_printk(CYAN,BLACK,"acpi rsdp address:%#018lx xsdt address:%#018lx\n",(uint64_t)rsdp_table,(uint64_t)xsdt_table);
	for(i=0;i<(xsdt_table->header.length-sizeof(struct acpi_table_header))/8;i++){
		struct acpi_table_header* hd=(struct acpi_table_header*)Phy_To_Virt(xsdt_table->table_ptrs[i]);
		//color_printk(CYAN,BLACK,"%#018lx %c%c%c%c\n",(unsigned long)hd,hd->signature[0],hd->signature[1],hd->signature[2],hd->signature[3]);
		if(hd->signature[0]=='A'&&hd->signature[1]=='P'&&hd->signature[2]=='I'&&hd->signature[3]=='C')	madt_table=(struct acpi_madt*)hd;
		if(hd->signature[0]=='H'&&hd->signature[1]=='P'&&hd->signature[2]=='E'&&hd->signature[3]=='T')	hpet_table=(struct acpi_hpet*)hd;
		if(hd->signature[0]=='F'&&hd->signature[1]=='A'&&hd->signature[2]=='C'&&hd->signature[3]=='P')	acpi_parse_fadt(hd);
	}
}
/* the fadt reset register lives at byte 117 (12 byte gas) and the reset
   value at byte 129,see acpi spec table 5.12 */
static void acpi_parse_fadt(struct acpi_table_header* hd){
	uint8_t* f=(uint8_t*)hd;
	uint32_t flags;
	if(hd->length<130)	return;
	flags=*((uint32_t*)(f+113));
	if(!(flags&(1u<<10)))	return;		/* RESET_REG_SUP */
	acpi_reset_reg.address_space_id=f[117];
	acpi_reset_reg.register_bit_width=f[118];
	acpi_reset_reg.register_bit_offset=f[119];
	acpi_reset_reg.access_size=f[120];
	acpi_reset_reg.address=*((uint64_t*)(f+121));
	acpi_reset_value=f[129];
	color_printk(CYAN,BLACK,"ACPI: reset reg space:%d addr:%#018lx val:%#04x size:%d\n",
		acpi_reset_reg.address_space_id,acpi_reset_reg.address,acpi_reset_value,acpi_reset_reg.access_size);
}
int ACPI_reset_available(){
	return acpi_reset_reg.address!=0;
}
static void acpi_reset_write(){
	uint64_t addr=acpi_reset_reg.address;
	uint64_t size=acpi_reset_reg.access_size;
	if(size==0)	size=acpi_reset_reg.register_bit_width/8;
	if(acpi_reset_reg.address_space_id==1){		/* system i/o */
		if(size<=1)	io_out8((uint16_t)addr,acpi_reset_value);
		else if(size==2)	io_out16((uint16_t)addr,acpi_reset_value);
		else	io_out32((uint16_t)addr,acpi_reset_value);
		return;
	}
	if(acpi_reset_reg.address_space_id==0){		/* system memory */
		map_4K_page(addr&(~0xffful),PAGE_PCD);
		volatile uint8_t* p=(volatile uint8_t*)Phy_To_Virt(addr);
		if(size<=1)	*p=acpi_reset_value;
		else if(size==2)	*(volatile uint16_t*)p=acpi_reset_value;
		else	*(volatile uint32_t*)p=acpi_reset_value;
	}
}
void ACPI_reboot(){
	color_printk(RED,BLACK,"[reboot] via acpi reset register\n");
	if(ACPI_reset_available())	acpi_reset_write();
	/* fallbacks when the firmware lied about the reset register */
	io_out8(0xcf9,0x06);		/* chipset reset */
	io_out8(0x64,0xfe);		/* 8042 pulse reset line */
	io_out8(0xcf9,0x0e);
	while(1)	halt();
}
