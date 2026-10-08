.text
.globl _world_bypass_test_call
/* address, ECX, EDX, callee-pop bytes, output (16-byte aligned).
   The six stack words are deliberate fixtures, not arbitrary caller memory. */
_world_bypass_test_call:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    subl $24, %esp
    movl 28(%ebp), %eax
    movl %eax, (%esp)
    movl $5, 4(%esp)
    movl $6, 8(%esp)
    movl $7, 12(%esp)
    movl $8, 16(%esp)
    movl $9, 20(%esp)
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
