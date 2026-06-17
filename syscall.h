#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "printk.h"
#include "ptrace.h"
#define MAX_SYSTEM_CALL_NR 128
typedef unsigned long(*system_call_t)(struct pt_regs* regs);
extern system_call_t system_call_table[MAX_SYSTEM_CALL_NR];
extern unsigned long syscall_rsp;
#endif
