#ifdef _WIN32
#define C(n) _##n
#else
#define C(n) n
#endif
.text
.macro panel name,kind
.globl \name
\name:
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    subl $8,%esp
    pushl $\kind
    pushl %ebx
    call C(mnm_panel_route)
    addl $16,%esp
    testl %eax,%eax
    jz 1f
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_panel_resume)+4*\kind
1:  fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_panel_original)+4*\kind
.endm
panel C(mnm_panel_bevel_enter),0
panel C(mnm_panel_halo_enter),1
panel C(mnm_panel_wash_enter),2
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
