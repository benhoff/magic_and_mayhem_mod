.text
.globl _glyph_shadow_leave
_glyph_shadow_leave:
    /* Original RET16 leaves ESP=entry+20. Save below the dead argument area,
     * preserving all four original rewritten arguments for comparison. */
    leal -36(%esp), %esp
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    movl %esp, %eax
    subl $8, %esp
    pushl %eax
    pushl %ebx
    call _glyph_live_leave
    addl $16, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    leal 16(%esp), %esp
    ret $16
