#ifndef __GATE_H__
#define __GATE_H__ 1
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
struct IDT_entry{
	uint16_t addr15_0;
	uint16_t segment;
	uint16_t IST :3,
			 reserved_1 :5,
			 type :4,
			 reserved_2 :1,
			 DPL :2,
			 present :1;
	uint16_t addr31_16;
	uint32_t addr63_32;
	uint32_t reserved;
}__attribute__((packed));
extern struct IDT_entry IDT_Table[];
struct GDT_entry{uint64_t x;};
extern struct GDT_entry GDT_Table[];
static inline __always_inline void set_intr_gate(struct IDT_entry* entry_addr,uint8_t ist,uintptr_t addr){
	entry_addr->addr15_0=addr&0xffff;
	entry_addr->segment=0x8;
	entry_addr->IST=ist;
	entry_addr->reserved_1=0;
	entry_addr->type=0xe;
	entry_addr->reserved_2=0;
	entry_addr->DPL=0;
	entry_addr->present=1;
	entry_addr->addr31_16=(addr>>16)&0xffff;
	entry_addr->addr63_32=addr>>32;
	entry_addr->reserved=0;
}
static inline __always_inline void set_trap_gate(struct IDT_entry* entry_addr,uint8_t ist,uintptr_t addr){
	entry_addr->addr15_0=addr&0xffff;
	entry_addr->segment=0x8;
	entry_addr->IST=ist;
	entry_addr->reserved_1=0;
	entry_addr->type=0xf;
	entry_addr->reserved_2=0;
	entry_addr->DPL=0;
	entry_addr->present=1;
	entry_addr->addr31_16=(addr>>16)&0xffff;
	entry_addr->addr63_32=addr>>32;
	entry_addr->reserved=0;
}
static inline __always_inline void set_system_gate(struct IDT_entry* entry_addr,uint8_t ist,uintptr_t addr){
	entry_addr->addr15_0=addr&0xffff;
	entry_addr->segment=0x8;
	entry_addr->IST=ist;
	entry_addr->reserved_1=0;
	entry_addr->type=0xf;
	entry_addr->reserved_2=0;
	entry_addr->DPL=3;
	entry_addr->present=1;
	entry_addr->addr31_16=(addr>>16)&0xffff;
	entry_addr->addr63_32=addr>>32;
	entry_addr->reserved=0;
}
static inline __always_inline void set_tss64(uint32_t* Table,uint64_t rsp0,uint64_t rsp1,uint64_t rsp2,uint64_t ist1,uint64_t ist2,uint64_t ist3,uint64_t ist4,uint64_t ist5,uint64_t ist6,uint64_t ist7){
	*(uint64_t*)(Table+1)=rsp0;
	*(uint64_t*)(Table+3)=rsp1;
	*(uint64_t*)(Table+5)=rsp2;
	*(uint64_t*)(Table+9)=ist1;
	*(uint64_t*)(Table+11)=ist2;
	*(uint64_t*)(Table+13)=ist3;
	*(uint64_t*)(Table+15)=ist4;
	*(uint64_t*)(Table+17)=ist5;
	*(uint64_t*)(Table+19)=ist6;
	*(uint64_t*)(Table+21)=ist7;	
}
static inline __always_inline void set_tss_descriptor(uint32_t n,void* addr){
	uint64_t limit=0x67;
	*(uint64_t*)(GDT_Table+n)=(limit&0xffff)|(((uint64_t)addr&0xffff)<<16)|(((uint64_t)addr>>16&0xff)<<32)|
									((uint64_t)0x89<<40)|((limit>>16&0xf)<<48)|(((uint64_t)addr>>24&0xff)<<56);
	*(uint64_t*)(GDT_Table+n+1)=((uint64_t)addr>>32&0xffffffff);
}
static inline __always_inline void load_TR(uint64_t n){
	__asm__ __volatile__(
		"ltr %%ax \n\t"
		:
		:"a"(n<<3)
		:"memory"
	);
}
#endif
