# Register envelope for the unchanged 4fd160..4fd1c0 block plus test RET.
.text
.globl rz_classify_block
.type rz_classify_block,@function
rz_classify_block:
 pushl %ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 movl 20(%esp),%eax
 movl 24(%esp),%esi
 xorl %ebx,%ebx
 movl $1,%edi
 movl $-1,%ebp
 call *%eax
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret
.size rz_classify_block,.-rz_classify_block
.section .note.GNU-stack,"",@progbits
