# Owner visibility production (NS77)

Pinned No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Read-only exports `working/tests/visibility-ns77-export-v1` through `v4` retain
exact disassembly, pseudocode and references. Claims here cover the declared
simulation projection, not complete fog, World or rendering ownership.

## Original production

`52b540` collects active actors in pool order and selects living observers when
pool ordinal modulo `manager+4 /10` equals `manager+260`. It increments/wraps
that phase once per call. The tested capacity125 gives12 phases. Active targets
include dead actors; living health is required for observers. Visibility words
at creature `24+4*owner` decay by one only when signed-positive, except the
actor's own owner or a nonzero relation18/share20 pair. Reaching zero clears
the corresponding `48+4*owner` word. That second array is an observer-pointer
array, **not** the maintenance age array at `e0b`.

Selected observers scan active targets of another owner with visibility at most15.
Type27 targets are excluded. Nonzero target8c hides it unless target664,668 or164
is set, or the observer is type22. An observer with664/668/164 set and64c naming
the target bypasses the geometric test. Otherwise `515270` must succeed.

`508ca0` refreshes the observing owner's sighting to30, clears its observer
pointer, then refreshes each owner related through the **source observer owner's**
relation18/share20 pair. Finally it stores the observer pointer only for the
source owner. The ordinary projection does not infer transitive sharing. The
helper's explicit false-sighting operation clears only the requested owner;
that branch is not used by the admitted pool driver. Existing `51a880` ownership
refresh remains a separate previously recovered initialization/transfer helper.

The local player is-1 during the compared perception pass. This excludes
first-discovery UI/tutorial/animation consequences and the local commander's819
special reveal. The original observation-point pass `52b8a0` executes its
zero-count exit. Native frame admission invokes this projection once per normal
frame; the complete World clock, pause and other driver gates remain pending.
Neutral owners, current-owner transfers and nonempty observation points are
outside this integration. The diplomacy/sharing corpus injects matrix inputs;
script56a8d0 and its immediate reveal/notification consequences remain pending.

## Geometry and orientation

`5150a0` checks an inclusive wrapped XY square using CFG38 range. Each wrap
comparison uses the original direct distance; ties keep the unadjusted target.
`515270` tests a creature's map position before constructing its fine aim.
`5151b0` tests a fine point's unsigned shifted map XY instead. Both build the
observer origin from fine XYZ, CFG198/19c center and CFG520+12*facing offsets,
then call `4e14a0` with range*32. Type22 bypasses column attenuation.

`4e14a0` normalizes fine XY by at most one period, shifts XYZ right by2,
initializes the real `4df3a0` trajectory with multiplier8 and advances `4df500`.
XY map cells have8 subcells; Z cells have4. Terrain occupancy uses the first32
bytes of each356-byte catalog record, MSB-first X bits, row `z*8+y`. Empty
terrain with occupantffff bypasses the bit test. Cell transitions query `534490`
against6-byte XY columns: a nonzero first word and inclusive signed lower/upper
Z admit attenuation. The counter increases by global6becc6. Endpoint equality
succeeds **before** checking obstruction or exhausted budget. Identical quarter
points succeed immediately, including zero range. The admitted direct corpus
has bounded coordinates and avoids unsafe original map-layer accesses.

The new view profile exposed a previously empty `51d3a0` threat-orientation
consequence. `5084f0` subtracts the observer's fine center and uses truncated
integer-degree23/68 sectors with quadrant/direction wrapping. The native
adapter's earlier cell rounding/no-op was insufficient. The new fine-facing
projection is enabled with visibility production and independently compared;
visual attachments and extreme full-word math remain outside its scope.

## Native admission and continuation

`native-encounter run-visibility INSTALL X Y WIDTH HEIGHT SEED TICKS SNAPSHOT`
selects geometry-driven sightings, then the recovered memory/danger/navigation
pipeline. Older run modes retain their existing observation policies. New mode
loads ViewRange and24 LRCOffsets from the embedded encrypted CFG. Its ordinary
center mapping follows the static type-initialization instructions50343b..503467:
width1 gives16, width2 gives32, and height gives8*height. Full original CFG-loader
execution and error/default behavior are not claimed. The installed type17
profile has range15, center32/16 and zero LRCOffsets.

The portable CMD1/v11 continuation retains phase, sharing flags, definitions and
logical observer IDs. It checks lengths/references/bounds and requires a fresh
knowledge/danger activation. Complete native frame publication remains
transactional. Older versions decode absent; lossy writes refuse. Saved
continuations consume embedded inputs without installed files or a PE process.

Native geometry currently admits **empty attenuation columns and an empty object
pool**. Direct helper comparisons exercise authored nonempty columns; production
of installed static-object columns is pending. Installed terrain collision bits
are used directly. Current per-owner visibility drives memory retention and
expiry, danger rows and downstream decisions. This is distinct from rendered
sprite clipping and complete explored-map/fog production.

## Validation boundary

The prospective NS77 execution compares96 authored24-actor fixtures over192
scan ticks each, plus24,576 direct ray, point-wrapper and fine-facing cases.
Authored masks change during scans. The fixture checks complete original actor
bytes and observer references, while native64 and ASan/UBSan/LSan results agree.
Three malformed pool capacities are native-only refusals.

Four installed32x32x23 ordinary-type17 journeys with seeds17/91 exercise travel,
hidden status/reappearance/death, diplomacy/shared-vision changes and combat.
Each stage independently advances original and native simulation, then replays
complete traces/final saves on native64 and sanitizers. Checkpoints include80
fresh processes and regular in-process restore. These are retained fixed-actor
headless scenarios; raw pointers, presentation, arbitrary species/status/form,
strategic tasks, full World/campaign and live replacement remain excluded.

The first development encounter caught a direction mismatch after perception.
A hardware watchpoint identified the original51d3a0 ->5084f0 write, motivating
the fine-center orientation dependency and its direct corpus. Earlier development
runs are not current evidence. The final source-bound run is recorded separately;
shared-source changes leave prior milestones' evidence historical.

Reproduce with `tools/test-owner-visibility-integration.py --install INSTALL
--build-root NEW_OR_REVIEWED_BUILD_ROOT --claims PROSPECTIVE_CLAIMS --output NEW_RUN`
under the main checkout's storage guard. The component-only runner is
`tools/test-owner-visibility.py`; original text remains RX and has no redirects
there. Composed journeys retain the explicit older presentation/resource/allocator
endpoints; new perception/geometry/facing bodies execute unmodified.


## Recorded execution

The immutable [NS77 report](owner-visibility-bound-ns77-20261016.json) records
7,170,048 component checks across18,432 scan ticks, with24,576 direct cases each
for rays, point wrappers and ordinary fine facing. All eight installed coupled
journeys pass18,432 paired ticks. Complete native64 and sanitizer replays match
their per-tick traces and final snapshots. There are2304
in-process journey restores and80 cold-process checkpoints. The native visibility
state suite passes4621 checks,
288 round trips and
10 refusals. Both core CTest suites,
installed encounter regressions and prior native knowledge/danger state suites
pass; CLI whole/split2304-tick runs produce identical saves with installation-free
resume. All283 declared sources,
258 compiler/configure inputs, five installed inputs
and2927 originals are stable within their stages. Original execution completed
in the preserved v1 engine stage. V2 verifies all retained artifacts, eight
embedded input sets, matching prospective scope and unchanged engine/paired
sources; it freshly completes the selected core suites and CLI checks. Only the
validation driver changes, and v2 does not repeat original execution.

Both travel seeds occupy their goal for1760 frames. Hidden-status journeys have
1338 hidden frames and1023 remembered-hidden frames before expiry, with one death.
Other geometric loss and reacquisition happen during travel. The synthetic
corpus has source/target owners0..3 and sharing flags for all eight viewers.
Journey goal counts are occupancy frames; hidden/remembered counts are frame
witnesses rather than distinct events. Exact combat and danger-row outcomes for
each seed remain in the report. This evidence promotes only the three declared
NS77 contracts; shared historical evidence retains its original hashes and may
be stale. Full game execution still requires Wine.


The immutable engine-stage report and exact superseded driver are preserved as
`owner-visibility-engine-stage-ns77-20261016.json` and
`owner-visibility-retained-sources-ns77-20261016.json`. Recovery admits only the
non-engine driver change and verifies the prior scope/build/claim set, complete
source-key set, every cached artifact, all replay/final-save bytes,80 checkpoint
outputs and eight sets of actual embedded input bytes. The failed driver checked
its computed compiler/configure closure but persisted that narrower map only on
success; v2 reconstructs it from retained preprocessor logs and unchanged CMake
source declarations. The final report marks this provenance explicitly. Five
negative guard checks refuse scope, engine-source, source-closure, input and
artifact changes; raw results remain under
`working/tests/visibility-ns77-accounting/reuse-negative-guards.json`.

Default reproduction executes the full corpus with the corrected five-test CTest
selection. `--reuse-engine-run` completes only the exact retained failure after
these checks; it is a source-bound composition, not fresh original execution.
The direct fine-facing corpus does not separately witness coincident-center
retention; that branch is statically recovered. All reported execution counts
remain limited to their explicit input corpus.
