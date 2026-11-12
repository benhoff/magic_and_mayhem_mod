.text
.globl boundary_probe
.type boundary_probe,@function
boundary_probe:
    pushfl
    pushal
    movl %esp,%ebp
    fninit
    fld1
    pcmpeqd %xmm0,%xmm0
    fxsave boundary_expected_fx
    movl 44(%ebp),%eax
    movl $0x40404040,%ecx
    movl $0x42424242,%edx
    movl $0x51515151,%ebx
    movl $0x61616161,%esi
    movl $0x71717171,%edi
    cmpl $0,48(%ebp)
    je 1f
    pushl $0x34343434
1:  pushl $0x602
    popfl
    call *%eax
    pushfl
    pushal
    cld
    fxsave boundary_actual_fx
    movl 40(%ebp),%edi
    movl %esp,%esi
    movl $9,%ecx
    rep movsl
    movl %ebp,%esp
    popal
    popfl
    ret
.size boundary_probe,.-boundary_probe
.globl boundary_original_0
boundary_original_0:
    movl $0x89898989,%eax
    ret
.globl boundary_original_4
boundary_original_4:
    movl $0x89898989,%eax
    ret $4
.section .note.GNU-stack,"",@progbits
