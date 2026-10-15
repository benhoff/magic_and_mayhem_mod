#ifdef _WIN32
#define C(n) _##n
#else
#define C(n) n
#endif
.text
.macro pixel name,kind
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
    call C(mnm_pixel_route)
    addl $16,%esp
    testl %eax,%eax
    jz 1f
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_pixel_resume)+4*\kind
1:  fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_pixel_original)+4*\kind
.endm
pixel C(mnm_pcx_surface_enter),0
pixel C(mnm_rgb_add_enter),1
pixel C(mnm_pcx_indices_enter),2
pixel C(mnm_pcx_raw_enter),3
pixel C(mnm_brighten_enter),4
pixel C(mnm_row_copy_enter),5
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
