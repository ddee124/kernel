#ifndef __LIB_H__
#define __LIB_H__
struct List{
	struct List *prev;
	struct List *next;
};
inline void list_init(struct List * list){
	list->prev=list;
	list->next=list;
}
inline void list_add_to_behind(struct List *entry,struct List *new){
	new->next=entry->next;
	new->prev=entry;
	new->next->prev=new;
	entry->next=new;
}
inline void list_add_to_before(struct List * entry,struct List *new){
	new->next=entry;
	entry->prev->next=new;
	new->prev=entry->prev;
	entry->prev=new;
}
inline void list_del(struct List *entry){
	entry->next->prev=entry->prev;
	entry->prev->next=entry->next;
}
inline long list_is_empty(struct List *entry){
	if(entry==entry->next&&entry->prev==entry)	return 1;
	else    return 0;
}
extern inline struct List *list_prev(struct List *entry){return entry->prev;}
extern inline struct List *list_next(struct List *entry){return entry->next;}
extern inline void list_init(struct List * list);
extern inline void list_add_to_behind(struct List *entry,struct List *new);
extern inline void list_add_to_before(struct List * entry,struct List *new);
extern inline void list_del(struct List *entry);
extern inline long list_is_empty(struct List *entry);
inline int strlen(char* s){
	register int __res;
	__asm__ __volatile__ (
	"cld \n\t"
	"repne \n\t"
	"scasb \n\t"
	"notl %0 \n\t"
	"decl %0 \n\t"
	:"=c"(__res)
	:"D"(s),"a"(0),"0"(0xffffffff)
	:
	);
	return __res;
}
inline int is_digit(char ch){
    int result;
    __asm__ __volatile__(
        "cmpb $'0', %1\n\t"
        "jb 0f\n\t"
        "cmpb $'9', %1\n\t"
        "ja 0f\n\t"
        "movl $1, %0\n\t"
        "jmp 1f\n"
        "0:\n\t"
        "xorl %0, %0\n"
        "1:"
        : "=r"(result)
        : "r"(ch)
        :"cc"
    );
    return result;
}
void memset(void* addr,unsigned char c,unsigned long len){
	unsigned char* p=addr;
	while(len--)	*(p++)=c;
}
void memcpy(void* addr1,void* addr2,unsigned long len){
	unsigned char* p1=addr1;
	unsigned char* p2=addr2;
	while(len--)	*(p2++)=*(p1++);
}
void cli(){__asm__("cli");}
void sti(){__asm__("sti");}
void io_out8(unsigned short port,unsigned char value){
	__asm__ __volatile__(
		"outb %0,%%dx \n\t"
		:
		:"a"(value),"d"(port)
		:"memory"
	);
}
unsigned char io_in8(unsigned short port){
	unsigned char tmp;
	__asm__ __volatile__(
		"inb %%dx,%0 \n\t"
		:"=a"(tmp)
		:"d"(port)
		:"memory"
	);
	return tmp;
}
void io_out32(unsigned short port,unsigned int value){
	__asm__ __volatile__(
		"outl %%eax,%%dx \n\t"
		:
		:"a"(value),"d"(port)
		:"memory"
	);
}
unsigned int io_in32(unsigned short port){
	unsigned int tmp;
	__asm__ __volatile__(
		"inl %%dx,%%eax \n\t"
		:"=a"(tmp)
		:"d"(port)
		:"memory"
	);
	return tmp;
}
void io_mfence(){
	__asm__ __volatile__("mfence \n\t":::);
}
extern inline unsigned long rdmsr(unsigned long address){
	unsigned long ret1,ret2;
	__asm__ __volatile__(
		"rdmsr \n\t"
		:"=a"(ret1),"=d"(ret2)
		:"c"(address)
		:"memory"
	);
	return (ret2<<32)|ret1;
}
extern inline void wrmsr(unsigned long address,unsigned long value){
	__asm__ __volatile__(
		"wrmsr \n\t"
		:
		:"d"(value>>32),"a"(value&0xffffffff),"c"(address)
		:"memory"
	);
}
extern inline void nop(){__asm__ __volatile__("nop":::);};
unsigned long get_rflags(){
	unsigned long ret=0;
	__asm__ __volatile__(
		"pushf \n\t"
		"popq %0 \n\t"
		:"=r"(ret)
		:
		:"memory"
	);
	return ret;
}
/*#define container_of(ptr,type,member)\
({\
	typeof(((type*)0)->member)*p=(ptr);\
	(type*)((unsigned long)p-(unsigned long)&(((type*)0)->member));\
})*/
#endif
