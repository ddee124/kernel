#ifndef __SMP_H__
#define __SMP_H__
#include "spinlock.h"
extern unsigned char _APU_boot_start[];
extern unsigned char _APU_boot_end[];
extern void boot_ap(unsigned int bsp_id);
struct spinlock_T SMP_lock;
#endif
