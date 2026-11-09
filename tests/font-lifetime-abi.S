.text
.globl font_load_probe
.type font_load_probe,@function
/* out/address/count/slot/source. Save the C caller, run actual production hook
 * assembly with DF set and caller register patterns, then copy the raw result.
 * EBP records the expected pre-argument stack; output ESP exposes RET cleanup. */
font_load_probe:
    pushfl
    pushal
    movl %esp,%ebp
    movl 44(%ebp),%eax
    movl 48(%ebp),%ecx
    movl $0x42424242,%edx
    movl $0x51515151,%ebx
    movl $0x61616161,%esi
    movl $0x71717171,%edi
    pushl 56(%ebp)
    pushl 52(%ebp)
    std
    call *%eax
    pushfl
    pushal
    cld
    movl 40(%ebp),%edi
    movl %esp,%esi
    movl $9,%ecx
    rep movsl
    movl %ebp,%esp
    popal
    popfl
    ret
.size font_load_probe,.-font_load_probe

.globl font_destroy_probe
.type font_destroy_probe,@function
font_destroy_probe:
    pushfl
    pushal
    movl %esp,%ebp
    movl 44(%ebp),%eax
    movl $0x40404040,%ecx
    movl $0x42424242,%edx
    movl $0x51515151,%ebx
    movl $0x61616161,%esi
    movl $0x71717171,%edi
    std
    call *%eax
    pushfl
    pushal
    cld
    movl 40(%ebp),%edi
    movl %esp,%esi
    movl $9,%ecx
    rep movsl
    movl %ebp,%esp
    popal
    popfl
    ret
.size font_destroy_probe,.-font_destroy_probe
.section .note.GNU-stack,"",@progbits
