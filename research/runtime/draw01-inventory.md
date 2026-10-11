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
