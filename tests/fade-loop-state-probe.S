.text
.globl mnm_fade_probe
mnm_fade_probe:
    pushfl
    pushal
    movl %esp,mnm_fade_saved_sp
    fxsave mnm_fade_host_fx
    fxrstor mnm_fade_seed_fx
    movl 40(%esp),%ecx
    movl $0xa1a2a3a4,%eax
    movl $0xb1b2b3b4,%ebx
    movl $0xd1d2d3d4,%edx
    movl $0xe1e2e3e4,%ebp
    movl $0x51525354,%esi
    movl $0x71727374,%edi
    pushl mnm_fade_seed_flags
    popfl
    call *mnm_fade_function
    movl %eax,mnm_fade_gpr
    movl %ecx,mnm_fade_gpr+4
    movl %edx,mnm_fade_gpr+8
    movl %ebx,mnm_fade_gpr+12
    movl %esp,mnm_fade_gpr+16
    movl %ebp,mnm_fade_gpr+20
    movl %esi,mnm_fade_gpr+24
    movl %edi,mnm_fade_gpr+28
    pushfl
    popl mnm_fade_after_flags
    fxsave mnm_fade_after_fx
    cld
    fxrstor mnm_fade_host_fx
    movl mnm_fade_saved_sp,%esp
    popal
    popfl
    ret
.globl mnm_fade_test_unlock
mnm_fade_test_unlock:
    pushfl
    popl mnm_fade_unlock_flags
    movl %eax,mnm_fade_unlock_gpr
    movl %ecx,mnm_fade_unlock_gpr+4
    movl %edx,mnm_fade_unlock_gpr+8
    movl 4(%esp),%eax
    movl %eax,mnm_fade_unlock_args
    movl 8(%esp),%eax
    movl %eax,mnm_fade_unlock_args+4
    incl mnm_fade_unlocks
    movl $0x9a7be51,%eax
    ret $8
.globl mnm_fade_test_original
mnm_fade_test_original:
    pushfl
    incl mnm_fade_original_calls
    popfl
    jmp *mnm_fade_trampoline
.section .note.GNU-stack,"",@progbits
