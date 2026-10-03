.text
.globl _shadow_enter
_shadow_enter:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _shadow_pre
    addl $4, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_shadow_trampoline
.globl _shadow_return
_shadow_return:
    pushl $0
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    call _shadow_post
    movl %eax, 36(%ebx)
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    ret
