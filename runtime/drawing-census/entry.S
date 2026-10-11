.text
.macro census_entry tag
.globl _census_enter_\tag
_census_enter_\tag:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl $\tag
    pushl %ebx
    call _census_enter_observe
    addl $8, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    jmp *_census_trampolines+4*\tag
.endm
/* Build script appends one macro invocation per signature-pinned hook. */
.globl _census_leave
_census_leave:
    leal -4(%esp), %esp
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %ebx
    call _census_leave_observe
    addl $4, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    ret
