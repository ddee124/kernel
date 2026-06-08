.text
.macro IRQ_ENTRY nr
.globl IRQ\()\nr\()_interrupt
.type  IRQ\()\nr\()_interrupt, @function
IRQ\()\nr\()_interrupt:
    pushq $0x00
    cld
    pushq %rax
    pushq %rax
    movq %es,%rax
    pushq %rax
    movq %ds,%rax
    pushq %rax
    xorq %rax,%rax
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq $0x10,%rdx
    movq %rdx,%ds
    movq %rdx,%es
    movq %rsp,%rdi
    leaq ret_from_intr(%rip),%rax
    pushq %rax
    movq $\nr,%rsi
    jmp do_IRQ
.endm

IRQ_ENTRY 0x20
IRQ_ENTRY 0x21
IRQ_ENTRY 0x22
IRQ_ENTRY 0x23
IRQ_ENTRY 0x24
IRQ_ENTRY 0x25
IRQ_ENTRY 0x26
IRQ_ENTRY 0x27
IRQ_ENTRY 0x28
IRQ_ENTRY 0x29
IRQ_ENTRY 0x2a
IRQ_ENTRY 0x2b
IRQ_ENTRY 0x2c
IRQ_ENTRY 0x2d
IRQ_ENTRY 0x2e
IRQ_ENTRY 0x2f
IRQ_ENTRY 0x30
IRQ_ENTRY 0x31
IRQ_ENTRY 0x32
IRQ_ENTRY 0x33
IRQ_ENTRY 0x34
IRQ_ENTRY 0x35
IRQ_ENTRY 0x36
IRQ_ENTRY 0x37
