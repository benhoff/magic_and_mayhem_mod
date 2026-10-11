.text
.globl mnm_glyph_probe
.type mnm_glyph_probe,@function
mnm_glyph_probe:
    pushfl
    pushal
    movl %esp, mnm_glyph_saved_sp
    fxsave mnm_glyph_host_fx
    fxrstor mnm_glyph_seed_fx
    movl %esp, %eax
    pushl $0x13579bdf
    pushl $0x2468ace0
    pushl $0x55aa55aa
    pushl $0xaa55aa55
    pushl 64(%eax)
    pushl 60(%eax)
    pushl 56(%eax)
    pushl 52(%eax)
    movl %esp, mnm_glyph_call_sp
    movl 44(%eax), %ecx
    movl 48(%eax), %edx
    movl $0xa1a2a3a4, %eax
    movl $0xb1b2b3b4, %ebx
    movl $0xe1e2e3e4, %ebp
    movl $0x51525354, %esi
    movl $0x71727374, %edi
    pushl mnm_glyph_seed_flags
    popfl
    fxsave mnm_glyph_before_fx
    call *mnm_glyph_function
    movl %eax, mnm_glyph_gpr
    movl %ecx, mnm_glyph_gpr+4
    movl %edx, mnm_glyph_gpr+8
    movl %ebx, mnm_glyph_gpr+12
    movl %esp, mnm_glyph_gpr+16
    movl %ebp, mnm_glyph_gpr+20
    movl %esi, mnm_glyph_gpr+24
    movl %edi, mnm_glyph_gpr+28
    /* RET16 has advanced ESP beyond arguments: capture flags below that
     * area so PUSHFL cannot overwrite the original blue argument slot. */
    leal -32(%esp), %esp
    pushfl
    popl mnm_glyph_after_flags
    leal 32(%esp), %esp
    fxsave mnm_glyph_after_fx
    cld
    movl mnm_glyph_call_sp, %eax
    movl 0(%eax), %edx
    movl %edx, mnm_glyph_arguments
    movl 4(%eax), %edx
    movl %edx, mnm_glyph_arguments+4
    movl 8(%eax), %edx
    movl %edx, mnm_glyph_arguments+8
    movl 12(%eax), %edx
    movl %edx, mnm_glyph_arguments+12
    movl 16(%eax), %edx
    movl %edx, mnm_glyph_guards
    movl 20(%eax), %edx
    movl %edx, mnm_glyph_guards+4
    movl 24(%eax), %edx
    movl %edx, mnm_glyph_guards+8
    movl 28(%eax), %edx
    movl %edx, mnm_glyph_guards+12
    fxrstor mnm_glyph_host_fx
    movl mnm_glyph_saved_sp, %esp
    popal
    popfl
    ret
.size mnm_glyph_probe,.-mnm_glyph_probe
.section .note.GNU-stack,"",@progbits
