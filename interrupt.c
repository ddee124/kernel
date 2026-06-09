#include "interrupt.h"
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
