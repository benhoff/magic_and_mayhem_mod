.text
.globl _fake_start
.globl _fake_end
_fake_start:
    subl $0x8c, %esp
    leal 0x8c(%esp), %eax
    pushl 16(%eax)
    pushl 12(%eax)
    pushl 8(%eax)
    pushl 4(%eax)
    pushl %ecx
    movl $_fake_body, %eax
    call *%eax
    addl $20, %esp
    addl $0x8c, %esp
    subl $4, %esp
    movl $0x3f80, (%esp)
    ldmxcsr (%esp)
    addl $4, %esp
    movl $0x76543210, %eax
    movd %eax, %xmm0
    movl $0x12345678, %eax
    stc
    ret $16
_fake_end:
.globl _invoke_checked
_invoke_checked:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl %esp, %ebp
    movl 20(%esp), %eax
    movl 24(%esp), %ecx
    movl $0x12345601, %ebx
    movl $0x12345602, %esi
    movl $0x12345603, %edi
    pushl 40(%esp)
    pushl 40(%esp)
    pushl 40(%esp)
    pushl 40(%esp)
    call *%eax
    jnc 1f
    cmpl %ebp, %esp
    jne 1f
    cmpl $0x12345601, %ebx
    jne 1f
    cmpl $0x12345602, %esi
    jne 1f
    cmpl $0x12345603, %edi
    jne 1f
    jmp 2f
1:
    movl $0xcafffe00, %eax
2:
    movl %ebp, %esp
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
