#ifdef _WIN32
#define C(n) _##n
#else
#define C(n) n
#endif
.text
.macro raster name,kind
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
    call C(mnm_raster_route)
    addl $16,%esp
    testl %eax,%eax
    jz 1f
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_raster_resume)+4*\kind
1:  fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_raster_original)+4*\kind
.endm
raster C(mnm_raster_enter_0),0
raster C(mnm_raster_enter_1),1
raster C(mnm_raster_enter_2),2
raster C(mnm_raster_enter_3),3
raster C(mnm_raster_enter_4),4
raster C(mnm_raster_enter_5),5
raster C(mnm_raster_enter_6),6
raster C(mnm_raster_enter_7),7
raster C(mnm_raster_enter_8),8
raster C(mnm_raster_enter_9),9
raster C(mnm_raster_enter_10),10
raster C(mnm_raster_enter_11),11
raster C(mnm_raster_enter_12),12
raster C(mnm_raster_enter_13),13
raster C(mnm_raster_enter_14),14
raster C(mnm_raster_enter_15),15
raster C(mnm_raster_enter_16),16
raster C(mnm_raster_enter_17),17
raster C(mnm_raster_enter_18),18
raster C(mnm_raster_enter_19),19
raster C(mnm_raster_enter_20),20
raster C(mnm_raster_enter_21),21
raster C(mnm_raster_enter_22),22
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
