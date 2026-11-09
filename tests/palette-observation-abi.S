.text
.globl palette_abi_call
.type palette_abi_call,@function
/* out/address/count/slot/source. Save the C caller, run actual production hook
 * assembly with DF set and caller register patterns, then copy the raw result.
 * EBP records the expected pre-argument stack; output ESP exposes RET cleanup. */
palette_abi_call:
    pushfl
    pushal
    movl %esp,%ebp
    movl 44(%ebp),%eax
    movl 48(%ebp),%ecx
    movl 52(%ebp),%edx
    movl $0x51515151,%ebx
    movl $0x61616161,%esi
    movl $0x71717171,%edi
    pushl 56(%ebp)
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
.size palette_abi_call,.-palette_abi_call
.globl palette_abi_profile_call
.type palette_abi_profile_call,@function
palette_abi_profile_call:
    pushfl
    pushal
    movl %esp,%ebp
    movl 44(%ebp),%eax
    movl $4,%ecx
    movl $0x42424242,%edx
    movl $0x51515151,%ebx
    movl $0x61616161,%esi
    movl $0x71717171,%edi
    pushl 68(%ebp)
    pushl 64(%ebp)
    pushl 60(%ebp)
    pushl 56(%ebp)
    pushl 52(%ebp)
    pushl 48(%ebp)
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
.size palette_abi_profile_call,.-palette_abi_profile_call
.globl palette_abi_refresh_call
.type palette_abi_refresh_call,@function
/* out/address/node/seven-argument-array; real fastcall RET28 production hook. */
palette_abi_refresh_call:
    pushfl
    pushal
    movl %esp,%ebp
    movl 44(%ebp),%eax
    movl 48(%ebp),%edx
    movl 52(%ebp),%esi
    pushl 24(%esi)
    pushl 20(%esi)
    pushl 16(%esi)
    pushl 12(%esi)
    pushl 8(%esi)
    pushl 4(%esi)
    pushl 0(%esi)
    movl $4,%ecx
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
.size palette_abi_refresh_call,.-palette_abi_refresh_call
.section .note.GNU-stack,"",@progbits
