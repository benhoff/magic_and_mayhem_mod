.text
.globl mnm_host_original_scalar
.globl mnm_host_original_forward
mnm_host_original_scalar:
    incl mnm_host_original_calls
    jmp 0x597086
mnm_host_original_forward:
    incl mnm_host_original_calls
    jmp 0x596cb8
.section .note.GNU-stack,"",@progbits
