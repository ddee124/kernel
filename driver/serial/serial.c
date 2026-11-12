#include "driver/serial/serial.h"
#include "lib/io.h"
/* 16550, polled TX only. No interrupt, no RX yet. */
#define REG_DATA 0
#define REG_IER  1
#define REG_FCR  2
#define REG_LCR  3
#define REG_MCR  4
#define REG_LSR  5
#define LSR_THRE 0x20

static uint16_t port=COM1_PORT;
void serial_init(){
	io_out8(port+REG_IER,0x00);	//disable all uart interrupts
	io_out8(port+REG_LCR,0x80);	//DLAB=1
	io_out8(port+REG_DATA,0x01);	//divisor=1 -> 115200
	io_out8(port+REG_IER,0x00);
	io_out8(port+REG_LCR,0x03);	//8N1
	io_out8(port+REG_FCR,0xc7);	//fifo on,clear,14 byte threshold
	io_out8(port+REG_MCR,0x0b);	//DTR|RTS|OUT2
}
void serial_putc(unsigned char ch){
	uint64_t timeout=1000000;
	while(!(io_in8(port+REG_LSR)&LSR_THRE)){
		if(!--timeout)	return;	//uart absent, do not hang the kernel
	}
	if(ch=='\n')	io_out8(port,'\r');
	io_out8(port,ch);
}
void serial_write(const unsigned char* buf,uint64_t len){
	uint64_t i;
	for(i=0;i<len;i++)	serial_putc(buf[i]);
}
