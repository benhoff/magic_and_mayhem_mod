.text
# cdecl call(entry,ecx,edx,count,args,calleePopBytes). Verify exact stack
# consumption before restoring the caller frame, including cdecl providers.
.globl actor_services_call
actor_services_call:
    push %ebp
    mov %esp,%ebp
    push %ebx
    push %esi
    push %edi
    mov %esp,%edi
    mov 20(%ebp),%ebx
    mov 24(%ebp),%esi
1:  test %ebx,%ebx
    jz 2f
    dec %ebx
    push (%esi,%ebx,4)
    jmp 1b
2:  mov 12(%ebp),%ecx
    mov 16(%ebp),%edx
    call *8(%ebp)
    mov 20(%ebp),%ecx
    shl $2,%ecx
    sub 28(%ebp),%ecx
    add %ecx,%esp
    cmp %edi,%esp
    jne 3f
    pop %edi
    pop %esi
    pop %ebx
    leave
    ret
3:  ud2
# cdecl cleanup(frame). The cold MSVC finally body reads its parent's EBP.
.globl actor_services_cleanup
actor_services_cleanup:
    push %ebp
    mov 8(%esp),%ebp
    mov $0x59c4ab,%eax
    call *%eax
    pop %ebp
    ret
.section .note.GNU-stack,"",@progbits
