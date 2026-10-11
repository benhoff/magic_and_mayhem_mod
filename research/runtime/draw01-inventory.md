# DRAW-01 scenario-tagged drawing inventory

The machine-readable [route register](drawing-inventory.json) implements DRAW-01
accounting for the pinned No-CD build. It records individual entry routes,
aggregate producer operations and original queue kinds separately. Each route
has stable IDs, behavior links, a subsystem owner, classification, explicit
support scope, required side effects, confidence and remaining gaps.

**DRAW-01 is not globally complete.** The route checker is implemented; fourteen
required scenario censuses, unclassified kinds, alternate/default dispatch and
unattributed callers remain visible. Existing bounded captures do not prove that
every original pixel writer was instrumented. The immutable original binary
inventory and all historical evidence hashes remain unchanged.

## Reproduce the accounting

```sh
python3 -B tests/test-drawing-inventory.py
python3 tools/report-drawing-inventory.py --require-accounted \
  --json working/tests/draw01-new.json --markdown working/tests/draw01-new.md
python3 tools/report-drawing-inventory.py --require-complete
```

The first report command fails for newly observed identities or producer hook
entries that lack reviewed routes. `--require-complete` additionally fails while
required scenario censuses, dispatch tables or support decisions remain open.
Report files must be new; the reporter refuses writes under `original/` or
`.git/`, including symlinked roots. It reads repository evidence without opening
original game/media artifacts and performs no binary changes or game execution.

Eight retained observation cohorts seed the inventory: sixteen-queue startup
producer operations and raw input kinds, the later four-return producer shadow,
the guarded sixteen-queue exact-entry World batch comparison, glyph shadow and
takeover cohorts, both enabled native intros, finite minimap motion and the
later 25-call guarded fade-loop takeover. The
scenario labels are bounded evidence collections. They do not imply that every
required scenario, phase, wrapper or caller was exercised. Operation totals never
become entry totals; independent cohorts are not summed into runtime frequencies.

The retained static export supplies twenty-four selected instruction windows,
all their direct caller sites, indirect calls and computed jumps. The checker
requires a route for every window and every current producer-hook entry, plus the
separate interior fade splice. An interior entry must name an explicit registered
original anchor and lie within that function's pinned discovered ranges; it does
not become an additional discovered function. Exact
CALL bytes determine a return PC; no nearest-function or nearest-PC assignment
is made. The historical `shared_sprite_dispatch` window at `581ec0` is labeled
as the recovered SFT glyph route. The unassigned `58d3b0..58d4a7` WORD-copy window
remains unassigned. Static call counts and instrumented entries are never live
activity evidence. Native support decisions are scoped to linked research;
they do not promote central implementation/comparison/replacement statuses.

## Expanded exact caller census — 2026-10-10

The supplemental [caller export](drawing-callers-expanded-20261010.json) covers
all 61 registered entry identities across the pinned executable disassembly,
retaining 909 exact rel32 CALL sites. It complements the historical 24-window
export without replacing it. The importer verifies each CALL's bytes, target
and exact return PC, accepts only registered targets, and rejects conflicts.
Discovered owners come only from containing recorded function ranges; empty
owner lists remain empty. Instrumented indirect calls still require observed
callee identities and separately reviewed ownership.

Against the diagnostic preferred-base interpretation of the retained four-return
journal, 53 of its 63 previously unattributed caller/operation groups have an
exact static match; ten raster groups still lack one. This is a static crosswalk,
not fresh process-base evidence or semantic runtime attribution. Historical
journal evidence retains its original hash and 63-group result. Fourteen scenario
closures, unknown writers and incomplete dispatch remain pending.

Reproduce in a new output file:

```sh
python3 -B tools/export-drawing-census.py --executable <pinned-Chaos.exe> \
  --output <new-report.json>
python3 -B tests/test-drawing-census.py
python3 -B tests/test-drawing-inventory.py
```

The full authorized work targets all fourteen required single-player journeys,
including sustained battles, interactions, transitions and shutdown. Incremental
commits do not close this milestone until scenario, caller, graphics-side-effect,
failure/lifecycle and dispatch evidence supports the strict completion gate.

## Original-active call metadata — 2026-10-10

The observation-only `runtime/drawing-census/` hook and V1 wire journal retain
actual callee entries, thread, incoming return address, paired return, unsigned
EAX, flags/LastError, six diagnostic argument words, selected drawing globals and
host scenario phases. The loader requires the actual module base `0x400000`,
checks all selected original signatures before mutation, and relocates copied
relative instructions. Staging checks the pinned executable hash and preserves
every original file-backed section; added DLL imports live in disposable files.
The original drawing bodies and menu callbacks execute. There is no native
rendering, original-body suppression or balance change.

The current build accepts **60 of 61 catalog sites**. The fade interior
`58edb0` cannot safely take a six-byte entry splice: the original branch at
`58edde` targets `58edb1`, inside those bytes. The first live attempt exposed this
and is retained as failed evidence. The builder now refuses incoming direct
branch/CALL targets inside a splice; it does not assert that indirect entry into
all copied bytes has been excluded. Fade wrapper `58ed80` remains observable.
An interior checkpoint records no guessed caller or paired return.

The fresh scope-bound menu repeat retained 83,178 events / 41,589 calls at 18
entries. Its result is registered as
`DRAW.census.menu-observation-20261010`; the journal/ABI/guard result is
`DRAW.census.journal-guards-20261010`. Neither result closes a scenario census.

The synthetic PE32 fixture executes the actual assembly and callbacks against
callee-pop 0/8/24-byte contracts, comparing full register/flags/x87/SSE snapshots
and LastError. It checks same-stack tail-call unwinding, interior forwarding,
recorded errors and frame-exhaustion forwarding with explicit loss. The journal
parser rejects malformed build/base/tag/extent/count/reserved fields, wrong
thread/stack/callee/phase returns, loss, open calls, IO failure and absent terminal
markers. Host phase labels express fixture intent, not scenario completion.

The menu fixture traverses Main -> Preferences -> Main -> Quick Battle -> Single
Player setup -> Map selection -> cancel back through Main -> original quit. A
closed diagnostic run retained 94,256 events / 47,128 calls at 20 entries in 112
entry/caller groups. These are that cohort's counts only. Empty discovered-owner
sets stay empty. Physical return addresses may include observation trampolines
for tail calls or the separate menu action bridge; they require explicit
call-flow classification before semantic attribution. Global snapshots identify
possible state changes; they do not inventory every side effect or pixel writer.

```sh
python3 -B tools/prepare-drawing-census.py --source-game <pinned-installation>
python3 -B tools/capture-drawing-census-game.py <fresh-experiment> \
  --prefix-template <stopped-Wine-prefix> --journey menus
python3 -B tools/test-drawing-census-abi.py \
  --prefix-template <stopped-Wine-prefix>
python3 -B tests/test-drawing-census-trace.py
```

Use a fresh experiment for each journey. The fixture owns a private Xvfb display
and Wine prefix, maintains the menu host heartbeat, retains failed reports and
checks staged binaries. A successful terminal proves only bounded journal
integrity: loss/open/IO counters must be zero. Busy loader detach does not wait on
terminated workers and cannot produce a valid terminal. Dynamic unload is
unsupported; partial hook installation refuses startup. Capture overhead has no
hard timing promise. Movies, all writer/API/borrowed-access routes, failures and
recovery, save/load, later map transitions, results and full dispatch still need
separate evidence. DRAW-01 remains incomplete.

## Inspect an existing producer journal

The optional importer reads only 64-byte envelopes and 96-byte record headers,
skipping source payloads. It validates V1/V2/V3 build/version/bounds, contiguous
record and World-return sequences, payload extents and a matching successful
`MNMPDONE` marker. It requires the journal SHA-256 recorded by the capture, checks
both inputs stayed unchanged, and preserves unknown opcodes and caller PCs.
Source assets and destination-oracle pixels are not opened.

```sh
python3 tools/report-drawing-inventory.py \
  --producer-stream <capture>/canvas-producers.bin \
  --done <capture>/canvas-producers.done \
  --stream-sha256 <recorded-journal-sha256> \
  --stream-scenario quick-battle-entry \
  --runtime-image-base <observed-process-module-base> \
  --json working/tests/draw01-journal-new.json
```

The runtime module base is explicit; the tool does not infer it from raw pointer
values or assume it persists across launches. Verify it against the captured
process before using normalized caller identities semantically. Exact static
callers may identify an entry. Other callers remain unattributed even when the
aggregate operation is understood. The importer checks journal accounting, not
the original success semantics of every drawing call or complete instrumentation.

## Completion and ownership

Scenario completion requires a separately retained live census report with an
explicit reviewed closure: complete caller attribution, required side-effect and
owner/classification inventories, zero lost events and zero unknown routes,
bound to the exact route register digest. A complete producer journal or matching
pixel corpus alone cannot supply this closure. A closure must explain how paths
outside the producer hooks, borrowed DC/Lock intervals and indirect/alternate
dispatch were accounted for. Updating the route register invalidates that closure
until it is reviewed against the new census. The initial fourteen scenarios have
no such closure; no new original observation is asserted here.

Rendering-side state/readbacks, allocation/generation transitions, original
caller results and retry/failure branches are inventory obligations alongside
visible pixel producers. The owner fields name repository subsystem boundaries,
not host pointers or named contributors. Unknown caller ownership, opaque import/
COM targets and unclassified table/default branches remain pending research.

Use the [coverage register](coverage/register.json) for behavior/evidence status,
the [ledger](coverage-ledger.md) for achieved scope, and the
[replacement plan](../../docs/live-drawing-replacement.md) for sequencing. This
route register complements the rendering API support report; neither replaces
the other or claims an overall completion percentage.
