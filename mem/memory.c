#include "mem/memory.h"
#include "lib/multiboot2.h"
#include "lib/lib.h"
#include "kernel/printk.h"
#include "mem/pagetable.h"
__attribute__((section(".bss"))) struct E820 e820[64];
__attribute__((section(".bss"))) struct Zone memory_zone[16];
uint64_t e820_entries;
uint64_t memory_entries;
uint64_t zone_high_idx;
uintptr_t end_ptr;
uint64_t page_init(struct Page* page,uint64_t flags){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	page->attribute|=flags;
	if(!page->ref_count){
		page->ref_count++;
	}
	
	local_irq_restore(rflags_save);
	return 1;
}
uint64_t page_clean(struct Page* page){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	page->ref_count--;
	if(!page->ref_count){
		page->attribute=0;
	}
	
	local_irq_restore(rflags_save);
	return 1;
}
uint64_t get_page_attribute(struct Page* page){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	if(page==0){
		color_printk(0xff0000,0,"get_page_attribute() ERROR: page==NULL\n");
		local_irq_restore(rflags_save);
		return 0;
	}
	else{
		local_irq_restore(rflags_save);
		return page->attribute;
	}
	local_irq_restore(rflags_save);
}
uint64_t set_page_attribute(struct Page* page,uint64_t flags){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	if(page==0){
		color_printk(0xff0000,0,"set_page_attribute() ERROR: page==NULL\n");
		local_irq_restore(rflags_save);
		return 0;
	}
	else{
		page->attribute=flags;
		local_irq_restore(rflags_save);
		return 1;
	}
	
	local_irq_restore(rflags_save);
}
struct Page* alloc_pages(uint64_t number,uint64_t page_flags,uint64_t flag){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	uint32_t i;
	uint64_t page=0;
	uint64_t attribute;
	if(number<=0){
		color_printk(0xff0000,0,"alloc_pages() ERROR:number is invalid\n");
		local_irq_restore(rflags_save);
		return 0;
	}
	if(number>64){
		if(!(flag&PAGE_LOW)){
			for(i=zone_high_idx;i<memory_entries;i++){
				if(memory_zone[i].page_free_count<number)	continue;
				uint64_t j;
				for(j=0;j+number<memory_zone[i].pages_count;j++){
					uint64_t k;
					for(k=0;k<number;k++){
						uint64_t *p=memory_zone[i].bits_map+((j+k)>>6);
						if((*p)&(1ul<<((j+k)%64))){
							j=j+k;
							break;
						}
					}
					if(k==number){
						uint64_t l;
						for(k=0;k<number;k++){
							struct Page* X=memory_zone[i].pages_group+j+k;
							*(memory_zone[i].bits_map+((j+k)>>6))|=1ul<<((j+k)%64);
							memory_zone[i].page_using_count++;
							memory_zone[i].page_free_count--;
							X->attribute=attribute;
						}
						local_irq_restore(rflags_save);
						return (struct Page*)(memory_zone[i].pages_group+j);
					}
				}
			}
		}
		for(i=0;i<zone_high_idx;i++){
			if(memory_zone[i].page_free_count<number)	continue;
			uint64_t j;
			for(j=0;j+number<memory_zone[i].pages_count;j++){
				uint64_t k;
				for(k=0;k<number;k++){
					uint64_t *p=memory_zone[i].bits_map+((j+k)>>6);
					if((*p)&(1ul<<((j+k)%64))){
						j=j+k;
						break;
					}
				}
				if(k==number){
					uint64_t l;
					for(k=0;k<number;k++){
						struct Page* X=memory_zone[i].pages_group+j+k;
						*(memory_zone[i].bits_map+((j+k)>>6))|=1ul<<((j+k)%64);
						memory_zone[i].page_using_count++;
						memory_zone[i].page_free_count--;
						X->attribute=attribute;
					}
					local_irq_restore(rflags_save);
					return (struct Page*)(memory_zone[i].pages_group+j);
				}
			}
		}
	}
	else{
		if(!(flag&PAGE_LOW)){
			for(i=zone_high_idx;i<memory_entries;i++){
				uint64_t j;
				if(memory_zone[i].page_free_count<number)	continue;
				for(j=0;j<memory_zone[i].pages_count;j+=64){
					uint64_t *p=memory_zone[i].bits_map+(j>>6);
					if(*p==-1ul)	continue;
					uint64_t k=0;
					uint64_t num=(number==64?-1ul:((1ul<<number)-1));
					for(k=0;k<64;k++){
						if(j+k+number>memory_zone[i].pages_count)	break;
						if(!((k?((*p>>k)|(*(p+1)<<(64-k))):*p)&(num))){
							uint64_t l;
							page=j+k;
							for(l=0;l<number;l++){
								struct Page* X=memory_zone[i].pages_group+page+l;
								*(memory_zone[i].bits_map+((page+l)>>6))|=1ul<<((page+l)%64);
								memory_zone[i].page_using_count++;
								memory_zone[i].page_free_count--;
								X->attribute=attribute;
							}
							local_irq_restore(rflags_save);
							return (struct Page*)(memory_zone[i].pages_group+page);
						}
					}
				}
			}
		}
		for(i=0;i<zone_high_idx;i++){
			uint64_t j;
			if(memory_zone[i].page_free_count<number)	continue;
			for(j=0;j<memory_zone[i].pages_count;j+=64){
				uint64_t *p=memory_zone[i].bits_map+(j>>6);
				if(*p==-1ul)	continue;
				uint64_t k=0;
				uint64_t num=(number==64?-1ul:((1ul<<number)-1));
				for(k=0;k<64;k++){
					if(j+k+number>memory_zone[i].pages_count)	break;
					if(!((k?((*p>>k)|(*(p+1)<<(64-k))):*p)&(num))){
						uint64_t l;
						page=j+k;
						for(l=0;l<number;l++){
							struct Page* X=memory_zone[i].pages_group+page+l;
							*(memory_zone[i].bits_map+((page+l)>>6))|=1ul<<((page+l)%64);
							memory_zone[i].page_using_count++;
							memory_zone[i].page_free_count--;
							X->attribute=attribute;
						}
						local_irq_restore(rflags_save);
						return (struct Page*)(memory_zone[i].pages_group+page);
					}
				}
			}
		}
	}
	color_printk(0xff0000,0,"alloc_pages() ERROR:unable to alloc!\n");
	local_irq_restore(rflags_save);
	return 0;
}
void free_pages(struct Page* page,int number){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	int i=0;
	if(page==0){
		color_printk(0xff0000,0,"free_pages() ERROR:page is invalid\n");
		return;
	}
	if(number<=0){
		color_printk(0xff0000,0,"free_pages() ERROR:number is invalid\n");
		return;
	}
	uint64_t index=((uintptr_t)page-(uintptr_t)(page->zone_struct->pages_group))/sizeof(struct Page);
	for(int i=0;i<number;i++,page++,index++){
		*(page->zone_struct->bits_map+(index>>6))&=~(1ul<<(index%64));
		page->zone_struct->page_using_count--;
		page->zone_struct->page_free_count++;
		page->attribute=0;
	}
	
	local_irq_restore(rflags_save);
}
void tmp_map_4K_page(uint64_t phy_addr,uint64_t attr){
	uintptr_t virt_addr=Phy_To_Virt(phy_addr);
	uint64_t pml4_idx,pdpt_idx,pd_idx,pt_idx;
	pml4_idx=(virt_addr>>PAGE_GDT_SHIFT)&(0x1ff);
	pdpt_idx=(virt_addr>>PAGE_1G_SHIFT)&(0x1ff);
	pd_idx=(virt_addr>>PAGE_2M_SHIFT)&(0x1ff);
	pt_idx=(virt_addr>>PAGE_4K_SHIFT)&(0x1ff);
	uint64_t* pml4=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	if(!(pml4[pml4_idx]&PAGE_Present)){
		memset((void*)end_ptr,0,4096);
		pml4[pml4_idx]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
		end_ptr+=4096;
	}
	if(pml4[pml4_idx]&PAGE_PS)	return;
	uint64_t* pdpt=(uint64_t*)Phy_To_Virt(pml4[pml4_idx]&(~0x8000000000000ffful));
	if(!(pdpt[pdpt_idx]&PAGE_Present)){
		memset((void*)end_ptr,0,4096);
		pdpt[pdpt_idx]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
		end_ptr+=4096;
	}
	if(pdpt[pdpt_idx]&PAGE_PS)	return;
	uint64_t* pd=(uint64_t*)Phy_To_Virt(pdpt[pdpt_idx]&(~0x8000000000000ffful));
	if(!(pd[pd_idx]&PAGE_Present)){
		memset((void*)end_ptr,0,4096);
		pd[pd_idx]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
		end_ptr+=4096;
	}
	if(pd[pd_idx]&PAGE_PS)	return;
	uint64_t* pt=(uint64_t*)Phy_To_Virt(pd[pd_idx]&(~0x8000000000000ffful));
	pt[pt_idx]=phy_addr|PAGE_R_W|PAGE_Present|attr;
}
void tmp_map_2M_page(uint64_t phy_addr,uint64_t attr){
	uintptr_t virt_addr=Phy_To_Virt(phy_addr);
	uint64_t pml4_idx,pdpt_idx,pd_idx;
	pml4_idx=(virt_addr>>PAGE_GDT_SHIFT)&(0x1ff);
	pdpt_idx=(virt_addr>>PAGE_1G_SHIFT)&(0x1ff);
	pd_idx=(virt_addr>>PAGE_2M_SHIFT)&(0x1ff);
	uint64_t* pml4=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	if(!(pml4[pml4_idx]&PAGE_Present)){
		memset((void*)end_ptr,0,4096);
		pml4[pml4_idx]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
		end_ptr+=4096;
	}
	if(pml4[pml4_idx]&PAGE_PS)	return;
	uint64_t* pdpt=(uint64_t*)Phy_To_Virt(pml4[pml4_idx]&(~0x8000000000000ffful));
	if(!(pdpt[pdpt_idx]&PAGE_Present)){
		memset((void*)end_ptr,0,4096);
		pdpt[pdpt_idx]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
		end_ptr+=4096;
	}
	if(pdpt[pdpt_idx]&PAGE_PS)	return;
	uint64_t* pd=(uint64_t*)Phy_To_Virt(pdpt[pdpt_idx]&(~0x8000000000000ffful));
	pd[pd_idx]=phy_addr|PAGE_KERNEL_Page|attr;
}
void tmp_map_1G_page(uint64_t phy_addr,uint64_t attr){
	uintptr_t virt_addr=Phy_To_Virt(phy_addr);
	uint64_t pml4_idx,pdpt_idx;
	pml4_idx=(virt_addr>>PAGE_GDT_SHIFT)&(0x1ff);
	pdpt_idx=(virt_addr>>PAGE_1G_SHIFT)&(0x1ff);
	uint64_t* pml4=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	if(!(pml4[pml4_idx]&PAGE_Present)){
		memset((void*)end_ptr,0,4096);
		pml4[pml4_idx]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
		end_ptr+=4096;
	}
	if(pml4[pml4_idx]&PAGE_PS)	return;
	uint64_t* pdpt=(uint64_t*)Phy_To_Virt(pml4[pml4_idx]&(~0x8000000000000ffful));
	pdpt[pdpt_idx]=phy_addr|PAGE_KERNEL_Page|attr;
}
void tmp_map_range_4k(uint64_t start,uint64_t end,uint64_t attr){
	uint64_t start_4k=(start+PAGE_4K_SIZE-1)&PAGE_4K_MASK;
	uint64_t end_4k=end&PAGE_4K_MASK;
	while(start_4k<end_4k){
		tmp_map_4K_page(start_4k,attr);
		start_4k+=PAGE_4K_SIZE;
	}
}
void tmp_map_range_2m(uint64_t start,uint64_t end,uint64_t attr){
	uint64_t start_2m=(start+PAGE_2M_SIZE-1)&PAGE_2M_MASK;
	uint64_t end_2m=end&PAGE_2M_MASK;
	uint64_t tmp=start_2m;
	while(tmp<end_2m){
		tmp_map_2M_page(tmp,attr);
		tmp+=PAGE_2M_SIZE;
	}
	if(attr&PAGE_PAT){
		attr^=PAGE_PAT;
		attr|=PAGE_PS;
	}
	if(start_2m>=end_2m)	tmp_map_range_4k(start,end,attr);
	else{
		if(start<start_2m)	tmp_map_range_4k(start,start_2m,attr);
		if(end_2m<end)	tmp_map_range_4k(end_2m,end,attr);
	}
}
void tmp_map_range(uint64_t start,uint64_t end,uint64_t attr){
	uint64_t start_1g=(start+PAGE_1G_SIZE-1)&PAGE_1G_MASK;
	uint64_t end_1g=end&PAGE_1G_MASK;
	uint64_t tmp=start_1g;
	while(tmp<end_1g){
		tmp_map_1G_page(tmp,attr);
		tmp+=PAGE_1G_SIZE;
	}
	if(start_1g>=end_1g)	tmp_map_range_2m(start,end,attr);
	else{
		if(start<start_1g)	tmp_map_range_2m(start,start_1g,attr);
		if(end_1g<end)	tmp_map_range_2m(end_1g,end,attr);
	}
}

void init_memory(struct multiboot2_tag_mmap* mmap_tag){
	wrmsr(0x277,0x0007010600070106);//init PAT
	unsigned long totalmem=0;
	uint64_t i,j;
	uintptr_t ptr=(uintptr_t)mmap_tag+sizeof(struct multiboot2_tag_mmap);
	e820_entries=((uintptr_t)(mmap_tag->size)-sizeof(struct multiboot2_tag_mmap))/mmap_tag->entry_size;
	for(i=0;i<e820_entries;i++){
		struct multiboot2_mmap_entry *p=(struct multiboot2_mmap_entry*)ptr;
		e820[i].address=p->addr;
		e820[i].length=p->length;
		e820[i].type=p->type;
		ptr+=mmap_tag->entry_size;
	}
	//page table init 
	end_ptr=((uintptr_t)&_end+0xfff)&(~0xffful);
	for(i=0;i<e820_entries;i++){
		if(e820[i].type==1)	tmp_map_range(e820[i].address,e820[i].address+e820[i].length,0);
		else    tmp_map_range(e820[i].address,e820[i].address+e820[i].length,PAGE_PCD);
	}
	//remap the low 2M memory
	uint64_t* tmp=(uint64_t*)Phy_To_Virt(get_cr3()&(~0xffful));
	tmp=(uint64_t*)Phy_To_Virt(tmp[0x100]&(~0x8000000000000ffful));
	tmp=(uint64_t*)Phy_To_Virt(tmp[0]&(~0x8000000000000ffful));
	uint64_t* tmp1=(uint64_t*)end_ptr;
	memset(tmp1,0,4096);
	for(i=256;i<512;i++)	tmp1[i]=(i<<PAGE_4K_SHIFT)|PAGE_Present|PAGE_R_W;
	tmp[0]=Virt_To_Phy(end_ptr)|PAGE_KERNEL_Dir;
	end_ptr+=4096;
	flush_tlb();
	for(i=0;i<e820_entries;i++){
		if(e820[i].address+e820[i].length>0x100000)	break;
		if(e820[i].type==1)	tmp_map_range(e820[i].address,e820[i].address+e820[i].length,0);
		else    tmp_map_range(e820[i].address,e820[i].address+e820[i].length,PAGE_PCD);
	}
	flush_tlb();
	//page struct init
	j=0;
	for(i=0;i<e820_entries;i++){
		if(e820[i].type!=1)	continue;
		memory_zone[j].zone_start_address=e820[i].address;
		memory_zone[j].zone_length=e820[i].length;
		memory_zone[j].zone_end_address=e820[i].address+e820[i].length;
		memory_zone[j].end_ptr=Phy_To_Virt(e820[i].address);
		if(e820[i].address<=0x100000&&e820[i].address+e820[i].length>0x100000)	memory_zone[j].end_ptr=end_ptr;
		memory_zone[j].pages_count=memory_zone[j].zone_length>>PAGE_4K_SHIFT;
		memory_zone[j].bits_map=(uint64_t*)((memory_zone[j].end_ptr+7)&(~7ul));
		memory_zone[j].bits_length=((memory_zone[j].pages_count+63)&(~63ul))/64*8;
		memset(memory_zone[j].bits_map,0xff,memory_zone[j].bits_length);
		memory_zone[j].end_ptr=((uintptr_t)memory_zone[j].bits_map+memory_zone[j].bits_length);
		memory_zone[j].end_ptr=(memory_zone[j].end_ptr+7)&(~7ul);
		memory_zone[j].pages_group=(struct Page*)memory_zone[j].end_ptr;
		memory_zone[j].pages_length=(memory_zone[j].pages_count*sizeof(struct Page)+7)&(~7ul);
		memset(memory_zone[j].pages_group,0x00,memory_zone[j].pages_length);
		memory_zone[j].end_ptr=memory_zone[j].end_ptr+memory_zone[j].pages_length;
		j++;
	}
	memory_entries=j;
	zone_high_idx=memory_entries;
	for(i=0;i<memory_entries;i++){
		if(memory_zone[i].zone_start_address>=0x100000000ul&&zone_high_idx==memory_entries)	zone_high_idx=i;
		uint64_t start,end;
		struct Page* P;
		uint64_t *b;
		start=memory_zone[i].zone_start_address;
		end=memory_zone[i].zone_end_address;
		memory_zone[i].page_free_count=memory_zone[i].pages_count;
		memory_zone[i].page_using_count=0;
		P=memory_zone[i].pages_group;
		for(j=0;j<memory_zone[i].pages_count;j++,P++){
			P->zone_struct=&memory_zone[i];
			P->PHY_address=start+PAGE_4K_SIZE*j;
			P->attribute=0;
			P->ref_count=0;
			P->map_count=0;
			P->private=0;
			P->index=0;
			*(memory_zone[i].bits_map+(j>>6))^=1ul<<(j%64);
		}
		uint64_t reserved_num=(Virt_To_Phy(memory_zone[i].end_ptr)-memory_zone[i].zone_start_address+PAGE_4K_SIZE-1)>>PAGE_4K_SHIFT;
		P=memory_zone[i].pages_group;
		for(j=0;j<reserved_num;j++,P++){
			P->attribute=PG_reserved;
			P->ref_count=1;
			P->map_count=1;
			P->zone_struct->page_free_count--;
			P->zone_struct->page_using_count++;
			*(memory_zone[i].bits_map+(j>>6))^=1ul<<(j%64);
		}
	}
	//map fb
	map_range(Virt_To_Phy(Pos.FB_addr)&(~0xffful),(Virt_To_Phy(Pos.FB_addr+Pos.FB_length)+0xfff)&(~0xffful),PAGE_PWT);
	flush_tlb();
}
void display_memory_info(){
	uint64_t i;
	color_printk(0xff,0,"Display Memory Structure,%d entries\n",e820_entries);
	for(i=0;i<e820_entries;i++){
		if(e820[i].type==1)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:RAM\n",e820[i].address,e820[i].length);
		if(e820[i].type==2)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ROM or Reserved\n",e820[i].address,e820[i].length);
		if(e820[i].type==3)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ACPI Reclaim Memory\n",e820[i].address,e820[i].length);
		if(e820[i].type==4)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ACPI NVS Memory\n",e820[i].address,e820[i].length);
		if(e820[i].type>4)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:Unknown:%#010lx\n",e820[i].address,e820[i].length,e820[i].type);
	}
	for(int i=0;i<memory_entries;i++){
		color_printk(0xffff00,0,"zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,total_pages:%#018lx\n",memory_zone[i].zone_start_address,memory_zone[i].zone_end_address,memory_zone[i].zone_length,memory_zone[i].pages_count);
	}
	uint64_t total_mem,used_mem,free_mem;
	total_mem=0;used_mem=0;free_mem=0;
	for(int i=0;i<memory_entries;i++){
		total_mem+=memory_zone[i].pages_count<<PAGE_4K_SHIFT;
		used_mem+=memory_zone[i].page_using_count<<PAGE_4K_SHIFT;
		free_mem+=memory_zone[i].page_free_count<<PAGE_4K_SHIFT;
	}
	color_printk(0xff00,0,"total memory:%#018lx\tused memory:%#018lx\tfree memory:%#018lx\n",total_mem,used_mem,free_mem);
	color_printk(0xff00,0,"total memory:%luKiB\tused memory:%luKiB\tfree memory:%luKiB\n",total_mem>>10,used_mem>>10,free_mem>>10);
	color_printk(0xff00,0,"total memory:%luMiB\tused memory:%luMiB\tfree memory:%luMiB\n",total_mem>>20,used_mem>>20,free_mem>>20);
}
