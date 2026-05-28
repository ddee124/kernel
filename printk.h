#ifndef __PRINTK_H__
#define __PRINTK_H__
#include<stdarg.h>
#include "font.h"
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
#endif
