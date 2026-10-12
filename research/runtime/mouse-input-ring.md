# Original mouse ring and Region callback anchors

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Static evidence: `draw02-mouse-dispatch-static-02.json`. Isolated unchanged
original writer evidence: `draw02-mouse-ring-original-02.json`, 55,296 cells.
Confidence is high within the stated writer matrix; dispatch/lifetime semantics
remain partial. Cursor painting/polling is a separate path.

The inventory-bounded initializer `4a3720` seeds a 32-record ring at `641ab8`,
sets selected index `641fbc` and tail `641a74` to zero, and write index `641a88`
to one. Each record is 40 bytes. Writer `4a3800` takes flags in ECX, X in EDX
and Y on the stack, masks each to low 16 bits, and returns with `RET 4`.

| Offset | Writer value |
| --- | --- |
| 0, 4 | Unsigned low-16 X, Y |
| 8 | Coordinates differ from previous ring record |
| 12, 16 | Flags bits 0, 1 (left/right button values) |
| 20, 24 | Button values differ from previous record |
| 28, 32 | Flags bits 2, 3 (Shift/Ctrl values) |
| 36 | Sign of low WORD from one `GetAsyncKeyState(18)` call |

Previous index is write minus one, wrapping zero to 31. The writer increments
write and selected indices independently, wrapping 32 to zero. If the new write
index equals the tail, it advances the tail with the same wrap. This names the
observed writes; queue consumption and concurrent access are not recovered.
The isolated matrix checks valid indices, every 4-bit flag combination, ignored
upper input bits, three coordinates including `ffff`, equal/changed previous
coordinates/buttons, three Alt responses and collision/noncollision tails.
Invalid indices, real OS state, input delivery, race handling and gameplay
outcomes are outside this execution evidence.

Two exact calls at `4ea052` and `4ea11f` feed this writer from message-handling
instruction windows. Neither caller is assigned a whole discovered function.
The bounded camera input routine `469b30` reads the selected mouse record;
it does not establish selection dispatch.

Region constructor `4bfda0`, reached by the exact static call at `4bdd55` with
ECX `689930`, sets table `5c6a54`. Its first three slots are `4fdad0`, `464320`
and `4a4e20`. The first slot has no discovered whole-function entry; the retained
bounded prefix reads hit class `6a5134`. Full callback behavior is pending.
Helper `4a5000` copies the selected 40-byte record onto the stack, invokes
virtual slot zero with event mask `40` when object+4 is valid, conditionally
clears `5e04cc`, then clears object+8. Actual event meanings, complete control
dispatch, persistent selection, tooltip and source/lifetime remain unverified.

Negative evidence is preserved separately: the initial static inspector refused
the mistaken `GetKeyState` label; the first sandboxed 32-bit execution terminated
with SIGSYS before emitting any case row. The denied syscall is unassigned.
Neither failure is successful game execution or semantic validation.
