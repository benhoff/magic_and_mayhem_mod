#pragma once
/* Native diagnostic policy, not recovered engine behavior. Call only with
 * caller-pinned, distinct live handles. No handle close or game mutation.
 * acquired retains levels whose release fails: never claim they unwound. */
typedef struct {
    void* outer;
    void* inner;
    unsigned acquired;
    unsigned failure;
} MnmCursorSourceGate;
typedef unsigned (*MnmCursorGateWait)(void*, unsigned);
typedef unsigned (*MnmCursorGateRelease)(void*);

static int mnm_cursor_gate_release(MnmCursorSourceGate* gate,
                                   MnmCursorGateRelease release) {
    int ok = 1;
    if (gate->acquired & 2) {
        if (release(gate->inner)) gate->acquired &= ~2u;
        else ok = 0;
    }
    if (gate->acquired & 1) {
        if (release(gate->outer)) gate->acquired &= ~1u;
        else ok = 0;
    }
    if (!ok) gate->failure = 6;
    return ok;
}

/* gate must be zero-initialized. Outstanding levels forbid reacquisition. */
static int mnm_cursor_gate_acquire(MnmCursorSourceGate* gate, void* outer,
                                  void* inner, MnmCursorGateWait wait,
                                  MnmCursorGateRelease release) {
    if (gate->acquired) {
        gate->failure = 5;
        return 0;
    }
    gate->outer = outer;
    gate->inner = inner;
    gate->failure = 0;
    if (!outer || !inner || outer == inner || !wait || !release) {
        gate->failure = 1;
        return 0;
    }
    unsigned result = wait(outer, 0);
    if (result == 0 || result == 0x80) gate->acquired = 1;
    if (result) {
        gate->failure = result == 0x80 ? 2 : result == 0x102 ? 3 : 4;
        mnm_cursor_gate_release(gate, release);
        return 0;
    }
    result = wait(inner, 0);
    if (result == 0 || result == 0x80) gate->acquired |= 2;
    if (result) {
        gate->failure = result == 0x80 ? 2 : result == 0x102 ? 3 : 4;
        mnm_cursor_gate_release(gate, release);
        return 0;
    }
    return 1;
}
