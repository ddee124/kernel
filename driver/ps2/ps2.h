#ifndef __PS2_H__
#define __PS2_H__ 1
#include<stdint.h>
/* the 8042 controller layer: ports, command byte, byte pump.
   keyboard.c registers a byte sink, mouse support can come later. */
#define PS2_DATA_PORT   0x60
#define PS2_STATUS_PORT 0x64
#define PS2_CMD_PORT    0x64

#define PS2_ST_OBF 0x01		/* output buffer full */
#define PS2_ST_IBF 0x02		/* input buffer full */
#define PS2_ST_AUX 0x20		/* byte came from the aux (mouse) port */

/* the user of these macros must include lib/io.h */
#define PS2_WAIT_WRITE() while((io_in8(PS2_STATUS_PORT)&PS2_ST_IBF))
#define PS2_WAIT_READ()  while(!(io_in8(PS2_STATUS_PORT)&PS2_ST_OBF))

extern void ps2_init();
extern void ps2_register_keyboard(void (*handler)(uint8_t byte));
extern void ps2_register_aux(void (*handler)(uint8_t byte));
extern int  ps2_write_aux(uint8_t data,uint8_t* ack);
#endif
