#ifndef __MOUSE_H__
#define __MOUSE_H__
#define KBCMD_SENDTO_MOUSE 0xd4
#define MOUSE_ENABLE 0xf4
#define KBCMD_EN_MOUSE_INTFACE 0xa8
struct mouse_packet{
	unsigned char Byte0;
	char Byte1;
	char Byte2;
};
struct mouse_packet mouse;
extern void mouse_init();
extern struct keyboard_inputbuffer* p_mouse;
extern void analysis_mousecode();
#endif
