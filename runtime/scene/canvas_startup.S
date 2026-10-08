.text
/* Preserve the engine ABI and floating-point state at entry and return. */
.macro observe tag
.globl _canvas_enter_\tag
_canvas_enter_\tag:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl $\tag
    pushl %ebx
    call _canvas_enter_observe
    addl $8, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_canvas_trampolines+4*\tag
.endm
observe 0
observe 1
observe 2
observe 3
observe 4
observe 5
observe 6
observe 7
observe 8
observe 9
observe 10
observe 11
.globl _canvas_leave
_canvas_leave:
    leal -4(%esp), %esp
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _canvas_leave_observe
    addl $4, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    ret
