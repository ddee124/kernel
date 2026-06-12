#include "interrupt.h"
#include "APIC.h"
#include "printk.h"
void(*interrupt[24])(void)={
	IRQ0x20_interrupt,IRQ0x21_interrupt,IRQ0x22_interrupt,IRQ0x23_interrupt,
	IRQ0x24_interrupt,IRQ0x25_interrupt,IRQ0x26_interrupt,IRQ0x27_interrupt,
	IRQ0x28_interrupt,IRQ0x29_interrupt,IRQ0x2a_interrupt,IRQ0x2b_interrupt,
	IRQ0x2c_interrupt,IRQ0x2d_interrupt,IRQ0x2e_interrupt,IRQ0x2f_interrupt,
	IRQ0x30_interrupt,IRQ0x31_interrupt,IRQ0x32_interrupt,IRQ0x33_interrupt,
	IRQ0x34_interrupt,IRQ0x35_interrupt,IRQ0x36_interrupt,IRQ0x37_interrupt
};
void(*SMP_interrupt[24])(void)={
	IRQ0xc8_interrupt,IRQ0xc9_interrupt,IRQ0xca_interrupt,IRQ0xcb_interrupt,
	IRQ0xcc_interrupt,IRQ0xcd_interrupt,IRQ0xce_interrupt,IRQ0xcf_interrupt
};
int register_irq(unsigned long irq,void* arg,void(*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),unsigned long parameter,struct hw_int_controller* controller,char* irq_name){
	struct irq_desc_T *p=&interrupt_desc[irq-32];
	p->controller=controller;
	p->irq_name=irq_name;
	p->parameter=parameter;
	p->flags=0;
	p->handler=handler;
	p->controller->install(irq,arg);
	p->controller->enable(irq);
	return 1;
}
int unregister_irq(unsigned long irq){
	struct irq_desc_T *p=&interrupt_desc[irq-32];
	p->controller->disable(irq);
	p->controller->uninstall(irq);
	p->controller=0;
	p->irq_name=0;
	p->parameter=0;
	p->flags=0;
	p->handler=0;
	return 1;
}
void do_IRQ(struct pt_regs *regs,unsigned long nr){
	if(nr&0x80){
		color_printk(RED,BLACK,"SMP IPI :%d\n",nr);
		Local_APIC_edge_level_ack(nr);
	}
	else{
		struct irq_desc_T* irq=&interrupt_desc[nr-32];
		if(irq->handler!=0)	irq->handler(nr,irq->parameter,regs);
		if(irq->controller!=0&&irq->controller->ack!=0)	irq->controller->ack(nr);
	}
}
