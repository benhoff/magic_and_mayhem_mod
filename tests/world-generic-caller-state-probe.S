.text
.globl mnm_pixel_probe
mnm_pixel_probe:
    pushfl
    pushal
    movl %esp,pixel_saved_sp
    fxsave pixel_host_fx
    fxrstor pixel_seed_fx
    pushl pixel_args+16
    pushl pixel_args+12
    pushl pixel_args+8
    pushl pixel_args+4
    pushl pixel_args
    movl %esp,generic_call_sp
    movl $0xa1a2a3a4,%eax
    movl $0xc1c2c3c4,%ecx
    movl $0xd1d2d3d4,%edx
    movl $0xb1b2b3b4,%ebx
    movl $0xe1e2e3e4,%ebp
    movl $0x51525354,%esi
    movl $0x71727374,%edi
    pushl pixel_seed_flags
    popfl
    call *pixel_function
    movl %eax,pixel_gpr
    movl %ecx,pixel_gpr+4
    movl %edx,pixel_gpr+8
    movl %ebx,pixel_gpr+12
    movl %esp,pixel_gpr+16
    movl %ebp,pixel_gpr+20
    movl %esi,pixel_gpr+24
    movl %edi,pixel_gpr+28
    pushfl
    popl pixel_after_flags
    fxsave pixel_after_fx
    movl (%esp),%eax
    movl %eax,generic_after_args
    movl 4(%esp),%eax
    movl %eax,generic_after_args+4
    movl 8(%esp),%eax
    movl %eax,generic_after_args+8
    movl 12(%esp),%eax
    movl %eax,generic_after_args+12
    movl 16(%esp),%eax
    movl %eax,generic_after_args+16
    cld
    fxrstor pixel_host_fx
    movl pixel_saved_sp,%esp
    popal
    popfl
    ret
.globl mnm_pixel_test_unlock
mnm_pixel_test_unlock:
    ud2
.section .note.GNU-stack,"",@progbits
