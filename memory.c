#include "memory.h"
#include "multiboot2.h"
#include "lib.h"
#include "printk.h"
__attribute__((section(".bss"),aligned(4096))) static unsigned char page_reserved[4*4096]={0};
static unsigned int page_reserved_used=0;
struct Global_Memory_Descriptor memory_management_struct;
unsigned long* Global_CR3;
unsigned long page_init(struct Page* page,unsigned long flags){
	page->attribute|=flags;
	if(!page->reference_count||(page->attribute&PG_Shared)){
		page->reference_count++;
		page->zone_struct->total_pages_link++;
	}
	return 1;
}
unsigned long page_clean(struct Page* page){
	page->reference_count--;
	page->zone_struct->total_pages_link--;
	if(!page->reference_count){
		page->attribute&=PG_PTable_Maped;
	}
	return 1;
}
unsigned long get_page_attribute(struct Page* page){
	if(page==0){
		color_printk(0xff0000,0,"get_page_attribute() ERROR: page==NULL\n");
		return 0;
	}
	else{
		return page->attribute;
	}
}
unsigned long set_page_attribute(struct Page* page,unsigned long flags){
	if(page==0){
		color_printk(0xff0000,0,"set_page_attribute() ERROR: page==NULL\n");
		return 0;
	}
	else{
		page->attribute=flags;
		return 1;
	}
}
struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags){
	int i;
	unsigned long page=0;
	int zone_start=0;
	int zone_end=0;
	unsigned long attribute;
	if(number>=64||number<=0){
		color_printk(0xff0000,0,"alloc_pages() ERROR:number is invalid\n");
		return 0;
	}
	switch(zone_select){
		case ZONE_DMA:
		    zone_start=0;
		    zone_end=ZONE_DMA_INDEX;
		    attribute=PG_PTable_Maped;
		    break;
		case ZONE_NORMAL:
		    zone_start=ZONE_DMA_INDEX;
		    zone_end=memory_management_struct.zones_size-1;//temporary hack
		    attribute=PG_PTable_Maped;
		    break;
		case ZONE_UNMAPED:
		    zone_start=ZONE_UNMAPED_INDEX;
		    zone_end=memory_management_struct.zones_size-1;
		    attribute=0;
		    break;
		default:
		    color_printk(0xff0000,0,"alloc_pages() ERROR: zone_select index is invalid\n");
		    return 0;
		    break;
	}
	for(i=zone_start;i<=zone_end;i++){
		struct Zone *Z;
		unsigned long j,start,end;
		unsigned long tmp;
		if((memory_management_struct.zones_struct+i)->page_free_count<number)	continue;
		Z=memory_management_struct.zones_struct+i;
		start=(Z->zone_start_address)>>PAGE_2M_SHIFT;
		end=(Z->zone_end_address)>>PAGE_2M_SHIFT;
		tmp=(64-start)%64;
		for(j=start;j<end;j+=j%64?tmp:64){
			unsigned long *p=memory_management_struct.bits_map+(j>>6);
			unsigned long shift=j%64;
			unsigned long k=0;
			unsigned long num=(1ul<<number)-1;
			for(k=shift;k<64;k++){
				if(!((k?((*p>>k)|(*(p+1)<<(64-k))):*p)&(num))){
					unsigned long l;
					page=j-shift+k;
					for(l=0;l<number;l++){
						struct Page* X=memory_management_struct.pages_struct+page+l;
						*(memory_management_struct.bits_map+((X->PHY_address>>PAGE_2M_SHIFT)>>6))|=1ul<<((X->PHY_address>>PAGE_2M_SHIFT)%64);
						Z->page_using_count++;
						Z->page_free_count--;
						X->attribute=attribute;
					}
					return (struct Page*)(memory_management_struct.pages_struct+page);
				}
			}
		}
	}
	return 0;
}
void free_pages(struct Page* page,int number){
	int i=0;
	if(page==0){
		color_printk(0xff0000,0,"free_pages() ERROR:page is invalid\n");
		return;
	}
	if(number>=64||number<=0){
		color_printk(0xff0000,0,"free_pages() ERROR:number is invalid\n");
		return;
	}
	for(int i=0;i<number;i++,page++){
		*(memory_management_struct.bits_map+((page->PHY_address>>PAGE_2M_SHIFT)>>6))&=~(1ul<<((page->PHY_address>>PAGE_2M_SHIFT)%64));
		page->zone_struct->page_using_count--;
		page->zone_struct->page_free_count++;
		page->attribute=0;
	}
}
struct Slab* kmalloc_create(unsigned long size){
	int i;
	struct Slab* slab=0;
	struct Page* page=0;
	unsigned long* vaddress=0;
	long structsize=0;
	page=alloc_pages(ZONE_NORMAL,1,0);
	if(page==0){
		color_printk(0xff0000,0,"kmalloc_create()->alloc_pages()=>page == NULL\n");
		return 0;
	}
	page_init(page,PG_Kernel);
	switch(size){
		case 32:
		case 64:
		case 128:
		case 256:
		case 512:
			vaddress=Phy_To_Virt(page->PHY_address);
			structsize=sizeof(struct Slab)+PAGE_2M_SIZE/size/8;
			slab=(struct Slab*)((unsigned char*)vaddress+PAGE_2M_SIZE-structsize);
			slab->color_map=(unsigned long*)((unsigned char*)slab+sizeof(struct Slab));
			slab->free_count=(PAGE_2M_SIZE-(PAGE_2M_SIZE/size/8)-sizeof(struct Slab))/size;
			slab->using_count=0;
			slab->color_count=slab->free_count;
			slab->Vaddress=vaddress;
			slab->page=page;
			list_init(&slab->list);
			slab->color_length=((slab->color_count+sizeof(unsigned long)*8-1)>>6)<<3;
			memset(slab->color_map,0xff,slab->color_length);
			for(i=0;i<slab->color_length;i++)	*(slab->color_map+(i>>6))^=1ul<<(i%64);
			break;
		case 1024:
		case 2048:
		case 4096:
		case 8192:
		case 16384:
		case 32768:
		case 65536:
		case 131072:
		case 262144:
		case 524288:
		case 1048576:
			slab=(struct Slab*)kmalloc(sizeof(struct Slab),0);
			slab->free_count=PAGE_2M_SIZE/size;
			slab->using_count=0;
			slab->color_count=slab->free_count;
			slab->color_length=((slab->color_count+sizeof(unsigned long)*8-1)>>6)<<3;
			slab->color_map=(unsigned long*)kmalloc(slab->color_length,0);
			memset(slab->color_map,0xff,slab->color_length);
			slab->Vaddress=Phy_To_Virt(page->PHY_address);
			slab->page=page;
			list_init(&slab->list);
			for(i=0;i<slab->color_count;i++)	*(slab->color_map+(i>>6))^=1ul<<(i%64);
			break;
		default:
			color_printk(0xff0000,0,"kmalloc_create() ERROR: wrong size:%08d\n",size);
			free_pages(page,1);
			return 0;
	}
	return slab;
};
void* kmalloc(unsigned long size,unsigned long gfp_flags){
	int i,j;
	struct Slab* slab=0;
	if(size>1048576){
		color_printk(0xff0000,0,"kmalloc() ERROR:kmalloc size too long:%08d\n",size);
		return 0;
	}
	for(i=0;i<16;i++){
		if(kmalloc_cache_size[i].size>=size)	break;
	}
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
			color_printk(0xff0000,0,"kmalloc()->kmalloc_create()=>slab == NULL\n");
			return 0;
		}
		kmalloc_cache_size[i].total_free+=slab->color_count;
		color_printk(0x00ffff,0,"kmalloc()->kmalloc_create()<=size:%#010x\n",kmalloc_cache_size[i].size);
		list_add_to_before(&kmalloc_cache_size[i].cache_pool->list,&slab->list);
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
			return (void*)((char*)slab->Vaddress+kmalloc_cache_size[i].size*j);
		}
	}
	color_printk(0xff0000,0,"kmalloc() ERROR:no memory can alloc\n");
	return 0;
}
unsigned long kfree(void* address){
	int i;int idx;
	struct Slab* slab=0;
	void* page_base_addr=(void*)((unsigned long)address&PAGE_2M_MASK);
	for(i=0;i<16;i++){
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
					if(kmalloc_cache_size[i].size>512)	kfree(slab);
				}
				return 1;
			}
			else    slab=(struct Slab*)(list_next(&slab->list));
		}while(slab!=kmalloc_cache_size[i].cache_pool);
	}
	color_printk(0xff0000,0,"kfree() ERROR: can`t free memory\n");
	return 0;
}
struct Slab_cache* slab_create(unsigned long size,void*(*constructor)(void* Vaddress,unsigned long arg),void*(*destructor)(void* Vaddress,unsigned long arg)){
	struct Slab_cache* slab_cache=0;
	slab_cache=(struct Slab_cache*)kmalloc(sizeof(struct Slab_cache),0);
	if(slab_cache==0){
		color_printk(0xff0000,0,"slab_create()->kmalloc()=>slab_cache==NULL\n");
		return 0;
	}
	memset(slab_cache,0,sizeof(struct Slab_cache));
	slab_cache->size=SIZEOF_LONG_ALIGN(size);
	slab_cache->total_using=0;
	slab_cache->total_free=0;
	slab_cache->cache_pool=(struct Slab*)kmalloc(sizeof(struct Slab),0);
	if(slab_cache->cache_pool==0){
		color_printk(0xff0000,0,"slab_create()->kmalloc()=>slab_cache->cache_pool==NULL\n");
		kfree(slab_cache);
		return 0;
	}
	memset(slab_cache->cache_pool,0,sizeof(struct Slab));
	slab_cache->cache_dma_pool=0;
	slab_cache->constructor=constructor;
	slab_cache->destructor=destructor;
	list_init(&slab_cache->cache_pool->list);
	slab_cache->cache_pool->page=alloc_pages(ZONE_NORMAL,1,0);
	if(slab_cache->cache_pool->page==0){
		color_printk(0xff0000,0,"slab_create()->alloc_pages()=>slab_cache->cache_pool->page==NULL\n");
		kfree(slab_cache->cache_pool);
		kfree(slab_cache);
		return 0;
	}
	page_init(slab_cache->cache_pool->page,PG_Kernel);
	slab_cache->cache_pool->using_count=0;
	slab_cache->cache_pool->free_count=PAGE_2M_SIZE/slab_cache->size;
	slab_cache->total_free=slab_cache->cache_pool->free_count;
	slab_cache->cache_pool->Vaddress=Phy_To_Virt(slab_cache->cache_pool->page->PHY_address);
	slab_cache->cache_pool->color_count=slab_cache->cache_pool->free_count;
	slab_cache->cache_pool->color_length=((slab_cache->cache_pool->color_count+sizeof(unsigned long)*8-1)>>6)<<3;
	slab_cache->cache_pool->color_map=(unsigned long*)kmalloc(slab_cache->cache_pool->color_length,0);
	if(slab_cache->cache_pool->color_map==0){
		color_printk(0xff0000,0,"slab_create()->alloc_pages()=>slab_cache->cache_pool->color_map==NULL\n");
		free_pages(slab_cache->cache_pool->page,1);
		kfree(slab_cache->cache_pool);
		kfree(slab_cache);
		return 0;
	}
	memset(slab_cache->cache_pool->color_map,0,slab_cache->cache_pool->color_length);
	return slab_cache;
};
unsigned long slab_destroy(struct Slab_cache* slab_cache){
	struct Slab* slab_p=slab_cache->cache_pool;
	struct Slab* tmp_slab=0;
	if(slab_cache->total_using!=0){
		color_printk(0xff0000,0,"slab_cache->total_using!=0\n");
		return 0;
	}
	while(!list_is_empty(&slab_p->list)){
		tmp_slab=slab_p;
		//slab_p=container_of(list_next(&slab_p->list),struct Slab,list);
		slab_p=(struct Slab*)list_next(&slab_p->list);
		list_del(&tmp_slab->list);
		kfree(tmp_slab->color_map);
		page_clean(tmp_slab->page);
		free_pages(tmp_slab->page,1);
		kfree(tmp_slab);
	}
	kfree(slab_p->color_map);
	page_clean(slab_p->page);
	free_pages(slab_p->page,1);
	kfree(slab_p);
	kfree(slab_cache);
	return 1;
}
void* slab_malloc(struct Slab_cache* slab_cache,unsigned long arg){
	struct Slab* slab_p=slab_cache->cache_pool;
	struct Slab* tmp_slab=0;
	unsigned int j=0;
	if(slab_cache->total_free==0){
		tmp_slab=(struct Slab*)kmalloc(sizeof(struct Slab),0);
		if(tmp_slab==0){
			color_printk(0xff0000,0,"slab_malloc()->kmalloc()=>tmp_slab==NULL\n");
			return 0;
		}
		memset(tmp_slab,0,sizeof(struct Slab));
		list_init(&tmp_slab->list);
		tmp_slab->page=alloc_pages(ZONE_NORMAL,1,0);
		if(tmp_slab->page==0){
			color_printk(0xff0000,0,"slab_malloc()->alloc_pages()=>tmp_slab->page==NULL\n");
			kfree(tmp_slab);
			return 0;
		}
		page_init(tmp_slab->page,PG_Kernel);
		tmp_slab->using_count=0;
		tmp_slab->free_count=PAGE_2M_SIZE/slab_cache->size;
		tmp_slab->Vaddress=Phy_To_Virt(tmp_slab->page->PHY_address);
		tmp_slab->color_count=tmp_slab->free_count;
		tmp_slab->color_length=((tmp_slab->color_count+sizeof(unsigned long)*8-1)>>6)<<3;
		tmp_slab->color_map=(unsigned long*)kmalloc(tmp_slab->color_length,0);
		if(tmp_slab->color_map==0){
			color_printk(0xff0000,0,"slab_malloc()->kmalloc()=>tmp_slab->color_map==NULL\n");
			free_pages(tmp_slab->page,1);
			kfree(tmp_slab);
			return 0;
		}
		memset(tmp_slab->color_map,0,tmp_slab->color_length);
		list_add_to_behind(&slab_cache->cache_pool->list,&tmp_slab->list);
		slab_cache->total_free+=tmp_slab->free_count;
		for(j=0;j<tmp_slab->color_count;j++){
			if((*(tmp_slab->color_map+(j>>6))&(1ul<<(j%64)))==0){
				*(tmp_slab->color_map+(j>>6))|=1ul<<(j%64);
				tmp_slab->using_count++;
				tmp_slab->free_count--;
				slab_cache->total_using++;
				slab_cache->total_free--;
				if(slab_cache->constructor!=0){
					return slab_cache->constructor((char*)tmp_slab->Vaddress+slab_cache->size*j,arg);
				}
				else{
					return (void*)((char*)tmp_slab->Vaddress+slab_cache->size*j);
				}
			}
		}
	}
	else{
		do{
			if(slab_p->free_count==0){
				slab_p=(struct Slab*)list_next(&slab_p->list);
				continue;
			}
			for(j=0;j<slab_p->color_count;j++){
				if(*(slab_p->color_map+(j>>6))==0xfffffffffffffffful){
					j+=63;
					continue;
				}
				if((*(slab_p->color_map+(j>>6))&(1ul<<(j%64)))==0){
					*(slab_p->color_map+(j>>6))|=1ul<<(j%64);
					slab_p->using_count++;
					slab_p->free_count--;
					slab_cache->total_using++;
					slab_cache->total_free--;
					if(slab_cache->constructor!=0){
						return slab_cache->constructor((char*)slab_p->Vaddress+slab_cache->size*j,arg);
					}
					else{
						return (void*)((char*)slab_p->Vaddress+slab_cache->size*j);
					}
				}
			}
		}while(slab_p!=slab_cache->cache_pool);
	}
	color_printk(0xff0000,0,"slab_malloc() ERROR:cannot alloc\n");
	if(tmp_slab!=0){
		list_del(&tmp_slab->list);
		kfree(tmp_slab->color_map);
		page_clean(tmp_slab->page);
		free_pages(tmp_slab->page,1);
		kfree(tmp_slab);
	}
	return 0;
}
unsigned long slab_free(struct Slab_cache* slab_cache,void* address,unsigned long arg){
	struct Slab* slab_p=slab_cache->cache_pool;
	int idx=0;
	do{
		if(slab_p->Vaddress<=address&&address<slab_p->Vaddress+PAGE_2M_SIZE){
			idx=(address-slab_p->Vaddress)/slab_cache->size;
			*(slab_p->color_map+(idx>>6))^=1ul<<(idx%64);
			slab_p->free_count++;
			slab_p->using_count--;
			slab_cache->total_free++;
			slab_cache->total_using--;
			if(slab_cache->destructor!=0){
				slab_cache->destructor((char*)slab_p->Vaddress+slab_cache->size*idx,arg);
			}
			if((slab_p->using_count==0)&&(slab_cache->total_free>=slab_p->color_count*3/2)){
				list_del(&slab_p->list);
				slab_cache->total_free-=slab_p->color_count;
				kfree(slab_p->color_map);
				page_clean(slab_p->page);
				free_pages(slab_p->page,1);
				kfree(slab_p);
			}
			return 1;
		}
		else{
			slab_p=(struct Slab*)list_next(&slab_p->list);
			continue;
		}
	}while(slab_p!=slab_cache->cache_pool);
	color_printk(0xff0000,0,"slab_free() ERROR:address not in slab\n");
	return 0;
}
unsigned long slab_init(){
	struct Page* page=0;
	unsigned long* virtual=0;
	unsigned long i,j;
	unsigned long tmp_address=memory_management_struct.end_of_struct;
	for(i=0;i<16;i++){
		kmalloc_cache_size[i].cache_pool=(struct Slab* )memory_management_struct.end_of_struct;
		memory_management_struct.end_of_struct=memory_management_struct.end_of_struct+sizeof(struct Slab)+sizeof(long)*10;
		list_init(&kmalloc_cache_size[i].cache_pool->list);
		kmalloc_cache_size[i].cache_pool->using_count=0;
		kmalloc_cache_size[i].cache_pool->free_count=PAGE_2M_SIZE/kmalloc_cache_size[i].size;
		kmalloc_cache_size[i].cache_pool->color_length=((PAGE_2M_SIZE/kmalloc_cache_size[i].size+sizeof(unsigned long)*8-1)>>6)<<3;
		kmalloc_cache_size[i].cache_pool->color_count=kmalloc_cache_size[i].cache_pool->free_count;
		kmalloc_cache_size[i].cache_pool->color_map=(unsigned long*)memory_management_struct.end_of_struct;
		memory_management_struct.end_of_struct=(unsigned long)(memory_management_struct.end_of_struct+kmalloc_cache_size[i].cache_pool->color_length+sizeof(long)*10)&(~(sizeof(long)-1));
		memset(kmalloc_cache_size[i].cache_pool->color_map,0xff,kmalloc_cache_size[i].cache_pool->color_length);
		for(j=0;j<kmalloc_cache_size[i].cache_pool->color_count;j++){
			*(kmalloc_cache_size[i].cache_pool->color_map+(j>>6))^=1ul<<(j%64);
		}
		kmalloc_cache_size[i].total_free=kmalloc_cache_size[i].cache_pool->color_count;
		kmalloc_cache_size[i].total_using=0;
	}
	i=Virt_To_Phy(memory_management_struct.end_of_struct)>>PAGE_2M_SHIFT;
	for(j=PAGE_2M_ALIGN(Virt_To_Phy(tmp_address))>>PAGE_2M_SHIFT;j<=i;j++){
		page=memory_management_struct.pages_struct+j;
		*(memory_management_struct.bits_map+((page->PHY_address>>PAGE_2M_SHIFT)>>6))|=1ul<<((page->PHY_address>>PAGE_2M_SHIFT)%64);
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		page_init(page,PG_PTable_Maped|PG_Kernel_Init|PG_Kernel);
	}
	//color_printk(0xffff00,0,"2.memory_management_struct.bits_map:%#018lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n",*memory_management_struct.bits_map,memory_management_struct.zones_struct->page_using_count,memory_management_struct.zones_struct->page_free_count);
	//temporary hack
	/*for(i=0;i<16;i++){
		virtual=(unsigned long*)((memory_management_struct.end_of_struct+PAGE_2M_SIZE*i+PAGE_2M_SIZE-1)&PAGE_2M_MASK);
		//page=Virt_To_2M_Page(virtual);
		page=memory_management_struct.pages_struct+(Virt_To_Phy(virtual)>>PAGE_2M_SHIFT);
		*(memory_management_struct.bits_map+((page->PHY_address>>PAGE_2M_SHIFT)>>6))|=1ul<<((page->PHY_address>>PAGE_2M_SHIFT)%64);
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		page_init(page,PG_PTable_Maped|PG_Kernel_Init|PG_Kernel);
		kmalloc_cache_size[i].cache_pool->page=page;
		kmalloc_cache_size[i].cache_pool->Vaddress=virtual;
	}*/
	for(i=0;i<16;i++){
		page=alloc_pages(ZONE_NORMAL,1,0);
		page_init(page,PG_Kernel|PG_Kernel_Init|PG_PTable_Maped);
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		kmalloc_cache_size[i].cache_pool->page=page;
		kmalloc_cache_size[i].cache_pool->Vaddress=Phy_To_Virt(page->PHY_address);
		//color_printk(0xff00,0,"address:%#018lx\n",page->PHY_address);
	}
	//color_printk(0xffff00,0,"3.memory_management_struct.bits_map:%#018lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n",*memory_management_struct.bits_map,memory_management_struct.zones_struct->page_using_count,memory_management_struct.zones_struct->page_free_count);
	//color_printk(0xffff00,0,"start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",memory_management_struct.start_code,memory_management_struct.end_code,memory_management_struct.end_data,memory_management_struct.end_brk,memory_management_struct.end_of_struct);
	return 1;
}
unsigned long alloc_reserved_page(void){
	unsigned long phys=(unsigned long)&page_reserved[page_reserved_used];
	page_reserved_used+=4096;
	if(page_reserved_used>sizeof(page_reserved)){
		return 0;
	}
	return phys-0xffff800000000000;
}
void map_FB_2M_page(unsigned long phys_addr){
	if(phys_addr>=0xffff800000000000)	phys_addr-=0xffff800000000000;
	unsigned long* pml4=(unsigned long*)(((unsigned long)Global_CR3)&(~0xffful));
	unsigned long vaddr=phys_addr;
	unsigned short pml4_idx=(vaddr>>39)&0x1ff;
	unsigned short pdpt_idx=(vaddr>>30)&0x1ff;
	unsigned short pd_idx=(vaddr>>21)&0x1ff;
	if(!(pml4[pml4_idx]&1)){
		unsigned long new_pdpt=alloc_reserved_page();
		pml4[pml4_idx]=new_pdpt|0x3;
		pml4[pml4_idx|0x100]=new_pdpt|0x3;
	}
	unsigned long pdpt_phys=pml4[pml4_idx]&(~0xffful);
	unsigned long *pdpt=(unsigned long*)(pdpt_phys);
	if(!(pdpt[pdpt_idx]&1)){
		unsigned long new_pd=alloc_reserved_page();
		pdpt[pdpt_idx]=new_pd|0x3;
	}
	unsigned long pd_phys=pdpt[pdpt_idx]&(~0xffful);
	unsigned long *pd=(unsigned long*)(pd_phys);
	pd[pd_idx]=phys_addr|PAGE_KERNEL_Page|PAGE_PWT|PAGE_PCD;
}
void pagetable_init(){
	unsigned long i,j;
	unsigned long* tmp=0;
	Global_CR3=Get_gdt();
	for(int i=0;i<memory_management_struct.zones_size;i++){
		struct Zone* z=memory_management_struct.zones_struct+i;
		struct Page* p=z->pages_group;
		for(j=0;j<z->pages_length;j++,p++){
			unsigned long pml4_idx=0,pdpt_idx=0,pd_idx=0;
			pml4_idx=((unsigned long)Phy_To_Virt(p->PHY_address)>>PAGE_GDT_SHIFT)&0x1ff;
			pdpt_idx=((unsigned long)Phy_To_Virt(p->PHY_address)>>PAGE_1G_SHIFT)&0x1ff;
			pd_idx=((unsigned long)Phy_To_Virt(p->PHY_address)>>PAGE_2M_SHIFT)&0x1ff;
			tmp=Phy_To_Virt((unsigned long)Global_CR3&(~0xffful));
			if(tmp[pml4_idx]==0){
				unsigned long *virtual=kmalloc(PAGE_4K_SIZE,0);
				tmp[pml4_idx]=Virt_To_Phy(virtual)|PAGE_KERNEL_Dir;
			}
			//tmp=(unsigned long*)((unsigned long)Phy_To_Virt(tmp[pml4_idx]&(~0xffful)));
			tmp=Phy_To_Virt((unsigned long)tmp[pml4_idx]&(~0xffful));
			if(tmp[pdpt_idx]==0){
				unsigned long *virtual=kmalloc(PAGE_4K_SIZE,0);
				tmp[pdpt_idx]=Virt_To_Phy(virtual)|PAGE_KERNEL_Dir;
			}
			tmp=Phy_To_Virt((unsigned long)tmp[pdpt_idx]&(~0xffful));
			tmp[pd_idx]=p->PHY_address|PAGE_KERNEL_Page;
			//if(j%64==0)	color_printk(GREEN,BLACK,"@:%#018lx,%#018lx\t\n",(unsigned long)(tmp+pd_idx),tmp[pd_idx]);
		}
	}
	flush_tlb();
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
		/*if(p->type==1){
			unsigned long start=(p->addr)&(~0x1fffff);
			unsigned long end=(p->addr+p->length+0x1fffff)&(~0x1fffff);
			for(;start<end;start+=0x200000){
				map_2M_page(start);
			}
		}*/
	}
	unsigned long start=((unsigned long)Pos.FB_addr)&(~0x1fffff);
	unsigned long end=((unsigned long)Pos.FB_addr+Pos.FB_length+0x1fffff)&(~0x1fffff);
	for(;start<end;start+=0x200000){
		map_FB_2M_page(start);
	}
	flush_tlb();
	unsigned long* tmp=Pos.FB_addr;
	*tmp=0x00ff00;
	/*color_printk(0xff,0,"Display Memory Structure,%d entries\n",memory_management_struct.e820_length);
	for(i=0;i<=memory_management_struct.e820_length;i++){
		if(memory_management_struct.e820[i].type==1)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:RAM\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
		if(memory_management_struct.e820[i].type==2)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ROM or Reserved\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
		if(memory_management_struct.e820[i].type==3)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ACPI Reclaim Memory\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
		if(memory_management_struct.e820[i].type==4)	color_printk(0xffff00,0,"Address:%#018lx\tLength:%#018lx\tType:ACPI NVS Memory\n",memory_management_struct.e820[i].address,memory_management_struct.e820[i].length);
	}*/
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
	ZONE_UNMAPED_INDEX=0;
	for(int i=0;i<memory_management_struct.zones_size;i++){
		struct Zone *Z=memory_management_struct.zones_struct+i;
		color_printk(0xffff00,0,"zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",Z->zone_start_address,Z->zone_end_address,Z->zone_length,Z->pages_group,Z->pages_length);
		if(Z->zone_start_address>=0x100000000&&!ZONE_UNMAPED_INDEX)	ZONE_UNMAPED_INDEX=i;
	}
	memory_management_struct.end_of_struct=(unsigned long)((unsigned long)memory_management_struct.zones_struct+memory_management_struct.zones_length+sizeof(long)*32)&(~(sizeof(long)-1));
	color_printk(0xffff00,0,"start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",memory_management_struct.start_code,memory_management_struct.end_code,memory_management_struct.end_data,memory_management_struct.end_brk,memory_management_struct.end_of_struct);
	i=Virt_To_Phy(memory_management_struct.end_of_struct)>>PAGE_2M_SHIFT;
	for(j=1;j<=i;j++){
		//page_init(memory_management_struct.pages_struct+j,PG_PTable_Maped|PG_Kernel_Init|PG_Active|PG_Kernel);
		struct Page* tmp_page=memory_management_struct.pages_struct+j;
		page_init(tmp_page,PG_PTable_Maped|PG_Kernel_Init|PG_Kernel);
		*(memory_management_struct.bits_map+((tmp_page->PHY_address>>PAGE_2M_SHIFT)>>6))|=1ul<<((tmp_page->PHY_address>>PAGE_2M_SHIFT)%64);
		tmp_page->zone_struct->page_using_count++;
		tmp_page->zone_struct->page_free_count--;
	}
	//color_printk(0x00ffff,0,"Global_CR3\t:%#018lx\n",Global_CR3);
	//color_printk(0x00ffff,0,"*Global_CR3\t:%#018lx\n",*Phy_To_Virt(Global_CR3)&(~0xff));
	//color_printk(0x00ffff,0,"**Global_CR3\t:%#018lx\n",*Phy_To_Virt(*Phy_To_Virt(Global_CR3)&(~0xff))&(~0xff));
	//color_printk(0xffff00,0,"1.memory_management_struct.bits_map:%#018lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n",*memory_management_struct.bits_map,memory_management_struct.zones_struct->page_using_count,memory_management_struct.zones_struct->page_free_count);
	slab_init();
	pagetable_init();
	for(int i=0;i<10;i++)	*(Phy_To_Virt(Global_CR3)+i)=0ul;
	flush_tlb();
}
