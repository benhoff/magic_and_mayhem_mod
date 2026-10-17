#pragma once
/* Prospective native diagnostic forwarding adapter. Environment must provide
 * full512FX save/restore and LastError callbacks. No recorder locks here.
 * Outstanding release faults remain visible in gate; the owning diagnostic
 * must handle them explicitly after forwarding, never claim clean fallback. */
typedef unsigned (*MnmCursorOriginalCall)(void*);
typedef unsigned (*MnmCursorErrorGet)(void);
typedef void (*MnmCursorErrorSet)(unsigned);
typedef void (*MnmCursorFpState)(void*);
static unsigned mnm_cursor_gate_forward(MnmCursorSourceGate* gate,
    void* outer, void* inner, MnmCursorGateWait wait,
    MnmCursorGateRelease release, MnmCursorOriginalCall original, void* argument,
    MnmCursorErrorGet get_error, MnmCursorErrorSet set_error,
    MnmCursorFpState save_fp, MnmCursorFpState restore_fp) {
    unsigned char fp[512] __attribute__((aligned(16)));
    unsigned error=get_error();save_fp(fp);
    int accepted=mnm_cursor_gate_acquire(gate,outer,inner,wait,release);
    restore_fp(fp);set_error(error);
    unsigned result=original(argument);
    error=get_error();save_fp(fp);
    if(accepted)mnm_cursor_gate_release(gate,release);
    restore_fp(fp);set_error(error);
    return result;
}
