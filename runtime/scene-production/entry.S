.text
.macro save_call name arguments
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    .if \arguments == 2
    pushl %esp
    .endif
    .if \arguments != 0
    pushl %ebx
    .endif
    call _\name
    addl $(4*\arguments),%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
.endm
.globl _production_enter
_production_enter:
    save_call production_before 1
    jmp *_production_trampoline
.globl _production_leave
_production_leave:
    pushl $0
    save_call production_after 2
    ret
.globl _production_queue
_production_queue:
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _production_queue_seen
    addl $4,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *_queue_target
.globl _production_queue_end_first
_production_queue_end_first:
    save_call production_queue_done 0
    movl $0x6a49e0,%ecx
    jmp *_queue_resume_first
.globl _production_queue_end_second
_production_queue_end_second:
    save_call production_queue_done 0
    movl 0x6c482c,%eax
    jmp *_queue_resume_second
.globl _production_repeat_full
.globl _production_history
_production_history:
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _production_history_gate
    addl $4,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    cmpl $0,12(%esp)
    jne history_skip
    popal
    popfl
    jmp *_history_target
history_skip:
    popal
    popfl
    ret $8
_production_repeat_full:
    movl 8(%esp),%eax
    fxrstor (%eax)
    movl 4(%esp),%ecx
    jmp *_production_trampoline
.globl _production_repeat_slice
_production_repeat_slice:
    movl 8(%esp),%eax
    fxrstor (%eax)
    movl 4(%esp),%ecx
    pushl %ebx
    pushl %ebp
    pushl %esi
    movl %ecx,%esi
    xorl %ebx,%ebx
    pushl %edi
    movl $0,0x1e5(%esi)
    movl %esp,_production_slice_stack
    jmp *_slice_start
.globl _production_draw_end
_production_draw_end:
    pushfl
    pushl %eax
    leal 8(%esp),%eax
    cmpl %eax,_production_slice_stack
    popl %eax
    jne normal_end
    popfl
    popl %edi
    popl %esi
    popl %ebp
    popl %ebx
    ret
normal_end:
    popfl
    save_call production_middle 2
    movb 0x5e140c,%al
    testb %al,%al
    jmp *_draw_resume
.data
_queue_target: .long 0x5002a0
_queue_resume_first: .long 0x4fd4b5
_queue_resume_second: .long 0x4fd55e
_slice_start: .long 0x4fd315
_draw_resume: .long 0x4fd5c8
_history_target: .long 0x51fa60
