#ifndef __MULTIBOOT2_H__
#define __MULTIBOOT2_H__
struct multiboot2_info{
	unsigned int total_size;
	unsigned int reserved;
};
struct multiboot2_tag{
	unsigned int type;
	unsigned int size;
};
struct multiboot2_mmap_entry{
	unsigned long addr;
	unsigned long length;
	unsigned int type;
	unsigned int reserved;
};
struct multiboot2_tag_mmap{
	unsigned int type;
	unsigned int size;
	unsigned int entry_size;
	unsigned int entry_version;
	struct multiboot2_mmap_entry entries[];
};
struct multiboot2_tag_framebuffer{
	unsigned int type;
	unsigned int size;
	unsigned long addr;
	unsigned int pitch;
	unsigned int width;
	unsigned int height;
	unsigned char bpp;
	unsigned char framebuffer_type;
	unsigned char reserved;
	struct{
		unsigned char red_field_position;
		unsigned char red_mask_size;
		unsigned char green_field_position;
		unsigned char green_mask_size;
		unsigned char blue_field_position;
		unsigned char blue_mask_size;
	}color_info;
};
struct multiboot2_tag_end{
	unsigned int type;
	unsigned int size;
};
void* get_tag(unsigned long mbi_addr,unsigned int target){
	struct multiboot2_info *mbi=(struct multiboot2_info*)mbi_addr;
	unsigned char *ptr=(unsigned char*)mbi_addr+(sizeof(struct multiboot2_info));
	unsigned char *end=(unsigned char*)mbi_addr+mbi->total_size;
	while(ptr<end){
		struct multiboot2_tag *tag=(struct multiboot2_tag*)ptr;
		if(tag->type==target)	return (void*)tag;
		if(tag->type==0)	break;
		ptr+=(tag->size+7)&(~7ul);
	}
	return 0;
}
#endif
