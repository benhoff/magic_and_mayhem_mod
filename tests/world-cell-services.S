.text
.macro cell_service name,argc
.globl cell_service_\name
cell_service_\name:
    pushal
    cld
    movl %esp,%eax
    pushl $\argc
    pushl $0x\name
    pushl %eax
    call cell_service_dispatch
    addl $12,%esp
    movl %eax,28(%esp)
    popal
    ret $(4*\argc)
.endm
#define CELL_SERVICE(address,argc) cell_service address,argc
#include "world-cell-services.inc"
#undef CELL_SERVICE
cell_service 57dcf0,2
cell_service 57ddb0,2
cell_service 4eadf0,2
.section .note.GNU-stack,"",@progbits
