# Compact continuous owned unlock updates

2026-10-06. Native policy `NR.owned-session-unlock-packed` now sends the smallest
rectangle containing changed native storage pixels for a successful continuous
full writable Unlock of an existing, matching resource. Its initial checkpoint
still uses CREATE. Missing baselines and changed layouts use complete replacement;
partial locks and default bounded capture retain their previous serialization.
No opcode, wire version, queue capacity, surface limit or timeout changes.

## Ownership and ordering

Lock admission transfers an eligible independently owned checkpoint into the
pending lock's private `base`, just as partial locks already did. The pixels are
removed from authoritative surface storage, charged to pending storage, and cannot
be used for drawing or recovery while borrowed. Eligibility requires a successful
admitted writable full lock and an exact dimensions/format/flags/masks match.
There is no extra baseline allocation and no retained application pixel pointer.
Failed Unlock retains the descriptor and baseline for the existing retry path.
Epoch changes, retirement, replacement and normal lock cleanup free the pending
baseline under the existing tracker and storage accounting.

Only after successful original Unlock does publication compare normalized owned
input against that baseline. All native bytes count, including unused RGB32 bits.
The envelope is tightly packed using the existing scratch allocation/accounting
path. No changes means no UPDATE, while palette handling, eligible PRESENT,
successful-operation counting and authoritative checkpoint maintenance continue.
First CREATE, failure handling, strict borrowed-resource refusal and sticky queue
overflow remain intact. Neither comparison CHECK output nor GPU readback supplies
render input. DC handoffs and constant fills retain their existing complete input
publication; this increment does not invent an earlier DC baseline.

The extra scan is bounded by the admitted image size. Baseline, pending snapshot,
owned surfaces and temporary packed bytes remain subject to the 64-MiB ownership
budget; queue storage remains 32 MiB and ring storage 1 MiB. A dense full overwrite
still emits the full rectangle. There is no command dropping, session
resynchronization, consumer wait or relaxation of overload refusal.

## Validation and remaining boundaries

The native and ASan/UBSan matrix independently reconstructs after pixels from each
envelope and checks changed pixels on all four edges for minimality: 5,928 cases
per build across 8/16/24/32-bit storage, single corners, disjoint corners, unchanged,
dense and deterministic sparse writes. The actual synthetic PE32 fixture uses
padded borrowed rows poisoned by original Unlock, changes one pixel, changes only
the RGB32 high byte, and leaves every third successful unlock unchanged. It compares
all 70 complete displayed frames against independent Python fixture pixels and
checks exact archived UPDATE lengths, locations, native final bytes and ordering.

The three-case continuity regression also includes 70 dense full-overwrite frames
beyond 64 MiB/4096 records and a two-frame finite archive. Strict checkpoint and
administrative recovery matrices retain mixed operations, borrowed ownership,
cleanup, stale ACK, cancellation, timeout, overflow and claimed-failure guards.
The bounded original-game experiment requires at least 100 frames spanning
11 seconds in one healthy recovered session for both attachment and automatic
recovery. Independent original-game pixel equivalence, gameplay/movie interaction,
long-running counter exhaustion and throughput on other drivers remain pending.

Validation uses a preserved source snapshot with the concurrently committed v3
renderer. Earlier exploratory source snapshots and failed staging/fixture runs
remain under `working/tests/render-delta-source/`; they do not establish current
code validation. A symlinked working installation in the first isolated staging
attempt was restored to its exact verified executable/preference hashes, and its
patched artifacts were preserved. Final staging uses a real directory copy.
Original inputs are read-only and verified before/after the game experiments.

Reproduce native checks with `python3 tools/test-render-dirty-region.py`; use the
continuous harness's `--case delta --case no-archive --case short-archive`, the
strict checkpoint and administrative recovery harnesses, and the game harness's
`--require-sustained` option under Xvfb. These are observation/preview results;
original drawing remains active and no live replacement is claimed.

Final committed-renderer observations presented 208 frames after checkpoint
attachment and 235 after automatic recovery, each with one healthy session for
12 seconds. Early attachment refused incomplete state. Cleanup reported zero live
native resources and no ordinary readbacks/uploads, and immutable manifests
verified 2,927 files before and after. The initial old stream was deliberately
abandoned; this does not assert that an unattached reader can accept startup
traffic forever. Parallel fresh-prefix recovery fixture startup failed and was
rerun sequentially. A later blocked-callback test exposed its fixed-sleep timing
assumption: a request delayed past the sleep could validly recover. The fixture
now holds the callback until a matched REFUSED response, then releases it, with
its existing finite wait deadline. Failed outputs are retained as exploratory
runs, not passing evidence.

Fresh execution records: [native and sanitizer envelopes](opengl-unlock-delta-native-20261006.json),
[finite queue regression](opengl-unlock-delta-queue-20261006.json),
[continuous independent frames](opengl-unlock-delta-continuous-verified-20261006.json),
[strict checkpoints](opengl-unlock-delta-checkpoint-verified-20261006.json),
[administrative recovery](opengl-unlock-delta-host-verified-20261006.json), and
[bounded original-game sustained recovery](opengl-unlock-delta-game-verified-20261006.json).

Retaining a baseline can reach the existing ownership ceiling sooner than dropping
it on full Lock. This remains an explicit admission/publication failure, not an
unbounded allocation. Forced packed-scratch allocation failure and sustained
high-memory scenes remain pending beyond the recorded budget/refusal fixtures.
