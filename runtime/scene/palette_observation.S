.text
.macro palette_observe tag
.globl _palette_enter_\tag
_palette_enter_\tag:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    movl %esp, %eax
    subl $4, %esp
    pushl %eax
    pushl $\tag
    pushl %ebx
    call _palette_enter_observe
    addl $16, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_palette_trampolines+4*\tag
.endm
palette_observe 0
palette_observe 1
palette_observe 2
palette_observe 3
palette_observe 4
palette_observe 5
palette_observe 6
palette_observe 7
palette_observe 8
palette_observe 9
palette_observe 10
palette_observe 11
palette_observe 12
palette_observe 13
.globl _palette_leave
_palette_leave:
    leal -4(%esp), %esp
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    movl %esp, %eax
    subl $8, %esp
    pushl %eax
    pushl %ebx
    call _palette_leave_observe
    addl $16, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    ret
