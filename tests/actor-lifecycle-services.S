.text
.macro actor_service name,argc
.globl actor_service_\name
actor_service_\name:
    pushal
    cld
    movl %esp,%eax
    pushl $\argc
    pushl $0x\name
    pushl %eax
    call actor_service_dispatch
    addl $12,%esp
    movl %eax,28(%esp)
    popal
    ret $(4*\argc)
.endm
#define ACTOR_SERVICE(address,argc) actor_service address,argc
#include "actor-lifecycle-services.inc"
.section .note.GNU-stack,"",@progbits
