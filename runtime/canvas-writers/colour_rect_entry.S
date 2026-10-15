#ifdef _WIN32
#define C(n) _##n
#else
#define C(n) n
#endif
.text
.globl C(mnm_colour_rect_enter)
C(mnm_colour_rect_enter):
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    subl $12,%esp
    pushl %ebx
    call C(mnm_colour_rect_route)
    addl $16,%esp
    testl %eax,%eax
    jz 1f
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_colour_rect_resume)
1:  fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_colour_rect_original)
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
