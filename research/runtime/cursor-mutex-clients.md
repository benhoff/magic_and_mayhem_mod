# Cursor mutex clients

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Evidence: `draw02-cursor-mutexes-static-01.json` and
`draw02-cursor-waits-original-01.json`. Confidence is high for exact instructions
and the stated owned gate matrix; complete ownership/lifetime remains partial.

Foreground setup `4a30b0` calls `CreateMutexA` at `4a3700`, with three literal
zero arguments, and stores the result at `642c58`. Startup `4e8d80` zeroes EBP,
pushes it three times at `4e8f6c..6e`, calls the same import at `4e8f6f`, and
stores the result at `6e1ec0`. These are constructor argument/store observations;
runtime handle identities and lifetime are not inferred from the globals.

Foreground cursor painter `4a45b0` waits on `642c58` for 10 ms and performs
its admitted copy path only for a zero result. The timer `4a4910` first rejects
cursor type 22, negative types and types at least 22. It then waits on `6e1ec0`
for 10 ms. A nonzero return exits without calling its release client. A zero
result reaches screen/countdown/cursor gates and ultimately releases `6e1ec0`.

When the timer repaints, it also waits on `642c58`, passing `ffffffff` at
`4a4a8a` before the indirect call at `4a4a8d`. Its result is not tested before
the subsequent copy path. It releases `642c58` at `4a4c4a`, then releases
`6e1ec0` at `4a4dd4`. The earlier description of only the outer timer mutex
was incomplete. This exact nested-client order does not prove all callers,
aliases, runtime ownership or safe additional diagnostic locking.

The privately mapped unchanged original timer fixture fixes the inner cursor
enabled field `642c60` to zero. Its 240 cells cover five types (`ffffffff`, 0,
21, 22, 23), screen 0/1, raw Wait results 0/80/102/ffffffff, countdown 0/1/2
and Release results 0/1. Exactly 96 cells call the owned fake outer Wait with
the expected handle and timeout; 24 call Release. Countdown decrements only
after a zero outer Wait, active screen and positive countdown. All original
timer bytes and checked unrelated globals remain unchanged. No real OS mutex,
cursor coordinate, pixel copy or nested-wait execution is claimed.

Foreground disposal `4a37d0` optionally calls `CloseHandle`, ignores its result
and clears `642c58`. The bounded `4e9df6..4e9e10` window does the same for
`6e1ec0`; its enclosing whole function is absent from the inventory. Unknown
ranges remain intact. Static nonzero Wait branches do not establish Windows
abandoned/failure ownership semantics. Before changing capture synchronization,
observe actual constructor/wait/release/close identities and ordering in a
bounded original run. Source allocation/aliasing, complete concurrency and
physical/native parity remain independent gaps.

The separate original call observer now retains two tracked creations/closes
and 286 complete timer nested wait/release chains in one normal-Quit run.
See `draw02-cursor-mutex-order-analysis-01.json`; all raw Wait values are zero
and Release values one in this run. This is bounded observed order, without
proving all ownership, aliases or safe added capture locking.

The longer source/mutex run reproduces the negative queue28/lease124foreign
cursor caller while the latest timer inner Wait on ID2 returns zero at the same
tick. `draw02-cursor-refusal-mutex-analysis-02.json` binds the retained cursor
record and refusal. No later matching release or completion marker is retained;
two IDs remain live. The API terminal row and missing completion marker are
separate facts. This does not prove source overlap, ownership or safe locking.
