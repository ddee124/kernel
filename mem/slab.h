#ifndef __SLAB_H__
#define __SLAB_H__ 1
#include "lib/list.h"
struct Slab{
	struct List list;
	struct Page* page;
	uint64_t using_count;
	uint64_t free_count;
	uintptr_t Vaddress;
	uint64_t color_length;
	uint64_t color_count;
	uint64_t* color_map;
};
struct Slab_cache{
	uint64_t size;
	uint64_t total_using;
	uint64_t total_free;
	struct Slab* cache_pool;
};
struct Slab_cache kmalloc_cache_size[9]={
	{8,0,0,0},
	{16,0,0,0},
	{32,0,0,0},
	{64,0,0,0},
	{128,0,0,0},
	{256,0,0,0},
	{512,0,0,0},
	{1024,0,0,0},
	{2048,0,0,0}
};
extern void slab_init();
extern uint64_t kfree(uintptr_t address);
extern uintptr_t kmalloc(uint64_t size,uint64_t gfp_flags);
extern struct Slab* kmalloc_create(uint64_t size);
extern void slab_test();
#endif
