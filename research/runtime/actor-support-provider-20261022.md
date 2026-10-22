# Actor support provider and borrowed ownership (2026-10-22)

## Scope and confidence

The raw NoCD4f3320 support query now composes with actual4f4330 aggregation in
the actor backend, extending215 to216 public routes. The pre-existing typed
pathfinding implementations are reused as a separate cross-check. Their source
and historical evidence remain unchanged; this batch closes the actor's
supplied-query boundary for initialized owned inputs. It does not establish
original world production, whole-search or owner/planner/live equivalence.

Input Chaos.exe SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Fresh read-only Ghidra decompilation, callers and complete instruction ranges
are retained in `actor-support-results-20261022/inspection-2.json` through SQLite.
Both roots already exist in the pinned function inventory; no boundary changes
or recovered-range exceptions are needed. Original manifests are verified before
and after every consuming experiment. The first inspection used an unavailable
headless launcher path; its failed report remains retained separately.

## Recovered behavior

The receiver borrows cell backing at+4c, plane stride at+10, layer offsets at+6432
and row offsets at+64b2. Cells are12 bytes, with an unsigned terrain WORD at0 and
flags WORD at+0a. Global65660c selects terrain records of stride164, classification
DWORD+94 and eligibility byte+b0. The packed22-byte parameter member supplies
special byte+10 and type DWORD+11; the surrounding meanings remain hypotheses.

Current flags20 without10 reject when the special byte is zero. Current4000
always rejects. Nonzero terrain without80 tests classification as unsigned<16,
then requires terrain byte+b0 bit8 or type22. A rejected current terrain record
never falls back. Only zero terrain or flag80 enables fallback, and only at Z>0.
The below address subtracts12 times the receiver plane stride from the current
pointer; it does not consult the Z-1 offset table. The restriction is checked
again. Below4000 immediately supports; otherwise a nonzero terrain without80
requires classification exactly16 and the same eligibility rule. Returns are
exactly0/1. No call, allocation, ownership transfer or write occurs in4f3320.

The actual4f4330 caller preserves one-cell coordinate passthrough and uses its
existing wrapped2×2 aggregation otherwise. Boundary-only hits do not establish
a requested-level hit. The fixture exercises ordinary and negative/multi-period
wrapped coordinates, valid one-cell inputs and negative-Z aggregate rejection.
Its existing coordinate helpers and the lower predicate remain original in the
PE32 comparison; the supplied4f3320 trampoline is removed entirely.

See the earlier [typed support](pathfinding-cell-support.md) and
[aggregation](pathfinding-record-query.md) research for their separate history.

## Borrowed storage and comparison

Each independently authored case allocates two cell buffers and one terrain
record using composed allocator wrappers. It performs six checked queries:
initial backing, caller mutation, rebind to the second buffer, changed borrowed
parameters, null-terrain lookup-bypass inputs, then reuse of the first buffer.
Every query must match the typed model and preserve the receiver, parameter,
cell and terrain bytes, allocation count and external-callback count. Explicit
caller teardown retires all three buffers after the first state snapshot; the
second snapshot includes freed-buffer bytes and zero residual live allocations.
This demonstrates borrowed ownership in the selected domain, not recovery of
the allocator's operating-system backing or automatic lifetime enforcement.

All low flag bytes are covered in both special-byte and current4000 banks,
with current/below differences, classes0/15/16/17/-1, terrain eligibility and type22,
unsigned high terrain indices32768/65535, nonstandard row/layer tables and a
plane stride distinct from adjacent layer offsets. High-index fixtures use
explicit sparse address windows: the original terrain base is biased so that
its referenced record remains owned. The typed cross-check allocates the
corresponding index and uses independently declared bytes; no output becomes
another implementation's input. Corrupt and inaccessible windows are excluded.

The full frozen-source `support-matrix-3.json` passes2048 cases,12288 checked
queries and4096 before/after teardown snapshots per build. Each stream compares
2351845376 bytes; SHA-256`ff873837b7766b462251debf522e3adbd35c54385ca31e54b029ae9122e388e6`.
Confidence is high within this bounded normal-return domain. PE32, native64 and ASan/UBSan streams compare complete owned/global
bytes, meaningful returns, ordered allocation/free callbacks and all six results
before/after teardown. Selected original bodies are guarded before and after.
Each compiler translation unit has its own dependency fingerprint. Earlier
failed/prospective declarations are immutable and receive no validation credit.

## Remaining boundaries

The leaf has no newly discovered callees. Static reconciliation retains the
43 historical semantic leads as24 routes/19 embedded helpers, removes the one
support-query specification and preserves113 named lower endpoint specifications.
Endpoint counts are not missing-function counts. Other indirect targets remain
incomplete. The Critical parent stays partial for original scheduling/task
lifetime, remaining postprocess/particle/special/scenario providers, actual
file/profile/heap/atexit/COM and platform backing, malformed/indeterminate/SEH
cleanup, full installed sessions, hardware and live replacement.

Incoming history dcb606bd..97c3f5b2 has33 exact parent/current file transitions
with no missing or mutated receipts. This is committed-history accounting only;
source changes and fresh execution claims in this batch are reviewed separately.

Dashboard regression review found stale expected totals after97c3f5b2: two
Critical mechanisms and one High shadow-stride mechanism now have scoped code.
Tests name those exact transitions, retain their authored validation/acceptance
requirements and update the partial counts (Critical26, High37, Medium3,
Supporting1, Low13). The new support provider is separately scoped High. The
failed UI run is retained without credit; these test corrections promote no
engine, live or external acceptance status.
