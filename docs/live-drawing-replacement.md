# Complete live drawing replacement

The [recovery and acceptance closure map](native-rendering-closure.md) reconciles
local master `c4510223` into separate bottom-up contracts, top-down journeys and
a finite blocker list. It preserves this plan's existing journey/operation scope
and the independent coverage dimensions.

Reviewed 2026-10-16 against `45af2ed6` (renderer sources unchanged since `7c40c187`). Native rendering is implemented for
substantial supported paths. **A complete-renderer launch/release contract is
still unavailable.** The original engine continues simulation and preparation;
selected native modes bypass admitted original pixel producers and World drawing.
Ordinary observation/partial modes retain their own forwarding policies.

This is the current scope and release plan. The
[code-checked integration backlog](../research/runtime/native-rendering-integration-gaps-20261015.md)
and its [review record](../research/runtime/native-rendering-gap-review-20261016.json)
separate implementation limits, supported subsets and missing acceptance evidence.
The [coverage register](../research/runtime/coverage/register.json) owns behavior
links and independent statuses; the [ledger](../research/runtime/coverage-ledger.md)
records execution milestones. Historical result hashes are never refreshed by
editing this plan.

## Target and boundaries

Keep original gameplay/simulation running while native rendering owns visible
game-scene pixels and their required history. Native services consume owned
assets, camera state, ordered drawing requests and palette/lighting state.
Original preparation, wrappers and required rendering-side effects may remain;
original dynamic rasterization/GDI output must not supply the completed frame
inside a declared complete-replacement scope. Native Qt menu layout is intentional
policy and needs semantic validation separately from original World pixel parity.

The initial supported original build is No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Required journeys include startup with enabled movies, menus/setup, campaign and
Quick Battle entry, active World/camera/creatures/effects, HUD/minimap/tooltips,
cursor/picking, pause/Preferences, Save/Load, later-map progression, results and
shutdown. Clean-executable and multiplayer support require separate evidence.
Unknown required routes remain explicit blockers; not every unimplemented
DirectDraw method is assumed to be needed by this game.

## Implemented and demonstrated within bounded scopes

- Native World delivery, source-owned primitive batches, GPU presentation and
  post-World UI composition exist. Original World drawing is bypassed in the
  declared resident path. [Master integration](../research/runtime/native-render-master-integration-20261014.md)
  has a retained combat/spell/portrait/menu/Quit journey with 5,719 balanced Worlds.
- The [native surface facade](../research/runtime/native-resource-state.md) owns
  formats, aliases, palettes, clippers, DCs and physical held-flip leases. Focused
  original-driver/GDI comparisons and native controls have their own scopes.
- All six valid optional pixel families can be staged and exercised together.
  The harness checks installed/native/forwarded counters, requires zero forwards
  and history refusal and can require a particular route to execute. The retired
  alleged BitBlt hook targeted ClientToScreen; it is not a missing renderer task.
- [Explicit history services](../research/runtime/native-history-readers.md)
  materialize selected GPU history for CPU readers and admit selected ordered
  GPU copies. Arbitrary CPU pointers do not acquire coherent rendered pixels
  merely because they are readable.
- [Save/Load](../research/runtime/native-render-save-load.md), Initiate tutorial
  input and [changed-canvas reload](../research/runtime/native-world-canvas-transition.md)
  have retained successful journeys. The [final pressure repair](../research/runtime/resident-command-pressure.md)
  completes both display rebuilds, Main Load, fresh native resume and normal Quit
  with 962 balanced Worlds and three actual pressure waits/resumes. These are
  implemented paths, no longer missing tasks.
- Resident terminal failure clears presentation, suspends input and exposes
  force-stop/fresh-launch. Synthetic worker/shortcut controls exist. In-place
  recovery into a failed resident producer is intentionally refused.

These bounded results are separate from complete presented-session comparison.
Older finite 16/32-queue startup experiments and prior same-canvas-only Load
results retain their historical meaning, without limiting the later resident path.

## Current work remaining

| Work | Classification and completion condition |
| --- | --- |
| Extend live transport duration and lifecycle coverage | The four cumulative byte ceilings are repaired and native/Qt paths pass actual traffic beyond 4 GiB. PE32 producer integration and matched V2/V3 original-game Save/Load/rebuild controls pass. A current V3 journey naturally completes both intros and the full storage flow with 480 balanced Worlds. All four storage/rebuild raster checkpoints also match unchanged original code. See the [startup/storage follow-up](../research/runtime/native-v3-live-startup-storage.md); unbounded duration remains outside the proof. |
| Cover remaining CPU/GPU dependencies | Selected services work. A current V3 Save/Load/rebuild/Quit journey validates 867 opaque disjoint GPU calls at three engine PCs with independent native destinations. Clipped/self/keyed GPU copies retain headless evidence; their live callers remain unexercised. Arbitrary raw readers remain outside admission; asynchronous World source production retains a zero-readback guard. See the [caller-bound copy result](../research/runtime/native-gpu-copy-live-validation.md). |
| Finish optional-route evidence | The subsequent nonresident native journey reaches 31/43 routes and all 24 formerly absent routes pass individual original-splice comparisons. Seven alternate CPU-family routes and five unresolved caller windows retain their natural-live boundary. See [the route follow-up](../research/runtime/uncovered-native-pixel-routes.md); do not invent a camera3 dispatcher branch. |
| Expand complete transition/interaction coverage | Display rebuild/reload is demonstrated; later-map progression, broad results/movie/Preferences and cursor/picking combinations still require declared resource, semantic and output checks. |
| Populate independent presented-session acceptance | The suite now retains one portable recorded-session adapter: all 323 observed original EGL frames pass native GPU and displayed-output comparison. Generic action-bound session acceptance for all 14 journey and 21 operation/failure requirements remains open. |
| Exercise actual-game terminal failure and restart | Verify owned cleanup, input/presentation retirement and fresh identity/channel launch after an actual game/renderer fault. Synthetic worker tests do not prove physical context-loss behavior or preserved unsaved gameplay. |
| Establish sustained performance acceptance | Keep engine cadence, native World generation, visible paint and scanout distinct. Bounded approximately 59.4 FPS historical hardware presentation and current software functional passes are not sustained complete-session 60 FPS proof. |
| Enable strict complete-native admission | Declare the required capability set and reject unsupported startup/runtime paths before a complete claim can continue through original pixel fallback. Current reports deliberately keep complete replacement false. |

The [integration backlog](../research/runtime/native-rendering-integration-gaps-20261015.md)
contains current code witnesses, exact scope corrections and remaining acceptance
conditions. The historical intermittent pixel mismatch is not a confirmed
current-code defect without reproduction. Evidence staleness after source changes
also does not establish a rendering regression.

## Implementation order

1. Extend original-game transition coverage on the new wrapping transport, starting
   with the enabled-intro history refusal and a complete Save/Load journey. The
   lifetime repair already passes actual traffic beyond 4 GiB and PE32 controls.
2. Follow actual unsupported reader/route/transition encounters, with narrow
   independent comparisons and explicit caller/ownership boundaries.
3. Grow the complete presented-session corpus and fault/restart journeys using
   exact current compiler/input provenance. Keep instrumentation/capture load
   separate from ordinary presentation performance.
4. Enable the complete-native launch contract only when its declared required
   routes, history, comparison and lifecycle checks pass. Fixed FPS is diagnostic
   under the2026-10-25 user decision; historical benchmark criteria remain unchanged.

## Release and evidence rules

A partial mode may retain documented original forwarding. Complete mode must
admit its capability set at startup and stop or end the claim before unsupported
original pixel production resumes. Original simulation and validated preparation
are allowed; blanket removal of original consumers is not the goal.

Required evidence distinguishes source understanding, implementation, isolated
original comparison, integration and live replacement. A complete result needs
independent outputs, relevant caller/side-effect checks and actual original pixel
bypass throughout its supported scope. Component comparison, a zero-forward
bounded run or a report flag alone cannot grant that milestone.

Update `research/runtime/coverage/register.json`, exact change reviews, relevant
subsystem research and the ledger alongside new findings. Retain failed runs,
unknown callers, unsupported states and old source hashes. No original media was
consumed by the current gap review; its boundary probes and portable checks do
not renew original comparisons or historical live performance claims.


## Command lifetime and recorded-session update — 2026-10-16

The [V3 byte-lifetime repair](../research/runtime/native-command-byte-lifetime.md)
removes cumulative limits from the owned emitter, private queue, mapped transport
and streaming decoder. Storage bounds, sequence exhaustion, ownership, version,
ACK and failure guards remain. Continuous public launches select V3; legacy V1/V2
retain their contracts. Native and Qt execution independently cross 4 GiB.

The [portable whole recorded interval](../research/runtime/recorded-native-session-comparison.md)
compares 323 complete original frames through both native GPU and displayed X11
output: 310,080,000 pixels, none outside RGB tolerance 1. It needs no installation
or original media. This closes the recorded interval comparison gap; original
observer completeness, generic action-bound journeys and complete live replacement
remain distinct acceptance boundaries.

The [active copy-route census](../research/runtime/native-copy-route-reachability.md)
distinguishes 1,828 CPU-backed keyed calls from 1,666 opaque GPU-source calls.
Clipped/self/keyed GPU-source branches retain headless evidence; seven terminal
callbacks and independent CPU-keyed output remain outside the active proof.
