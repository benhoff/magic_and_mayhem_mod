#ifdef _WIN32
#define C(name) _##name
#else
#define C(name) name
#endif
.text
.macro boundary_entry tag
.globl C(boundary_enter_)\tag
C(boundary_enter_)\tag:
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    subl $8,%esp
    pushl $\tag
    pushl %ebx
    call C(boundary_trace_enter)
    addl $16,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(boundary_trampolines)+4*\tag
.endm
boundary_entry 0
boundary_entry 1
boundary_entry 2
boundary_entry 3
boundary_entry 4
boundary_entry 5
boundary_entry 6
boundary_entry 7
boundary_entry 8
boundary_entry 9
boundary_entry 10
boundary_entry 11
boundary_entry 12
boundary_entry 13
boundary_entry 14
boundary_entry 15
boundary_entry 16
boundary_entry 17
boundary_entry 18
boundary_entry 19
boundary_entry 20
boundary_entry 21
boundary_entry 22
.globl C(boundary_trace_leave)
C(boundary_trace_leave):
    leal -4(%esp),%esp
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    subl $12,%esp
    pushl %ebx
    call C(boundary_trace_return)
    addl $16,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    ret
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
