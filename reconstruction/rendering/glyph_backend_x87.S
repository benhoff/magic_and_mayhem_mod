#ifdef _WIN32
#define C(name) _##name
#else
#define C(name) name
#endif
.text
.globl C(mnm_glyph_fp_execute)
C(mnm_glyph_fp_execute):
    pushl %ebp
    movl %esp,%ebp
    pushl %ebx
    subl $527,%esp
    andl $-16,%esp
    movl %esp,%ebx
    fxsave (%ebx)
    movl 8(%ebp),%eax
    /* Offsets below are enforced by compile-time checks in the harness. */
    movl 88(%eax),%edx
    fxrstor (%edx)
    subl $12,%esp
    pushl %eax
    call C(mnm_glyph_backend_execute)
    addl $16,%esp
    movl 8(%ebp),%eax
    movl 92(%eax),%edx
    fxsave (%edx)
    fxrstor (%ebx)
    movl -4(%ebp),%ebx
    leave
    ret
.globl C(mnm_glyph_fp_begin)
C(mnm_glyph_fp_begin):
    subl $8,%esp
    movl 16(%esp),%eax
    movl $0x3f800000,0(%esp)
    testl %eax,%eax
    jz 1f
    movl $0,0(%esp)
1:  flds 0(%esp)
    fcomps glyph_zero
    testl %eax,%eax
    jz 3f
    movl 12(%esp),%edx
    movl $0,0(%esp)
2:  fildl 0(%esp)
    fmuls glyph_multiplier
    fstps (%edx)
    addl $4,%edx
    incl 0(%esp)
    cmpl $64,0(%esp)
    jb 2b
3:  addl $8,%esp
    ret
.globl C(mnm_glyph_fp_run)
C(mnm_glyph_fp_run):
    pushl %ebp
    movl %esp,%ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    subl $24,%esp
    movl 8(%ebp),%edi
    movl 12(%ebp),%esi
    movl 16(%ebp),%ecx
    testl %ecx,%ecx
    jz 5f
    movl 24(%ebp),%eax
    fildl 0(%eax)
    fildl 4(%eax)
    fildl 8(%eax)
1:  movzwl (%edi),%ebx
    movzbl (%esi),%eax
    incl %esi
    movl 20(%ebp),%edx
    flds (%edx,%eax,4)
    fld %st(0)
    fsubrl glyph_one
    movl %ebx,%eax
    shrl $11,%eax
    movl %eax,-16(%ebp)
    fildl -16(%ebp)
    fmul %st(1),%st(0)
    fld %st(5)
    fmul %st(3),%st(0)
    faddp %st(0),%st(1)
    call glyph_truncate
    shll $11,%eax
    movl %eax,-20(%ebp)
    movl %ebx,%eax
    shrl $5,%eax
    andl $63,%eax
    movl %eax,-16(%ebp)
    fildl -16(%ebp)
    fmul %st(1),%st(0)
    fld %st(4)
    fmul %st(3),%st(0)
    faddp %st(0),%st(1)
    call glyph_truncate
    shll $5,%eax
    orl %eax,-20(%ebp)
    andl $31,%ebx
    movl %ebx,-16(%ebp)
    fildl -16(%ebp)
    fmulp %st(0),%st(1)
    fld %st(2)
    fmul %st(2),%st(0)
    faddp %st(0),%st(1)
    call glyph_truncate
    orl -20(%ebp),%eax
    movw %ax,(%edi)
    addl $2,%edi
    fstp %st(0)
    decl %ecx
    jnz 1b
    fstp %st(0)
    fstp %st(0)
    fstp %st(0)
5:  addl $24,%esp
    popl %edi
    popl %esi
    popl %ebx
    leave
    ret
glyph_truncate:
    subl $12,%esp
    fstcw 8(%esp)
    movw 8(%esp),%ax
    orw $0x0c00,%ax
    movw %ax,10(%esp)
    fldcw 10(%esp)
    fistpq 0(%esp)
    fldcw 8(%esp)
    movl 0(%esp),%eax
    addl $12,%esp
    ret
#ifdef _WIN32
.section .rdata,"dr"
#else
.section .rodata
#endif
.align 8
glyph_one: .double 1.0
glyph_zero: .long 0
glyph_multiplier: .long 0x3c820821
#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
