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
.data
_target_queue: .long 0x5002a0
_resume_sequence: .long 0x46b465
_resume_tick: .long 0x46b49b
