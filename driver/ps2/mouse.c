#include "driver/ps2/mouse.h"
#include "driver/ps2/ps2.h"
#include "kernel/printk.h"
#include "lib/lib.h"

#define MOUSE_TRACE 1
static struct mouse_packet mouse_buf[MOUSE_BUF_SIZE];
static uint64_t mouse_head=0,mouse_tail=0;
static uint8_t pkt[3];
static uint64_t idx=0;
static struct mouse_packet last;

/* three byte standard packet: byte0 flags,byte1 dx,byte2 dy */
void mouse_byte(uint8_t byte){
	if(idx==0&&!(byte&0x08))	return;		/* lost sync,wait for a header */
	pkt[idx++]=byte;
	if(idx<3)	return;
	idx=0;
	if(pkt[0]&0xc0)	return;			/* x/y overflow: drop the packet */
	struct mouse_packet p;
	p.raw[0]=pkt[0];
	p.raw[1]=pkt[1];
	p.raw[2]=pkt[2];
	p.buttons=pkt[0]&0x07;
	p.dx=(int32_t)pkt[1]-((pkt[0]&0x10)?256:0);
	p.dy=(int32_t)pkt[2]-((pkt[0]&0x20)?256:0);
	last=p;
	mouse_buf[mouse_head]=p;
	mouse_head=(mouse_head+1)%MOUSE_BUF_SIZE;
	if(mouse_head==mouse_tail)	mouse_tail=(mouse_tail+1)%MOUSE_BUF_SIZE;	/* overwrite oldest */
#if MOUSE_TRACE
	color_printk(WHITE,BLACK,"(M:%d,%d,%d)",p.dx,p.dy,p.buttons);
#endif
}
/* 1 if a packet was returned,0 when empty */
int mouse_get_packet(struct mouse_packet* out){
	if(mouse_head==mouse_tail)	return 0;
	*out=mouse_buf[mouse_tail];
	mouse_tail=(mouse_tail+1)%MOUSE_BUF_SIZE;
	return 1;
}
void mouse_init(){
	uint8_t ack=0;
	ps2_register_aux(mouse_byte);
	/* f6: set defaults, f4: enable data reporting */
	if(!ps2_write_aux(0xf6,&ack)||ack!=0xfa){
		color_printk(RED,BLACK,"[mouse] no device (f6 ack:%#02x)\n",ack);
		return;
	}
	if(!ps2_write_aux(0xf4,&ack)||ack!=0xfa){
		color_printk(RED,BLACK,"[mouse] enable reporting failed (ack:%#02x)\n",ack);
		return;
	}
	color_printk(GREEN,BLACK,"[mouse] reporting enabled\n");
}
