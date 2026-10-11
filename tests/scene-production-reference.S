.text
.globl _fixture_draw
_fixture_draw:
    call _fixture_work
    movl $0x689b59,%ecx
    call *_queue_call
    jmp *_draw_end
.globl _fixture_invoke
_fixture_invoke:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl 20(%esp),%eax
    movl 24(%esp),%ecx
    movl 28(%esp),%edx
    pushl %edx
    pushl %eax
    leal -4(%esp),%eax
    movl 4(%esp),%edx
    movl %eax,36(%edx)
    fninit
    fld1
    ldmxcsr _mxcsr_seed
    pxor %xmm0,%xmm0
    pxor %xmm1,%xmm1
    pxor %xmm2,%xmm2
    pxor %xmm3,%xmm3
    pxor %xmm4,%xmm4
    pxor %xmm5,%xmm5
    pxor %xmm6,%xmm6
    pxor %xmm7,%xmm7
    movl $0x12345678,%esi
    movl $2,%edi
    movl $0x1234,%edx
    movl $0x4567,%ebx
    movl $0x789a,%ebp
    xorl %eax,%eax
    stc
    call *(%esp)
    pushfl
    pushal
    movl 40(%esp),%eax
    fxsave 48(%eax)
    movl %esp,%esi
    movl %eax,%edi
    movl $9,%ecx
    cld
    rep movsl
    addl $44,%esp
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
.data
_queue_call: .long 0x4fd554
_draw_end: .long 0x4fd5ad
_mxcsr_seed: .long 0x1f80
