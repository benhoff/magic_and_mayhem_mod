.text
#ifdef _WIN32
#define MNM_CREATURE_PREFIX _mnm_creature_
#define MNM_CREATURE_DISPATCH _mnm_creature_draw_dispatch
#define MNM_CREATURE_RETURN _mnm_creature_queue_return
#else
#define MNM_CREATURE_PREFIX mnm_creature_
#define MNM_CREATURE_DISPATCH mnm_creature_draw_dispatch
#define MNM_CREATURE_RETURN mnm_creature_queue_return
#endif
.macro creature_entry name,kind
.globl MNM_CREATURE_PREFIX\name\()_entry
MNM_CREATURE_PREFIX\name\()_entry:
    pushfl
    pushal
    cld
    movl %esp,%eax
    pushl $\kind
    pushl %eax
    call MNM_CREATURE_DISPATCH
    addl $8,%esp
    popal
    popfl
.endm
creature_entry prepare,0
    ret
creature_entry history,1
    ret $8
creature_entry finish,2
    ret
creature_entry queue,3
    # Reproduce the original span's register/flag postconditions, after native
    # count/cursor retirement. These are native adapter instructions.
    xorl %ebx,%ebx
    movl 0x229(%esi),%edx
    jmp *MNM_CREATURE_RETURN
#ifdef _WIN32
.section .rdata,"dr"
#else
.section .rodata
#endif
MNM_CREATURE_RETURN: .long 0x4fd5c1
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
