.code64
.text
R15=0x00
R14=0x08
R13=0x10
R12=0x18
RBP=0x20
RBX=0x28
R11=0x30
R10=0x38
R9=0x40
R8=0x48
RAX=0x50
RCX=0x58
RDX=0x60
RSI=0x68
RDI=0x70
ERRCODE=0x78
RIP=0x80
CS=0x88
RFLAGS=0x90
RSP=0x98
SS=0xA0

RESTORE_ALL:
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rax
	popq %rcx
	popq %rdx
	popq %rsi
	popq %rdi
	addq $0x8,%rsp
	iretq
.global ret_from_int
ret_from_int:
	jmp RESTORE_ALL

.global divide_error
divide_error:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_divide_error
	jmp ret_from_int
.global debug
debug:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_debug
	jmp ret_from_int
.global nmi
nmi:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_nmi
	jmp ret_from_int
.global int3
int3:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_int3
	jmp ret_from_int
.global overflow
overflow:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_overflow
	jmp ret_from_int
.global bound
bound:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_bound
	jmp ret_from_int
.global invalid_opcode
invalid_opcode:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_invalid_opcode
	jmp ret_from_int
.global dev_not_available
dev_not_available:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_dev_not_available
	jmp ret_from_int
.global double_fault
double_fault:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_double_fault
	jmp ret_from_int
.global coprocessor_segment_overrun
coprocessor_segment_overrun:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_coprocessor_segment_overrun
	jmp ret_from_int
.global invalid_tss
invalid_tss:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_invalid_tss
	jmp ret_from_int
.global segment_not_present
segment_not_present:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_segment_not_present
	jmp ret_from_int
.global stack_fault
stack_fault:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_stack_fault
	jmp ret_from_int
.global general_protection
general_protection:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_general_protection
	jmp ret_from_int
.global page_fault
page_fault:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_page_fault
	jmp ret_from_int
.global x87_fpu_error
x87_fpu_error:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_x87_fpu_error
	jmp ret_from_int
.global alignment_check
alignment_check:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_alignment_check
	jmp ret_from_int
.global machine_check
machine_check:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_machine_check
	jmp ret_from_int
.global simd_exception
simd_exception:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_simd_exception
	jmp ret_from_int
.global virtualization_exception
virtualization_exception:
	cld
	pushq $0
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_virtualization_exception
	jmp ret_from_int
.global control_protection
control_protection:
	cld
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rax
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
    movq %rsp,%rdi
	callq do_control_protection
	jmp ret_from_int
