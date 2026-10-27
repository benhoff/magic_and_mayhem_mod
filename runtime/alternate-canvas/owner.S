.text
/* Preserve integer/control, last-error (in C), and FPU/SSE state. */
.macro entry tag
.globl _alternate_owner_enter_\tag
_alternate_owner_enter_\tag:
 pushfl
 pushal
 movl %esp,%ebx
 subl $527,%esp
 andl $-16,%esp
 fxsave (%esp)
 cld
 pushl $\tag
 pushl %ebx
 call _alternate_owner_enter
 addl $8,%esp
 fxrstor (%esp)
 movl %ebx,%esp
 popal
 popfl
 jmp *_alternate_owner_trampolines+4*\tag
.endm
entry 0
entry 1
entry 2
entry 3
entry 4
entry 5
.globl _alternate_owner_leave
_alternate_owner_leave:
 leal -4(%esp),%esp
 pushfl
 pushal
 movl %esp,%ebx
 subl $527,%esp
 andl $-16,%esp
 fxsave (%esp)
 cld
 pushl %ebx
 call _alternate_owner_return
 addl $4,%esp
 fxrstor (%esp)
 movl %ebx,%esp
 popal
 popfl
 ret
