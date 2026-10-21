.text
#ifdef _WIN32
#define CELL_ENTRY _mnm_world_cell_entry
#define CELL_DISPATCH _mnm_world_cell_dispatch
#else
#define CELL_ENTRY mnm_world_cell_entry
#define CELL_DISPATCH mnm_world_cell_dispatch
#endif
.globl CELL_ENTRY
CELL_ENTRY:
    pushal
    cld
    movl %esp,%eax
    pushl %eax
    call CELL_DISPATCH
    addl $4,%esp
    popal
    ret
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
