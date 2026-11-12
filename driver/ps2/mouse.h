#ifndef __MOUSE_H__
#define __MOUSE_H__ 1
#include<stdint.h>
struct mouse_packet{
	int32_t dx,dy;		/* dy positive is up,ps/2 convention */
	uint8_t buttons;	/* bit0 left,bit1 right,bit2 middle */
	uint8_t raw[3];
};
#define MOUSE_BUF_SIZE 8
extern void mouse_init();
extern void mouse_byte(uint8_t byte);
extern int  mouse_get_packet(struct mouse_packet* out);
#endif
