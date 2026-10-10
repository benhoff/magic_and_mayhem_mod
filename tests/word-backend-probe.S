.text
.globl mnm_word_probe
.type mnm_word_probe,@function
mnm_word_probe:
    pushfl
    pushal
    movl %esp, mnm_probe_saved_esp
    fxsave mnm_probe_host_fx
    fxrstor mnm_probe_seed_fx
    movl %esp, %eax
    pushl $0x13579bdf
    pushl $0x2468ace0
    pushl $0x55aa55aa
    pushl $0xaa55aa55
    pushl 52(%eax)
    pushl 48(%eax)
    pushl 44(%eax)
    movl %esp, mnm_probe_call_esp
    movl $0xa1a2a3a4, %eax
    movl $0xc1c2c3c4, %ecx
    movl $0xd1d2d3d4, %edx
    movl $0xb1b2b3b4, %ebx
    movl $0xe1e2e3e4, %ebp
    movl $0x51525354, %esi
    movl $0x71727374, %edi
    pushl mnm_probe_seed_flags
    popfl
    fxsave mnm_probe_before_fx
    call *mnm_probe_function
    /* MOV/FXSAVE preserve the return flags. Capture before clearing DF or
     * returning to the host C++ ABI. No probe instruction touches the original. */
    movl %eax, mnm_probe_gpr
    movl %ecx, mnm_probe_gpr+4
    movl %edx, mnm_probe_gpr+8
    movl %ebx, mnm_probe_gpr+12
    movl %esp, mnm_probe_gpr+16
    movl %ebp, mnm_probe_gpr+20
    movl %esi, mnm_probe_gpr+24
    movl %edi, mnm_probe_gpr+28
    pushfl
    popl mnm_probe_after_flags
    fxsave mnm_probe_after_fx
    cld
    movl mnm_probe_call_esp, %eax
    movl 0(%eax), %edx
    movl %edx, mnm_probe_arguments
    movl 4(%eax), %edx
    movl %edx, mnm_probe_arguments+4
    movl 8(%eax), %edx
    movl %edx, mnm_probe_arguments+8
    movl 12(%eax), %edx
    movl %edx, mnm_probe_guards
    movl 16(%eax), %edx
    movl %edx, mnm_probe_guards+4
    movl 20(%eax), %edx
    movl %edx, mnm_probe_guards+8
    movl 24(%eax), %edx
    movl %edx, mnm_probe_guards+12
    fxrstor mnm_probe_host_fx
    movl mnm_probe_saved_esp, %esp
    popal
    popfl
    ret
.size mnm_word_probe,.-mnm_word_probe
.section .note.GNU-stack,"",@progbits
