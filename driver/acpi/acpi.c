#include "driver/acpi/acpi.h"
#include "kernel/printk.h"
#include "lib/multiboot2.h"
#include "mem/pagetable.h"
struct acpi_madt* madt_table;
struct acpi_hpet* hpet_table;
struct acpi_rsdp* rsdp_table;
struct acpi_xsdt* xsdt_table;
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
/*unsigned int madt_solve(struct acpi_madt* madt){
	if(other_broken(&madt->header)){
		color_printk(0xff0000,0,"MADT Table broken!");
		return 1;
	}
	//ioapic_map.phy_addr=madt->local_apic_address;
	color_printk(0xffff,0,"madt address:%#018lx\n",(unsigned long)madt);
	unsigned long start=(unsigned long)madt->entries;
	unsigned long end=(unsigned long)madt+madt->header.length;
	while(start<end){
		if(*((unsigned char*)(start))==1){
			//color_printk(0xffff,0,"I/O APIC address:%#018lx\n",*((unsigned int*)(start+4)));
			ioapic_map.phy_addr=*((unsigned int*)(start+4));
		}
		start=start+*((unsigned char*)(start+1));
	}
	return 0;
}*/
void ACPI_init(struct multiboot2_tag_acpi_new* acpi_tag){
	uint64_t i;
	rsdp_table=(struct acpi_rsdp*)acpi_tag->rsdp;
	if(rsdp_broken(rsdp_table)){
		color_printk(RED,BLACK,"RSDP Table broken!");
		while(1);
	}
	xsdt_table=(struct acpi_xsdt*)Phy_To_Virt(rsdp_table->xsdt_address);
	color_printk(CYAN,BLACK,"acpi rsdp address:%#018lx xsdt address:%#018lx\n",(uint64_t)rsdp_table,(uint64_t)xsdt_table);
	for(i=0;i<(xsdt_table->header.length-sizeof(struct acpi_table_header))/8;i++){
		struct acpi_table_header* hd=(struct acpi_table_header*)Phy_To_Virt(xsdt_table->table_ptrs[i]);
		color_printk(CYAN,BLACK,"%#018lx %c%c%c%c\n",(unsigned long)hd,hd->signature[0],hd->signature[1],hd->signature[2],hd->signature[3]);
		/*if(hd->signature[0]=='A'&&hd->signature[1]=='P'&&hd->signature[2]=='I'&&hd->signature[3]=='C'){
			madt_addr=(struct acpi_madt*)hd;
			if(madt_solve((struct acpi_madt*)hd))	while(1);
			//break;
		}
		if(hd->signature[0]=='H'&&hd->signature[1]=='P'&&hd->signature[2]=='E'&&hd->signature[3]=='T')	hpet=(struct acpi_hpet*)hd;*/
	}
}
