OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    .multiboot2 : {
		*(.multiboot2)
	}
	. = 0xffff800000000000 + 0x101000;
    .text : AT(ADDR(.text)-0xffff800000000000) {
		_text = . ;
		*head.o(.text)
        *(.text)
        _etext = . ;
    }
    . = ALIGN(8);
    .data : AT(ADDR(.data)-0xffff800000000000) {
		_data = . ;
        *(.data)
        _edata = . ;
    }
    .rodata : AT(ADDR(.rodata)-0xffff800000000000) {
		_rodata = . ;
		*(.rodata)
		_erodata = . ;
	}
	. = ALIGN(32768);
	.data.init_task : AT(ADDR(.data.init_task)-0xffff800000000000) {
		*(.data.init_task)
	}
    .bss : AT(ADDR(.bss)-0xffff800000000000) {
		_bss = . ;
        *(.bss)
        _ebss = . ;
    }
    _end = . ;
}
