#include "memory.h"
#include "multiboot2.h"
#include "lib.h"
#include "printk.h"
__attribute__((section(".bss"),aligned(4096))) static unsigned char page_reserved[1024*1024]={0};
static unsigned int page_reserved_used=0;
struct Global_Memory_Descriptor memory_management_struct;
unsigned long* Global_CR3;
unsigned long page_init(struct Page* page,unsigned long flags){
	if(!page->attribute){
		*(memory_management_struct.bits_map+((page->PHY_address>>PAGE_2M_SHIFT)>>6))|=1ull<<(page->PHY_address>>PAGE_2M_SHIFT)%64;
		page->attribute=flags;
		page->reference_count++;
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		page->zone_struct->total_pages_link++;
	}
	else{
		if((page->attribute&PG_Referenced)||(page->attribute&PG_K_Share_To_U)||(flags&PG_Referenced)||(flags&PG_K_Share_To_U)){
			page->attribute|=flags;
			page->reference_count++;
			page->zone_struct->total_pages_link++;
		}
		else{
			*(memory_management_struct.bits_map+((page->PHY_address>>PAGE_2M_SHIFT)>>6))|=1ull<<(page->PHY_address>>PAGE_2M_SHIFT)%64;
			page->attribute|=flags;
		}
	}
	return 0;
}
struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags){
	int i;
	unsigned long page=0;
	int zone_start=0;
	int zone_end=0;
	switch(zone_select){
		case ZONE_DMA:
		    zone_start=0;
		    zone_end=ZONE_DMA_INDEX;
		    break;
		case ZONE_NORMAL:
		    zone_start=ZONE_DMA_INDEX;
		    zone_end=ZONE_NORMAL_INDEX;
		    break;
		case ZONE_UNMAPED:
		    zone_start=ZONE_UNMAPED_INDEX;
		    zone_end=memory_management_struct.zones_size-1;
		    break;
		default:
		    color_printk(0xff0000,0,"alloc_pages error zone_select index\n");
		    return 0;
		    break;
	}
	for(i=zone_start;i<=zone_end;i++){
		struct Zone *Z;
		unsigned long j,start,end,length;
		unsigned long tmp;
		if((memory_management_struct.zones_struct+i)->page_free_count<number)	continue;
		Z=memory_management_struct.zones_struct+i;
		start=(Z->zone_start_address)>>PAGE_2M_SHIFT;
		end=(Z->zone_end_address)>>PAGE_2M_SHIFT;
		length=(Z->zone_length)>>PAGE_2M_SHIFT;
		tmp=(64-start)%64;
		for(j=start;j<=end;j+=j%64?tmp:64){
			unsigned long *p=memory_management_struct.bits_map+(j>>6);
			unsigned long shift=j%64;
			unsigned long k;
			for(k=shift;k<64-shift;k++){
				if(!(((*p>>k)|(*(p+1)<<(64-k)))&(number==64?0xfffffffffffffffful:((1ul<<number)-1)))){
					unsigned long l;
					page=j+k-1;
					for(l=0;l<number;l++){
						struct Page* X=memory_management_struct.pages_struct+page+l;
						page_init(X,page_flags);
					}
					goto find_free_pages;
				}
			}
		}
	}
	return 0;
	find_free_pages:
	return (struct Page*)(memory_management_struct.pages_struct+page);
}
unsigned long alloc_reserved_page(void){
	unsigned long phys=(unsigned long)&page_reserved[page_reserved_used];
	page_reserved_used+=4096;
	if(page_reserved_used>sizeof(page_reserved)){
		return 0;
	}
	return phys-0xffff800000000000;
}
void map_2M_page(unsigned long phys_addr){
	if(phys_addr>=0xffff800000000000)	phys_addr-=0xffff800000000000;
	unsigned long* pml4=(unsigned long*)(((unsigned long)Global_CR3)&(~0xffful));
	unsigned long vaddr=phys_addr;
	unsigned short pml4_idx=(vaddr>>39)&0x1ff;
	unsigned short pdpt_idx=(vaddr>>30)&0x1ff;
	unsigned short pd_idx=(vaddr>>21)&0x1ff;
	if(!(pml4[pml4_idx]&1)){
		unsigned long new_pdpt=alloc_reserved_page();
		pml4[pml4_idx]=new_pdpt|0x7;
		pml4[pml4_idx|0x100]=new_pdpt|0x7;
	}
	unsigned long pdpt_phys=pml4[pml4_idx]&(~0xffful);
	unsigned long *pdpt=(unsigned long*)(pdpt_phys);
	if(!(pdpt[pdpt_idx]&1)){
		unsigned long new_pd=alloc_reserved_page();
		pdpt[pdpt_idx]=new_pd|0x7;
	}
	unsigned long pd_phys=pdpt[pdpt_idx]&(~0xffful);
	unsigned long *pd=(unsigned long*)(pd_phys);
	pd[pd_idx]=phys_addr|0x87;
}
void init_memory(struct multiboot2_tag_mmap *mmap_tag){
	Global_CR3=Get_gdt();
	unsigned long totalmem=0;
	int i,j;
	//struct E820 *p=0;
	//p=(struct E820*)0xffff800000007e00;
	unsigned char* ptr=(unsigned char*)mmap_tag;
	ptr+=sizeof(struct multiboot2_tag_mmap);
	unsigned int entry_size=mmap_tag->entry_size;
	unsigned int total_entries=(mmap_tag->size-sizeof(struct multiboot2_tag_mmap))/entry_size;
	for(i=0;i<total_entries;i++){
		struct multiboot2_mmap_entry *p=(struct multiboot2_mmap_entry*)ptr;
		if(p->type==1)	totalmem+=p->length;
		memory_management_struct.e820[i].address=p->addr;
		memory_management_struct.e820[i].length=p->length;
		memory_management_struct.e820[i].type=p->type;
		memory_management_struct.e820_length=i;
		ptr+=entry_size;
		if(p->type==1){
			unsigned long start=(p->addr)&(~0x1fffff);
			unsigned long end=(p->addr+p->length+0x1fffff)&(~0x1fffff);
			for(;start<end;start+=0x200000){
				map_2M_page(start);
			}
		}
	}
	unsigned long start=((unsigned long)Pos.FB_addr)&(~0x1fffff);
	unsigned long end=((unsigned long)Pos.FB_addr+Pos.FB_length+0x1fffff)&(~0x1fffff);
	for(;start<end;start+=0x200000){
		map_2M_page(start);
	}
	flush_tlb();
	unsigned long* tmp=Pos.FB_addr;
	*tmp=0x00ff00;
	color_printk(0xff,0,"Display Memory Structure,%d entries\n",memory_management_struct.e820_length);
	for(i=0;i<=memory_management_struct.e820_length;i++){
		if(memory_management_struct.e820[i].type==1)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:RAM\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
		if(memory_management_struct.e820[i].type==2)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ROM or Reserved\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
		if(memory_management_struct.e820[i].type==3)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ACPI Reclaim Memory\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
		if(memory_management_struct.e820[i].type==4)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ACPI NVS Memory\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
	}
	color_printk(0xffff00,0,"OS Can Used Total RAM:%#018lx\n",totalmem);
	totalmem=0;
	for(i=0;i<=memory_management_struct.e820_length;i++){
		unsigned long start,end;
		if(memory_management_struct.e820[i].type!=1)	continue;
		start=PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		end=((memory_management_struct.e820[i].address+memory_management_struct.e820[i].length)>>PAGE_2M_SHIFT)<<PAGE_2M_SHIFT;
		if(end<=start)	continue;
		totalmem+=(end-start)>>PAGE_2M_SHIFT;
	}
	color_printk(0xffff00,0,"OS Can Used Total 2M PAGEs:%#010x=%010d\n",totalmem,totalmem);
	totalmem=memory_management_struct.e820[memory_management_struct.e820_length].address+memory_management_struct.e820[memory_management_struct.e820_length].length;
	//bits map init
	memory_management_struct.bits_map=(unsigned long*)((memory_management_struct.end_brk+PAGE_4K_SIZE-1)&PAGE_4K_MASK);
	memory_management_struct.bits_size=totalmem>>PAGE_2M_SHIFT;
	memory_management_struct.bits_length=(((unsigned long)(totalmem>>PAGE_2M_SHIFT)+(sizeof(long))*8-1)/8)&(~(sizeof(long)-1));
	memset(memory_management_struct.bits_map,0xff,memory_management_struct.bits_length);
	//pages init
	memory_management_struct.pages_struct=(struct Page*)(((unsigned long)memory_management_struct.bits_map+memory_management_struct.bits_length+PAGE_4K_SIZE-1)&PAGE_4K_MASK);
	memory_management_struct.pages_size=totalmem>>PAGE_2M_SHIFT;
	memory_management_struct.pages_length=((totalmem>>PAGE_2M_SHIFT)*sizeof(struct Page)+sizeof(long)-1)&(~(sizeof(long)-1));
	memset(memory_management_struct.pages_struct,0x00,memory_management_struct.pages_length);
	//zone init
	memory_management_struct.zones_struct=(struct Zone *)(((unsigned long)memory_management_struct.pages_struct+memory_management_struct.pages_length+PAGE_4K_SIZE-1)&PAGE_4K_MASK);
	memory_management_struct.zones_size=0;
	memory_management_struct.zones_length=(5*sizeof(struct Zone)+sizeof(long)-1)&(~(sizeof(long)-1));
	memset(memory_management_struct.zones_struct,0x00,memory_management_struct.zones_length);
	for(int i=0;i<=memory_management_struct.e820_length;i++){
		unsigned long start,end;
		struct Zone* Z;
		struct Page* P;
		unsigned long *b;
		if(memory_management_struct.e820[i].type!=1)	continue;
		start=PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		end=((memory_management_struct.e820[i].address+memory_management_struct.e820[i].length)>>PAGE_2M_SHIFT)<<PAGE_2M_SHIFT;
		if(end<=start)	continue;
		//setup zone
		Z=memory_management_struct.zones_struct+memory_management_struct.zones_size;
		memory_management_struct.zones_size++;
		Z->zone_start_address=start;
		Z->zone_end_address=end;
		Z->zone_length=end-start;
		Z->page_using_count=0;
		Z->page_free_count=(end-start)>>PAGE_2M_SHIFT;
		Z->total_pages_link=0;
		Z->attribute=0;
		Z->GMD_struct=&memory_management_struct;
		Z->pages_length=(end-start)>>PAGE_2M_SHIFT;
		Z->pages_group=(struct Page*)(memory_management_struct.pages_struct+(start>>PAGE_2M_SHIFT));
		//setup page
		P=Z->pages_group;
		for(j=0;j<Z->pages_length;j++,P++){
			P->zone_struct=Z;
			P->PHY_address=start+PAGE_2M_SIZE*j;
			P->attribute=0;
			P->reference_count=0;
			P->age=0;
			*(memory_management_struct.bits_map+((P->PHY_address>>PAGE_2M_SHIFT)>>6))^=1ul<<(P->PHY_address>>PAGE_2M_SHIFT)%64;
		}
	}
	//init address 0
	memory_management_struct.pages_struct->zone_struct=memory_management_struct.zones_struct;
	memory_management_struct.pages_struct->PHY_address=0;
	memory_management_struct.pages_struct->attribute=0;
	memory_management_struct.pages_struct->reference_count=0;
	memory_management_struct.pages_struct->age=0;
	memory_management_struct.zones_length=(memory_management_struct.zones_size*sizeof(struct Zone)+sizeof(long)-1)&(~(sizeof(long)-1));
	color_printk(0xffff00,0,"bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n",memory_management_struct.bits_map,memory_management_struct.bits_size,memory_management_struct.bits_length);
	color_printk(0xffff00,0,"pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n",memory_management_struct.pages_struct,memory_management_struct.pages_size,memory_management_struct.pages_length);
	color_printk(0xffff00,0,"zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n",memory_management_struct.zones_struct,memory_management_struct.zones_size,memory_management_struct.zones_length);
	ZONE_DMA_INDEX=0;
	ZONE_NORMAL_INDEX=0;
	for(int i=0;i<memory_management_struct.zones_size;i++){
		struct Zone *Z=memory_management_struct.zones_struct+i;
		color_printk(0xffff00,0,"zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",Z->zone_start_address,Z->zone_end_address,Z->zone_length,Z->pages_group,Z->pages_length);
		if(Z->zone_start_address==0x100000000)	ZONE_UNMAPED_INDEX=i;
	}
	memory_management_struct.end_of_struct=(unsigned long)((unsigned long)memory_management_struct.zones_struct+memory_management_struct.zones_length+sizeof(long)*32)&(~(sizeof(long)-1));
	color_printk(0xffff00,0,"start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",memory_management_struct.start_code,memory_management_struct.end_code,memory_management_struct.end_data,memory_management_struct.end_brk,memory_management_struct.end_of_struct);
	i=Virt_To_Phy(memory_management_struct.end_of_struct)>>PAGE_2M_SHIFT;
	for(j=0;j<=i;j++){
		page_init(memory_management_struct.pages_struct+j,PG_PTable_Maped|PG_Kernel_Init|PG_Active|PG_Kernel);
	}
	color_printk(0x00ffff,0,"Global_CR3\t:%#018lx\n",Global_CR3);
	color_printk(0x00ffff,0,"*Global_CR3\t:%#018lx\n",*Phy_To_Virt(Global_CR3)&(~0xff));
	color_printk(0x00ffff,0,"**Global_CR3\t:%#018lx\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3)&(~0xff))&(~0xff));
	//for(int i=0;i<10;i++)	*(Phy_To_Virt(Global_CR3)+i)=0ul;
	//flush_tlb();
}
