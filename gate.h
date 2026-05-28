#ifndef __GATE_H__
#define __GATE_H__
struct desc_struct {
	unsigned char x[8];
};
struct gate_struct{
	unsigned char x[16];
};
extern struct desc_struct GDT_Table[];
extern struct gate_struct IDT_Table[];
extern unsigned int TSS64_Table[26];
extern void sys_vector_init();
extern inline void set_intr_gate(unsigned int n,unsigned char ist,void * addr);
extern inline void set_trap_gate(unsigned int n,unsigned char ist,void * addr);
extern inline void set_system_gate(unsigned int n,unsigned char ist,void * addr);
#define _set_gate(gate_selector_addr,attr,ist,code_addr)\
__asm__ __volatile__(\
	"movw %%dx,%%ax \n\t"\
	"andq $07,%%rcx \n\t"\
	"addq %2,%%rcx \n\t"\
	"shlq $32,%%rcx \n\t"\
	"addq %%rcx,%%rax \n\t"\
	"xorq %%rcx,%%rcx \n\t"\
	"movl %%edx,%%ecx \n\t"\
	"shrq $16,%%rcx \n\t"\
	"shlq $48,%%rcx \n\t"\
	"addq %%rcx,%%rax \n\t"\
	"movq %%rax,%0 \n\t"\
	"shrq $32,%%rdx \n\t"\
	"movq %%rdx,%1 \n\t"\
	:"=m"(*((unsigned long*)(gate_selector_addr))),\
	"=m"(*(1+(unsigned long*)(gate_selector_addr)))\
	:"i"(attr<<8),"d"((unsigned long*)(code_addr)),"a"(0x8<<16),"c"(ist)\
	:"memory"\
);\

#define load_TR(n) \
do{\
	__asm__ __volatile__(\
	    "ltr %%ax"\
	    :\
	    :"a"(n<<3)\
	    :"memory"\
	);\
}while(0)

inline void set_intr_gate(unsigned int n,unsigned char ist,void * addr){
	_set_gate(IDT_Table+n,0x8E,ist,addr);//P,DPL=0,TYPE=E
}
inline void set_trap_gate(unsigned int n,unsigned char ist,void * addr){
	_set_gate(IDT_Table+n,0x8F,ist,addr);//P,DPL=0,TYPE=F
}
inline void set_system_gate(unsigned int n,unsigned char ist,void * addr){
	_set_gate(IDT_Table+n,0xEF,ist,addr);//P,DPL=3,TYPE=F
}
void set_tss64(unsigned long rsp0,unsigned long rsp1,unsigned long rsp2,unsigned long ist1,unsigned long ist2,unsigned long ist3,unsigned long ist4,unsigned long ist5,unsigned long ist6,unsigned long ist7){
	*(unsigned long *)(TSS64_Table+1) = rsp0;
	*(unsigned long *)(TSS64_Table+3) = rsp1;
	*(unsigned long *)(TSS64_Table+5) = rsp2;
	*(unsigned long *)(TSS64_Table+9) = ist1;
	*(unsigned long *)(TSS64_Table+11) = ist2;
	*(unsigned long *)(TSS64_Table+13) = ist3;
	*(unsigned long *)(TSS64_Table+15) = ist4;
	*(unsigned long *)(TSS64_Table+17) = ist5;
	*(unsigned long *)(TSS64_Table+19) = ist6;
	*(unsigned long *)(TSS64_Table+21) = ist7;	
}
#endif
