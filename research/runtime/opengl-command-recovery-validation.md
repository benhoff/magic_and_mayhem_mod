# Rendering recovery failure validation, 2026-10-06

The native administrative checkpoint policy passed 20 bounded PE32/Qt cases and
294 independent GPU frame comparisons. These fixtures compare original synthetic
operation pixels with native presentation; they do not establish original game
or driver equivalence. Historical evidence is retained without updating hashes.

The complete cases establish 27 fresh sessions across RGB16, RGB24, RGB32,
indexed and mixed resources. They cover continued partial writes, fills, copies,
flips and shared palette updates after resource-prefix serialization. The palette
case recreates120 objects at reused pointers across three fresh sessions. Repeated
fixture factory installation now preserves its installed hook, fixing a test
fixture defect exposed by these cycles.

Overflow, prolonged FULL and foreign/out-of-range acknowledgement injection each
invalidate three streams, followed by explicit CHECKPOINT attachment from owned
state. Failed rings remain unchanged after retirement. These cases do not prove
arbitrary acknowledgements in a separate stale file can affect a new mapping.
Strict refusal cases cover incomplete pixels/palettes, held locks/DCs, uncertain
metadata and finite capacity. Selftest-only allocation, mapping and worker-start
faults verify joined workers, no queue or channel storage and no reserved borrowed
bytes. Complete192-byte owned fixture storage remains available after refusal;
producer storage and consumer resources have separate lifetimes.

The fresh [fixture report](opengl-command-recovery-failure-matrix.json) records
all 20 cases. The [host regression report](opengl-command-recovery-host-regressions-20261006.json)
adds four cases and 83 GPU frame comparisons, plus native/sanitized client checks.
The [queue regression report](opengl-command-recovery-queue-regressions-20261006.json)
records native and ASan/UBSan storage/publication checks. Production PE32 and Qt
shell builds and three control/palette/channel CTests passed. Fault injection and
storage exports are excluded from production builds.

## Original-game observation

The [bounded startup record](opengl-command-recovery-game-startup-20261006.json)
pins No-CD build 40209ca7 and source/artifact hashes. The harness stages an isolated
working executable with the existing import-patch script and verifies the original
manifest before and after. Two runs wait for observed original drawing, then two
seconds, cancel the deliberately unconsumed old stream and request attachment.
The consumer polls at 1ms after requesting recovery. This intentionally tests a
failed reader at startup; it is not a healthy steady-state gameplay measurement.

CHECKPOINT was refused at stage 4, complete-state admission. That diagnostic does
not identify which completeness predicate failed. Ordinary RECOVER reached READY
but its fresh stream became invalid (reason 2) before the first native frame.
Both consumers fell back and retired every consumer resource. The original
process remained alive and screenshots show its main menu. Neither run presented
a native frame, so READY is only negotiation evidence, not recovery success.

Earlier exploratory output under working/tests/live-render-recovery/run-pg947r_o
used earlier timing and remains historical; the pinned record uses run-qc6gt4y0.
Gameplay routes, sustained original-game recovery, unsupported drawing branches,
driver/pixel equivalence, movie interaction and live replacement remain pending.
The next implementation step is to diagnose stage 4 completeness and fresh-stream
reason 2 from bounded original drawing, then rerun independent validation.
