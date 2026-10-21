.text
#ifdef _WIN32
#define ACTOR_PREFIX _mnm_actor_
#define ACTOR_DISPATCH _mnm_actor_dispatch
#else
#define ACTOR_PREFIX mnm_actor_
#define ACTOR_DISPATCH mnm_actor_dispatch
#endif
.macro actor_entry name,kind,pop
.globl ACTOR_PREFIX\name\()_entry
ACTOR_PREFIX\name\()_entry:
    pushal
    cld
    movl %esp,%eax
    pushl $\kind
    pushl %eax
    call ACTOR_DISPATCH
    addl $8,%esp
    popal
    ret $\pop
.endm
#define MNM_ACTOR_ENTRY(name,kind,pop) actor_entry name,kind,pop
#include "entries.inc"
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
