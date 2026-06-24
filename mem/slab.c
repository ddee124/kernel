#include "mem/slab.h"
#include "mem/memory.h"
#include "mem/pagetable.h"
#include "kernel/printk.h"
#include "lib/lib.h"
struct Slab* kmalloc_create(uint64_t size){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	uint64_t i;
	struct Slab* slab=0;
	struct Page* page=0;
	uintptr_t vaddress=0;
	uint64_t structsize=0;
	page=alloc_pages(1,0,0);
	if(page==0){
		color_printk(RED,BLACK,"kmalloc_create()->alloc_pages()=>page == NULL\n");
		local_irq_restore(rflags_save);
		return 0;
	}
	//page_init(page,PG_Kernel);
	switch(size){
		case 8:
		case 16:
		case 32:
		case 64:
			vaddress=Phy_To_Virt(page->PHY_address);
			structsize=sizeof(struct Slab)+PAGE_4K_SIZE/size/8;
			slab=(struct Slab*)(vaddress+PAGE_4K_SIZE-structsize);
			slab->color_map=(uint64_t*)((uintptr_t)slab+sizeof(struct Slab));
			slab->free_count=(PAGE_4K_SIZE-(PAGE_4K_SIZE/size/8)-sizeof(struct Slab))/size;
			slab->using_count=0;
			slab->color_count=slab->free_count;
			slab->Vaddress=vaddress;
			slab->page=page;
			list_init(&slab->list);
			slab->color_length=((slab->color_count+63)>>6)<<3;
			memset(slab->color_map,0xff,slab->color_length);
			for(i=0;i<slab->color_length;i++)	*(slab->color_map+(i>>6))^=1ul<<(i%64);
			break;
		case 128:
		case 256:
		case 512:
		case 1024:
		case 2048:
			slab=(struct Slab*)kmalloc(sizeof(struct Slab),0);
			slab->free_count=PAGE_4K_SIZE/size;
			slab->using_count=0;
			slab->color_count=slab->free_count;
			slab->color_length=((slab->color_count+63)>>6)<<3;
			slab->color_map=(uint64_t*)kmalloc(slab->color_length,0);
			memset(slab->color_map,0xff,slab->color_length);
			slab->Vaddress=Phy_To_Virt(page->PHY_address);
			slab->page=page;
			list_init(&slab->list);
			for(i=0;i<slab->color_count;i++)	*(slab->color_map+(i>>6))^=1ul<<(i%64);
			break;
		default:
			color_printk(RED,BLACK,"kmalloc_create() ERROR: wrong size:%lu\n",size);
			free_pages(page,1);
			local_irq_restore(rflags_save);
			return 0;
	}
	local_irq_restore(rflags_save);
	return slab;
};
uintptr_t kmalloc(uint64_t size,uint64_t gfp_flags){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	uint64_t i,j;
	struct Slab* slab=0;
	if(size>2048){
		color_printk(RED,BLACK,"kmalloc() ERROR: wrong size:%lu\n",size);
		local_irq_restore(rflags_save);
		return 0;
	}
	for(i=0;i<9;i++)	if(kmalloc_cache_size[i].size>=size)	break;
	slab=kmalloc_cache_size[i].cache_pool;
	if(kmalloc_cache_size[i].total_free!=0){
		do{
			if(slab->free_count==0)	slab=(struct Slab*)list_next(&slab->list);
			else break;
		}while(slab!=kmalloc_cache_size[i].cache_pool);
	}
	else{
		slab=kmalloc_create(kmalloc_cache_size[i].size);
		if(slab==0){
			color_printk(RED,BLACK,"kmalloc()->kmalloc_create()=>slab == NULL\n");
			local_irq_restore(rflags_save);
			return 0;
		}
		kmalloc_cache_size[i].total_free+=slab->color_count;
		color_printk(RED,BLACK,"kmalloc()->kmalloc_create()<=size:%#010x\n",kmalloc_cache_size[i].size);
		list_add_to_behind(&kmalloc_cache_size[i].cache_pool->list,&slab->list);
	}
	for(j=0;j<slab->color_count;j++){
		if(*(slab->color_map+(j>>6))==0xfffffffffffffffful){
			j+=63;
			continue;
		}
		if((*(slab->color_map+(j>>6))&(1ul<<(j%64)))==0){
			*(slab->color_map+(j>>6))|=1ul<<(j%64);
			slab->using_count++;
			slab->free_count--;
			kmalloc_cache_size[i].total_free--;
			kmalloc_cache_size[i].total_using++;
			local_irq_restore(rflags_save);
			return (uintptr_t)slab->Vaddress+kmalloc_cache_size[i].size*j;
		}
	}
	color_printk(RED,BLACK,"kmalloc() ERROR:no memory can alloc\n");
	local_irq_restore(rflags_save);
	return 0;
}
uint64_t kfree(uintptr_t address){
	uint64_t rflags_save;
	local_irq_save(rflags_save);
	
	uint64_t i,idx;
	struct Slab* slab=0;
	uintptr_t page_base_addr=address&PAGE_4K_MASK;
	for(i=0;i<9;i++){
		slab=kmalloc_cache_size[i].cache_pool;
		do{
			if(slab->Vaddress==page_base_addr){
				idx=(address-slab->Vaddress)/kmalloc_cache_size[i].size;
				*(slab->color_map+(idx>>6))^=1ul<<(idx%64);
				slab->free_count++;
				slab->using_count--;
				kmalloc_cache_size[i].total_free++;
				kmalloc_cache_size[i].total_using--;
				if((slab->using_count==0)&&(kmalloc_cache_size[i].total_free>=slab->color_count*3/2)&&(kmalloc_cache_size[i].cache_pool!=slab)){
					list_del(&slab->list);
					kmalloc_cache_size[i].total_free-=slab->color_count;
					page_clean(slab->page);
					free_pages(slab->page,1);
					if(kmalloc_cache_size[i].size>64){
						kfree((uintptr_t)slab->color_map);
						kfree((uintptr_t)slab);
					}
				}
				local_irq_restore(rflags_save);
				return 1;
			}
			else    slab=(struct Slab*)(list_next(&slab->list));
		}while(slab!=kmalloc_cache_size[i].cache_pool);
	}
	color_printk(RED,BLACK,"kfree() ERROR: can`t free memory\n");
	local_irq_restore(rflags_save);
	return 0;
}
void slab_init(){
	struct Page* page=0;
	uintptr_t virtual=0;
	uint64_t i,j;
	page=alloc_pages(1,0,0);
	uintptr_t end=page->PHY_address;
	for(i=0;i<9;i++){
		kmalloc_cache_size[i].cache_pool=(struct Slab*)end;
		end=(end+sizeof(struct Slab)+7)&(~7ul);
		list_init(&kmalloc_cache_size[i].cache_pool->list);
		kmalloc_cache_size[i].cache_pool->using_count=0;
		kmalloc_cache_size[i].cache_pool->free_count=PAGE_4K_SIZE/kmalloc_cache_size[i].size;
		kmalloc_cache_size[i].cache_pool->color_length=((PAGE_4K_SIZE/kmalloc_cache_size[i].size+63)>>6)<<3;
		kmalloc_cache_size[i].cache_pool->color_count=kmalloc_cache_size[i].cache_pool->free_count;
		kmalloc_cache_size[i].cache_pool->color_map=(uint64_t*)end;
		end=(end+kmalloc_cache_size[i].cache_pool->color_length+7)&(~7ul);
		memset(kmalloc_cache_size[i].cache_pool->color_map,0xff,kmalloc_cache_size[i].cache_pool->color_length);
		for(j=0;j<kmalloc_cache_size[i].cache_pool->color_count;j++){
			*(kmalloc_cache_size[i].cache_pool->color_map+(j>>6))^=1ul<<(j%64);
		}
		kmalloc_cache_size[i].total_free=kmalloc_cache_size[i].cache_pool->color_count;
		kmalloc_cache_size[i].total_using=0;
	}
	for(i=0;i<9;i++){
		page=alloc_pages(1,0,0);
		//page_init(page,PG_Kernel|PG_Kernel_Init|PG_PTable_Maped);
		kmalloc_cache_size[i].cache_pool->page=page;
		kmalloc_cache_size[i].cache_pool->Vaddress=Phy_To_Virt(page->PHY_address);
		color_printk(0xff00,0,"address:%#018lx\n",page->PHY_address);
	}
}
void slab_test(){
	uint64_t i;
	uintptr_t addr[5];
	for(i=0;i<5;i++){
		addr[i]=kmalloc(1024,0);
		color_printk(CYAN,BLACK,"1024byte addr:%#018lx\n",addr[i]);
	}
	for(i=0;i<5;i++)	kfree(addr[i]);
	addr[0]=kmalloc(1024,0);
	color_printk(CYAN,BLACK,"1024byte addr:%#018lx\n",addr[0]);
}
