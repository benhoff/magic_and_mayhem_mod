.text
.globl _word_enter_scalar
.globl _word_enter_forward
_word_enter_scalar:
    pushl $0x197086
    jmp word_common
_word_enter_forward:
    pushl $0x196cb8
word_common:
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl %esp /* saved caller FXSAVE image, second cdecl argument */
    pushl %ebx
    call _word_route
    addl $8, %esp
    testl %eax, %eax
    jz word_forward
    /* Admitted positive frames reproduce the selected original FILD/FIMUL/FISTP state.
     * Their exact bounded integer product clears C1 while retaining the
     * masked sticky flags and active stack values. */
    andw $0xfdff, 2(%esp)
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    leal 4(%esp), %esp
    xorl %eax, %eax
    ret
word_forward:
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
    leal 4(%esp), %esp
    jmp *_word_original
