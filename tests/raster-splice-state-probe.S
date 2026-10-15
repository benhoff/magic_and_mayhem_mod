.text
.globl mnm_pixel_probe
mnm_pixel_probe:
    pushfl
    pushal
    movl %esp,pixel_saved_sp
    fxsave pixel_host_fx
    fxrstor pixel_seed_fx
    cmpl $0,pixel_seed_stack
    je 4f
    movl $-128,%ecx
5:  movl $0,-16(%esp,%ecx)
    addl $4,%ecx
    jne 5b
4:
    movl $0xa1a2a3a4,%eax
    movl $0xb1b2b3b4,%ebx
    movl raster_x,%edx
    movl $0xe1e2e3e4,%ebp
    movl $0x51525354,%esi
    movl $0x71727374,%edi
    movl pixel_object,%ecx
    cmpl $5,pixel_argc
    jb 6f
    pushl pixel_args+16
6:
    cmpl $4,pixel_argc
    jb 0f
    pushl pixel_args+12
0:
    cmpl $3,pixel_argc
    jb 1f
    pushl pixel_args+8
1:  cmpl $2,pixel_argc
    jb 2f
    pushl pixel_args+4
2:  cmpl $1,pixel_argc
    jb 3f
    pushl pixel_args
3:  pushl pixel_seed_flags
    popfl
    call *pixel_function
    pushfl
    popl pixel_after_flags
    cmpl $0,raster_cdecl
    je 7f
    pushl %eax
    movl pixel_argc,%eax
    leal 4(%esp,%eax,4),%eax
    movl %eax,pixel_gpr+16
    popl %eax
    movl pixel_gpr+16,%esp
7:
    movl %eax,pixel_gpr
    movl %ecx,pixel_gpr+4
    movl %edx,pixel_gpr+8
    movl %ebx,pixel_gpr+12
    movl %esp,pixel_gpr+16
    movl %ebp,pixel_gpr+20
    movl %esi,pixel_gpr+24
    movl %edi,pixel_gpr+28
    fxsave pixel_after_fx
    cld
    fxrstor pixel_host_fx
    movl pixel_saved_sp,%esp
    popal
    popfl
    ret
.globl mnm_pixel_test_unlock
mnm_pixel_test_unlock:
    movl %eax,pixel_unlock_gpr
    movl %ecx,pixel_unlock_gpr+4
    movl %edx,pixel_unlock_gpr+8
    movl 4(%esp),%eax
    movl %eax,pixel_unlock_args
    movl 8(%esp),%eax
    movl %eax,pixel_unlock_args+4
    pushfl
    incl pixel_unlocks
    popfl
    movl $0x9a7be51,%eax
    ret $8
.section .note.GNU-stack,"",@progbits
