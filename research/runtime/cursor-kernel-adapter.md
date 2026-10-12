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
