.text
.globl _packet_fixture_call
_packet_fixture_call:
    pushl %ebp
    movl %esp,%ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    subl $8,%esp
    movl 12(%ebp),%eax
    movl 16(%ebp),%ebx
    movl 20(%ebp),%edi
    movl $0xabcdef01,%esi
    stc
    call *8(%ebp)
    movl %eax,(%edi)
    pushfl
    popl %eax
    movl %eax,4(%edi)
    movl (%esp),%eax
    movl %eax,8(%edi)
    addl $8,%esp
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
