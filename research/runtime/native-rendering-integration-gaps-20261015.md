# Native rendering integration work remaining

Reviewed 2026-10-15 against committed tree `76a75c0c`. Native World delivery,
combat/menu integration, GPU presentation and combined pixel-producer staging
already exist. Complete drawing replacement still needs the work below.
Several remaining items are missing acceptance evidence, rather than missing
rendering implementations.

The [code review record](native-rendering-integration-review-20261015.json)
retains 25 exact source excerpts with file hashes and line numbers, executable
policy checks, guard-test results and the committed-history inspection.
References below use line numbers at that commit; named functions are the
stable navigation points. Concurrent uncommitted native surface, palette and
clipper changes are excluded from implementation conclusions and are identified
by hash in the record. Those changes need their own integration validation.
No original game or graphics driver was executed for this review.

## Corrections to the earlier integration assessment

| Earlier assertion | What the code actually establishes | Remaining boundary |
| --- | --- | --- |
| Enable the replacements together | `selection()` already selects seven producer switches; public preparation stages them with the separate glyph adapter. | Execute and verify the combined installed hooks through gameplay. |
| Keep frame history coherent | Resident World ownership, CPU delta overlays, generation checks and deferred secondary writes already exist. | Establish complete CPU/GPU history dependencies for every admitted reader/writer. |
| Integrate whole-session transitions | Combat, spells, portrait stress, three menus/resumes and normal Quit have a retained passing journey. | Expand independently compared journeys to save/load, later maps, results and broader inputs with the combined routes. |
| Implement recovery | Generic command recovery and cleanup already exist. | Resident World explicitly refuses in-place recovery and requires a fresh producer. Validate the chosen restart/termination policy. |
| Compare complete sessions and fix the intermittent mismatch | Independent component/startup comparisons exist. | Complete the presented-session corpus. The historical intermittent mismatch has not been reproduced by this review; it is not established as a current defect. |
| Add a complete replacement flag | Existing native, resident and producer flags select partial capabilities. | Add a strict complete-mode contract only after its declared routes and scenarios pass. |

The [October 14 integration result](native-render-master-integration-20261014.md)
is evidence that gameplay integration exists: 5,719 balanced native World
completions and approximately 59.4 visible native World FPS in its recorded run.
It does not assert full original-pixel equivalence. Earlier prose limiting all
continuous World delivery to 16 or 32 startup queues is superseded for this
resident path; those limits belong to older bounded comparison experiments.

## RENDER INT 01 Validate the combined producer routes in the gameplay harness

**Open integration and observability work.**

Positive code witnesses:

- [`selection()`](../../tools/borrowed_pixel_backend_launch.py#L31) expands
  `MNM_OTHER_PIXEL_PRODUCERS=1` into panel, image/effect, JPEG, minimap,
  colour-rectangle, alternate-raster and DC-copy switches. Explicit individual
  zeroes win. Glyph selection is separate.
- [`prepare()`](../../tools/prepare-menu-observer.py#L90) stages those adapters
  together and adds the resident scene/glyph adapters. This wiring is present.
- [`test-pixel-staging.py`](../../tools/test-pixel-staging.py#L145) calls public
  resident preparation and authorization, then records staging success. That
  branch does not launch the staged process or inspect installed-hook execution.
- [`clean_environment()`](../../tools/test-native-campaign.py#L312) removes every
  inherited `MNM_*` variable. The campaign runner has no
  `MNM_OTHER_PIXEL_PRODUCERS` literal or explicit option restoring that group.
  Passing it only in the parent environment cannot exercise the group through
  this runner. The review executed both `selection()` and `clean_environment()`
  to confirm this behavior.
- [`validate_native_draw()`](../../tools/test-native-campaign.py#L321) checks
  native facade staging, module absence, refusal/status and resource bounds.
  The [resident acceptance block](../../tools/test-native-campaign.py#L741)
  checks completion/import/return balance and ordinary-readback absence. Neither
  provides a complete per-producer native/forwarded-call census.

Work to close this item:

1. Give the campaign harness an explicit combined-producer mode; restore only
   its authorized settings after environment cleanup. Declare all participating
   build/source dependencies before execution.
2. Verify the manifest, installed hooks and effective mode after process launch.
   Collect per-route admitted, natively executed, refused and originally executed
   counts. Existing counters can be reused where available; for example
   `dc_copy_bridge.c` exports native/original counters, while `raster_bridge.c`
   currently keeps per-site arrays without a production export.
3. Run startup/loading, combat/effects, camera and minimap changes, pause/resume,
   menu return and normal Quit with independent checkpoints. Distinguish
   unexercised routes from passing routes and every original forward from bypass.

Acceptance: the combined mode has an actual source-bound execution report and
independent output/caller checks for each route it claims. A successful stage
alone cannot close it. Relevant contracts include `NP.pixel-producer-launch`,
`NP.glyph-production-route`, `NP.panel-loop-launch`,
`NP.outside-world-raster-admission` and `NP.owned-dc-copy`.

## RENDER INT 02 Close CPU and GPU frame history dependencies

**Open ownership boundary, with substantial existing implementation.**

The resident GPU World and the PE32 CPU compatibility plane are deliberately
different representations:

- [`continuous_finish()`'s resident branch](../../runtime/scene/world_continuous.c#L201)
  accepts an acknowledgement, restores the original page protection and binds
  the World identity. It explicitly performs no pixel copy back to the CPU.
- [`ns_lock()`](../../runtime/native-draw/surfaces.h#L56) returns the CPU
  `NsState::pixels` allocation. [`NativeDrawPlaneV1()`](../../runtime/native-draw/factory.c#L107)
  resolves that current owned allocation and rejects active DC leases and
  unknown/retired allocations. It does not retrieve resident GPU pixels.
- [`session_resident_unlock()`](../../runtime/render/owned_session.h#L383)
  compares CPU before/after bytes and emits masked updates over the resident
  surface. This is an existing solution for changed CPU overlay pixels, not a
  promise that the CPU plane contains the completed GPU World.
- [`mnm_raster_route()`](../../runtime/canvas-writers/raster_bridge.c#L17)
  uses the borrowed CPU plane and current original workspace. Destination-reading
  operations therefore need an explicit history argument when composed with
  resident World drawing.
- [`mnm_world_defer_admitted()`](../../runtime/scene/world_deferred.h#L8)
  allows selected independent secondary operations, but rejects World source or
  destination access and global binds. Its 512-record/4-MiB limits remain
  enforced. [`batch_fault()`](../../runtime/scene/world_raster_batch.c#L14)
  terminates on access to the protected World allocation during the guarded batch.

Work to close this item: enumerate actual readers and writers in the combined
journey, determine which require completed World pixels, and give each such
dependency a valid owned representation or an explicit capability refusal.
Preserve ordering across blend destinations, partial writes, cursor save/restore,
DC copies and interleaved secondary work. Do not remove the current guards or
assume that readable CPU memory is a current rendered World.

Acceptance: dependency-sensitive pixel comparisons pass with original raster
bodies bypassed; accepted reads observe the required version; unknown or
unsupported dependencies end the declared replacement scope before use.
Relevant contracts include `NR.public-resident-world`,
`NR.world-raster-batch-transport`, `NP.owned-plane-lookup` and
`NP.outside-world-raster-admission`.

## RENDER INT 03 Define strict admission and original forwarding policy

**Open complete-mode contract. Partial-mode fallback is implemented.**

These are executable original-code continuations, not merely TODO comments:

- [`pixel_entry.S`](../../runtime/canvas-writers/pixel_entry.S#L20) branches to
  `mnm_pixel_original` when native admission returns zero.
- [`dib()`](../../runtime/canvas-writers/pixel_bridge.c#L79) calls the original
  imported DIB operation when the native route returns its refusal sentinel.
- [`copy()`](../../runtime/canvas-writers/dc_copy_bridge.c#L9) increments
  `original_calls` and invokes the original BitBlt import after native refusal.
- [`mnm_raster_route()`](../../runtime/canvas-writers/raster_bridge.c#L63)
  records per-site forwards on refused thread/input/storage admission.
- [`nd_init()`](../../runtime/native-draw/surfaces.h#L197) initializes method
  tables to refusal handlers and overwrites admitted slots.
  [`nd_terminal()`](../../runtime/native-draw/refusal.h#L4) records the unknown
  method and terminates. This is already explicit failure, not silent fallback.

Work to close this item: define which routes a complete session requires, check
that capability set before launch, and bind every refused/originally executed
pixel path to the session outcome. Existing partial modes may keep their forwards;
a complete-mode result must stop or explicitly end its claim before original
pixel production resumes. Cover encountered unresolved dispatch and API routes;
do not equate every unimplemented graphics method with a known required game path.

Acceptance: a deliberate unsupported pixel route cannot leave a successful
complete-replacement result, and the report accounts for every admitted required
route. Retaining original simulation, source preparation, wrappers and required
side effects is allowed. Rewriting whole original consumers is not itself a
completion requirement. `NR.live-drawing-replacement` owns this release policy.

## RENDER INT 04 Extend scene lifetime and transition coverage

**Existing functionality needs broader combined-session evidence.**

[`CanvasWorld::beginResident()`](../../compat/legacy/canvas_world.cpp#L138)
requires contiguous generations, the same resident surface/dimensions and a
consistent canvas identity. It rejects incompatible reuse. That guard does not
prove that save/load or map transitions are broken: a transition may preserve the
identity, or orchestration may create a fresh session. It identifies the concrete
handoff that must be exercised when identities or resources change.

The executable [journey requirements](../../tools/drawing_journeys.py#L12)
include save/load/restored World, leave/load/next map, victory/defeat, camera
boundaries, attachment/lighting/blending, tooltip lifecycle, hotspots/picking,
movie completion/skip and cleanup. Native menu storage dispatch already exists
in [`LiveMenuSession::requestStorage()`](../../apps/qt-shell/live_menu_session.cpp#L154);
its presence does not validate reconstructed rendering after load.

Work to close this item: run those transitions with the combined producer mode,
record surface/resource generations and retained/retired GPU leases, and compare
the resulting scenes and relevant input outcomes. Where a canvas changes,
validate the fresh-session handoff instead of bypassing identity checks. Verify
enabled movies and return to active World in that same declared session scope.

Acceptance: the required transition starts and ends with coherent native frame
ownership, no stale resources, correct picking/hotspots and independent scene or
semantic checks appropriate to the declared native UI policy. The existing
combat/menu success remains valid historical progress; this is an expansion.

## RENDER INT 05 Validate resident failure and restart behavior

**Directly open in code. Generic recovery must not be counted as resident recovery.**

[`LiveCommandSession::recover()`](../../apps/qt-shell/live_command_session.cpp#L38)
starts with `if(residentWorld)return fail(...)` and the message that a fresh
producer is required. Generic nonresident sessions can allocate a new command
session and request recovery. [`fail()`](../../apps/qt-shell/live_command_session.cpp#L32)
aborts and clears presentation. The shell's
[resident error branch](../../apps/qt-shell/main.cpp#L270) aborts the resident
session, stops delivery and suspends input rather than showing an original
World viewport. Automatic recovery into a coherent resident generation is
therefore not implemented by that generic recovery path.

The committed native system-memory facade also has an explicit no-loss policy:
[`ns_lost()` and `ns_restore()`](../../runtime/native-draw/surfaces.h#L130)
return not-lost/success, or busy while locked/DC-held. They do not simulate a
driver loss, recreate resources or reconstruct native World history. This is
native storage policy, not by itself a defect or evidence of driver equivalence.

Work to close this item: choose and test the supported outcome for producer exit,
render-owner failure, queue timeout, incompatible canvas recreation and GL
resource/context failure. A clean terminal stop can be an allowed outcome;
transparent recovery is not mandatory for every failure. If continuing the
session is claimed, implement and compare a fresh producer/canvas/command handoff
that retires old leases and preserves simulation-visible state. Returning to
original drawing needs its own coherent transition and ends the native claim.

Acceptance: failure tests establish the advertised stop/restart/recovery behavior
while original drawing is bypassed, including no stale display, borrowed memory
or pending resource leases. Existing shutdown and generic recovery controls
remain component evidence, not proof of these broader cases.

## RENDER INT 06 Complete independent comparisons and refresh affected evidence

**Positively demonstrated acceptance gap; historical pixel failures need reproduction.**

The shipped [`drawing-journeys/suite.json`](../../tests/fixtures/drawing-journeys/suite.json)
contains four adapters: `components`, `scene`, `producer_sequence` and
`publication`. It contains zero `session` adapters. The corresponding
[`run()` branches](../../tools/check-drawing-journeys.py#L20) assign empty journey
requirements and no action binding to these cases. Only the session branch
produces `presented_frame` observations with action-bound requirements.
[`coverage()`](../../tools/drawing_journeys.py#L97) requires independent,
contiguous, action-bound presented frames before crediting a complete journey.
Thus even successful replay of every currently listed case cannot close the
14-journey acceptance gate. This is an executable corpus limitation; it does not
negate separate gameplay observations or imply that all 14 features lack code.

Work to close this item:

1. Add independent original presented-session references and native adapters for
   the required journeys, with exact frame/input identities and caller results.
   Extend the 21 operation/failure requirements as actually exercised. Native Qt
   menu layout is intentional policy; validate its intended semantics separately
   from original game-scene pixel equality.
2. Diagnose/reproduce the [historical intermittent pixel mismatch](native-palette-row-cache.md)
   on a declared current source/input set. Neither a matching repetition nor
   static inspection establishes repair. This review does not identify its
   cause or establish that current code still reproduces it.
3. Rerun affected comparisons after source changes. The review record compares
   source hashes in the retained final October 14 gameplay and combined staging
   reports with the inspected working files; both have changed-source entries.
   Preserve their original hashes and results. A changed fingerprint establishes
   a validation freshness limit, not a rendering regression.

Acceptance: the complete declared corpus passes independent comparison and
caller checks on its exact current sources, with failures/refusals preserved.
The required 60 FPS performance target is a separate measured goal; the
approximately 59.4 FPS observation does not prove sustained 60 FPS or scanout.

## Release order

Start with RENDER INT 01: the current campaign harness cannot select the producer
group whose integration it needs to validate. Use that mode to expose concrete
history/forwarding failures in INT 02 and INT 03. Extend transitions and failure
cases in INT 04 and INT 05, then close the independent corpus in INT 06. Add the
complete launcher mode only after those declared gates pass.

Existing [`--resident-world` and `--independent-world` selection](../../apps/qt-shell/main.cpp#L910)
checks mode prerequisites; it does not implement the complete capability gate.
The [campaign report initialization](../../tools/test-native-campaign.py#L465)
explicitly records `complete_drawing_replacement: False`. The authoritative
[replacement plan](../../docs/live-drawing-replacement.md) retains the release
criteria; this review makes the current implementation and validation boundaries
concrete without promoting any replacement status.

## Review verification

The code review record contains positive source witnesses, not just searches for
TODO text or register status. It also records:

- Direct execution of group selection, individual-disable preservation and
  campaign environment cleanup.
- Admission of the shipped suite and verification that no full-session adapter
  is present; acceptance logic refuses component-only closure.
- Passing existing `tests/test-drawing-journeys.py` and
  `tests/test-drawing-session.py` suites.
- Compilation and execution of `tests/world-deferred-test.cpp`, checking immutable
  payloads, World-access refusals, 512 cursor records and exact capacity bounds.
- Exact committed parent/current file hashes and receipt chains for
  `b30ff77f..76a75c0c`: 209 transitions with no missing receipts. This includes
  reinspection of the retained controls history and its subsequent accounting
  commit; it does not assert a past gate pass or new engine validation.

Raw check logs and audit reports are under
`working/tests/native-rendering-integration-review-20261015/`. To recheck a source
witness after edits, compare its recorded SHA-256 and excerpt against
`git show 76a75c0c:<path>` and the current file separately. Run the existing guard
commands listed in the JSON under the repository storage guard. A source review
and synthetic admission tests do not establish current live pixel equivalence.

This read-only check verifies every recorded source identity and exact code
excerpt against the reviewed commit, independently of later working-tree edits:

```python
import hashlib, json, subprocess
from pathlib import Path
r = json.loads(Path("research/runtime/native-rendering-integration-review-20261015.json").read_text())
blobs = {p: subprocess.check_output(["git", "show", r["reviewed_head"] + ":" + p])
         for p in r["sources"]}
assert all(hashlib.sha256(blobs[p]).hexdigest() == h for p, h in r["sources"].items())
for w in r["source_witnesses"]:
    lines = blobs[w["path"]].decode().splitlines()
    assert "\n".join(lines[w["first_line"] - 1:w["last_line"]]) == w["excerpt"]
print("Verified", len(r["source_witnesses"]), "code witnesses at", r["reviewed_head"])
```
