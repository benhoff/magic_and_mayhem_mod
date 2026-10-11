.text
/* CALL-site stubs preserve the original ESP. No return or argument is replaced. */
.macro api_stub tag,event,label,target
.globl _api_\label\()_\tag
_api_\label\()_\tag:
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    pushl $\event
    pushl $\tag
    pushl %ebx
    call _api_observe
    addl $12,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    jmp *_api_\target+4*\tag
.endm
.macro api_pair tag
    api_stub \tag,1,before,trampolines
    api_stub \tag,2,after,remainders
.endm
