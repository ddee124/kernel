#include "syscall.h"
unsigned long no_system_call(struct pt_regs* regs){
	color_printk(0xff00ff,0,"no_system_call is calling,NR:%#04x\n",regs->rax);
	return -1;
}
unsigned long sys_printf(struct pt_regs* regs){
	color_printk(0,0xffffff,(char*)(regs->rdi));
	return 1;
}
system_call_t system_call_table[MAX_SYSTEM_CALL_NR]={[0]=no_system_call,[1]=sys_printf,[2 ... MAX_SYSTEM_CALL_NR-1]=no_system_call};
unsigned long syscall_rsp=0x7c00;
