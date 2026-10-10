#ifdef _WIN32
#define C(name) _##name
#else
#define C(name) name
#endif
.text
.globl C(mnm_glyph_enter)
C(mnm_glyph_enter):
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    movl %esp,%eax
    subl $8,%esp /* align the cdecl call site; C owns an aligned FX image */
    pushl %eax
    pushl %ebx
    call C(mnm_glyph_route)
    addl $16,%esp
    testl %eax,%eax
    jz 1f
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    ret $16
1:  fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_glyph_original)
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
