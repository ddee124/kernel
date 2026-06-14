#include "task.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"
#include "lib.h"
#include "ptrace.h"
#include "schedule.h"
extern void ret_from_intr();
extern void kernel_thread_func(void);
extern unsigned long ret_system_call();
extern void system_call();
unsigned long system_call_function(struct pt_regs* regs){
	return system_call_table[regs->rax](regs);
}
void user_level_function(){
	long ret=0;
	char string[]="Hello World!\n";
	__asm__ __volatile__(
		"syscall \n\t"
		:"=a"(ret)
		:"a"(1),"D"(string)
		:"memory","rcx","r11"
	);
	while(1);
}
void map_user_2M(unsigned long vaddr,unsigned long phyaddr){
	unsigned long pml4_idx=0,pdpt_idx=0,pd_idx=0;
	pml4_idx=(vaddr>>PAGE_GDT_SHIFT)&0x1ff;
	pdpt_idx=(vaddr>>PAGE_1G_SHIFT)&0x1ff;
	pd_idx=(vaddr>>PAGE_2M_SHIFT)&0x1ff;
	unsigned long* tmp=Phy_To_Virt(Global_CR3);
	if(!(tmp[pml4_idx]&PAGE_Present)){
		unsigned long* virt=kmalloc(4096,0);
		tmp[pml4_idx]=Virt_To_Phy(virt)|PAGE_USER_Dir;
		memset(virt,0,4096);
	}
	tmp=Phy_To_Virt(tmp[pml4_idx]&(~0xfff));
	if(!(tmp[pdpt_idx]&PAGE_Present)){
		unsigned long* virt=kmalloc(4096,0);
		tmp[pdpt_idx]=Virt_To_Phy(virt)|PAGE_USER_Dir;
		memset(virt,0,4096);
	}
	tmp=Phy_To_Virt(tmp[pdpt_idx]&(~0xfff));
	if(!(tmp[pd_idx]&PAGE_Present))	tmp[pd_idx]=phyaddr|PAGE_USER_Page;
}
unsigned long do_execve(struct pt_regs* regs){
	struct Page* page=0;
	page=alloc_pages(ZONE_NORMAL,1,0);
	map_user_2M(0,page->PHY_address);
	page=alloc_pages(ZONE_NORMAL,1,0);
	map_user_2M(0x200000,page->PHY_address);
	flush_tlb();
	regs->rip=0;
	regs->rsp=0x200000;
	regs->rax=1;
	regs->ds=0;
	regs->es=0;
	regs->rflags=1<<9;
	color_printk(0xff0000,0,"do_execve task is running\n");
	memcpy(user_level_function,(void*)regs->rip,1024);
	if(!(current->flags&PF_KTHREAD))	current->addr_limit=0xffff800000000000;
	return 0;
}
__asm__(
	"kernel_thread_func: \n\t"
	"popq %r15 \n\t"
	"popq %r14 \n\t"
	"popq %r13 \n\t"
	"popq %r12 \n\t"
	"popq %r11 \n\t"
	"popq %r10 \n\t"
	"popq %r9 \n\t"
	"popq %r8 \n\t"
	"popq %rbx \n\t"
	"popq %rcx \n\t"
	"popq %rdx \n\t"
	"popq %rsi \n\t"
	"popq %rdi \n\t"
	"popq %rbp \n\t"
	"popq %rax \n\t"
	"movq %rax,%ds \n\t"
	"popq %rax \n\t"
	"movq %rax,%es \n\t"
	"popq %rax \n\t"
	"addq $0x38,%rsp \n\t"
	"movq %rdx,%rdi \n\t"
	"callq *%rbx \n\t"
	"movq %rax,%rdi \n\t"
	"callq do_exit \n\t"
);
inline void __switch_to(struct task_struct *prev,struct task_struct *next){
	init_tss[0].rsp0=next->thread->rsp0;
	set_tss64(TSS64_Table,init_tss[0].rsp0,init_tss[0].rsp1,init_tss[0].rsp2,init_tss[0].ist1,init_tss[0].ist2,init_tss[0].ist3,init_tss[0].ist4,init_tss[0].ist5,init_tss[0].ist6,init_tss[0].ist7);
	__asm__ __volatile__ ("movq %%fs,%0 \n\t":"=a"(prev->thread->fs));
	__asm__ __volatile__ ("movq %%gs,%0 \n\t":"=a"(prev->thread->gs));
	__asm__ __volatile__ ("movq %0,%%fs \n\t"::"a"(next->thread->fs));
	__asm__ __volatile__ ("movq %0,%%gs \n\t"::"a"(next->thread->gs));
	//wrmsr(0x175,next->thread->rsp0);
	syscall_rsp=current->thread->rsp0;
	color_printk(0xffffff,0,"prev->thread->rsp0:%#018lx\n",prev->thread->rsp0);
	color_printk(0xffffff,0,"prev->thread->rsp:%#018lx\n",prev->thread->rsp);
	color_printk(0xffffff,0,"prev->thread->rip:%#018lx\n",prev->thread->rip);
	color_printk(0xffffff,0,"next->thread->rsp0:%#018lx\n",next->thread->rsp0);
	color_printk(0xffffff,0,"next->thread->rsp:%#018lx\n",next->thread->rsp);
	color_printk(0xffffff,0,"next->thread->rip:%#018lx\n",next->thread->rip);
}
extern inline void __switch_to(struct task_struct *prev,struct task_struct *next);
unsigned long do_fork(struct pt_regs *regs,unsigned long clone_flags,unsigned long stack_start,unsigned long stack_size){
	struct task_struct *tsk=0;
	struct thread_struct *thd=0;
	struct Page *p=0;
	color_printk(0xffffff,0,"alloc_pages,bitmap:%#018lx\n",*memory_management_struct.bits_map);
	p=alloc_pages(ZONE_NORMAL,1,PG_PTable_Maped|PG_Kernel);
	color_printk(0xffffff,0,"alloc_pages,bitmap:%#018lx\n",*memory_management_struct.bits_map);
	tsk=(struct task_struct*)Phy_To_Virt(p->PHY_address);
	color_printk(0xffffff,0,"struct task_struct address:%#018lx\n",(unsigned long)tsk);
	memset(tsk,0,sizeof(*tsk));
	*tsk=*current;
	list_init(&tsk->list);
	list_add_to_before(&init_task_union.task.list,&tsk->list);
	tsk->pid++;
	tsk->state=TASK_UNINTERRUPTIBLE;
	tsk->priority=2;
	thd=(struct thread_struct*)(tsk+1);
	tsk->thread=thd;
	memset(thd,0,sizeof(struct thread_struct));
	memcpy(regs,(void*)((unsigned long)tsk+STACK_SIZE-sizeof(struct pt_regs)),sizeof(struct pt_regs));
	thd->rsp0=(unsigned long)tsk+STACK_SIZE;
	thd->rip=regs->rip;
	thd->rsp=(unsigned long)tsk+STACK_SIZE-sizeof(struct pt_regs);
	thd->fs=KERNEL_DS;
	thd->gs=KERNEL_DS;
	if(!(tsk->flags&PF_KTHREAD))	thd->rip=regs->rip=(unsigned long)ret_system_call;
	tsk->state=TASK_RUNNING;
	insert_task_queue(tsk);
	return 1;
}
unsigned long do_exit(unsigned long code){
	color_printk(0xff0000,0,"exit task is running,arg:%#018lx\n",code);
	while(1);
}
int kernel_thread(unsigned long (*fn)(unsigned long),unsigned long arg,unsigned long flags){
	struct pt_regs regs;
	memset(&regs,0,sizeof(regs));
	regs.rbx=(unsigned long)fn;
	regs.rdx=(unsigned long)arg;
	regs.ds=KERNEL_DS;
	regs.es=KERNEL_DS;
	regs.cs=KERNEL_CS;
	regs.ss=KERNEL_DS;
	regs.rflags=(1<<9);
	regs.rip=(unsigned long)kernel_thread_func;
	return do_fork(&regs,flags,0,0);
}
unsigned long init(unsigned long arg){
	struct pt_regs *regs;
	color_printk(0xff0000,0,"init task is running,arg:%#018lx\n",arg);
	current->thread->rip=(unsigned long)ret_system_call;
	current->thread->rsp=(unsigned long)current+STACK_SIZE-sizeof(struct pt_regs);
	current->flags=0;
	regs=(struct pt_regs*)current->thread->rsp;
	__asm__ __volatile__(
		"movq %1,%%rsp \n\t"
		"pushq %2 \n\t"
		"jmp do_execve \n\t"
		:
		:"D"(regs),"m"(current->thread->rsp),"m"(current->thread->rip)
		:"memory"
	);
	return 1;
}
void task_init(){
	struct task_struct *p=0;
	init_mm.pgd=(pml4t_t*)Global_CR3;
	init_mm.start_code=memory_management_struct.start_code;
	init_mm.end_code=memory_management_struct.end_code;
	init_mm.start_data=(unsigned long)&_data;
	init_mm.end_data=memory_management_struct.end_data;
	init_mm.start_rodata=(unsigned long)&_rodata;
	init_mm.end_rodata=(unsigned long)&_erodata;
	init_mm.start_brk=0;
	init_mm.end_brk=memory_management_struct.end_brk;
	init_mm.start_stack=_stack_start;
	/*wrmsr(0x174,KERNEL_CS);
	wrmsr(0x175,current->thread->rsp0);
	wrmsr(0x176,(unsigned long)system_call);*/
	wrmsr(0xC0000081,(((unsigned long)KERNEL_CS)<<32)|(((unsigned long)USER_CS-16)<<48));
	wrmsr(0xC0000082,(unsigned long)system_call);
	wrmsr(0xC0000084,0);
	syscall_rsp=current->thread->rsp0;
	struct task_struct* tmp=0;
	set_tss64(TSS64_Table,init_thread.rsp0,init_tss[0].rsp1,init_tss[0].rsp2,init_tss[0].ist1,init_tss[0].ist2,init_tss[0].ist3,init_tss[0].ist4,init_tss[0].ist5,init_tss[0].ist6,init_tss[0].ist7);
	init_tss[0].rsp0=init_thread.rsp0;
	list_init(&init_task_union.task.list);
	kernel_thread(init,10,CLONE_FS|CLONE_FILES|CLONE_SIGNAL);
	init_task_union.task.state=TASK_RUNNING;
	//p=container_of(list_next(&current->list),struct task_struct,list);
	//tmp=(struct task_struct*)list_next(&task_schedule.task_queue.list);
	//switch_to(current,tmp);
}
