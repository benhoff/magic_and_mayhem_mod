.text
/* These are complete guarded call instructions, not copied relative branches.
 * Preserve EAX/HRESULT, EFLAGS, LastError and the original floating-point state. */
.macro record kind
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
    call _drawing_publication_observe
    addl $16,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
.endm
.globl _drawing_publication_blt
_drawing_publication_blt:
    call *20(%edx)
    record 1
    pushl $0x58c05d
    ret
.globl _drawing_publication_fast
_drawing_publication_fast:
    call *28(%edx)
    record 2
    movl %eax,%esi
    pushl $0x58c05f
    ret
