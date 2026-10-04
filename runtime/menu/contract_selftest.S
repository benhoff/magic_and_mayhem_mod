.text
.globl _invoke_action
_invoke_action:
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %ebp
    movl 20(%esp),%eax
    movl 24(%esp),%ecx
    movl 28(%esp),%edx
    movl $0x11223344,%ebx
    movl $0x55667788,%esi
    movl $0x99aabbcc,%edi
    movl $0xddeeff00,%ebp
    movl %esp,_saved_esp
    pushl %edx
    call *%eax
    movl %eax,_callback_result
    cmpl _saved_esp,%esp
    jne bad
    cmpl $0x11223344,%ebx
    jne bad
    cmpl $0x55667788,%esi
    jne bad
    cmpl $0x99aabbcc,%edi
    jne bad
    cmpl $0xddeeff00,%ebp
    jne bad
    jmp done
bad:
    movl $1,_abi_failure
done:
    movl _saved_esp,%esp
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx
    movl _callback_result,%eax
    ret
