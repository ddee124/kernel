#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "lib/multiboot2.h"
#include "lib/lib.h"
#define PG_locked (1ul<<0)
#define PG_dirty (1ul<<1)
#define PG_uptodate (1ul<<2)
#define PG_active (1ul<<3)
#define PG_referenced (1ul<<4)
#define PG_swapcache (1ul<<5)
#define PG_reserved (1ul<<6)
#define PAGE_LOW 1
struct E820{
	uint64_t address;
	uint64_t length;
	uint32_t type;
}__attribute__((packed));
struct Zone{
	struct Page* pages_group;
	uint64_t pages_length;
	uint64_t* bits_map;
	uint64_t bits_length;
	uint64_t zone_start_address;
	uint64_t zone_end_address;
	uint64_t zone_length;
	uint64_t pages_count;
	uintptr_t end_ptr;
	uint64_t page_using_count;
	uint64_t page_free_count;
};
struct Page{
	struct Zone* zone_struct;
	uint64_t PHY_address;
	uint64_t attribute;
	uint32_t ref_count;
	uint32_t map_count;
	void* private;
	uint64_t index;
};
extern struct E820 e820[64];
struct Zone memory_zone[16];
extern uint64_t e820_entries;
extern uint64_t _end;
extern uintptr_t end_ptr;
extern uint64_t memory_entries;
extern uint64_t page_init(struct Page* page,uint64_t flags);
extern uint64_t page_clean(struct Page* page);
extern uint64_t get_page_attribute(struct Page* page);
extern uint64_t set_page_attribute(struct Page* page,uint64_t flags);
extern struct Page* alloc_pages(uint64_t number,uint64_t page_flags,uint64_t flag);
extern void free_pages(struct Page* page,int number);
extern void init_memory(struct multiboot2_tag_mmap* mmap_tag);
extern void display_memory_info();
#endif
