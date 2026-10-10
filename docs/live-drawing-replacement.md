# Complete live drawing replacement

Reviewed 2026-10-10. **Complete live drawing replacement is not available.**
`./tools/run-qt-shell.sh` selects native capture/replay presentation while the
original game continues its simulation and drawing. No current launcher flag
enables complete drawing replacement.

This is the authoritative plan for replacing original drawing: scope, remaining
work, implementation order and release criteria. The
[coverage register](../research/runtime/coverage/register.json) owns detailed
behavior links and independent validation statuses; the
[coverage ledger](../research/runtime/coverage-ledger.md) records achieved
milestones. Subsystem research owns build-specific facts and evidence. Update
this plan when those results change the replacement scope or next step; retain
historical evidence at its recorded hashes.

The [binary/API coverage reconciliation](../research/runtime/rendering-coverage.md)
and `python3 tools/report-rendering-coverage.py` report scoped implementation,
independent original/driver comparison and actual bypass against reviewed
denominators. Categories with some support are not complete categories. No
overall effort/completion percentage is inferred. The newer World queue/batch
proofs below supersede the older next-step prose for their bounded scopes only.

## Meaning of complete replacement

The target keeps the original simulation running and makes native rendering
responsible for the game's visible output. Native services consume assets,
camera state, ordered drawing inputs and palette/lighting state. Original
rendering paths are bypassed within the declared supported build and scenarios.
Native asset decoding, layout or other CPU preparation is allowed; complete
replacement does not mean every graphics-related operation must execute on a GPU.

Dynamic pixels produced by original sprite rasterization, terrain drawing,
locked-buffer writes or GDI painting cannot supply the completed native frame.
Replacing DirectDraw copies and presentation alone is partial replacement while
those original producers continue. A native scene must also preserve any
rendering-side state changes and readback results needed by original gameplay,
including picking and coordinate conversions. Bypassing an entire consumer is
safe only after its required side effects have been identified and retained.

The initial build scope is the No-CD executable with SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Required scenarios cover single-player startup and enabled movies, Main/setup
menus, campaign and quick-battle entry, active World drawing, camera movement
and rotation, creatures and effects, HUD/minimap/tooltips, cursor interaction,
pause/Preferences, save/load and map transitions, battle results and shutdown.
The clean executable and multiplayer need separate scope and evidence before
support can be claimed. Unknown routes in the supported scenarios remain blockers.

An experimental mode may replace a named subset and fall back for the rest.
Its name and report must identify that subset. A complete-replacement mode must
reject unsupported capability sets at startup. If an unsupported route appears
during execution, it must stop that mode or explicitly end the replacement claim
before a validated transition to original drawing. Silent fallback cannot count
as a successful complete-replacement session.

## Current capabilities and dependencies

The native surface backend, SPR drawing, resource cache, scene service and GPU
presentation provide useful implementation foundations. Their scoped offline
and preview results do not establish a live drawing takeover. The live surface
bridge still forwards original calls and uploads snapshots of application-held
pixels. The scene observer exports bounded display records and forwards the
original queue consumer; its native adapter requires unshaded replay and can
explicitly permit partial previews.

The [owned World frame path](../research/runtime/native-world-frame-rendering.md)
extends that boundary with effective primitive order, actual colours and clips,
complete resource admission and six native raster modes. It compares finite
full World consumer canvases against independent original output. It has its
own strict adapter; the earlier snapshot preview retains its unshaded policy.
The [continuous World shadow viewer](../research/runtime/native-world-live-rendering.md)
now publishes owned requests through a bounded two-slot channel and draws finite
batches into the Qt GPU viewport. Ordinary mode performs no CPU pixel readback.
The [initial-canvas gap](../research/runtime/native-world-live-background-gap.md),
later HUD/window composition and complete consumer bypass remain separate
milestones. Complete delivery and native presentation do not establish baseline
equivalence for an unknown initial canvas.

The [native canvas history increment](../research/runtime/native-world-canvas-history.md)
adds contiguous source admission and owned GPU canvas retention. Before/after
original diagnostics establish retained pre-consumer pixels for selected failing
frames. An explicitly zero-initialized native history chain matches private
original execution. Startup already has nonzero canvas contents and refused
drawing paths; its producers/ownership are still unresolved. Continuous history
requires every intervening queue despite publication drops and superseding.

The [direct-word sprite MVP](../research/runtime/native-word-sprite-mvp.md) adds
an explicit partial CPU raster route at the two selected original word-backend
entries. Fully in-bounds admitted SPR draws can bypass those original routines;
clipped and unsupported draws retain their original path. Original caller-side
auxiliary passes, indexed World sprites and the other frame producers remain
active. Its bounded shadow/takeover comparisons are scoped primitive evidence,
not a complete scene or supported-session milestone. A separate
[clipped pixel comparison](../research/runtime/word-sprite-clipping.md) now matches
960 native GPU placements against both original backends. [Clipped caller-state recovery](../research/runtime/word-backend-state.md) now
compares complete workspace/arguments and selected original ABI in 4,448 fixtures.
It fixes existing unclipped workspace/C1 compatibility and validates a fresh
finite startup takeover. Clipped native raster admission and full Win32 exception
behavior remain open, so live admission is unchanged.

The [complete startup World raster queue](../research/runtime/native-world-raster-queue.md)
and [guarded batch return](../research/runtime/native-world-raster-batch.md)
now have actual original-body suppression and independent comparisons across
queues1..16. Ten of thirteen admitted entries are exercised in the scalar and
initial batch proofs. A later identity-cache batch exercises nine. Each run
retains its source fingerprints. The newer owned-source batch extends the
independent comparison to queues1..32: 63,124 precise raster canvases and AX
results, 1,224 completed checkpoints and 32 live finals match original execution.
Ten admitted entries are exercised. Its software-Mesa median queue time of
223.83 ms leaves real-time continuous delivery unvalidated. Longer gameplay,
outside-World producers and complete consumer side effects remain open.

| Area | Current boundary | Work required for complete replacement |
| --- | --- | --- |
| Copies and fills | Native operations and selected original/driver comparisons exist; live hooks still call original drawing. | Route admitted operations directly to native storage, preserving clipping, keys, flags, partial writes and caller-visible results. |
| CPU pixel writes | Capture reads game-held Lock/Unlock buffers after original CPU work. Most producer callers remain unclassified. | Identify and replace active raster producers above raw memory writes; provide compatible owned read/write access where gameplay requires it. |
| GDI and text | The bridge checkpoints original held-DC bitmap output. | Supply native image/text composition and compatible DC/access semantics where required; remove dependence on original GDI painting. |
| Scene input and resources | Bounded queue snapshots map selected SPR identities to native resources. | Complete resource binding, draw-kind dispatch, camera/viewport state, ordering and required consumer side effects. |
| Terrain and background | Native ordinary/generated terrain previews cover selected contracts. | Feed the live map/background, objects, overlays and water routes; complete camera and visibility behavior. |
| Sprites and palettes | Native SPR drawing and selected animation/shading contracts exist. | Integrate live animation, attachments, clipping, palette remaps/cycling and all required sprite backends. |
| Effects and lighting | Selected effect state, animation and lighting contracts are reconstructed offline. | Connect live effect inputs to complete draw admission, blending/shading and ordered raster output. |
| HUD and cursor | Native menus, fonts and cursor assets have separate scoped implementations. | Render the complete in-game HUD, minimap, tooltips and cursor; preserve hotspots, logical coordinates and picking. |
| Movies and transitions | Optional Qt media routes and native menu bridges have bounded coverage. | Integrate enabled media, screen ownership and return to World without original graphics fallback in complete mode. |
| Surface lifecycle and recovery | Native ownership/storage and transport recovery have scoped tests. | Integrate aliases, palettes, clip state, borrowed intervals, reads, flips, loss/Restore, transitions and coherent recovery while original raster work is bypassed. |

The [owned minimap viewer](../research/runtime/native-minimap-interactive.md)
adds opt-in V2 terrain, marker and camera-corner composition to the finite public
startup-history launcher. Paced rotation/pan comparison and independent unpaced
diagnostic costs are separate from original minimap suppression, unbounded
gameplay and complete HUD replacement. Original drawing stays active.

The [drawing inventory](../research/runtime/render-drawing-inventory.md) owns
caller/branch classifications. The
[surface operation matrix](../research/runtime/native-surface-operation-matrix.json)
owns required and conditional low-level operation cases. Both must remain linked
to this plan; completing the surface matrix alone does not complete the scene,
CPU raster, GDI or live takeover work.

## Implementation order and milestone gates

These milestones are planned work. Foundation and observation evidence exists;
no milestone below currently establishes complete live replacement. Surface
research's historical milestone numbers retain their local meaning.

| Milestone | Deliverable | Current state | Exit gate |
| --- | --- | --- | --- |
| DRAW-01 Drawing coverage | Scenario-tagged inventory of active producers, consumers and graphics side effects. | Partial static and bounded live inventory. | Every encountered route has a behavior ID, owner, classification and explicit support decision; unresolved indirect flows remain visible. |
| DRAW-02 Independent comparison | Original-output corpus for surfaces and complete scenes, with frame/input identities. | Full World consumer inputs and independent whole-canvas comparisons added; broader session corpus remains open. | Native output is compared with independent original output for the selected scenario; oracle pixels never drive native rendering. |
| DRAW-03 Native scene completeness | Asset-backed rendering for the admitted draw kinds, backgrounds, shading and UI. | Six raster modes, actual colours, clips and strict complete World resource binding implemented. Whole-consumer side effects and later HUD/window output remain open. | The declared scene is complete, including required side effects; diagnostic backgrounds, omitted draws and unshaded approximations cannot pass baseline comparison. |
| DRAW-04 Continuous scene delivery | Versioned live drawing inputs from original simulation to native services. | Owned World two-slot delivery and GPU shadow presentation exist; original initialization and complete intervening history remain open. | Complete frame admission, identity/lifetime, backpressure and transitions pass without sourcing dynamic frames from original raster output. |
| DRAW-05 Scoped live takeover | Bypass original drawing for one validated route or complete scene, with explicit mode identity. | Direct-word sprite MVP and complete admitted raster-body suppression across startup World queues1..32 have bounded independent takeover proof. Whole-session and complete consumer takeover remain outstanding. | Independent live comparison and counters prove the selected original work was skipped; caller-visible behavior, readbacks and cleanup pass. |
| DRAW-06 Supported session coverage | Expand takeover to every required drawing route and scenario. | Outstanding. | Full-session corpus, failure/recovery and supported physical-driver checks pass; every fallback or unsupported route is reported as a separate outcome. |
| DRAW-07 Complete launcher mode | Expose a capability-checked complete-replacement option. | Unavailable. | All completion criteria below pass for the declared build/scenario scope, and the register records the appropriate replacement evidence. |

The initial implementation path is `runtime/scene/`, `compat/legacy/` and
`renderer/scenes/`. The owned World frame increment supplies actual colours,
effective draw order/clips and complete original batch output for finite
comparison. The continuous World shadow viewer supplies bounded immutable publication,
backpressure and Qt GPU presentation. Source-only startup/HUD reconstruction and
bounded complete raster-body suppression now support selected queues1..32.
The current work priority is completing the other pixel producers, starting
with exact text pixels and continuing through source images, HUD/minimap/cursor
composition and remaining CPU writers. World delivery beyond queue32 remains
a separate pending integration milestone. Extend the DRAW-02 corpus with camera
actions and active battles while classifying additional draw kinds and required
consumer side effects before complete scene takeover.

### Other pixel producers: current work priority

Complete each selected producer in bounded steps: owned inputs and native pixel
implementation, independent original pixel/state comparison, original-active
live observation/shadow, then an explicitly admitted bypass. Existing native
previews and captured-destination uploads cannot satisfy the bypass step.

| Order | Producer work | Concrete completion boundary |
| --- | --- | --- |
| 1 | SFT tinted glyph raster and original text consumers | Finish the dedicated [glyph comparison](../research/runtime/font-glyph-raster.md); preserve original contour spacing/cursor behavior before live admission. General text layout and original font lifecycle remain separate from glyph pixels. |
| 2 | BMP/DIB/PCX/JPEG source image writers and panel/fade operations | Exercise the memory-DIB and positioned-image branches beyond startup, preserve source cropping, padded rows and caller/DC lifetime, and admit native output without original GDI destination pixels. |
| 3 | HUD, tooltips, minimap and cursor | Attribute active callers and compose complete output from owned state/resources. Existing minimap four-view comparisons and native cursor/menu assets are foundations; validate live updates, hotspots, picking and producer suppression. |
| 4 | Remaining Lock/Unlock writers and alternate dispatch | Attribute unclassified writes and effect-to-pixel routes; recover their inputs, destination dependence and required side effects. Keep unknown routes visible and outside replacement admission. |
| 5 | Enabled movies and screen transitions | Validate owned native decoding/presentation, completion/cancellation and return to menus/World while replacement owns the screen. |

Each completed pixel contract retains its own scope and evidence. Wider live
takeover needs new evidence after shared producer or renderer sources change;
historical hashes are preserved. Surface recovery and sustained World delivery
remain completion dependencies even while this producer work takes priority.

DRAW-05 is the first milestone that removes original drawing work. It may cover
a small route before the whole game is ready. Its experiment must identify the
exact original routine bypassed, its effects and native inputs, and the scenarios
in which bypass is permitted. Keep unknown routes on the original path in that
explicitly partial mode. Expand coverage through DRAW-06 before exposing DRAW-07.

CPU raster work outside the scene consumer needs separate replacement boundaries.
Replacing `Blt`, `BltFast` or `Flip` alone cannot skip code that writes pixels
between Lock and Unlock. Likewise, replaying a held-DC bitmap does not replace
the GDI calls that painted it. Preserve the original call ABI, return/error
semantics and required state changes when replacing either route.

## Integration responsibilities

| Boundary | Responsibility |
| --- | --- |
| `runtime/` | Hash/signature-checked build-specific observation and scoped bypass; original ABI and side effects; versioned owned inputs and truthful execution counters. |
| `compat/legacy/` | Decode and validate inputs, bind original identities to native resources, and translate supported drawing requests without leaking legacy pointers into native services. |
| `renderer/`, `assets/` | Own native resources, raster operations, complete scene composition and presentation; remain independent of original addresses and application widgets. |
| Qt application/session layer | Select capabilities and modes, coordinate input/media/lifecycle, and report replacement, refusal and fallback distinctly. Widgets express semantic actions. |
| Reconstruction and research | Recover baseline contracts, maintain independent comparisons and classify remaining branches. Simulation timing and gameplay changes remain separate. |

Define frame publication and takeover as one explicit contract: which thread
owns inputs, when they are immutable, when the native result is accepted, and
which original work can then be skipped. Capacity, timeout, reentry, producer
exit and device-loss behavior must preserve that contract. A failed native frame
must not leave an original caller using stale pixels or a partially updated
graphics state. Re-entering original drawing requires coherent state and a
validated transition; the current shadow-mode fallback is not proof of that
transition after original drawing has been suppressed.

## Criteria for enabling the complete launcher flag

- [ ] The manifest pins the supported original build and native source/binary
  hashes, selected mode and required capabilities.
- [ ] Native inputs cover every required draw route in the declared scenarios;
  unknown kinds, incomplete resources and unresolved rendering side effects
  cannot be silently omitted.
- [ ] Original raster work is bypassed. Per-route counters distinguish observed,
  originally executed, natively executed, bypassed, refused and fallback calls.
  CPU pixel writes and GDI painting are included, not only DirectDraw dispatch.
- [ ] Required rendered pixels and observable behavior agree with independent
  original outputs under declared comparison rules. Full scenes, transitions,
  cursor/picking and error/retry cases have explicit checks; stable menu regions
  alone cannot validate a complete World frame.
- [ ] Surface access, aliases, palettes, clipping, loss/Restore, flips and
  lifetime behavior work while original drawing is bypassed. Recovery or
  shutdown leaves no borrowed intervals or native resource leaks.
- [ ] Continuous gameplay, effects, camera changes, HUD, movies, menu/World
  transitions and fresh/reloaded sessions pass on the declared target driver.
  Register exact scenarios and durations; a bounded idle test cannot stand in
  for active battle coverage or indefinite stability.
- [ ] The launcher rejects unsupported configurations, runtime refusals are
  visible, and every return to original drawing ends the replacement result.
- [ ] Performance reports separate original drawing removed, producer/adapter
  cost, native render cost, queue age and input-to-screen latency. Confirm the
  selected physical GPU. Captured-stream throughput is not live game FPS.
- [ ] Fresh bypass execution evidence, behavior/scenario links, census and exact
  review receipts pass the coverage audit and change gate. Understanding,
  implementation, comparison, integration and replacement advance independently.

The complete flag's spelling will be chosen when these gates are met. No dormant
or alias flag should imply that current native capture/replay satisfies them.
Independent cursor presentation may improve responsiveness earlier, and rendering
cadence may later be separated from simulation, but neither proves drawing
replacement or authorizes gameplay timing changes.

## Evidence and maintenance

Use these subordinate contracts when implementing a milestone:

- [Scene snapshots and current unsupported modes](../research/runtime/native-scene-snapshot.md)
  and [native scene service](../research/runtime/native-shared-scene-renderer.md).
- [Required surface cases and independent fixture procedure](../research/runtime/native-surface-fixtures.md).
- [Current launch policy](../research/runtime/native-command-launch.md) and
  [live surface-command transport](../research/runtime/opengl-live-command-transport.md).
- [Observer lock avoidance](../research/runtime/render-surface-contention.md)
  and [capture tracker contention](../research/runtime/native-tracker-investigation.md).
- [Replacement promotion rule](../research/runtime/coverage-ledger.md#maintaining-the-ledger)
  and [coverage accounting workflow](../research/runtime/coverage/README.md).

For each milestone change, update its current state and remaining blockers here,
then link exact behavior IDs, implementation/tests, scenario and immutable evidence
in the register and ledger. Preserve separate results for native implementation,
independent original comparison, live observation and actual bypass. Historical
subsystem reports retain their dates and scopes; their local next-step notes
yield to this implementation order. Mark a milestone complete only within its
recorded scope. Add newly encountered routes to the inventory and required matrix
before extending a replacement claim; retain conditional and unresolved branches.
