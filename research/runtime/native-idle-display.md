# NS18: native retained display after Stop

This is an intentional native presentation policy, not a recovered idle/action
animation contract. A creature with a displayed movement ANI bitmap retains an
owned pair of sequence index and sequence-relative record index when Stop clears
its route, fine motion, previous segment and segment ticks. Its position follows
the existing Stop policy: the last committed logical cell and bound terrain
height, rather than the abandoned partial segment position. No movement timers,
ANI program counter, repeat state or segment history are retained for display.

The static pose is a fallback. A current fine-motion display has priority, then
a previous completed-segment display, then the retained pose. A move command
captures the current display before resetting its driver, so suppressed planning
and a newly blocked route remain visible. A rejected edge captures its display
before clearing continuation state. Repeated Stop preserves the fallback.
Cleanup clears it and excludes the actor from drawing; release and generation
reuse cannot transfer the pose. Merely cancelling queued moves changes neither
the active controller nor its pose.

Snapshot v8 adds an optional pose to every owned motion record. Writers choose
v8 only when a retained pose exists, including a checkpoint containing a pending
Stop. States without poses keep their previous minimum format (v1-v7). The core
checks bounds, continuous-driver policy, cleanup and ANI binding. The resource
adapter validates that the pose belongs to one of the eight bound directional
movement sequences and references a nonnegative opcode-0 bitmap record. Invalid
resource restoration preserves both the old state and old navigation resource.
Scene composition additionally refuses invalid sequence/record/frame indices.

Evidence is recorded in `native-idle-display.json`. The synthetic process test
checks Stop, static admitted ticks, resource refusal and rollback, reorders,
FIFO Stop/restart, cleanup/release/reuse, and independent v8 wire layout and
malformed input. The actual entry-point window experiment uses installed Forest
8x8 geometry/navigation and Redcap movement ANI base 0/SPR in all four diagnostic
views. Qt mouse/button events and its real timer stop a moving actor, retain its
body, pause, perform modal Save, restore in a fresh window, run idle ticks and
restart movement. Every captured frame is compared to independently decoded SPR
pixels, and restored state/frame artifacts match uninterrupted execution.

The original manifest brackets installed-input experiments. Original comparison
and live replacement remain absent. Older NS17 evidence is retained unchanged:
its missing Stop body documents the earlier implementation, not the current
retained pose. Source fingerprints on older evidence can therefore be stale.
Initially spawned idle creatures without any prior display remain outside this
slice, as do original idle sequence selection, idle animation timing, action
transitions, other installed creature profiles and full-game admission.
