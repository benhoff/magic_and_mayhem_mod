.text
.globl contract_original_call
contract_original_call:
    pushfl
    pushal
    movl %esp, contract_host_sp
    movl 40(%esp), %eax
    movl %eax, contract_function
    movl 44(%esp), %eax
    movl %eax, contract_self
    movl 48(%esp), %ecx
    movl %ecx, contract_argc
    movl 52(%esp), %edx
    pushl $0x13579bdf
    pushl $0x2468ace0
1:  testl %ecx, %ecx
    jz 2f
    decl %ecx
    pushl (%edx,%ecx,4)
    jmp 1b
2:  movl %esp, contract_call_sp
    movl $0xa1a2a3a4, %eax
    movl contract_self, %ecx
    movl $0xd1d2d3d4, %edx
    movl $0xb1b2b3b4, %ebx
    movl $0xe1e2e3e4, %ebp
    movl $0x51525354, %esi
    movl $0x71727374, %edi
    pushl $0x202
    popfl
    call *contract_function
    movl %eax, contract_gpr
    movl %ecx, contract_gpr+4
    movl %edx, contract_gpr+8
    movl %ebx, contract_gpr+12
    movl %esp, contract_gpr+16
    movl %ebp, contract_gpr+20
    movl %esi, contract_gpr+24
    movl %edi, contract_gpr+28
    /* RET4 leaves the first dead argument directly below ESP. PUSHFL would
     * overwrite it, so preserve the mutated slot using flag-neutral MOVs. */
    movl contract_call_sp, %edx
    movl (%edx), %edx
    movl %edx, contract_first_arg
    pushfl
    popl contract_flags
    cld
    movl contract_call_sp, %eax
    movl contract_argc, %ecx
    leal (%eax,%ecx,4), %eax
    movl (%eax), %edx
    movl %edx, contract_guards
    movl 4(%eax), %edx
    movl %edx, contract_guards+4
    movl contract_host_sp, %esp
    popal
    popfl
    ret
.section .note.GNU-stack,"",@progbits
