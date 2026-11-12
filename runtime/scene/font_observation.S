.text
.macro font_observe tag
.globl _font_enter_\tag
_font_enter_\tag:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    movl %esp, %eax
    subl $8, %esp
    pushl $\tag
    pushl %ebx
    call _font_enter_observe
    addl $16, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_font_trampolines+4*\tag
.endm
font_observe 0
font_observe 1
font_observe 2
font_observe 3
font_observe 4
.globl _font_leave
_font_leave:
    leal -4(%esp), %esp
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    movl %esp, %eax
    subl $12, %esp
    pushl %ebx
    call _font_leave_observe
    addl $16, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    ret
