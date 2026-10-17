.text
/* Observations preserve all registers, flags, x87/SSE state and LastError.
 * Original routines execute once; all admitted calls have no stack arguments. */
.macro observe event
    pushfl
    pushal
    movl %esp, %ebx
    subl $527, %esp
    andl $-16, %esp
    fxsave (%esp)
    cld
    pushl $\event
    pushl %ebx
    call _cadence_record
    addl $8, %esp
    fxrstor (%esp)
    movl %ebx, %esp
    popal
    popfl
.endm
.macro pair name target before after
.globl _cadence_\name
_cadence_\name:
    observe \before
    call *_target_\name
    observe \after
    ret
.data
_target_\name: .long \target
.text
.endm
pair pacer 0x4e3e40 1 11
pair animation 0x464ec0 5 15
pair scheduler 0x52a780 6 16
pair maintenance 0x509e50 7 17
.globl _cadence_queue
_cadence_queue:
    observe 8
    /* Tail-forward: the existing scene hook retains the original return PC. */
    jmp *_target_queue
.globl _cadence_sequence
_cadence_sequence:
    movl %edi, 0xfd62(%esi)
    observe 2
    jmp *_resume_sequence
.globl _cadence_tick
_cadence_tick:
    movl %eax, 0x6c4830
    observe 3
    jmp *_resume_tick
.globl _cadence_draw_suppression
_cadence_draw_suppression:
    cmpl %ebx,0x6e1f00
    observe 9
    jmp *_resume_suppression
.globl _cadence_draw_transition
_cadence_draw_transition:
    cmpb %bl,0x100b2(%esi)
    observe 10
    jmp *_resume_transition
.globl _cadence_scene
_cadence_scene:
    observe 18
    jmp *_target_scene
.globl _cadence_results
_cadence_results:
    observe 20
    jmp *_target_results
.globl _cadence_target
_cadence_target:
    movl %eax,%ebx
    testl %ebx,%ebx
    observe 21
    jz 1f
    jmp *_resume_target_available
1:  jmp *_resume_target_missing
.data
_target_queue: .long 0x5002a0
_resume_sequence: .long 0x46b465
_resume_tick: .long 0x46b49b
_resume_suppression: .long 0x46bba3
_resume_transition: .long 0x46bbab
_target_scene: .long 0x4fd280
_target_results: .long 0x557040
_resume_target_available: .long 0x4fd53c
_resume_target_missing: .long 0x4fd5ad
