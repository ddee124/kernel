#include "mem/pagetable.h"
#include "mem/memory.h"
void map_4K_page(uint64_t phy_addr,uint64_t attr){
	uintptr_t virt_addr=Phy_To_Virt(phy_addr);
	uint64_t pml4_idx,pdpt_idx,pd_idx,pt_idx;
	pml4_idx=(virt_addr>>PAGE_GDT_SHIFT)&(0x1ff);
	pdpt_idx=(virt_addr>>PAGE_1G_SHIFT)&(0x1ff);
	pd_idx=(virt_addr>>PAGE_2M_SHIFT)&(0x1ff);
	pt_idx=(virt_addr>>PAGE_4K_SHIFT)&(0x1ff);
	uint64_t* pml4=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	if(!(pml4[pml4_idx]&PAGE_Present)){
		struct Page* page=alloc_pages(1,0,0);
		memset((uint64_t*)Phy_To_Virt(page->PHY_address),0,4096);
		pml4[pml4_idx]=page->PHY_address|PAGE_KERNEL_Dir;
	}
	if(pml4[pml4_idx]&PAGE_PS)	return;
	uint64_t* pdpt=(uint64_t*)Phy_To_Virt(pml4[pml4_idx]&(~0x8000000000000ffful));
	if(!(pdpt[pdpt_idx]&PAGE_Present)){
		struct Page* page=alloc_pages(1,0,0);
		memset((uint64_t*)Phy_To_Virt(page->PHY_address),0,4096);
		pdpt[pdpt_idx]=page->PHY_address|PAGE_KERNEL_Dir;
	}
	if(pdpt[pdpt_idx]&PAGE_PS)	return;
	uint64_t* pd=(uint64_t*)Phy_To_Virt(pdpt[pdpt_idx]&(~0x8000000000000ffful));
	if(!(pd[pd_idx]&PAGE_Present)){
		struct Page* page=alloc_pages(1,0,0);
		memset((uint64_t*)Phy_To_Virt(page->PHY_address),0,4096);
		pd[pd_idx]=page->PHY_address|PAGE_KERNEL_Dir;
	}
	if(pd[pd_idx]&PAGE_PS)	return;
	uint64_t* pt=(uint64_t*)Phy_To_Virt(pd[pd_idx]&(~0x8000000000000ffful));
	pt[pt_idx]=phy_addr|PAGE_R_W|PAGE_Present|attr;
}
void map_2M_page(uint64_t phy_addr,uint64_t attr){
	uintptr_t virt_addr=Phy_To_Virt(phy_addr);
	uint64_t pml4_idx,pdpt_idx,pd_idx;
	pml4_idx=(virt_addr>>PAGE_GDT_SHIFT)&(0x1ff);
	pdpt_idx=(virt_addr>>PAGE_1G_SHIFT)&(0x1ff);
	pd_idx=(virt_addr>>PAGE_2M_SHIFT)&(0x1ff);
	uint64_t* pml4=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	if(!(pml4[pml4_idx]&PAGE_Present)){
		struct Page* page=alloc_pages(1,0,0);
		memset((uint64_t*)Phy_To_Virt(page->PHY_address),0,4096);
		pml4[pml4_idx]=page->PHY_address|PAGE_KERNEL_Dir;
	}
	if(pml4[pml4_idx]&PAGE_PS)	return;
	uint64_t* pdpt=(uint64_t*)Phy_To_Virt(pml4[pml4_idx]&(~0x8000000000000ffful));
	if(!(pdpt[pdpt_idx]&PAGE_Present)){
		struct Page* page=alloc_pages(1,0,0);
		memset((uint64_t*)Phy_To_Virt(page->PHY_address),0,4096);
		pdpt[pdpt_idx]=page->PHY_address|PAGE_KERNEL_Dir;
	}
	if(pdpt[pdpt_idx]&PAGE_PS)	return;
	uint64_t* pd=(uint64_t*)Phy_To_Virt(pdpt[pdpt_idx]&(~0x8000000000000ffful));
	pd[pd_idx]=phy_addr|PAGE_KERNEL_Page|attr;
}
void map_1G_page(uint64_t phy_addr,uint64_t attr){
	uintptr_t virt_addr=Phy_To_Virt(phy_addr);
	uint64_t pml4_idx,pdpt_idx;
	pml4_idx=(virt_addr>>PAGE_GDT_SHIFT)&(0x1ff);
	pdpt_idx=(virt_addr>>PAGE_1G_SHIFT)&(0x1ff);
	uint64_t* pml4=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	if(!(pml4[pml4_idx]&PAGE_Present)){
		struct Page* page=alloc_pages(1,0,0);
		memset((uint64_t*)Phy_To_Virt(page->PHY_address),0,4096);
		pml4[pml4_idx]=page->PHY_address|PAGE_KERNEL_Dir;
	}
	if(pml4[pml4_idx]&PAGE_PS)	return;
	uint64_t* pdpt=(uint64_t*)Phy_To_Virt(pml4[pml4_idx]&(~0x8000000000000ffful));
	pdpt[pdpt_idx]=phy_addr|PAGE_KERNEL_Page|attr;
}
void map_range_4k(uint64_t start,uint64_t end,uint64_t attr){
	if(start>=end)	return;
	uint64_t start_4k=(start+PAGE_4K_SIZE-1)&PAGE_4K_MASK;
	uint64_t end_4k=end&PAGE_4K_MASK;
	while(start_4k<end_4k){
		map_4K_page(start_4k,attr);
		start_4k+=PAGE_4K_SIZE;
	}
}
void map_range_2m(uint64_t start,uint64_t end,uint64_t attr){
	if(start>=end)	return;
	uint64_t start_2m=(start+PAGE_2M_SIZE-1)&PAGE_2M_MASK;
	uint64_t end_2m=end&PAGE_2M_MASK;
	uint64_t tmp=start_2m;
	while(tmp<end_2m){
		map_2M_page(tmp,attr);
		tmp+=PAGE_2M_SIZE;
	}
	if(attr&PAGE_PAT){
		attr^=PAGE_PAT;
		attr|=PAGE_PS;
	}
	if(start_2m==end_2m)	map_range_4k(start,end,attr);
	else{
		if(start<start_2m)	map_range_4k(start,start_2m,attr);
		if(end_2m<end)	map_range_4k(end_2m,end,attr);
	}
}
void map_range(uint64_t start,uint64_t end,uint64_t attr){
	if(start>=end)	return;
	cli();
	uint64_t start_1g=(start+PAGE_1G_SIZE-1)&PAGE_1G_MASK;
	uint64_t end_1g=end&PAGE_1G_MASK;
	uint64_t tmp=start_1g;
	while(tmp<end_1g){
		map_1G_page(tmp,attr);
		tmp+=PAGE_1G_SIZE;
	}
	if(start_1g==end_1g)	map_range_2m(start,end,attr);
	else{
		if(start<start_1g)	map_range_2m(start,start_1g,attr);
		if(end_1g<end)	map_range_2m(end_1g,end,attr);
	}
	sti();
}
