#ifdef _WIN32
#define C(n) _##n
#else
#define C(n) n
#endif
.text
.macro map name,kind
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
    call C(mnm_map_route)
    addl $16,%esp
    testl %eax,%eax
    jz 1f
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
.if \kind == 0
    /* The original post-Lock splice precedes these three saved registers.
     * Its unchanged Unlock tail expects the matching stack and locals. */
    pushl %ebx
    pushl %ebp
    pushl %edi
.elseif \kind == 1
    /* Both original cell paths pop these before completing their WORD stores.
     * The native kernel completed every store; retain the original RET16. */
    popl %edi
    popl %esi
    popl %ebp
    popl %ebx
.endif
    jmp *C(mnm_map_resume)+4*\kind
1:  fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(mnm_map_original)+4*\kind
.endm
map C(mnm_map_terrain_enter),0
map C(mnm_map_cell_enter),1
map C(mnm_map_creature_enter),2
map C(mnm_map_camera0_enter),3
map C(mnm_map_camera1_enter),4
map C(mnm_map_camera2_enter),5
map C(mnm_map_camera3_enter),6
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
