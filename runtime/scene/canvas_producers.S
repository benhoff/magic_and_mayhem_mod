.text
/* Preserve the engine ABI and floating-point state at entry and return. */
.macro observe tag
.globl _producer_enter_\tag
_producer_enter_\tag:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl $\tag
    pushl %ebx
    call _producer_enter_observe
    addl $8, %esp
    testl %eax, %eax
    jnz 1f
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_producer_trampolines+4*\tag
1:
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
.if \tag == 11 || \tag == 18 || \tag == 19 || \tag == 20
    ret $12
.elseif \tag == 21
    ret $8
.elseif \tag == 25
    ret $4
.else
    ret
.endif
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
observe 12
observe 13
observe 14
observe 15
observe 16
observe 17
observe 18
observe 19
observe 20
observe 21
observe 22
observe 23
observe 24
observe 25
observe 26
observe 27
observe 28
observe 29
observe 30
observe 31
observe 32
observe 33
observe 34
observe 35
observe 36
observe 37
observe 38
observe 39
observe 40
observe 41
observe 42
observe 43
observe 44
observe 45
observe 46
observe 47
observe 48
observe 49
observe 50
observe 51
.globl _producer_leave
_producer_leave:
    leal -4(%esp), %esp
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _producer_leave_observe
    addl $4, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    ret
