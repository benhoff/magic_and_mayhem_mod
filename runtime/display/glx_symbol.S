.text
.globl dlsym
.type dlsym,@function
.hidden mnm_delivery_symbol_target
dlsym:
    pushq %rdi
    pushq %rsi
    subq $8,%rsp
    call mnm_delivery_symbol_target
    addq $8,%rsp
    popq %rsi
    popq %rdi
    /* Unwrapped and RTLD_NEXT calls keep the real original caller address. */
    jmp *%rax
.size dlsym,.-dlsym
.section .note.GNU-stack,"",@progbits
