#include "driver/ps2/keyboard.h"
#include "driver/ps2/ps2.h"
#include "mem/memory.h"
#include "mem/slab.h"
#include "kernel/printk.h"
#include "lib/lib.h"
#include "driver/acpi/acpi.h"

#define KB_QUEUE 64

struct keyboard_inputbuffer* p_kb=0;
static uint8_t shift_l,shift_r,ctrl_l,ctrl_r,alt_l,alt_r;
static uint8_t caps_lock=0;
static uint8_t e0_prefix=0;		/* last byte was 0xe0 */
static uint8_t e1_remain=0;		/* inside a 0xe1 pause sequence */
static uint8_t kb_queue[KB_QUEUE];
static uint64_t kb_qhead=0,kb_qtail=0;

/* pop one raw scancode,0 when the buffer is empty */
uint8_t keyboard_get_scancode(){
	uint8_t ret;
	if(p_kb==0||p_kb->count==0)	return 0;
	if(p_kb->p_tail==p_kb->buf+KB_BUF_SIZE)	p_kb->p_tail=p_kb->buf;
	ret=*p_kb->p_tail++;
	p_kb->count--;
	return ret;
}
/* pop one decoded ascii byte,-1 when empty */
int keyboard_getchar(){
	int c;
	if(kb_qhead==kb_qtail)	return -1;
	c=kb_queue[kb_qtail];
	kb_qtail=(kb_qtail+1)%KB_QUEUE;
	return c;
}
static void kb_push(uint8_t c){
	kb_queue[kb_qhead]=c;
	kb_qhead=(kb_qhead+1)%KB_QUEUE;
	if(kb_qhead==kb_qtail)	kb_qtail=(kb_qtail+1)%KB_QUEUE;	/* overwrite oldest */
}
static void kb_echo(uint8_t c){
	switch(c){
		case '\b':
			if(Pos.XPosition)	color_printk(WHITE,BLACK,"\b");
			break;
		case '\n':
			color_printk(WHITE,BLACK,"\n");
			break;
		case '\t':
			color_printk(WHITE,BLACK,"\t");
			break;
		case 0x1b:
			color_printk(WHITE,BLACK,"<ESC>");
			break;
		default:
			color_printk(WHITE,BLACK,"%c",c);
			break;
	}
}
/* scancode set 1 -> ascii,with modifiers,turn make codes into key events */
static void keyboard_decode(uint8_t byte){
	uint8_t code,release,ch;
	if(e1_remain){
		e1_remain--;
		return;
	}
	if(byte==0xe0){
		e0_prefix=1;
		return;
	}
	if(byte==0xe1){			/* pause/break: drop the whole sequence */
		e0_prefix=0;
		e1_remain=5;
		return;
	}
	release=byte&0x80;
	code=byte&0x7f;
	if(e0_prefix){			/* extended keys */
		e0_prefix=0;
		switch(code){
			case 0x1d:	ctrl_r=!release;	break;
			case 0x38:	alt_r=!release;		break;
			case 0x53:	/* delete */
				if(!release&&(ctrl_l||ctrl_r)&&(alt_l||alt_r)){
					color_printk(RED,BLACK,"\n[reboot] ctrl+alt+del\n");
					ACPI_reboot();
				}
				break;
			default:	break;		/* arrows/home/... not mapped yet */
		}
		return;
	}
	switch(code){
		case 0x2a:	shift_l=!release;	return;
		case 0x36:	shift_r=!release;	return;
		case 0x1d:	ctrl_l=!release;	return;
		case 0x38:	alt_l=!release;		return;
		case 0x3a:	if(!release)	caps_lock^=1;	return;
	}
	if(release)	return;			/* ignore break codes */
	ch=keycode_map_normal[code*MAP_COLS+((shift_l||shift_r)?1:0)];
	switch(code){
		case 0x0e:	ch='\b';	break;	/* backspace */
		case 0x1c:	ch='\n';	break;	/* enter */
		case 0x0f:	ch='\t';	break;	/* tab */
		case 0x01:	ch=0x1b;	break;	/* esc */
	}
	if(caps_lock&&((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')))	ch^=0x20;
	if(ch==0)	return;
	kb_push(ch);
	kb_echo(ch);
}
/* called by the ps/2 controller layer for every keyboard byte */
void keyboard_byte(uint8_t byte){
	if(p_kb==0)	return;
	if(p_kb->p_head==p_kb->buf+KB_BUF_SIZE)	p_kb->p_head=p_kb->buf;
	*p_kb->p_head++=byte;
	p_kb->count++;
	keyboard_decode(byte);
}
void keyboard_init(){
	p_kb=(struct keyboard_inputbuffer*)kmalloc(sizeof(struct keyboard_inputbuffer),0);
	if(p_kb==0){
		color_printk(RED,BLACK,"keyboard: no memory\n");
		return;
	}
	p_kb->p_head=p_kb->buf;
	p_kb->p_tail=p_kb->buf;
	p_kb->count=0;
	memset(p_kb->buf,0,KB_BUF_SIZE);
	shift_l=0;shift_r=0;ctrl_l=0;ctrl_r=0;alt_l=0;alt_r=0;
	caps_lock=0;e0_prefix=0;e1_remain=0;
	ps2_register_keyboard(keyboard_byte);
}
