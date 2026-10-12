# Isolated diagnostic gate forwarding

This is a native diagnostic adapter, not recovered engine behavior or a live
replacement. It wraps one supplied original callback with an isolated mutex gate.
It saves LastError and full512FX before extra acquisition calls, restores both
before invoking the original once, then saves original poststate/result and
restores those after reverse release. The raw original return is unchanged.

A refused acquisition still forwards the original exactly once; the gate retains
its failure and any level whose release failed. Such a release fault is explicit,
not a claim that fallback unwound all ownership. Future live diagnostic code must
handle faults, lifetime/token validation, disabled/budget paths and record them.

The prospective PE32 fixture crosses64outer/inner raw wait and release success
cells. Diagnostics deliberately change control word and LastError. The supplied
original body checks its input full512FX/error, writes distinct poststate/error,
and returns a distinctive raw result. Comparison verifies all512saved bytes and
one forward per cell, plus retained failed-release levels. No actual game,
private handle lifecycle, source pages, worker callbacks or pixel parity occur.
Confidence remains provisional until fresh source/contract-bound execution.

Caller must not interpret the adapter returning the original result as success
of diagnostic release. It inspects `gate.failure` and `gate.acquired` separately.
A failed release remains outstanding and prevents clean retirement/drain in the
future lifetime adapter. The synthetic fixture retains these bits; an actual
bounded-abort disposition and journal are separate unimplemented policies.

`draw02-cursor-gate-forward-fixture-01.json` passes all64cells with current
source/contract hashes and one original forward per cell. Exact error/result and
all512FXbytes pass. Confidence is high only within the owned synthetic fixture.
