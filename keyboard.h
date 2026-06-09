#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include "lib.h"
#define KB_BUF_SIZE 100
struct keyboard_inputbuffer{
	unsigned char* p_head;
	unsigned char* p_tail;
	int count;
	unsigned char buf[KB_BUF_SIZE];
};
#define PORT_KB_DATA 0x60
#define PORT_KB_STATUS 0x64
#define PORT_KB_CMD 0x64
#define KBCMD_WRITE_CMD 0x60
#define KBCMD_READ_CMD 0x20
#define KB_INIT_MODE 0x47
#define KB_STATUS_IBF 0x02
#define KB_STATUS_OBF 0x01
#define wait_KB_write()	while(io_in8(PORT_KB_STATUS)&KB_STATUS_IBF)
#define wait_KB_read()	while(io_in8(PORT_KB_STATUS)&KB_STATUS_OBF)

#define NR_SCAN_CODES 0x80
#define MAP_COLS 2
#define PAUSEBREAK 1
#define PRINTSCREEN 2
#define OTHERKEY 4
#define FLAG_BREAK 0x80
unsigned char pausebreak_scode[]={0xE1,0x1D,0x45,0xE1,0x9D,0xC5};
extern void keyboard_init();
extern void analysis_keycode();
unsigned int keycode_map_normal[NR_SCAN_CODES*MAP_COLS]={
	/*0x00*/ 0,         0,
	/*0x01*/ 0,         0, //ESC
	/*0x02*/ '1',     '!',
	/*0x03*/ '2',     '@',
	/*0x04*/ '3',     '#',
	/*0x05*/ '4',     '$',
	/*0x06*/ '5',     '%',
	/*0x07*/ '6',     '^',
	/*0x08*/ '7',     '&',
	/*0x09*/ '8',     '*',
	/*0x0A*/ '9',     '(',
	/*0x0B*/ '0',     ')',
	/*0x0C*/ '-',     '_',
	/*0x0D*/ '=',     '+',
	/*0x0E*/   0,       0, //BACKSPACE
	/*0x0F*/   0,       0, //TAB
	/*0x10*/ 'q',     'Q',
	/*0x11*/ 'w',     'W',
	/*0x12*/ 'e',     'E',
	/*0x13*/ 'r',     'R',
	/*0x14*/ 't',     'T',
	/*0x15*/ 'y',     'Y',
	/*0x16*/ 'u',     'U',
	/*0x17*/ 'i',     'I',
	/*0x18*/ 'o',     'O',
	/*0x19*/ 'p',     'P',
	/*0x1A*/ '[',     '{',
	/*0x1B*/ ']',     '}',
	/*0x1C*/   0,       0, //ENTER
	/*0x1D*/ 0x1d,   0x1d, //CTRL left
	/*0x1E*/ 'a',     'A',
	/*0x1F*/ 's',     'S',
	/*0x20*/ 'd',     'D',
	/*0x21*/ 'f',     'F',
	/*0x22*/ 'g',     'G',
	/*0x23*/ 'h',     'H',
	/*0x24*/ 'j',     'J',
	/*0x25*/ 'k',     'K',
	/*0x26*/ 'l',     'L',
	/*0x27*/ ';',     ':',
	/*0x28*/ '\'',    '"',
	/*0x29*/ '`',     '~',
	/*0x2A*/ 0x2a,   0x2a, //SHIFT left
	/*0x2B*/ '\\',    '|',
	/*0x2C*/ 'z',     'Z',
	/*0x2D*/ 'x',     'X',
	/*0x2E*/ 'c',     'C',
	/*0x2F*/ 'v',     'V',
	/*0x30*/ 'b',     'B',
	/*0x31*/ 'n',     'N',
	/*0x32*/ 'm',     'M',
	/*0x33*/ ',',     '<',
	/*0x34*/ '.',     '>',
	/*0x35*/ '/',     '?',
	/*0x36*/ 0x36,   0x36, //SHIFT right
	/*0x37*/ '*',     '*',
	/*0x38*/ 0x38,   0x38, //ALT left
	/*0x39*/  ' ',    ' ',
	/*0x3A*/    0,      0,//CAPS LOCK
	/*0x3B*/    0,      0,//F1
	/*0x3C*/    0,      0,//F2
	/*0x3D*/    0,      0,//F3
	/*0x3E*/    0,      0,//F4
	/*0x3F*/    0,      0,//F5
	/*0x40*/    0,      0,//F6
	/*0x41*/    0,      0,//F7
	/*0x42*/    0,      0,//F8
	/*0x43*/    0,      0,//F9
	/*0x44*/    0,      0,//F10
	/*0x45*/    0,      0,//NUM LOCK
	/*0x46*/    0,      0,//SCROLL LOCK
	/*0x47*/  '7',      0,//PAD HOME
	/*0x48*/  '8',      0,//PAD UP
	/*0x49*/  '9',      0,//PAD PAGEUP
	/*0x4A*/  '-',      0,//PAD MINUS
	/*0x4B*/  '4',      0,//PAD LEFT
	/*0x4C*/  '5',      0,//PAD MID
	/*0x4D*/  '6',      0,//PAD RIGHT
	/*0x4E*/  '+',      0,//PAD PLUS
	/*0x4F*/  '1',      0,//PAD END
	/*0x50*/  '2',      0,//PAD DOWN
	/*0x51*/  '3',      0,//PAD PAGEDOWN
	/*0x52*/  '0',      0,//PAD INS
	/*0x53*/  '.',      0,//PAD DOT
	/*0x54*/    0,      0,
	/*0x55*/    0,      0,
	/*0x56*/    0,      0,
	/*0x57*/    0,      0,//F11
	/*0x58*/    0,      0,//F12
	/*0x59*/    0,      0,
	/*0x5A*/    0,      0,
	/*0x5B*/    0,      0,
	/*0x5C*/    0,      0,
	/*0x5D*/    0,      0,
	/*0x5E*/    0,      0,
	/*0x5F*/    0,      0,
	/*0x60*/    0,      0,
	/*0x61*/    0,      0,
	/*0x62*/    0,      0,
	/*0x63*/    0,      0,
	/*0x64*/    0,      0,
	/*0x65*/    0,      0,
	/*0x66*/    0,      0,
	/*0x67*/    0,      0,
	/*0x68*/    0,      0,
	/*0x69*/    0,      0,
	/*0x6A*/    0,      0,
	/*0x6B*/    0,      0,
	/*0x6C*/    0,      0,
	/*0x6D*/    0,      0,
	/*0x6E*/    0,      0,
	/*0x6F*/    0,      0,
	/*0x70*/    0,      0,
	/*0x71*/    0,      0,
	/*0x72*/    0,      0,
	/*0x73*/    0,      0,
	/*0x74*/    0,      0,
	/*0x75*/    0,      0,
	/*0x76*/    0,      0,
	/*0x77*/    0,      0,
	/*0x78*/    0,      0,
	/*0x79*/    0,      0,
	/*0x7A*/    0,      0,
	/*0x7B*/    0,      0,
	/*0x7C*/    0,      0,
	/*0x7D*/    0,      0,
	/*0x7E*/    0,      0,
	/*0x7F*/    0,      0,
};
#endif
