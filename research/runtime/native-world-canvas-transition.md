# Resident World canvas transitions

The prechange `CanvasWorld::beginResident` explicitly requires the same canvas
ID, borrowed GPU surface and extent for the whole session. This is a confirmed
code-level admission boundary, not proof that ordinary map transitions recreate
that canvas. Three cold Initiate storage journeys retained canvas2. Older
DrawingScene lifecycle tests exercise a different native path.

The bounded live scenario under investigation leaves the saved map normally,
applies the original Main Preferences resolution rebuild to640x480 and back
to800x600, then loads that same saved map through the original Main Load callback.
It must prove a different owned World canvas ID and fresh source generations,
without changing saved bytes or bypassing original display/storage callbacks.
This is a display-rebuild plus same-map reload scenario, not proof of a later
campaign-map transition or general pixel equivalence. The first live attempt fails during the first display rebuild, before Main Load.
The source-bound negative records below distinguish this failure from the later
World identity guard.

## Confirmed display history boundary

The [first live probe](world-canvas-display-history-refusal-20261015.json) completes
its initial storage journey but stops after the first original Preferences OK.
The lifecycle trace records `blit_untracked` and `blit_commit_refused` on the
background thread at caller0x58c488, then a session history gap. Both surface
objects exist; the source primary has metadata but no current CPU snapshot.
The later native-control failure in the flow is a consequence, not its cause.
Declared sources remain stable, original manifests verify before/after, and
private Wine is stopped. These failed runs do not reach the final staged-input
fingerprint check.

The [first initialization attempt](world-canvas-primary-descriptor-refusal-20261015.json)
also refuses: it requires a complete creation descriptor, whereas an original
primary descriptor supplies only caps. No initialization diagnostic is recorded.
The corrected policy obtains dimensions and RGB565 authority from the owned
facade's existing CPU identity service at creation, before exposing the result.
It requires exact current unborrowed ownership, zero generation/nonce, bounded
pitch/dimensions and all zero logical words. Padding is excluded from image words.
Only the just-created simple primary is eligible. Driver-backed, stale GPU,
nonzero/unknown and later CPU snapshots remain excluded. This records the
facade's existing VirtualAlloc zero policy; it is not original-driver allocation
equivalence and does not authorize a private write-mask baseline as image history.

## Resident scene continuation

The source-only prechange probe reaches the `Resident World identity or sequence`
refusal on its first valid new canvas. The diagnostic binary and logs are pinned
under `working/tests/resident-canvas-prechange-20261015`; its source was subsequently
extended with context accounting, so it is not fresh validation of current code.
An initial control incorrectly expected an ordinary resident lease to freeze
while the same owner deliberately wrote new Worlds. The corrected control follows
the existing shared-owner contract and checks retention when physical allocation
is released/recreated. No historical lease behavior is reclassified as a defect.

`beginResident` now validates a new borrowed scene only between closed queues,
retains the global contiguous source ordinal and discards old prepared/retained
baselines when the canvas, surface generation or extent changes. Construction
checks the new native surface and defined history before retiring the old scene.
No original pixels initialize the replacement. ID changes and scene-context
changes are counted separately, since recreating one ID can change its allocation.
Sequence gaps, incomplete queues and undefined destinations remain terminal.

The independent CPU-verified synthetic sequence covers seven queues in each of
resident, prepared replay and retained-image modes: repeated history, a new ID and
extent, return to retained history, same-ID/same-size recreation, same-ID resize,
and return to the other retained canvas. Old physical leases survive recreation,
prepared replay uses the correct baseline, ordinary World readbacks stay zero and
all surfaces retire. Existing512-queue, prepared and retained regressions run
separately. The first runner invocation passes its execution controls but rejects
its own missing declarations for two regression sources; final validation must
use a new source-bound result rather than repairing that old declaration.

Live changed-canvas acceptance remains pending until a report contains a different
first/current World canvas, at least one context rebuild, a fresh fourth source
checkpoint, original640x480/800x600 leave observations, Main Load and normal Quit.

## Source-lane initialization and bounded prefix

The [next live probe](world-canvas-source-history-refusal-20261015.json) passes both
original display rebuilds. Main Load then encounters an undefined new source
canvas before a context replacement. Fresh native allocation history must be
represented in both the command tracker and the independent source producer.
The source producer now checks the owned allocation at wrapper creation and emits
an existing FILL operation with zero value and policy marker0x4e495a31 in word22.
This is explicitly native initialization, not an observed original FILL call.
The existing undefined-destination guards remain in force for other histories.

The process-local identity/zero checks are shared from
`runtime/native-draw/initial_history.h`. Historical evidence keeps its old source
key and hash with an explicit binding from the earlier render-local path; no old
result is refingerprinted. Successful source admissions are retained at shutdown
in `owned-initial-history-0000.bin`: magicMNMINIT1, version1 and count in a16-byte
header, followed by at most512 rows of canvas ID,width,height,policy marker.
Only fully written rows enter the published count; initialization emits no
per-draw diagnostic disk I/O. The journey requires both old and new World IDs in
this trace before it can pass.

One [source-lane attempt](world-canvas-prefix-budget-refusal-20261015.json) reaches
the existing131,072-record prefix limit before the first Load, at generation170.
Its pending prefix contains109,416 glyph records,5,933 checkpoints,5,487 clips,
5,486 binds,2,135 copies and smaller source/resource groups. The source byte budget
is not exhausted. The producer stops safely; terminal cancellation is not treated
as a successful reload. This exposes a bounded menu/loading capture limit rather
than a proven World identity failure. No bound was raised for the repeat.

A repeat reaches canvas2→86 with382 balanced completions/imports/returns and
original normal exit0, but its [10FPS session-floor result](world-canvas-session-fps-refusal-20261015.json)
is failed at9.2123FPS. That aggregate spans the resolution-menu interval. It is
retained as a performance failure, with no hardware or60FPS claim. The next
functional run explicitly selects5FPS. Its [old checker result](world-canvas-main-load-checker-refusal-20261015.json)
is also retained as failed: Main Load initializes World rather than calling the
Mini-resume method, and the startup World-tick sample budget is already exhausted.

The corrected Main Load check requires a returned original storage callback at
Main depth3, the acknowledged Main parent, a later original initialized campaign
Mini (exact object/vtable, mode2/caller5, active owner and depth5) before Quit,
and the fourth checkpoint's fresh captured original World generations. In-game
Save/Load still require their original World-resume event between callbacks.
Portable controls reject a wrong Main parent or Mini caller, absent display
rebuild, wrong dimensions, stale checkpoint and missing second Quit confirmation.
A read-only check of the old completed trace passes these predicates, but does
not convert its failed execution report into current validation. A new run is
required. Initialization/source-prefix bounds remain enforced; the source-side
zero policy admits at most512 fresh allocations per process and does not assert
unbounded resource churn.

## Current bounded live acceptance

The [new source-bound functional run](world-canvas-reviewed-functional-live-20261015.json)
passes the corrected checks, with original manifests verified before/after,
unchanged declared sources/staged inputs, unchanged saved bytes/owned-store merge,
and private Wine stopped after original exit0. It requires a genuinely changed
World ID/context, four ordered fresh source checkpoints and both World IDs in
the initialization witness trace. There are no optional pixel-producer forwards,
history refusals, pending ownership, native recovery or visible original fallback.
Ordinary World/native/RGBA readbacks remain zero; explicit existing CPU-reader
materialization is separately counted. This is a same-map reload after original
display rebuild, not a later campaign-map journey or complete original pixels.

The [final111-input controls](world-canvas-reviewed-controls-20261015.json) and
33 portable campaign/storage checks also pass against this code. Comparison and
replacement remain none. The10FPS session metric and131,072-record loading-prefix
failure remain open boundaries; one successful functional journey does not prove
reliable unbounded loading or resource churn. Older evidence and failed attempts
retain their hashes and outcomes. The [incoming history review](coverage/world-canvas-incoming-history-20261015.json)
covers a5cd6514..2022359a (20 exact files,150 behavior fingerprints, no gaps).

```sh
python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-native-campaign.py --recreate-world --difficulty 1 \
  --resident-world --other-pixel-producers --legacy-audio --skip-movies \
  --min-fps 5 --timeout 480 \
  --source-game working/tests/pixel-staging/run-qeshupqr/source-game
python3 tools/working-retention.py run --reserve-gib 2 -- \
  python3 -B tools/test-native-canvas-transition.py
```

The final run changes canvas2→90 with one context rebuild and438 matching
completions/imports/returns. Its checkpoints are generations47/117/252/383.
It executes12,288 optional native calls with zero forwards/history refusals and
retains153 verified native source initializations. Measured aggregate engine FPS
is10.3258 under the declared5FPS functional gate; this single timing does not erase
the earlier10FPS failure or establish consistent performance.

The final ownership review routes source initialization through the process-local
`RenderOwnedInitialZeroV1(object,width,height)` service. The service holds the
existing callback and copy leases while checking the facade registry, identity
and zero words. It returns scalar proof only; a CPU pixel pointer never escapes
the lease. The source hook still calls it solely at fresh wrapper creation.
This serializes lookup with intercepted surface creation/release/copy operations.
The earlier passing run remains historical after this tightening; current
acceptance requires a new source-bound run of the same journey and controls.

## Failed native copy results

Two serialized-query attempts reach a display transition where an original cursor
background-copy wrapper returns native HRESULT0x88760096. The [retained source
witness](world-canvas-copy-rectangle-witness-20261015.json) pins the closed channel
and exact record17950: source81 is640x480, requested rectangle(371,556)..(411,596),
destination85, caller0x4a4c26. The owned facade preflights this rectangle before any
pixel write. Earlier source replay ignored the captured result and attempted the
invalid copy, producing the [recorded failure](world-canvas-copy-rectangle-refusal-20261015.json).

Public owned-facade source capture now marks this exact NS_RECT result, and the
consumer retains the record as a no-op with a separate counter. It does not mark
the destination dirty or emit a mutation checkpoint. Normal/unmarked copies keep
their strict rectangle/history checks; malformed marked status/payload pairs and
unknown markers refuse. This is a scoped native-facade result contract, not a
claim about all original drivers or unclassified HRESULTs. See the [wire and
initialization format](../formats/owned-initial-history-v1.md).

Controls verify unchanged destination pixels for the marked refusal, strict
unmarked out-of-bounds rejection, wrong status/marker rejection and the positive
wire round trip. An intermediate implementation passed direct replay controls but
missed the envelope's reserved-word guard; its live run correctly failed with
`Invalid producer record`. The corrected decoder explicitly admits only the
closed V3 marked form. Earlier consumers still reject that field, so they cannot
silently apply the new no-op record as a normal copy.

## Final code acceptance and remaining boundaries

The [final lifecycle run](world-canvas-complete-functional-live-20261015.json)
passes after the serialized query and refusal-record decoder changes: canvas2→90,
one context rebuild,463 matching completions/imports/returns, source checkpoints
52/129/266/409,153 initialization witnesses and16,709 optional native calls.
Forwards, history refusals, pending ownership and ordinary World/native/RGBA
readbacks are zero; original Quit exits0, private Wine stops, original manifests
verify and staged inputs/sources remain unchanged. The declared functional floor
is5FPS; measured aggregate is10.8309FPS. The earlier10FPS failure is retained.

This capture has zero rectangle-refusal records. Therefore
`NP.native-source-copy-refusal` has partial implementation and headless
wire/consumer support from the [current111-input controls](world-canvas-complete-controls-20261015.json),
including the five refusal cases and valid round trip. Live execution of the
accepted live consumption of a marked refusal remains pending; its prospective claim is intentionally
not linked as successful live support. The lifecycle/initialization, startup and
bounded storage scopes have current live evidence. Current combined-rebuild proof
uses Apprentice; older Initiate execution keeps its historical source hashes.

Known remaining limits are the131,072-record loading/menu prefix, consistent
session performance, live classified-copy branch coverage, other/unclassified
HRESULTs, later maps, arbitrary resource churn, hardware and complete presented
original-pixel/restored-simulation equivalence. None is converted to a passing
result by this change. Source rectangle validation remains strict outside the
explicit recorded native refusal form.

Live World drawing in this proof remains800x600. The640x480 interval exercises
Main menus and display reconstruction; other World extents have synthetic support
only. Native zero initialization is an intentional facade policy, not recovered
original allocation equivalence. No gameplay balance or simulation behavior is
changed.

The [intermediate producer-marker witness](world-canvas-copy-marker-witness-20261015.json)
contains the actual NS_RECT marker emitted before the old reserved-word decoder
refused it. The producer source hash still matches current code. This establishes
that the marking branch was observed; it does not turn that failed session into
successful live replay. The remaining branch boundary is consumption of a marked
refusal in an accepted live journey under the corrected decoder. Current
component/wire tests cover that handling, while the final accepted live run has
counter0.
