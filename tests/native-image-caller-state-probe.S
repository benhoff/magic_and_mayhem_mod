#define C(n) _##n
.text
.globl C(mnm_image_state_call)
C(mnm_image_state_call):
    pushfl
    pushal
    movl %esp,C(image_state_saved_sp)
    fxsave C(image_state_host_fx)
    fxrstor C(image_state_seed_fx)
    subl C(image_state_stack_padding),%esp
    pushl C(image_state_args)+20
    pushl C(image_state_args)+16
    pushl C(image_state_args)+12
    pushl C(image_state_args)+8
    pushl C(image_state_args)+4
    pushl C(image_state_args)
    movl $0xa1a2a3a4,%eax
    movl $0xb1b2b3b4,%ebx
    movl $0xd1d2d3d4,%edx
    movl $0xe1e2e3e4,%ebp
    movl $0x51525354,%esi
    movl $0x71727374,%edi
    movl C(image_state_object),%ecx
    pushl C(image_state_seed_flags)
    popfl
    call *C(image_state_function)
    movl %eax,C(image_state_after)
    movl %ecx,C(image_state_after)+4
    movl %edx,C(image_state_after)+8
    movl %ebx,C(image_state_after)+12
    movl %esp,C(image_state_after)+16
    movl %ebp,C(image_state_after)+20
    movl %esi,C(image_state_after)+24
    movl %edi,C(image_state_after)+28
    /* Save all dead RET24 argument slots before PUSHFL can overwrite one. */
    movl -24(%esp),%eax
    movl %eax,C(image_state_after)+36
    movl -20(%esp),%eax
    movl %eax,C(image_state_after)+40
    movl -16(%esp),%eax
    movl %eax,C(image_state_after)+44
    movl -12(%esp),%eax
    movl %eax,C(image_state_after)+48
    movl -8(%esp),%eax
    movl %eax,C(image_state_after)+52
    movl -4(%esp),%eax
    movl %eax,C(image_state_after)+56
    pushfl
    popl C(image_state_after)+32
    fxsave C(image_state_after_fx)
    cld
    fxrstor C(image_state_host_fx)
    movl C(image_state_saved_sp),%esp
    popal
    popfl
    ret
