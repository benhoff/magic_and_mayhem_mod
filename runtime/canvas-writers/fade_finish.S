#ifdef _WIN32
#define C(n) _##n
#else
#define C(n) n
#endif
.text
.globl C(mnm_fade_finish_entry)
C(mnm_fade_finish_entry):
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    call C(mnm_fade_finish)
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_fade_tail)
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
