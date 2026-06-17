#ifndef __PRINTK_H__
#define __PRINTK_H__
#include<stdarg.h>
#include "font.h"
#include "SMP.h"
#include "spinlock.h"
typedef struct position{
	void* FB_addr;
	unsigned long FB_length;
	unsigned int XResolution;
	unsigned int YResolution;
	unsigned int XPosition;
	unsigned int YPosition;
	unsigned int XCharSize;
	unsigned int YCharSize;
	unsigned int pitch;
	struct spinlock_T printk_lock;
}position;
position Pos;
extern int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char* fmt,...);
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
