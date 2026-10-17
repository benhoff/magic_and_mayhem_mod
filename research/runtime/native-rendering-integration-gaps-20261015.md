# Native rendering: current gaps and verified progress

Reviewed 2026-10-16; the original `45af2ed6` review is superseded for byte lifetime and recorded GPU session comparison by the source-bound results below. This is the current integration backlog;
the filename is retained for existing links and stable RENDER-INT identifiers.
The [new code/control review](native-rendering-gap-review-20261016.json) records
exact source hashes, line witnesses, native C boundary probes, portable guards
and inspected retained results. The [October 15 review](native-rendering-integration-review-20261015.json)
remains immutable historical evidence. Its claims that the campaign runner lacks
combined-producer selection/counters and that changed-canvas Load is unproved
have been superseded by the implementations and results below.

The initial review was static/native-control work. Subsequent work executes native
GPU and physical X11 comparisons plus isolated PE32 producer controls. Two fresh
original-game campaign attempts do not complete and remain explicit limitations.
Concurrent Cure and native campaign-persistence work is outside this renderer scope.

## Current status

| Area | What the current code establishes | Remaining gap |
| --- | --- | --- |
| Long sessions | V3 wrapping transport, private queue, owned emitter and decoder pass actual traffic beyond 4 GiB with bounded storage. | Finite sequences, capacity/deadlines and unbounded original-game duration remain separate; matched controls and a natural-intro 480-World V3 storage journey now pass. |
| CPU/GPU history | Explicit ordered CPU_READ, physical identity/generation/nonce restoration, Save/Load synchronization and selected GPU copies exist. | Arbitrary raw readers are outside interception; selected unsupported dependencies still refuse. |
| Complete-native release | Combined mode requires zero forwards/history refusal and positive execution for requested routes. | No capability-checked complete-renderer launch/release contract; reports explicitly retain complete-replacement false. |
| Optional routes | Six valid families and 43 installed routes; 19 execute across the reviewed camera/spell/storage reports. | 24 are unexercised in those reports, with caller/reachability and World-batching distinctions retained. |
| Session composition/parity | Historical Save/Load/changed-canvas journeys pass; all 323 observed original Quick Battle frames now pass native GPU and displayed-output comparison in a portable corpus. | Generic action-bound/global presentation coverage, later maps and broader movies/input remain incomplete. |
| Failure handling | Terminal resident cleanup and force-stop/fresh-launch exist; resident in-place recovery explicitly refuses. | Actual-game injected failure followed by restart and physical context-loss behavior remain unproved. |
| Performance | Retained hardware and software results distinguish engine cadence from presentation. | Sustained complete-session hardware/60 FPS acceptance remains unproved. This review does not probe hardware availability. |

## Completed work that must not remain on the missing-implementation list

- `tools/test-native-campaign.py` has `--other-pixel-producers` and
  `--require-pixel-route`, restores the authorized group after environment cleanup,
  verifies staged compiler/DLL provenance and reads per-route counters.
  `tools/pixel_route_requirements.py::validate_routes()` rejects any original
  forward, history refusal, missing native traffic or unexecuted required route.
- The old alleged BitBlt hook was actually ClientToScreen and is retired.
  `selection()` now enables six valid optional families; glyph/World adapters
  have separate contracts. The old seven-family/53-route counts are historical.
- The native facade has implemented owned formats, palettes, clipping, physical
  leases and held-flip behavior. Its focused comparisons and limits are in
  [native resource/state handling](native-resource-state.md) and
  [completion contracts](native-render-completion.md).
- [Native Save/Load](native-render-save-load.md) and its selected independent
  World comparisons exist. [Initiate tutorial input](native-mini-dispatch-admission.md),
  [changed-canvas reload](native-world-canvas-transition.md) and the
  [resident pressure repair](resident-command-pressure.md) add later bounded
  live results. The final delayed Initiate run completes 962 balanced Worlds,
  both display rebuilds, a changed canvas and normal Quit; three pressure waits
  resume, private queue peak is 9,035,414 bytes and maximum wait is 273 ms.
  The former 707-World overflow is retained negative evidence, not a current
  reproduced failure after this repair.

## RENDER-INT-01: Remaining route execution and reachability

**Combined selection, execution and counter enforcement are implemented. The
remaining work is branch/caller coverage and independent output proof.**

The review checks the retained camera, spell, Apprentice storage and final
Initiate results separately. Camera/spell each execute 19 of 43 optional routes;
these two storage journeys each execute 17. Their union is still 19. All four
reports succeed with zero optional forwards/history refusal. These are results
at their recorded source hashes, not newly executed or renewed comparisons.

The 24 unexercised routes are `pixel/rgb_add`, `pixel/pcx_words`,
`pixel/brighten`, `pixel/row_copy`, `colour_rect/rectangle`, `minimap/camera3`,
and raster splices 0, 1, 4–8, 11–20 and 22. Their implementations and isolated
comparisons must not be described as absent merely because an optional counter
is zero. World batching intercepts some operations before those CPU interiors.
The reviewed minimap switch selects camera0/2/0/1; it does not select camera3.
Other camera3 callers are unresolved, not proved dead code.

See [targeted route scenarios](native-pixel-route-scenarios.md). Close each
claimed route with an actual caller/scenario and independent caller/output
checks; retain genuinely unassigned indirect reachability. Do not invent gameplay
or alter dispatch merely to make every optional counter advance.

## RENDER-INT-02: Remaining CPU/GPU dependency boundaries

**Selected history services exist; general CPU coherence does not.**

`NativeDrawCpuHistoryV1` marks resident CPU backing stale and makes selected
stale reads terminal before original fallback. `NativeDrawCpuTransferV1` restores
only a matching unborrowed RGB565 backing. Stream V5 CPU_READ validates ticket,
session, extent, generation and nonce and executes after preceding GPU work.
`RenderPrepareStorageV1` synchronizes selected storage panels before original
callbacks. These are current implementations, not TODOs.

`NativeDrawGpuCopyPlanV1` now admits owned non-stretched RGB565 GPU copies with
up to 256 canonical clip pieces and an immutable GPU snapshot for aliased/self
sources, including single-key overlap. It retains successful pieces before a
later source-bounds error. All 23 actual facade/bridge/GL controls pass, with
untouched stale CPU storage and temporary resource cleanup. The legacy
`NativeDrawGpuCopyV1` retains its strict full opaque bootstrap role. See
[copy-plan evidence and boundaries](native-gpu-copy-plan.md). The
[current live journey](native-gpu-copy-live-validation.md) validates 867 opaque
disjoint GPU calls at three engine PCs and independently checks their complete
destinations; clipped/self/keyed GPU callers remain unexercised live. Partial writes cannot establish previously
unknown/full CPU history. Arbitrary pointer reads outside selected adapters are
not intercepted.

The earlier blanket phrase "asynchronous CPU reads unsupported" needs precision:
explicit command CPU_READ and the asynchronous native host worker are separate
from asynchronous **World GPU source production**. The latter's completion guard
in `ResidentWorldSession::pollAsynchronous()` still requires zero native/RGBA
readbacks. Synchronous source composition separately accounts for explicit
CPU-reader snapshots. The review establishes those guards, not that all async
readback would inherently be impossible.

Close actual new readers with an ordered owned representation or a documented
capability refusal. Preserve the distinction between current native pixels and
readable but stale CPU allocations. See [reader routing](native-history-readers.md).

## RENDER-INT-03: Strict complete-native release policy

**Still missing as a complete-target contract. Existing combined admission is
already strict about its declared producer counters.**

The campaign report sets `complete_drawing_replacement: false`, and counter/
route results set `complete_replacement=False`. Partial adapters retain original
continuations outside their admitted domains; an unsupported facade call can
also terminate. Neither a native-mode switch nor a zero-forward bounded run
certifies every required game route.

A complete mode needs a declared capability set, startup admission and a session
outcome that ends its complete claim before any unsupported original pixel
production resumes. Preserve intentional native Qt menu semantics and original
simulation/required preparation side effects. No current unimplemented graphics
API is assumed to be a required game path without a caller.

## RENDER-INT-04: Broader transition and interaction coverage

**Save/Load and changed-canvas display rebuild/reload are no longer missing.
Broader journey composition remains incomplete.**

The retained final Initiate run verifies generation/import/original-return
balance, a changed canvas, fresh post-storage World sources and normal Quit.
This corrects the old same-canvas-only boundary. Rebuilding the display and
reloading a saved map is not proof of leaving one map and progressing to a
separate later campaign map. The current result also skips movies.

Required expansion still includes later-map resource churn, both result paths,
enabled movies and return to World, broader tooltip/cursor/hotspot/picking and
Preferences combinations, with scene/resource identity and independent semantic
or pixel checks. Existing successful individual observations remain scoped;
whole declared transition coverage is not inferred from shared code.

## RENDER-INT-05: Failure and fresh restart acceptance

**Terminal cleanup and hard restart are implemented. In-place resident recovery
is intentionally unsupported; it is not a missing requirement by itself.**

`LiveCommandSession::recover()` refuses resident retry into the failed producer.
`main.cpp::residentFailed()` cancels/retire owners, clears GUI frames, suspends
input and offers Hard restart. `hardRestart()` calls the owning process/menu
force-stop and starts a fresh session only after successful stop.

The existing `resident-terminal-restart-test.cpp` models malformed commands,
producer failure, first-frame timeout and GUI abort, then creates fresh synthetic
workers/channels. Its comment and result explicitly exclude actual-game restart
and physical GL context loss. Shortcut dispatch has its own synthetic test.
These implementations and historical controls must not be listed as absent.

Remaining acceptance is an actual-game failure→cleanup→fresh-launch journey,
plus declared physical context/owner failure outcomes and concurrent lifecycle
bounds. A safe terminal stop is allowed; transparent continuation or preservation
of unsaved progress is not promised. See [completion contracts](native-render-completion.md).

## RENDER-INT-06: Presented-session comparison and performance

**Still acceptance gaps, not evidence that all named drawing features lack code.**

`tests/fixtures/drawing-journeys/suite.json` now has a fifth adapter,
`recorded_session`, alongside components, scene, producer_sequence and publication.
The portable [recorded interval](recorded-native-session-comparison.md) retains all
323 independent original EGL frames, including duplicates, with exact source/API/
closure and asset bindings. Current native GPU composition and post-swap X11 output
both match: 310,080,000 pixels, zero outside RGB tolerance 1, alpha excluded.

This is `observed_egl_session` evidence. No expected original plane is passed to
the native renderer. It covers Main/setup/loading/Quick Battle/camera/Results/
returned Main, without proving every global original presentation route, product
Qt menu pixel identity, live replacement, or complete action semantics. There are
still zero generic `session` adapters. The 14 journey and 21 operation/failure
requirements retain their full denominators. New campaign/SaveLoad/movie intervals
and independent contiguous action-bound presentations remain acceptance work.
The historical intermittent mismatch is not reclassified as a current defect.

Performance is a separate acceptance axis. The retained TITAN integration run
records roughly 59.4 visible native World FPS while engine cadence is roughly
19.4 Hz. The latest Initiate pressure proof uses llvmpipe and a 5 FPS functional
floor (observed aggregate engine rate 13.7225). These measure different work and
cannot establish sustained complete-session 60 FPS or current hardware behavior.
No hardware availability or new throughput measurement is claimed here.

## RENDER-INT-07: Cumulative command-byte lifetime — repaired

Four independent ceilings were confirmed: owned session emission, private queue,
mapped V2 publication and streaming decoder total. All four are repaired. V3
uses atomic32 modulo positions and bounded unsigned progress; public continuous
launches select it. V2 keeps its explicit legacy nonwrapping guard. Emitter and
decoder count cumulative bytes only for bounded files; archive budgets remain
separate from live production.

[Final lifetime controls](render-command-lifetime-controls-06-20261016.json) pass
4,297,064,448 actual bytes through the production emitter/queue/ring and
4,295,064,708 through Qt channel/decoder/consumer. Native/sanitizer guards retain
capacity, wrong identity/version, ACK regression, cancellation and cleanup checks.
Actual PE32 producer-to-GPU controls pass 210 complete frame comparisons across
three valid V3 sessions and two explicit refusals. This is execution beyond the
boundary, not merely seeded-counter arithmetic or a multi-hour original run.

[Protocol and remaining live limitations](native-command-byte-lifetime.md) retain
finite command/identity domains, 32 MiB callback capacity, pressure deadlines and
unsupported history. Initial enabled-intro execution terminates with unclassified
GAP4 before World; another no-start attempt has no bridge traffic. Later matched
V2/V3 storage controls pass with 495/479 balanced Worlds and normal Quit; Intro0
is dismissed through ordinary input. See the [startup/storage follow-up](native-v3-live-startup-storage.md)
for the passing natural-intro 480-World journey and separate independent transition
assertions. Historical V2 Save/Load
evidence retains its old source hashes. The initial two-ceiling review
and all unsuccessful development/build attempts remain immutable.

## Reproduce this review

```sh
python3 tools/working-retention.py run --reserve-gib 1 -- \
  python3 -B tools/review-native-rendering-gaps.py \
    --output working/tests/native-render-gap-review-<new-id>.json
```

The output retains exact code witnesses and hashes, raw C/Python commands/logs,
route unions and acceptance-suite structure. It runs no original artifacts and
therefore grants no new original comparison, live integration or replacement
milestone. Historical results and their source hashes remain unchanged.
