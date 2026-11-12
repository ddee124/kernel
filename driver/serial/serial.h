#ifndef __SERIAL_H__
#define __SERIAL_H__ 1
#include<stdint.h>
#define COM1_PORT 0x3f8
extern void serial_init();
extern void serial_putc(unsigned char ch);
extern void serial_write(const unsigned char* buf,uint64_t len);
#endif
