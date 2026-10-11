.text
.globl mnm_glyph_counted_original
mnm_glyph_counted_original:
    pushfl
    incl mnm_glyph_original_calls
    popfl
    jmp *mnm_glyph_pinned_original
.section .note.GNU-stack,"",@progbits
