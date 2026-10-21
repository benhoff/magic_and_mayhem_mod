.text
.globl creature_queue_call
.type creature_queue_call,@function
creature_queue_call:
    pushl %ebx
    pushl %esi
    movl 12(%esp),%esi
    movl $0xa5a5a5a5,%ebx
    call *16(%esp)
    # Verify the exact queue span's register and meaningful flag outputs.
    pushfl
    popl %eax
    andl $0x8c5,%eax
    cmpl $0x44,%eax
    jne 1f
    testl %ebx,%ebx
    jne 1f
    cmpl 0x229(%esi),%edx
    jne 1f
    movl $1,%eax
    jmp 2f
1:  xorl %eax,%eax
2:  popl %esi
    popl %ebx
    ret
.section .note.GNU-stack,"",@progbits
