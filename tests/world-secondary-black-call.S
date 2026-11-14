.text
.globl _secondary_black_call
/* cdecl(frame,x,y,node,probe). The original fragment pushes its own three/five
 * raster arguments and stores AX. A private RET at500d05 ends the fixture. */
_secondary_black_call:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl 20(%esp),%edi
    movl 24(%esp),%ebp
    movl 28(%esp),%ebx
    movl 32(%esp),%esi
    movl 36(%esp),%edx
    movl %esp,20(%edx)
    pushl $1f
    movl $0x500a0e,%eax
    jmp *%eax
1:
    movl 36(%esp),%edx
    movl %eax,0(%edx)
    movl %ebx,4(%edx)
    movl %esi,8(%edx)
    movl %edi,12(%edx)
    movl %ebp,16(%edx)
    movl %esp,%eax
    subl 20(%edx),%eax
    movl %eax,20(%edx)
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
