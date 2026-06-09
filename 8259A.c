#include "trap.h"
#include "lib.h"
#include "gate.h"
#include "8259A.h"
#include "interrupt.h"
void(*interrupt[24])(void)={
	IRQ0x20_interrupt,IRQ0x21_interrupt,IRQ0x22_interrupt,IRQ0x23_interrupt,
	IRQ0x24_interrupt,IRQ0x25_interrupt,IRQ0x26_interrupt,IRQ0x27_interrupt,
	IRQ0x28_interrupt,IRQ0x29_interrupt,IRQ0x2a_interrupt,IRQ0x2b_interrupt,
	IRQ0x2c_interrupt,IRQ0x2d_interrupt,IRQ0x2e_interrupt,IRQ0x2f_interrupt,
	IRQ0x30_interrupt,IRQ0x31_interrupt,IRQ0x32_interrupt,IRQ0x33_interrupt,
	IRQ0x34_interrupt,IRQ0x35_interrupt,IRQ0x36_interrupt,IRQ0x37_interrupt
};
void init_8259A(){
	int i;
	for(i=32;i<56;i++)	set_intr_gate(i,2,interrupt[i-32]);
	color_printk(0xff0000,0,"8259A init\n");
	//8259A_master ICW1-4
	io_out8(0x20,0x11);
	io_out8(0x21,0x20);
	io_out8(0x21,0x04);
	io_out8(0x21,0x01);
	//8259A_slave ICW 1-4
	io_out8(0xa0,0x11);
	io_out8(0xa1,0x28);
	io_out8(0xa1,0x02);
	io_out8(0xa1,0x01);
	//8259A-M/S OCW1
	io_out8(0x21,0x00);
	io_out8(0xa1,0x00);
	__asm__ __volatile__("inb $0x21,%%al \n orb $0x01,%%al \n outb %%al,$0x21":::"al");//disable irq0(clock)
	sti();
	
}
void do_IRQ(unsigned long regs,unsigned long nr){
	unsigned char x;
	color_printk(0xff0000,0,"do_IRQ:%#08x\t",nr);
	x=io_in8(0x60);
	color_printk(0xff0000,0,"key code:%#08x\n",x);
	if(nr==0x21&&x==0x13){
		io_out8(0xCF9,0x0E);
	}
	if(nr>=0x28)	io_out8(0xA0,0x20);
	io_out8(0x20,0x20);
}
