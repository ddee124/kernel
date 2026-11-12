#ifndef __HPET_H__
#define __HPET_H__ 1
#include<stdint.h>
/* main counter frequency in Hz, 0 when HPET is not available */
extern uint64_t HPET_frequency();
extern uint64_t HPET_ticks();
extern void HPET_init();
extern void HPET_dump();
#endif
