#include "memory.h"
#include "ACPI.h"
#include "printk.h"
#include "APIC.h"
struct acpi_madt* madt_addr;
struct acpi_hpet* hpet;
void map_ACPI_2M_page(unsigned long phys_addr){
	unsigned long* pml4=(unsigned long*)(((unsigned long)Global_CR3)&(~0xffful));
	unsigned long vaddr=phys_addr|0xffff800000000000;
	unsigned short pml4_idx=(vaddr>>39)&0x1ff;
	unsigned short pdpt_idx=(vaddr>>30)&0x1ff;
	unsigned short pd_idx=(vaddr>>21)&0x1ff;
	if(!(pml4[pml4_idx]&1)){
		unsigned long* new_pdpt=(unsigned long*)kmalloc(PAGE_4K_SIZE,0);
		memset(new_pdpt,0,PAGE_4K_SIZE);
		pml4[pml4_idx]=Virt_To_Phy(new_pdpt)|0x3;
	}
	unsigned long pdpt_phys=pml4[pml4_idx]&(~0xffful);
	unsigned long *pdpt=(unsigned long*)(pdpt_phys);
	if(!(pdpt[pdpt_idx]&1)){
		unsigned long* new_pd=(unsigned long*)kmalloc(PAGE_4K_SIZE,0);
		memset(new_pd,0,PAGE_4K_SIZE);
		pdpt[pdpt_idx]=Virt_To_Phy(new_pd)|0x3;
	}
	unsigned long pd_phys=pdpt[pdpt_idx]&(~0xffful);
	unsigned long *pd=(unsigned long*)(pd_phys);
	if(!(pd[pd_idx]&PAGE_Present))	pd[pd_idx]=phys_addr|PAGE_KERNEL_Page|PAGE_PCD|PAGE_PWT;
}
void map_range(unsigned long start,unsigned long end){
	start=start&(~0x1ffffful);
	end=(end+0x1ffffful)&(~0x1ffffful);
	for(;start<end;start+=0x200000){
		map_ACPI_2M_page(start);
	}
}
struct acpi_rsdp* rsdp=0;
unsigned int rsdp_broken(struct acpi_rsdp* rsdp){
	if(rsdp->signature[0]!='R')	return 1;
	if(rsdp->signature[1]!='S')	return 1;
	if(rsdp->signature[2]!='D')	return 1;
	if(rsdp->signature[3]!=' ')	return 1;
	if(rsdp->signature[4]!='P')	return 1;
	if(rsdp->signature[5]!='T')	return 1;
	if(rsdp->signature[6]!='R')	return 1;
	unsigned char sum=0;
	for(unsigned int i=0;i<rsdp->length;i++)	sum+=*((unsigned char*)rsdp+i);
	//color_printk(0xff0000,0,"%x\n",(unsigned int)sum);
	if(sum)	return 1;
	return 0;
}
unsigned int other_broken(struct acpi_table_header* hd){
	unsigned char sum=0;
	for(unsigned int i=0;i<hd->length;i++)	sum+=*((unsigned char*)hd+i);
	//color_printk(0xff0000,0,"%x\n",(unsigned int)sum);
	if(sum)	return 1;
	return 0;
}
unsigned int madt_solve(struct acpi_madt* madt){
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
}
void ACPI_init(struct multiboot2_tag_acpi_new* acpi_tag){
	int i;
	for(i=0;i<=memory_management_struct.e820_length;i++){
		unsigned long start,end;
		if(memory_management_struct.e820[i].type==1)	continue;
		start=(memory_management_struct.e820[i].address)&(~0x1ffffful);
		end=(memory_management_struct.e820[i].address+memory_management_struct.e820[i].length+0x1ffffful)&(~0x1ffffful);
		for(;start<end;start+=0x200000){
			map_ACPI_2M_page(start);
			//color_printk(0xffff,0,"%#018lx\t",start);
		}
	}
	flush_tlb();
	rsdp=(struct acpi_rsdp*)((unsigned long)acpi_tag->rsdp|0xffff800000000000);
	if(rsdp_broken(rsdp)){
		color_printk(0xff0000,0,"RSDP Table broken!");
		while(1);
	}
	struct acpi_xsdt* xsdt=(struct acpi_xsdt*)((unsigned long)rsdp->xsdt_address|0xffff800000000000);
	color_printk(0xffff,0,"acpi rsdp address:%#018lx xsdt address:%#018lx\n",(unsigned long)rsdp,(unsigned long)xsdt);
	//color_printk(0xffff,0,"%c%c%c%c\n",xsdt->header.signature[0],xsdt->header.signature[1],xsdt->header.signature[2],xsdt->header.signature[3]);
	for(i=0;i<(xsdt->header.length-36)/8;i++){
		struct acpi_table_header* hd=(struct acpi_table_header*)Phy_To_Virt(xsdt->table_ptrs[i]);
		//color_printk(0xffff,0,"%#018lx\n",(unsigned long)hd);
		color_printk(0xffff,0,"%#018lx %c%c%c%c\n",(unsigned long)hd,hd->signature[0],hd->signature[1],hd->signature[2],hd->signature[3]);
		if(hd->signature[0]=='A'&&hd->signature[1]=='P'&&hd->signature[2]=='I'&&hd->signature[3]=='C'){
			madt_addr=(struct acpi_madt*)hd;
			if(madt_solve((struct acpi_madt*)hd))	while(1);
			//break;
		}
		if(hd->signature[0]=='H'&&hd->signature[1]=='P'&&hd->signature[2]=='E'&&hd->signature[3]=='T')	hpet=(struct acpi_hpet*)hd;
	}
}
