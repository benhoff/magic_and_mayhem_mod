# DRAW02 original-active producer recorder thread policy

`NR.producer-recorder-thread-policy` is an intentional diagnostic policy, separate
from recovered producer behavior `RS.canvas-producer-sequence`. It is opt-in through
`MNM_DRAW02_PRODUCER_BIND=1`; absent flags preserve the existing mode. Malformed,
empty and unsupported flags refuse configuration. The mode admits original-active
producer observation and refuses configured glyph replacement, raster/batch/
continuous replacement and the separate opaque-publication interposer.

Entry, return and close observers own one recursive Win32 critical section while
accessing recorder state. Cleanup releases it before returning to an original
body and restores the captured LastError. Original surface bytes and driver calls
are outside this mutex; it must never be described as making source pixels atomic.
Primitive nesting lives in32bounded thread slots in this mode. Private return
frames retain their observed owner and refuse a different return owner or slot
exhaustion with98. Local thread IDs and private stack addresses never imply stable
identities across launches. Existing modes retain the shared nesting state.

The PE32 helper fixture exercises actual Win32 threads/critical sections. Both
threads keep independent nesting while their simulated original-body intervals
remain open, serialize2000increments and preserve LastError. Five malformed modes
and the32slot ceiling are separate processes. These checks do not execute the
original game, producer source snapshots or physical presentation.

The legacy producer fixture initially omitted existing glyph/publication link
objects; its repaired link uses their current compiled objects. Its added entry52
also inherited an incompatible rotated-raster synthetic tail. The first repair
misread the allocation and used0x18; the actual prologue reserves0x24bytes. The
corrected synthetic body matches that allocation and callee-pop8. Both execute
faults (0x7 then0x4) and the pre-execution link refusal remain retained. These are
fixture failures, not established production renderer failures.

Positive legacy admission now requires an exact32byte`MNMPFP01`completion marker
written after all checks, in addition to exit0. The final legacy run checks all53
sites, registers/flags/x87/SSE/LastError, signature refusal, existing controlled
raster/batch cases, diagnostic budgets and malformed stream/configuration cases.
It observes the default mode, independently from the thread policy helper.

Remaining: actual opt-in entry/return snapshots and logical surface association
at original API points; complete producer source ownership; primary-writer causal
identity; alternate/GDI/movie routes; semantic actions and complete journeys. No
new original execution, pixels, complete drawing gate or replacement is asserted
by these synthetic results. Confidence is high within the checked helper/default
forwarding scope; original trace-mode coupling is pending.
