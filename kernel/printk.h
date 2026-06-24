#ifndef __PRINTK_H__
#define __PRINTK_H__
#include<stdarg.h>
#include<stdint.h>
struct position{
	uintptr_t FB_addr;
	uint64_t FB_length;
	uint32_t XResolution;
	uint32_t YResolution;
	uint32_t XPosition;
	uint32_t YPosition;
	uint32_t XCharSize;
	uint32_t YCharSize;
	uint32_t pitch;
	//struct spinlock_T printk_lock;
};
extern struct position Pos;
extern int32_t color_printk(uint32_t FRcolor,uint32_t BKcolor,const char* fmt,...);
extern uint8_t font_res[256][16];
#define LEFT 1
#define PLUS 2
#define SPACE 4
#define SPECIAL 8
#define ZEROPAD 16
#define SMALL 32
#define SIGN 64

#define RED 0xff0000
#define GREEN 0x00ff00
#define BLUE 0x0000ff
#define BLACK 0x000000
#define WHITE 0xffffff
#define YELLOW 0xffff00
#define CYAN 0x00ffff
#define PINK 0xff00ff
#define ORANGE 0xff7f00
#endif

