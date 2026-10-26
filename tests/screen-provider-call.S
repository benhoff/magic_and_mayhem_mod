.text
# Direct invocation of an MSVC cleanup funclet with the supplied parent EBP.
# This observes the cleanup body; it does not emulate Windows SEH dispatch.
.globl screen_provider_cleanup
screen_provider_cleanup:
    push %ebp
    mov 8(%esp),%eax
    mov 12(%esp),%ebp
    call *%eax
    pop %ebp
    ret
.section .note.GNU-stack,"",@progbits
