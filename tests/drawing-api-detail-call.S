.text
.globl _api_detail_test_call
/* address, ECX, EDX, callee-pop bytes, output (16-byte aligned).
   Sixteen explicit stack words or the optional sixth-argument array are fixtures, not arbitrary caller memory. */
_api_detail_test_call:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    subl $64, %esp
    movl $4, 0(%esp)
    movl $5, 4(%esp)
    movl $6, 8(%esp)
    movl $7, 12(%esp)
    movl $8, 16(%esp)
    movl $9, 20(%esp)
    movl $10, 24(%esp)
    movl $11, 28(%esp)
    movl $12, 32(%esp)
    movl $13, 36(%esp)
    movl $14, 40(%esp)
    movl $15, 44(%esp)
    movl $16, 48(%esp)
    movl $17, 52(%esp)
    movl $18, 56(%esp)
    movl $19, 60(%esp)
    movl 28(%ebp), %esi
    testl %esi, %esi
    jz 1f
    movl %esp, %edi
    movl $16, %ecx
    cld
    rep movsl
1:
    movl 12(%ebp), %ecx
    movl 16(%ebp), %edx
    movl $0x12345678, %eax
    movl $0xabcdef01, %ebx
    movl $0xabcdef02, %esi
    movl $0xabcdef03, %edi
    fninit
    fld1
    xorps %xmm0, %xmm0
    cmpeqps %xmm0, %xmm0
    movaps %xmm0, %xmm1
    movaps %xmm0, %xmm2
    movaps %xmm0, %xmm3
    movaps %xmm0, %xmm4
    movaps %xmm0, %xmm5
    movaps %xmm0, %xmm6
    movaps %xmm0, %xmm7
    pushl $0x247
    popfl
    call *8(%ebp)
    pushfl
    pushal
    movl 24(%ebp), %edi
    movl %esp, %esi
    movl $9, %ecx
    cld
    rep movsl
    addl $12, %edi
    fxsave (%edi)
    leal -12(%ebp), %esp
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
