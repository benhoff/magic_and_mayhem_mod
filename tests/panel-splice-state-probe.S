.text
.globl mnm_panel_probe
mnm_panel_probe:
    pushfl
    pushal
    movl %esp,panel_saved_sp
    fxsave panel_host_fx
    fxrstor panel_seed_fx
    movl $0xa1a2a3a4,%eax
    movl $0xb1b2b3b4,%ebx
    movl $0xd1d2d3d4,%edx
    movl $0xe1e2e3e4,%ebp
    movl $0x51525354,%esi
    movl $0x71727374,%edi
    movl panel_object,%ecx
    cmpl $0,panel_kind
    jne 1f
    pushl panel_args+12
    pushl panel_args+8
    pushl panel_args+4
1:  pushl panel_args
    pushl panel_seed_flags
    popfl
    call *panel_function
    movl %eax,panel_gpr
    movl %ecx,panel_gpr+4
    movl %edx,panel_gpr+8
    movl %ebx,panel_gpr+12
    movl %esp,panel_gpr+16
    movl %ebp,panel_gpr+20
    movl %esi,panel_gpr+24
    movl %edi,panel_gpr+28
    pushfl
    popl panel_after_flags
    fxsave panel_after_fx
    cld
    fxrstor panel_host_fx
    movl panel_saved_sp,%esp
    popal
    popfl
    ret
.globl mnm_panel_test_unlock
mnm_panel_test_unlock:
    movl %eax,panel_unlock_gpr
    movl %ecx,panel_unlock_gpr+4
    movl %edx,panel_unlock_gpr+8
    movl 4(%esp),%eax
    movl %eax,panel_unlock_args
    movl 8(%esp),%eax
    movl %eax,panel_unlock_args+4
    pushfl
    incl panel_unlocks
    popfl
    movl $0x9a7be51,%eax
    ret $8
.section .note.GNU-stack,"",@progbits
