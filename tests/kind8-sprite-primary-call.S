.text
.globl _kind8_sprite_primary_call
/* cdecl(row). Execute the original primary fragment and its virtual RET8.
 * The private fixture exits immediately after the original AX metadata store. */
_kind8_sprite_primary_call:
    pushl %ebp
    movl %esp,%ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl 8(%ebp),%esi
    pushl $1f
    movl $0x5006a2,%eax
    jmp *%eax
1:
    popl %edi
    popl %esi
    popl %ebx
    leave
    ret
