.set MAGIC,0xe85250d6
.set ARCH,0
.set LENGTH,(header_end-header_start)
.section .multiboot2, "a", @progbits
header_start:
	.long MAGIC
	.long ARCH
	.long LENGTH
	.long -(MAGIC+ARCH+LENGTH)
	//information
	.align 8
	.short 1
	.short 0
	.long 16
	.long 6
	.long 8
	//fb
	.align 8
	.short 5
	.short 0
	.long 20
	.long 1440
	.long 900
	.long 32
	//end
	.align 8
	.short 0
	.short 0
	.long 8
header_end:
.section .text
.code32
.globl _start
_start:
	cli
    movl $0x7e00,%esp
	mov %ebx,%edi
	lgdt (GDT_POINTER-0xffff800000000000)
	movl %cr4,%eax
    orl $0x20,%eax
    movl %eax,%cr4
    movl $(__PML4E-0xffff800000000000),%eax
    movl %eax,%cr3
    mov $0xC0000080,%ecx
    rdmsr
    orl $0x100,%eax
    wrmsr
	mov %cr0,%eax
	orl $0x80000001,%eax
    mov %eax,%cr0
    ljmp $0x08,$(_start64-0xffff800000000000)
.code64
.globl _start64
_start64:
	movq $0x10,%rax
    movq %rax,%ds
    movq %rax,%es
    movq %rax,%fs
    movq %rax,%ss
    movq $0x7e00,%rsp
    pushq $0x8
    movabsq $_start64h,%rax
    pushq %rax
    lretq
.globl _start64h
_start64h:
    lgdt GDT_POINTER(%rip)
	movq $0x10,%rax
    movq %rax,%ds
    movq %rax,%es
    movq %rax,%fs
    movq %rax,%gs
    movq %rax,%ss
    movq $0x7e00,%rsp
    
    movq %rdi,%r15
	//load GDTR
	lidt IDT_POINTER(%rip)
    movq _stack_start(%rip),%rsp
    movq $0xc0000080,%rcx
    rdmsr
    orq $1,%rax
    wrmsr
    movq $0x1b,%rcx
    rdmsr
    bt $8,%rax
    jnc start_smp
setup_TSS64:
    leaq TSS64_Table(%rip),%rdx
    xorq %rax,%rax
    xorq %rcx,%rcx
    movq $0x89,%rax
    shlq $40,%rax
    movl %edx,%ecx
    shrl $24,%ecx
    shlq $56,%rcx
    addq %rcx,%rax
    xorq %rcx,%rcx
    movl %edx,%ecx
    andl $0xffffff,%ecx
    shlq $16,%rcx
    addq %rcx,%rax
    addq $103,%rax
    leaq GDT_Table(%rip),%rdi
    movq %rax,80(%rdi)
    shrq $32,%rdx
    movq %rdx,88(%rdi)
    //mov $0x50,%ax
    //ltr %ax
    
    movq %r15,%rdi
    movq go_to_kernel(%rip),%rax
    pushq $0x08
    pushq %rax
    lretq
go_to_kernel:
    .quad Start_Kernel
start_smp:
	movq go_to_smp_kernel(%rip),%rax
	pushq $0x08
	pushq %rax
	lretq
go_to_smp_kernel:
	.quad Start_SMP
.globl _stack_start
_stack_start:
    .quad init_task_union+32768
.align 8
.org 0x1000
__PML4E:
    .quad 0x103003
    .fill 255,8,0
    .quad 0x103003
    .fill 255,8,0
.org 0x2000
__PDPTE:
    .quad 0x104003
    .fill 511,8,0
.org 0x3000
__PDE:
    .quad 0x0000083
    .quad 0x0200083
    .quad 0x0400083
    .quad 0x0600083
    .quad 0x0800083
    .quad 0x0a00083
    .quad 0x0c00083
    .quad 0x0e00083
    .quad 0x1000083
    .quad 0x1200083
    .quad 0x1400083
    .quad 0x1600083
    .quad 0x1800083
    .quad 0x1a00083
    .quad 0x1c00083
    .quad 0x1e00083
    .quad 0x2000083
    .quad 0x2200083
    .quad 0x2400083
    .quad 0x2600083
    .quad 0x2800083
    .quad 0x2a00083
    .quad 0x2c00083
    .quad 0x2e00083
    .fill 504,8,0
//GDT Table
.section .data
.globl GDT_Table
GDT_Table:
    .quad 0x0000000000000000 /*NULL descriptor*/
    .quad 0x0020980000000000 /*KERNEL Code 64-bit Segment 08*/
    .quad 0x0000920000000000 /*KERNEL Data 64-bit Segment 10*/
    .quad 0x0000000000000000 /*USER Code 32-bit Segment 18*/
    //.quad 0x0000000000000000 /*USER Code 32-bit Segment 20*/
    .quad 0x0000f20000000000 /*USER Data 64-bit Segment 20*/
    .quad 0x0020f80000000000 /*USER Code 64-bit Segment 28*/
    .quad 0x00cf9a000000ffff /*KERNEL Code 32-bit Segment 30*/
    .quad 0x00cf92000000ffff /*KERNEL Data 32-bit Segment 38*/
    .fill 100,8,0
GDT_END:
GDT_POINTER:
GDT_LIMIT: .word GDT_END - GDT_Table -1
GDT_BASE: .quad GDT_Table
//IDT Table
.globl IDT_Table
IDT_Table:
    .fill 512,8,0
IDT_END:
IDT_POINTER:
IDT_LIMIT: .word IDT_END - IDT_Table -1
IDT_BASE: .quad IDT_Table
//TSS64 Table
.globl TSS64_Table
TSS64_Table:
    .fill 13,8,0
TSS64_END:
TSS64_POINTER:
TSS64_LIMIT: .word TSS64_END - TSS64_Table -1
TSS64_BASE: .quad TSS64_Table
