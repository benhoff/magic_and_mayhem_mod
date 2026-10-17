# Owning diagnostic cursor kernel lease state

Native diagnostic phase model, isolated from game hooks and capture. Each state
phase must be caller-serialized. Real Duplicate/Wait/Release/Close calls occur
between phases outside state locks. Constructor return must still own the new
handle when duplicating; no arbitrary global handle duplication is implemented.

Private duplicates and actual gate state remain in the owning module. A numeric
token binds owner and both identities to stored acquired levels. Return checks
the stored gate, so a requester cannot falsely supply zero to drain references.
Retirement blocks new reservations, retains active duplicates until gate release
and token return, and permits one private-close claim after drainage. A scripted
failed close retains the duplicate and identity for explicit retry.

If token/gate storage disagrees, the model freezes admission and retains uncertain
references as a fault; it does not silently unwind them. Full fault journal and
bounded abort disposition remain unimplemented. No pointers/handles or this host
layout may be transported across modules; scalar token exports remain pending.

The prospective PE32 fixture creates and duplicates actual Wine kernel mutexes,
acquires a stored gate, refuses wrong-owner/outstanding-level returns, retires
originals, refuses admission/closing, closes originals during the active lease,
then releases duplicates and drains token/refcounts. It tests a scripted close
failure followed by real successful private close. No actual concurrent close,
game installation/forwarding, worker callback, source pages or parity occurs.

`draw02-cursor-kernel-fixture-01.json` passes the current-source owned PE32
fixture. Real duplicate lifetime is bounded to this owned active-lease case;
private close failure is scripted, not a failed real CloseHandle. No actual
game/concurrent-close/worker synchronization or reference/parity claim.

## Synchronized foreign-thread retirement fixture

The next owned PE32 fixture uses real event and thread handles. Main acquires
both private mutex levels and holds its diagnostic token. Another thread waits
for a go event, retires both original identities under a caller critical section,
drops the section, then closes both originals and signals completion. Main waits
for this observed completion before checking retired admission/close refusal,
releasing private levels and draining the token/refcounts.

This verifies the private duplicate retention boundary across foreign-thread
original Close. Events define exact test phases, not arbitrary sleep timing.
The model requires serialized state phases; source shows OS closes outside the
fixture lock, but no broad game callback/worker lock-order claim follows. The
scripted private-close failure and later actual successful close stay separate.
No original imports/installer, source protection, real worker pipeline or pixel
comparison is present. Previous fixture01 remains immutable historical evidence
after test/runner/scope changes; a fresh result must bind the extended sources.

`draw02-cursor-kernel-fixture-02.json` passes the fresh synchronized foreign-
thread retirement/close fixture, binding current test/runner/scope. Fixture01
remains immutable historical evidence. No complete game callback/worker
lock-order or actual original handle lifecycle is inferred.
