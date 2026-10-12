.text
.globl _wine_blit_packet_entry
_wine_blit_packet_entry:
    pushfl
    pushal
    movl %esp,%ebx
    subl $527,%esp
    andl $-16,%esp
    fxsave (%esp)
    cld
    subl $12,%esp
    pushl %ebx
    call _wine_blit_packet_observe
    addl $16,%esp
    fxrstor (%esp)
    movl %ebx,%esp
    popal
    popfl
    movl (%ebx),%eax
    movl %esi,4(%esp)
    jmp *_wine_blit_packet_resume
