#ifndef __MULTIBOOT2_H__
#define __MULTIBOOT2_H__
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
struct multiboot2_info{
	uint32_t total_size;
	uint32_t reserved;
}__attribute__((packed));
struct multiboot2_tag{
	uint32_t type;
	uint32_t size;
}__attribute__((packed));
struct multiboot2_mmap_entry{
	uint64_t addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
}__attribute__((packed));
struct multiboot2_tag_mmap{
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
	struct multiboot2_mmap_entry entries[];
}__attribute__((packed));
struct multiboot2_tag_framebuffer{
	uint32_t type;
	uint32_t size;
	uint64_t addr;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	uint8_t framebuffer_type;
	uint8_t reserved;
	struct{
		uint8_t red_field_position;
		uint8_t red_mask_size;
		uint8_t green_field_position;
		uint8_t green_mask_size;
		uint8_t blue_field_position;
		uint8_t blue_mask_size;
	}color_info;
}__attribute__((packed));
struct multiboot2_tag_end{
	uint32_t type;
	uint32_t size;
}__attribute__((packed));
struct multiboot2_tag_acpi_new{
	uint32_t type;
	uint32_t size;
	uint8_t rsdp[36];
}__attribute__((packed));
uintptr_t get_tag(void* mbi_addr,uint32_t target){
	struct multiboot2_info* mbi=(struct multiboot2_info*)mbi_addr;
	uintptr_t ptr=(uintptr_t)mbi_addr+(sizeof(struct multiboot2_info));
	uintptr_t end=(uintptr_t)mbi_addr+mbi->total_size;
	while(ptr<end){
		struct multiboot2_tag* tag=(struct multiboot2_tag*)ptr;
		if(tag->type==target)	return (uintptr_t)tag;
		if(tag->type==0)	break;
		ptr+=(tag->size+7)&(~7ul);
	}
	return 0;
}
#endif
