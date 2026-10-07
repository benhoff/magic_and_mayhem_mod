.text
.globl _scene_enter
_scene_enter:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _scene_observe
    addl $4, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_scene_trampoline
