.text
.macro primitive tag
.globl _world_primitive_\tag
_world_primitive_\tag:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl $\tag
    pushl %ebx
    call _world_observe
    addl $8, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_world_trampolines+4*\tag
.endm
primitive 0
primitive 1
primitive 2
primitive 3
primitive 4
primitive 5
primitive 6
primitive 7
primitive 8
primitive 9
primitive 10
.globl _world_leave
_world_leave:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _world_complete
    addl $4, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_world_return
