#ifdef _WIN32
#define C(name) _##name
#else
#define C(name) name
#endif
.text
.macro font_entry tag
.globl C(font_trace_enter_)\tag
C(font_trace_enter_)\tag:
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
    call C(font_trace_enter)
    addl $16,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *C(font_trace_trampolines)+4*\tag
.endm
font_entry 0
font_entry 1
font_entry 2
font_entry 3
font_entry 4
font_entry 5
font_entry 6
font_entry 7
font_entry 8
font_entry 9
font_entry 10
.globl C(font_trace_leave)
C(font_trace_leave):
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
    call C(font_trace_return)
    addl $16,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    ret
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
