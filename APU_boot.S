.balign 0x1000
.text
.code16
.globl _APU_boot_start
_APU_boot_start:
	_APU_boot_base = .
	cli
	wbinvd
	mov %cs,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%fs
	mov %ax,%gs
	movl $(_APU_boot_tmp_Stack_end - _APU_boot_base),%esp
	mov %cs,%ax
	movzx %ax,%esi
	shll $4,%esi
	leal (_APU_code32 - _APU_boot_base)(%esi),%eax
	movl %eax,_APU_code32_vector - _APU_boot_base
	leal (_APU_tmp_GDT - _APU_boot_base)(%esi),%eax
	movl %eax,(_APU_tmp_GDT +2 - _APU_boot_base)
	lidtl _APU_tmp_IDT - _APU_boot_base
	lgdtl _APU_tmp_GDT - _APU_boot_base
	movl %cr0,%eax
	or $1,%ax
	movl %eax,%cr0
	ljmpl *(_APU_code32_vector - _APU_boot_base)
.code32
.balign 4
_APU_code32:
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%ss
	mov %ax,%fs
	mov %ax,%gs
	ljmpl $0x08,$0x101000
.balign 4
_APU_tmp_IDT:
	.word 0
	.word 0,0
.balign 4
_APU_tmp_GDT:
	.short _APU_tmp_GDT_end - _APU_tmp_GDT -1
	.long _APU_tmp_GDT - _APU_boot_base
	.short 0
	.quad 0x00cf9a000000ffff
	.quad 0x00cf92000000ffff
_APU_tmp_GDT_end:
.balign 4
_APU_code32_vector:
	.long _APU_code32 - _APU_boot_base
	.word 0x08,0
.balign 4
_APU_boot_tmp_Stack_start:
	.org 0x400
_APU_boot_tmp_Stack_end:
.globl _APU_boot_end
_APU_boot_end:
