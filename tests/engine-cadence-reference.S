.text
.globl _fixture_original
_fixture_original:
    call _fixture_error
    xorl %eax,%eax
    stc
    movl $0x13579bdf,%eax
    movl $0x2468abcd,%ecx
    movl $0x98765432,%edx
    ret
.globl _fixture_sequence
_fixture_sequence:
    movl %edi,0xfd62(%esi)
    ret
.globl _fixture_tick
_fixture_tick:
    movl %eax,0x6c4830
    ret
.globl _fixture_invoke
_fixture_invoke:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl 20(%esp),%eax
    movl 24(%esp),%edx
    pushl %edx
    pushl %eax
    fninit
    fld1
    movl $0x6deffc,%ecx
    movl $0x6cbb78,%esi
    movl $2,%edi
    movl $0x1234,%edx
    movl $0x4567,%ebx
    movl $0x789a,%ebp
    xorl %eax,%eax
    stc
    movl $5,%eax
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
    addl $36,%esp
    addl $8,%esp
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
