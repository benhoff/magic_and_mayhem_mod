# Idle stale CPU reader guard

`MNM_NATIVE_CPU_GUARD=1` enables an opt-in census in the native facade. Its
page-private allocations become PAGE_NOACCESS when GPU history is stale and no
CPU Lock or DC lease exposes the backing. It protects the allocation's rounded
pages, including the compatibility trailing WORD. Default launches keep the
existing permissions and admission policy.

The first matching access violation records instruction, read/write/execute
kind, thread, physical base/extent, generation and nonce, then exits124.
Module bases are cached during initialization, outside exception dispatch.
No pixel bytes, GPU/Qt operations, heap allocation, loader query, retry or renderer
wait occur in the handler. Foreign faults continue the existing exception chain.
The fixed96-byte record is a local process diagnostic, not a wire pointer ABI.

Protection follows the physical backing across flips. Selected facade writes
and private baseline capture open only their existing ordered operation window;
partial writes rearm stale authority. Full CPU fills/copies and exact native
transfer restoration can establish complete current CPU history. Invalid transfer
identity/length never opens the allocation. A stale explicitly read-only Lock
refuses in guarded mode. Ordinary writable leases are intentionally open, so the
guard cannot distinguish loads from stores inside them or inside private facade
operation windows. It does not establish safety against concurrent escapees.

This is discovery and terminal admission, not transparent CPU coherence. Actual
readers found by guarded original-game scenarios still need a safe ordered
materialization boundary, or native GPU routing. Do not invoke a readback from an
access-violation handler. Active lease reader discovery, live game acceptance,
independent presented output, and complete rendering remain separate milestones.

[The source-bound controls](native-cpu-guard-controls-20261017.json) pass20 actual
PE32 schedules and six existing default history/GPU-copy/resource/refusal
controls. The default resource run checks76330 conditions over80 five-format
lifetimes,32 clipping lifetimes,32 flip/DC lifetimes and256 palette owners, with
zero final objects. Tests disable display access and use an exclusively reserved
private Wine prefix. No original-game or presented-renderer result is asserted.

Both raw reads and writes fault at the expected instruction/access/physical
identity; wrong native replies and partial fills remain guarded. Full overwrite
and exact restoration become readable. Held flips preserve physical authority.
DC release rearms stale backing; unexpected external permissions refuse. Explicit
DLL unload removes the process exception callback before a foreign fault runs
through the surviving probe handler. The active writable-lease control explicitly
reads stale bytes successfully: that discovery gap remains real.

Reproduce with a stopped isolated prefix below this checkout's working/tests:

```sh
python3 tools/draft-coverage-claims.py --behavior NP.idle-cpu-history-guard \
  --scenario NP.idle-cpu-history-guard.controls --output working/tests/guard-claims.json
python3 tools/working-retention.py run --reserve-gib 0.2 -- \
  python3 tools/test-native-cpu-guard.py --claims working/tests/guard-claims.json \
  --wine-prefix working/tests/<owned-prefix>
```

Earlier setup/link failures and the17/18-case intermediate acquisitions remain
retained in the experiment directories. Their attempted Xvfb wrapper could not
create a /tmp lock file; those acquisitions do not prove a private displayed
session. The final20-case run is explicitly display-free and exits0. See
[acquisition provenance](native-cpu-guard-provenance-20261017.json).

## Active Lock discovery (2026-10-17)

`MNM_NATIVE_CPU_GUARD=2` additionally keeps stale backing inaccessible during
writable Locks. Selected scalar MOV stores and non-repeated STOS instructions
may execute under one owned PE32 single-step trap; that trap rearms protection
before returning to the next instruction. Loads terminate with the existing
96-byte fault record. RMW, REP, SIMD, LOCK-prefixed and unknown store encodings
are terminal candidates, not automatically classified as independent writes.
An existing trap flag, nested exception or foreign thread's trap is not resumed.
This is a single-thread discovery instrument: the temporary writable interval
cannot exclude an unobserved concurrent reader. It is not a production CPU
coherence guarantee, and it does not observe reads of a separately held GDI DIB.

The census must distinguish actual pixel reads from permission queries and
private compatibility checkpoints. Live investigation identified these barriers:

| Boundary | Observation | Route |
| --- | --- | --- |
| Scene return checkpoint | First World produced terminal110/detail2 after `resident_world_bound`; continuous checkpoints emit metadata, but required readable pixels | Keep layout checks; require readable pixels only in the branch that copies oracle bytes |
| Held Lock checkpoint at Unlock | `unlock_memory` rejected protected private write-mask storage | `NativeDrawCpuLeaseSnapshotV1` copies only an exact owned full RGB565 layout under the existing ordered bridge lease, then rearms protection |
| Later World guard | The scene's independent mandatory NOACCESS guard needs ownership of the same physical pages | `NativeDrawCpuGuardWorldV1` lends protected stale backing while the Lock is held; a newer bound history returns ownership; unfinished Unlock and Flip refuse |
| WORD raster splice9 | Native forwarding refusal family5/route9 occurred before its source-only stores because VirtualQuery saw NOACCESS | CPU-history action3 answers only the bounded protected active-Lock extent query; actual instructions remain protected |

The private snapshot layout is four process-local DWORDs: physical base, width,
height and byte pitch. It rejects other identities/layouts, wrong byte counts,
DC-held or unborrowed storage, and output overlapping any owned allocation.
Its output is a write-mask baseline, never current GPU history. A successful
snapshot does not clear stale authority. Both resident binding and full GPU-owned
Unlock checkpoints use this service; partial GPU-owned Lock snapshots remain
outside this routing scope.

Action3 accepts a nonzero requested byte count in the first scalar argument
only when the whole span lies inside protected backing under an active writable
Lock in mode2. Other scalar arguments must be zero. It returns no pixels and
never changes permissions or authorizes a CPU reader. Existing action0 remains
the current-history requirement. Selected raster, minimap, pixel, colour, panel and glyph adapters use action3 to avoid
rejecting the census's own permission state before an instruction can execute.

Live acquisition uses `tools/test-native-campaign.py --cpu-guard 2` with a fresh
private Wine prefix, the exact admitted source-game hash, native producers,
no-forwarding and bounded runtime/storage. Fault reports bind the physical
history, Lock balance, cached module bases, image RVA and staged image hash.
The handler performs no GPU work, loader lookup or renderer wait. A reader found
here still needs an explicitly ordered matching-history route before acceptance;
unknown readers remain terminal. Failed/intermediate acquisitions and source
changes remain evidence limitations, not successful live acceptance.

[Initial isolated controls](native-active-lock-controls-20261017.json)
pass28 PE32 schedules and all six default regressions. Besides the existing20
idle schedules, they execute loads after byte/WORD/DWORD stores, rearming after
Unlock/new history, terminal RMW/REP candidates, exact and rejected private
snapshots, a protected World interval, refusal to snapshot before its restoration,
and unfinished-loan Unlock refusal. The module catalog is checked independently
in all mode2 schedules. The separate broad `active_cpu_lease_reads_covered` flag
stays false: this is bounded writable-Lock discovery, not all CPU leases.

[Incoming committed history](coverage/active-lock-reader-incoming-history-20261017.json)
reviews49932e4a..ac848056:60 exact file transitions, zero unresolved non-journal
transitions. This is receipt accounting, independent of the uncommitted changes
and actual execution results.

The [retained live observation](native-active-lock-live-observation-20261017.json)
completed gameplay, Mini Cancel, Quit No, Quit Yes and original exit0 with stable
acquisition sources and immutable-input bookends. It observed36 ordered GetDC
snapshots,1082 native producer calls with zero forwards, and8 balanced World
imports/returns. The overall report remains **failed**: the ordinary64-World
requirement was unmet and no valid FPS proof was established. Twenty-seven
installed routes were unexercised; no new arbitrary stale pixel reader was found.
This report is historical after subsequent adapter/profile edits.

The explicit `--cpu-census-only` profile requires mode2, exclusive census claims,
at least one balanced World, normal Quit and the existing native/no-forwarding
admission. It retains ordinary count/FPS outcomes separately and cannot grant
ordinary campaign acceptance. [Six policy controls](native-active-lock-policy-controls-20261017.json)
pass. The subsequent scoped live attempt `run-bv6t92oq` is inconclusive because
shared sources changed during execution. The corresponding guard control run
`run-5jep6rte` passed all28 schedules and six regressions, but its source-stability
check failed too. Neither is promoted to current validation. Production coherence, concurrent readers and separate DC DIB accesses remain
open; this instrument alone is not a general coherence guarantee.

[Current stable-source controls](native-active-lock-app-controls-20261017.json)
subsequently pass all28 PE32 schedules and six default regressions. The separate
six policy tests also pass with stable source fingerprints. These results validate
the bounded headless paths; at that point the scoped live profile had no successful
current-source acquisition. No production CPU coherence was claimed.

## Bounded live follow-up (2026-10-18)

The [new census](native-active-lock-live-census-20261018.json) passes under
mode2 with an immutable source view, isolated native build outputs and private
Wine/X state. Its1464 acquisition source fingerprints still match the main
checkout when the result is registered. The original-manifest bookends, stable
staged inputs and stopped private Wine server pass. Gameplay, Mini Cancel,
Quit No, Quit Yes and normal original exit complete. Nine World imports/returns
retire without pending work;31 explicit ordered GetDC snapshots materialize
current GPU history;1047 native producer calls use zero original forwards.
There are no unknown-reader or guard/history/transfer refusal records.

The [fresh facade controls](native-active-lock-frozen-controls-20261018.json)
pass all28 PE32 schedules and six default regressions; the
[six policy controls](native-active-lock-frozen-policy-20261018.json) pass too.
They use the same frozen source view as the live acquisition.
[Acquisition provenance](native-active-lock-frozen-acquisition-20261018.json)
retains the exact source manifest and namespace launch. The earlier
`run-e_zkm99d` attempt changed a shared pixel adapter during staging and remains
failed, with successful original bookends and private-prefix cleanup.

This establishes bounded live observation and routing of the known GetDC and
private-baseline readers. It does not establish that every application reader
has been found:27 installed producer routes remain unexercised, and concurrent
readers and separately held GDI DIB reads remain outside the guard's evidence.
The ordinary64-World and FPS gates are recorded as unmet, and
`ordinary_campaign_accepted` remains false. No original pixel equivalence,
production CPU coherence or live replacement status is advanced. Unknown
active-Lock loads still terminate for separate investigation and ordered routing.

## Protected storage and ordered active readers (2026-10-18)

The new [lease coherence policies](native-cpu-lease-coherence.md) supersede the
mode2 concurrency and active typed-reader implementation gaps above. Resident
sessions default to mode3: stale public views remain inaccessible while admitted
pure stores and private snapshots use an internal mapping. Known typed readers
restore through the existing ordered FIFO, preserving byte-exact pending writes,
including same-value stores. Partial-byte Unlock restores the native companion
byte before publishing a complete WORD. DC ownership and GPU invalidation are
checked for the full lease lifetime.

[Final PE32 controls](native-cpu-coherence-joined-controls-20261018.json) pass all five
schedules and37,909 decoder checks. They exercise19 ordered restorations with38
CPU WORD updates, full/partial Locks, same-value writes, byte merges, foreign
owners, concurrent page observation, and explicit protected World begin/end.
[Native OpenGL replay](native-cpu-coherence-joined-gl-20261018.json) compares9,120,000
reader WORDs and9,600,000 presentation WORDs across20 frames; ordinary World
readbacks remain zero. [Legacy controls](native-cpu-coherence-joined-legacy-20261018.json)
pass28 schedules plus six facade regressions, and
[eight policy controls](native-cpu-coherence-joined-policy-20261018.json) pass.
[Acquisition provenance](native-cpu-coherence-joined-acquisition-20261018.json) retains
source freezes and failed/superseded runs. These are bounded native integration
results, not original pixel equivalence or proof that every raw application
instruction has been discovered. Unknown raw loads and unsupported stores remain
terminal diagnostics. Mode2 remains a historical opt-in discovery mechanism.


The [protected mode3 live journey](native-cpu-coherence-joined-live-20261018.json)
now passes14 balanced World returns,41 GetDC restorations and1,155 native
producer calls with zero forwards/unknown-reader/history/transfer refusals and
normal original exit. The shared joined-shutdown path persists exact counters
without I/O under reader/tracker/copy ownership. Live typed-Lock and byte-Unlock
counts remain zero; those paths retain the independent current PE32/GL proof.
27 routes and ordinary64-World/FPS acceptance remain outside this live result.
