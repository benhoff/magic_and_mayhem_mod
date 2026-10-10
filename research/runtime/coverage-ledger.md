# Engine modernization coverage ledger

The [complete live drawing replacement plan](../../docs/live-drawing-replacement.md)
is the authoritative source for drawing takeover scope, implementation order,
remaining dependencies and complete launcher-mode criteria. This ledger and the
coverage register retain achieved milestones and their evidence. Current default
launches retain original drawing through native capture/replay presentation.

## Native World live startup history — 2026-10-08

`NR.world-history-channel` and `NR.world-live-history` connect the async CPU/GPU
preparation pipeline to retained native `SceneHistory`. The opt-in v2 journal
preserves every first1..16 original consumer queue with explicit source sequence,
logical canvas and native zero-reset metadata. Original drawing never waits for
the viewer. Gaps, missing capture, unsupported work, producer canvas changes and
incomplete termination refuse the prefix; final ENDED drains queued/pending work.

[Fresh synthetic execution](native-world-live-history-synthetic-20261008.json)
passes69 native tests and12 actual PE32 producer modes. A separate
[source-stable installed fixture](native-world-live-history-verified-20261008.json)
presents all16 first Quick Battle queues and matches7,680,000 original output
pixels with zero mismatches/drops/superseding. Adaptive pending polling measures
about3seconds per frame on software OpenGL; hard latency and real-time throughput
remain pending. A separate strict first-canvas attempt refuses2pixels; independent
original replay from zero matches native exactly, leaving external producer/init
coverage as a confirmed output gap with an unresolved exact cause. Retain/reset/resize,
whole-prefix draining and malformed/source-gap/refusal cases are exercised with
owned inputs. Installed presentation remains an observed fixed-canvas fixture, not all seeds. No original
initialization, HUD/input ownership, sustained channel, loading transitions,
hard driver latency, memory reservation or drawing bypass is inferred.
See [scope and launch instructions](native-world-live-history.md). Final frozen-source
normal/verified runs again present16 frames (normal zero readbacks; verified
7,680,000 exact pixels), and the69-test/12-PE32-mode corpus passes. Independent
canvas-producer protocol/hook/replay edits occurred during those runs; retained
hashes remain historical and current workspace promotion is explicitly pending.
The new behaviors therefore retain partial implementation/no integration rather
than asserting validation of later concurrent sources.

## Native World CPU asset preparation — 2026-10-08

`AS.resource-preparation` and `NR.world-preparation` introduce owned worker
preparation and owner-thread adoption. Live catalogue scanning, packet decoding,
pinned SPR reads/decoding and encoded/visual identity resolution run on one CPU
worker. GUI adoption reuses that decode; the live scene/cache refuse absent
residents instead of synchronously loading. Unload invalidates pending binding
tokens, and close cancels without joining worker I/O or accepting late results.
One pending packet retains existing channel acknowledgement semantics.

Synthetic preparation/Qt tests cover file deletion before adoption, warm reuse,
stale/foreign tokens, pins/budgets, held-load GUI heartbeat, cancelled work,
resident-only rendering and existing refusal/resize/mismatch/cleanup contracts.
See [ownership, evidence and remaining boundaries](native-world-asset-preparation.md).
GPU plane conversion/uploads, projected shadow generation, complete GUI scene
preflight, packet-copy cost, sustained CPU retirement and process RSS reservations
remain pending. Historical original/live evidence retains its recorded hashes;
this change does not establish current installed-game speed, initial-canvas
correctness, consumer bypass or full native rendering.

## Execution-claim accounting hardening — 2026-10-08

Coverage now distinguishes retained historical stages and source fingerprints
from exact current validation. New recorded-validation receipts require an
immutable execution claim bound to the behavior's scope/build/path identity,
all implementation and declared dependency fingerprints, and the exercised
scenario/test identity for integration or replacement. Scope expansion,
unrelated evidence and omitted source inputs cannot establish current readiness.
Existing unbound results remain historical; no original comparison was rerun or
retroactively rebound by this tooling change.

Exact parent receipts compose across a branch while final status promotions still
need current execution proof. Root build inputs enter the source census. Track
requirements use the correct independent integration/replacement values, and
dashboard current counts follow bound proof rather than merely fresh hashes.
The regression suite exercises these accounting failures with isolated fixtures.
It does not establish new engine equivalence, supported-session coverage or live
replacement. Full player-journey corpus, scientific scope review, transitive
dependency review and additional binary mappings remain outstanding.

[`COV01.execution-claims-20261008`](coverage/execution-claims-validation-20261008.json)
records 60 audit guards, 51 gate guards, 18 UI/HTTP tests and 29 desktop/mobile
browser checks with stable declared sources/scopes. It binds the toolkit and
coverage UI only; unrelated concurrent native/resource work remains outside the
execution result. The previously failing committed `HEAD~2..HEAD` comparison
also passes through exact receipt composition with historical accounting rules.

## Scoped direct-word sprite raster MVP — 2026-10-07

`RS.word-raster`, `RS.word-route` and `NR.word-admission` add an opt-in native CPU
raster route at pinned No-CD backends `0x596cb8` / `0x597086`. Fully in-bounds
contiguous direct-word SPR draws are admitted; original clipping, empty/indexed
requests and caller-side auxiliary passes remain on their original paths. Complete
World rendering and the ordinary capture/replay pipeline retain their boundaries.
This is a partial DRAW-05 route, not DRAW-03 scene completeness or DRAW-07.

Fresh original comparisons match 256 synthetic draws / 119,288 complete canvas
pixels; 647 native refusals leave destination bytes unchanged. The independent
64-bit CTest checks full pixels, opaque zero, origins, stride, edge refusal, late
malformation and aliases. A live shadow run through Quick Battle setup and World
entry matches 12,608 admitted destination/workspace pairs with zero mismatches
and 940 forwarded refusals. The separate takeover run bypasses at least 12,608
original backend calls, forwards 705 refusals, and captures eight complete native
before/after canvas/workspace samples. Independent unmodified original execution
matches all 1,070,400 sample pixels and selected workspace words. Original
simulation, other raster producers and auxiliary drawing remain active.

`MNM_WORD_SPRITES=takeover ./tools/run-qt-shell.sh --frame-readback --skip-movies`
selects the partial route; `shadow` selects comparison. The actual staging entry
point passes pinned import/DLL/mode/fresh-directory checks. GUI Launch-button,
full session/shutdown, register/exception-state stress, loss/recovery, sustained
play and physical driver validation remain pending. Diagnostics copy canvases;
no performance improvement is claimed. The initial call-site-only observation
saw zero calls and establishes no bypass; the failed relative-path report and
both corrected live results remain separate historical records. All installed
experiments verify the 2,927 immutable originals before and after.

See [contract and evidence](native-word-sprite-mvp.md),
[shadow result](native-word-sprite-shadow-20261007.json),
[takeover comparison](native-word-sprite-takeover-20261007.json), and
[launcher staging](native-word-sprite-launch-staging-20261007.json).

## Surface contention and application-held presentation — 2026-10-07

[Diagnosis and retained evidence](render-surface-contention.md) records four
original BltFast `DDERR_SURFACEBUSY` failures in the user's legacy observer-readback
run. Exact lock/DC ownership was not traced. Ordinary native-command launches
already imply application-held capture; explicit `--frame-readback` now does too,
removing observer DirectDraw locks from both ordinary presentation modes.
Six actual-shell synthetic launch cases, fourteen PE32 ownership cases and fresh
queue/input/fallback regressions pass. An isolated original campaign completes
120 seconds in the idle World,1393 native frames/104127 commands with no logged
surface failures or recovery requests, no ordinary native readbacks/uploads and
zero terminal resources. Original inputs verify before/after. This is bounded
Xvfb/Mesa observation with stable menu comparisons; hours-long active battles,
movies, user's physical driver and replacement remain pending. Explicit legacy
draw/history readback diagnostics retain their observational boundary. Historical
shared launch/test evidence keeps its original hashes; no status promotion.

## Version3 native clipping replay — 2026-10-06

[Protocol and evidence](../formats/render-surface-commands-v3.md):v3 carries owned
clip state, signed one-to-one Surface2 copy requests and independent HRESULT checks.
All408 retained Wine outputs pass408 result and816 native pixel checks; partial
errors retain earlier writes and allow subsequent commands. Uneven-fragment GPU
replay performs no ordinary readbacks, retires owned resources and preserves an
unrelated surface on failures/abort. Sixteen invalid/mismatch streams and21 native
ownership/diagnostic checks pass; v3 palette inheritance, legacy v1/v2 regressions,
direct backend408/18-policy checks and original opaque fixture replay pass.
Live v3 negotiation/producer region observation, other format/key/fill combinations,
original retry/Restore, native locks/loss and Windows/game equivalence remain pending.
Historical shared-protocol/consumer evidence remains at its recorded hashes.


## Native RGB565 clipping backend — 2026-10-06

[Backend policy and evidence](native-surface-clipping.md):408 independent retained
Wine Surface2 HRESULT/full native-output pairs match real native GL operations,
including the two six-pixel partial-write errors. Owned clip state distinguishes
detachment, missing/empty lists and ordered disjoint regions;18 native admission
and update/swap/destruction checks pass. No ordinary pixel uploads/readbacks.
Existing native surface/blit suites and five original opaque fixtures pass afresh.
Native policy is headless; wire clip state, other formats/key/fill interactions,
original error/retry/Restore and Windows-driver/game equivalence remain pending.
Historical shared renderer evidence retains its old fingerprints; these scoped
new comparisons do not refresh unrelated preview/live validations.


## Surface2 driver clipping and errors — 2026-10-06

[Independent external-driver evidence](surface-driver-clipping.md) now retains
408 real Wine 11.16 Surface2 RGB565 API calls and a portable before/after corpus.
Four offline tests reproduce all HRESULTs and full destination outputs. Two Blt
calls partially write a first clip region before a later invalid source piece
fails; failure atomicity is therefore removed from the acceptance matrix.
No-clipper bounds rejection, explicit/disjoint/empty/missing lists, BltFast clipper
rejection, busy surfaces and scoped flag/key results remain driver/environment
specific. Original wrapper error/retry/Restore handling, real game region use,
Windows-driver equivalence, native clipping/protocol integration and replacement
remain pending. Existing original RGB565 opaque fixtures were replayed afresh
through CPU/OpenGL after the matrix edit, retaining all previous evidence hashes.


## Original rectangle forwarding boundary (2026-10-06)

`RE.surface-rectangle-forwarding` now has 966 isolated original-wrapper argument
comparisons and four portable offline regression tests. Five selected opaque/keyed
and explicit-rectangle entries forward tested outside/empty/reversed inputs;
windowed primary translations, wait/key flags, wrapped coordinate arithmetic and
explicit destination-RECT mutation are scoped separately from driver output.
[Contract and remaining clipping work](surface-rectangle-forwarding.md).
Driver clipping, API failures/retries, clipper regions, native clipping and live
replacement remain pending. Existing pixel fixture source fingerprints and
comparison scopes are unchanged; the new trace corpus contains no driver pixels.

## Native surface operation matrix and independent offline fixtures (2026-10-06)

Milestone 3.1 now has a [required/conditional operation matrix](native-surface-operation-matrix.json)
and [portable original fixture corpus and procedure](native-surface-fixtures.md).
`RE.surface-copy.rgb565-bltfast` records only five retained original-game driver
opaque RGB565 BltFast outputs at return PC `0x0058c5be`; fresh CPU/OpenGL replay
matches all 2,400,000 destination pixels and independent CPU presentation colors.
`NR.surface-fixture-corpus` owns offline admission, provenance hashes and missing
operation reporting. Five meaningful corpus/refusal tests pass. Captured-after
pixels are comparison inputs only. Historical manifests pin executable/DLL
identities but lack collector source fingerprints; this is not current-hook,
physical-driver, live equivalence or replacement evidence.

Other formats, exact keys, palettes, clipping, fills, same-surface overlap,
Restore/loss/retry, update/lifetime and paired flip output fixtures remain pending.
Required rows retain their untested branches; ranged/destination keys, conversion,
stretch, extra effects and longer flip chains remain conditional discovery work.
Native command recovery and complete checkpoint attachment remain separate from
original restoration semantics. The fixture milestone reports incomplete.

Ledger reviewed: 2026-10-05 (cross-subsystem register, evidence, links and
remaining-work reconciliation; initial baseline: 2026-10-03). Dates on individual
milestones remain the dates recorded in their evidence, including 2026-10-06.
This ledger tracks reconstructed functionality,
modern implementations, and replacement of original work during live execution.
Those are separate milestones: offline tests and capture hooks do not establish
completed engine replacements.

This review reconciles the central register and retained research through
`62d8ac9`, including native audio boundaries, campaign menus through UI35,
native scenes through NS20, effect metadata regions and command transport.
Recovered audio models remain offline; Qt output and selected native voice
routing have separate host-backend and live user-observation evidence. Default
game launches retain Wine DirectSound.

The committed-baseline offline audit at `62d8ac9` passes with 364
behavior/policy/gap entries, 291 evidence records and 81 bounded scenarios. It
reports 141 stale evidence records and no source-census or focused-register
drift. No registered behavior in that baseline claims live replacement.
Concurrent renderer edits initially produced working-tree census/link drift.
A later audit after their census refresh passed with 366 entries, 1,104 indexed
files and 144 stale evidence records, while their change receipts were pending.
The cooperative ring adapter and bounded scene subsequently landed in
`d955595` and `c06fffa`; their scoped evidence is recorded below. New renderer
edits remain separate from this documentation review and require their own
accounting and validation.
This review checks documentation and accounting; it does not rerun original
comparisons or promote engine status. See the
[committed-baseline audit summary](coverage/summary-ledger-audit-20261005.md) and
[ledger review](coverage/ledger-review-20261005.json).

2026-10-06 native audio follow-up: the user reports working native audio in the
readback-disabled configuration. Latest matching experiment `run-f2336iox`
confirms one native selection, zero startup fallbacks and zero failed submitted
requests at inspection. Run association is inferred; level/duration and exact
sounds are unspecified. This adds live selection and user-reported audible
playback evidence for AU06, without promoting equivalence or complete
replacement. See [live observation](native-audio-voice-bridge.md#2026-10-06-user-live-observation)
and evidence `AU06.user-live-20261006`.

2026-10-05 native PCM core completion: supported mono/stereo 8/16-bit
formats, encoding-correct initial silence, alias/lock retirement and allocation
failure atomicity now have scoped headless execution evidence. Four core fixtures
pass normally and under ASan/UBSan with leak detection; all 60 standalone
audio/asset CTests pass. See [contract and execution](native-audio-boundaries.md)
and `NA.core-boundaries-20261005`. Physical output, concurrency, long sessions,
counter exhaustion execution and original equivalence remain separate boundaries.
Historical evidence fingerprints remain intact.

Current assessment: substantial pathfinding reconstruction and native rendering
infrastructure exist. Selected input and media paths have optional adapters.
Native menus and a bounded ANI/SPR scene run independently as previews; selected
original menu transitions have live observation evidence with original work retained.
No complete gameplay subsystem is established as replaced by the reviewed
evidence. Whole-game functional and performance coverage are **unknown**.

2026-10-04 menu follow-up `run-qyfqz9tj`: the user identifies a 20+ second stale
Forest of Pain difficulty picture while Wine remains smooth. The publication
journal records long early gaps. An isolated live reproduction confirms menu
contexts written through GDI were invalidated without a new checkpoint.
Application-held GetDC/ReleaseDC bitmap capture now restores those pixels;
the automated software-X11 check observes Forest of Pain within 2.449 seconds
and difficulty selection within 0.785 seconds, continues publishing during ten
idle seconds, and passes Qt readback (`run-reir4438/report.json`). Eight real-DIB fixtures
pass independent pixel/replay/Qt checks. This is bounded menu evidence with
original drawing retained; NVIDIA Qt latency and long battle sessions remain
unverified. See [GDI checkpoint evidence](opengl-game-owned-dc.md).

2026-10-04 performance follow-up `run-8s6ddeg4`: the user still reports slow Qt
presentation. Retained state has 165 updates, but post-exit inspection provides
no live rate. Primary-scoped contention remains in the bounded log. Bounded
cross-thread guard waiting now delivers 20/20 synthetic overlap updates versus
0/20 with immediate skipping; this is continuity evidence, not a live FPS gain.
A read-only per-run publication-rate log now supplies the missing timing boundary.
Live presentation performance remains unverified. See
[bounded-wait evidence](render-startup-black-screen.md#2026-10-04-low-qt-update-cadence-bounded-tracker-waiting).

2026-10-04 freeze follow-up `run-wf_pb32k`: the user reports intermittent menu and battle
freezes in Qt while Wine continues animating and responding. The retained frame
stream contains 61 frames; bounded lifecycle evidence records pixel contention,
whole-pixel resets and rejected incomplete bootstrap operations. Continuous live
presentation remains unreliable. Component-scoped pixel invalidation and guarded
rectangle conversion now have synthetic checks; their live effect is unverified.
See [latest freeze investigation](render-startup-black-screen.md#2026-10-04-intermittent-qt-freezes-after-performance-changes).

2026-10-04 live follow-up `run-yfi5mzh2`: the user confirms battle is now
visible in Qt, but Qt is very slow while Wine stays smooth. This establishes
live menu-to-battle presentation for the reported run, not scene equivalence or
acceptable performance. Capture optimizations have synthetic follow-up evidence;
live speed after those changes remains unmeasured. See
[performance follow-up](render-startup-black-screen.md#2026-10-04-battle-visible-qt-performance-follow-up).

2026-10-04 hands-on RE02 failure: default-readback experiment `run-gpaw8536`
returned application BltFast `DDERR_SURFACEBUSY`, matching the user's locked
surface dialog. Cause and live presentation equivalence remain unresolved;
The user reports successful operation in follow-up `run-88j862gr` with observer
readback disabled and no Qt frames, implicating readback as a contributor.
Game-owned capture/presentation remains to be checked separately. See
[run evidence](render-startup-black-screen.md#2026-10-04-hands-on-default-readback-failure).

Follow-up `run-wuk2aaev` confirms real game-owned RGB565 offscreen snapshots and
selected blit propagation while the user reports successful gameplay. Qt still
has no primary frame; the 16-snapshot diagnostic limit stops further copying.
This advances live capture observation only, not continuous Qt presentation or
render replacement. Primary admission remains unresolved live. Recording/checkpoint separation
now has a code fix and synthetic follow-up below (same linked run evidence).

RE02 continuity follow-up: `run-823dsolm` reproduces the sixteen-snapshot
capture limit and no recorded primary presentation. Lock/blit/flip checkpoint
maintenance and indexed publication now continue independently of bounded disk
recording; uncontended final surface Release preserves unrelated primary
metadata. Synthetic CPU/PE32/OpenGL/Qt checks pass for continued publication,
release ownership, failed calls and bounded session recording. Evidence and the
fresh live validation protocol are recorded
in [continuity fixes](render-startup-black-screen.md#2026-10-04-continuous-owned-checkpoint-fixes).
No successful continuous live Qt run or rendering replacement is claimed.

RE02 live follow-up `run-o70h037m`: eight menu frames were published, while the
user confirms Wine reaches battle and Qt retains the menu. Known-component
unmatched Unlock now invalidates only that component; flagged exact source keys
from application Lock/GetSurfaceDesc are admitted alongside SetColorKey evidence.
Ranges and unknown identities retain conservative rejection/invalidation.
Five bootstrap, fourteen lifetime and five primary blit fixtures pass, including
22-frame continuity after a known-component unmatched Unlock and independent
pixel checks for descriptor-derived transparency.
See [menu-to-battle evidence](render-startup-black-screen.md#2026-10-04-menu-captured-battle-presentation-stalls).
Continuous live battle presentation remains unvalidated.

RE02 follow-up `run-2i2a39c4` still publishes only five menu frames while Wine
reaches battle. Exact source keys are captured, but the log contains global
epoch invalidation and rejected source-NULL colour fills. Pixel uncertainty now
preserves validated screen metadata; uncertain identity/property changes retain
full invalidation. Supported native colour fills can initialize destinations
before transparent composition. Ordered owned sessions retain a fill GAP boundary.
Thirty-six synthetic checks pass: twelve bootstrap/fill/recovery cases, fourteen
Lock/Unlock lifetime cases, four partial-lock cases, three Flip cases and three
ordered-session cases. Pixel contention/unknown Unlock recovery publishes 22
exact frames; missed property or retirement tracking still discards metadata.
See [continued freeze evidence](render-startup-black-screen.md#2026-10-04-continued-menu-freeze-epoch-reset-and-colour-fills).
Live battle presentation and complete scene equivalence remain unvalidated.

IN01 hands-on follow-up: the user reports everything working in the native
Wine-window hosting run logged at `working/logs/run-20261004T160537Z.8Kl2VI/`.
This is live user observation of the optional embedding backend, separately
from the failing OpenGL frame-stream path. Individual focus/resize/detach
actions, map and duration were not enumerated; no whole-session equivalence
claim follows. See [hosting evidence](qt-shell-hosting.md).

## Scope and evidence rules

Primary runtime reference: No-CD PE32 executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Some configuration findings use the clean executable SHA-256
`124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`.
Follow each linked finding's build and address restrictions. This ledger does
not assert stable runtime pointers or compatibility with other builds.

Status applies to the behavior named in each row, not its entire subsystem.
These dimensions are independent; do not add them into a completion score.
Short ledger labels identify historical milestones, not canonical behavior IDs.
The atlas also used UI02/UI03/UI04 labels; its `TOOL.coverage-ui` registration is
separate from Map Selection, Main/Quick dispatch and Multiplayer Game Selection.
Use the register's stable behavior IDs when accounting for a changed contract.

| Dimension | Values | Meaning |
| --- | --- | --- |
| Understanding | Unknown; partial; scoped | Recovered knowledge within the named scope; a function link does not cover every branch |
| Implementation | None; partial; scoped | Code exists for the named scope; scoped does not mean the whole original contract |
| Original comparison | None; recorded | Matching-build execution evidence within its stated scope; current source freshness is separate |
| Integration | None; headless; preview; live observation; live equivalence | Observation retains original work; preview and headless checks do not imply game integration |
| Replacement | None; scoped live | Requires recorded comparison, live equivalence and evidence that the original work was bypassed |
| Evidence type | Static; synthetic; original comparison; installed assets; native integration; live observation/equivalence | Record all applicable kinds without promoting another status dimension |
| Confidence | Unknown; provisional; high within scope | Confidence in stated evidence, not untested compatibility or current fingerprint freshness |

Synthetic x86 tests establish particular ABI/lifecycle contracts. Installed
asset comparisons establish byte/decode agreement, not playback or loader
equivalence. Displaying captured game frames in Qt does not replace original
drawing. Hooks forwarding original operations count as observation.

Most baseline rows use source inspection and linked documents' recorded results;
their checks were not rerun for the original ledger change. Asset rows were
updated after the six-chunk native asset workflow: installed raw-byte/PCM
checks and the pinned Windows file API audit were rerun with original-manifest
verification before/after. This does not promote those paths to live replacement.

Validation counts below are historical results from the linked milestones,
not new results from this documentation update. Earlier rows describe their
bounded scenarios; later milestones may resolve a listed next step for another
scope. The central register records independent status dimensions and current
fingerprint freshness. Broad baseline entries with static `REVIEW.*` evidence
may remain partial even when their research documents report narrower tests;
this review does not infer missing semantic mappings or promote those entries.
Incomplete declarations and inspection tools without a documented validated
contract do not advance coverage.

## Subsystem coverage

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| PF01 Route request, search ordering, budgets and continuation | Scoped host models; offline replay and observation tools; original search retained | Static/synthetic; high within recovered rules. [Models](../../reconstruction/pathfinding/README.md), [sequence capture](pathfinding-search-sequence.md) | Compare real-map fresh/resumed searches in both known contexts: route bytes, budgets and flags |
| PF02 Neighbors, acceptance, movement predicates and costs | Scoped reconstructed rules with explicit world inputs; offline | Static/synthetic and isolated original i386 comparisons; high for tested helpers. [Evidence](../../reconstruction/pathfinding/README.md#evidence-and-confidence) | Match real-map candidates and costs; helper agreement alone does not establish complete search agreement |
| PF03 Frozen world ownership and per-context replay | Scoped snapshots containing one creature; offline | Synthetic capture/replay checks; high within scope. [World format](../formats/route-world-snapshot.md), [continuation](pathfinding-search-sequence.md) | Validate coherent live capture and refreshed continuation inputs; multi-creature scheduling remains separate |
| RE01 Persistent surfaces, rectangular updates, opaque/keyed/masked copies, palettes and compatible swaps | Scoped OpenGL implementation; offline replay/demo and sprite upload | Synthetic CPU/OpenGL comparisons; selected installed SPR draws/presentation match isolated original routines. [Renderer](../../renderer/README.md), [sprites](native-sprite-rendering.md) | Inventory unsupported operations and real-map usage; original live drawing remains active |
| RE02 Frame capture, owned pixel tracking and Qt presentation | Scoped bridges; observation and optional viewport | Synthetic x86/CPU/OpenGL/Qt comparisons and bounded original startup command observation; high within bounded contracts. [Presentation](opengl-presentation.md), [owned session](opengl-owned-session.md), [successive startup frames](opengl-owned-session-sequence.md) | Validate complete maps/transitions without gaps; bounded captures do not prove continuous replacement rendering |
| RE03 Complete live rendering backend | Partial foundation; no complete replacement recorded | End-to-end equivalence unknown. [Renderer boundaries](../../renderer/README.md), [startup investigation](render-startup-black-screen.md) | Extend the bounded cooperative PE32/Qt ring adapter with idle retry/shutdown and streaming-limit handling; preserve CPU locks and recovery, inventory required drawing operations and validate sustained gameplay against the original driver |
| IN01 Qt shell and foreign-window hosting | Scoped X11/XWayland host; optional shell | Synthetic embedding/detachment and separate user-reported working live hosting; high for fixtures, bounded user observation. [Hosting](qt-shell-hosting.md) | Enumerate actual game focus, dialogs, resize, detach, exit and duration; the unenumerated user report does not establish full-session equivalence; native Wayland unsupported |
| IN02 Input forwarding and selected polling APIs | Scoped event forwarding and three USER32 hooks; optional adapter | Synthetic Qt/X11 and Qt-to-x86 checks; high within scope. [Input](qt-input-forwarding.md) | Live menus, movement, scrolling and focus; original window/message handling retained |
| ME01 Movies and supported WinMM file sounds | Scoped Qt playback/hooks; optional adapter with fallback | Synthetic ABI/lifecycle/decode and installed Intro0 decode probe; high within scope. [Media](qt-native-media.md) | Full live playback, audible output, skip/return and call frequency; unsupported calls remain legacy |
| AU01 DirectSound setup, static PCM upload and duplicate storage | Scoped reconstructed setup/native storage; offline | Static/synthetic and 356 installed WAV comparisons; high for bytes/ownership. [Evidence](directsound-buffer-setup.md), [storage](../../audio/README.md) | Optional selected voice routing is AU06; native voice/mixer/output evidence is separate in AU02–AU05. Default game launches retain Wine DirectSound; complete original manager and live upload/ownership equivalence remain unverified |
| AU02 Recovered voice controls and retirement contracts | Scoped selected engine blocks and fake backend; offline | Static/synthetic: play/stop, looping, volume/pan, status/zero reset, deadlines and bounded frequency/cursor audit; high within selected contracts. [Evidence](directsound-voice-controls.md) | Selected retirement, scheduling, positional and camera contracts have separate AU08–AU11 evidence; whole-program call coverage and live timing remain unverified |
| AU03 Native secondary voice state | Scoped native implementation; offline only | Synthetic source-frame advancement, independent duplicates/controls, completion/loops and ownership; high within tested native policies. [Evidence and repeatable check](native-audio-voice-state.md) | Qt output/routing are separate in AU05/AU06; no validated live replacement; completion/cursor policies are not demonstrated Wine equivalence; fractional output-frame timing is covered separately in AU04 |
| AU04 Native stereo PCM mixing and resampling | Scoped native implementation; offline only | Synthetic PCM fixtures and 120 independent format/rate/loop scenarios, exact split-block continuity, gain/clipping and lifecycle checks; high within tested native policies. [Evidence and repeatable check](native-audio-mixer.md) | Qt output/routing are separate in AU05/AU06; no validated live replacement; linear interpolation is not band-limited downsampling or bit-exact DirectSound; real-time allocation/latency remain unverified |
| AU05 Native Qt PCM output | Scoped native device adapter; fixture and host-backend smoke validated | Synthetic format negotiation/partial-write/backpressure/lifecycle tests; default host stereo Int16 48 kHz tone delivered before and after restart. [Evidence](native-audio-output.md) | Optional voice routing has separate AU06 evidence; speaker audibility, live x86 replacement, long sessions, latency and device-change recovery remain unverified |
| AU06 Selected x86 DirectSound voice routing | Optional native adapter; fixtures/staging and bounded live selection observation | Native PCM/control/lifetime and mapped-channel tests, PE32 stdcall COM fixture, stale-host timeout and guarded import checks; one native selection and user-reported audible playback in inferred run-f2336iox. [Evidence](native-audio-voice-bridge.md) | No original waveform/timing equivalence or complete replacement; primary caps remain scaffolding; pitch, nonzero seeks and cursor queries unsupported. Fallback/unsupported paths, transitions, duration and complete voice lifecycle remain unverified live |
| AU07 Primary audio and selected manager lifecycle | Static reconstruction and native/x86 fixtures; offline only | Startup volume capture, control gates, selected disable/shutdown ordering, native master attenuation and protocol-v2 COM controls; matrices, exact PCM, sanitizers and staging. [Evidence](primary-audio-manager.md) | Full wrapper/list/COM destruction, hardware format negotiation, primary pan and live/aural/timing equivalence remain unverified; manager model is not injected |
| AU08 Voice retirement and wrapper ownership | Static reconstruction and synthetic native fixtures; offline only | Stop/reset/notification failure ordering, duplicate traversal, postorder Release/free, reset fields and circular reusable-ring transforms; 432 retirement and 204 ring cases plus exact native PCM/ownership and sanitizers. [Evidence](audio-voice-lifetimes.md) | Full source-list/allocator ownership, initialization unwind, manager integration and live timing/thread behavior remain unverified; no runtime replacement |
| AU09 Voice scheduler selection and ordering | Static reconstruction and synthetic native fixtures; offline only | Free/expired selection, strict-volume admission, former-tail eviction, assignment, old-volume reordering and whole-ring clearing; 5184 selection/972 ordering/64 failure cases, exact native PCM and sanitizers. [Evidence](audio-voice-scheduler.md) | Full manager start/duplicate admission, source-list ownership and live thread/cadence compatibility remain unverified; no injected scheduler |
| AU10 Positional audio arithmetic and existing voice updates | Static reconstruction and synthetic native fixtures; offline only | Wrapped deltas, approximate distance, range attenuation, signed map-byte branch, four-orientation pan and update failure ordering; 2312 geometry/432 update cases, exact stereo PCM and sanitizers. [Evidence](positional-audio.md) | Selected camera projection has AU11 evidence; map-byte lookup ownership, floating edge equivalence and live audible/timing behavior remain unverified; no injected geometry |
| AU11 Audio listener camera projection | Static reconstruction and synthetic fixtures; offline only | Complete selected origin/projection, both audio screen centers, four orientations and map normalization; 1152 screen/map and 68644 division-boundary cases, positional integration and sanitizers. [Evidence](audio-camera-projection.md) | Live field capture, camera/viewport lifecycle and listener agreement remain unverified; map-byte lookup separate; no runtime injection |
| AU12 Positional attenuation map lookup and snapshot ownership | Static lookup reconstruction and synthetic fixtures; offline only | Signed bytes, captured row/layer tables, logical Z shift, deferred range/enable gates, owned snapshot lifetime and camera integration; 256 byte cases, 18 CTests and sanitizers. Original five-array ownership documented. [Evidence](audio-attenuation-map.md) | World map generation/update algorithms, live capture coherence, byte agreement and audible behavior unvalidated; no runtime injection |
| AU13 Voice admission and duplicate start | Selected static manager/duplicate reconstruction and offline/native fixtures | Manager gates, slot-before-load ordering, group/ID resolution, status decisions, retained duplicates on control failure, source-ring rotation and publication; corrected DirectSound loop status, 19 CTests, exact overlapping PCM, sanitizers and PE32 COM fixtures. [Evidence](audio-voice-admission.md) | Selected cache loading/recycling has AU14 evidence and profile/table initialization has AU15 evidence; allocation failure, RNG implementation and live ownership/thread/cadence/call coverage remain unverified; no injection |
| AU14 Source-cache loader and replacement ownership | Selected static reconstruction and offline native/file fixtures | 625 cache-selection cases, groups/cached hits, destructive release/open/upload ordering, profile-path boundary and corrected sound IDs; Qt file → cache → admission → exact PCM, 22 CTests and sanitizers. [Evidence](audio-source-cache.md) | Selected profile/table initialization now has AU15 evidence; WinMM internals, unsafe null-create cleanup, live cache ownership/thread/cadence and adapter integration remain unverified; no live replacement |
| AU15 Manager profile tables and audio pools | Selected static reconstruction and offline fixtures | Sound/group IDs including duplicate slack, comma tokenizer, per-map classifications, wrapping file-size budget, permanent preload retention and source/scheduler rings; 542,368 arithmetic cases, 23 CTests and sanitizers. [Evidence](audio-manager-configuration.md) | Decoded Windows profile/stat semantics delegated; selected aggregate startup/table cleanup has AU16 evidence; unsafe allocation domains, live map/thread/ownership and adapter integration remain unverified; no injection |
| AU16 Aggregate audio manager lifecycle | Selected static reconstruction and offline lifecycle fixtures | Catalog/device/primary/scheduler startup, exact failure ownership, shutdown and destructor order, retained primary/schedules and explicit unsafe dereference boundaries; 26 CTests and sanitizers. [Evidence](audio-manager-lifecycle.md) | Windows profile/stat semantics delegated; scoped native backend has AU17 evidence; COM/refcount/allocator and external primary ownership, live thread/timing and audible transition agreement remain unverified; no injection |
| AU17 Native manager backend and profile-to-PCM pipeline | Scoped native adapter and synthetic Qt/file/PCM integration; offline only | Bounded ASCII profile snapshots, WAV preloads/admission, exact overlapping/master-gain PCM, duplicate rotation ownership, one-shot completion, teardown silence and retained-primary restart; 29 CTests and sanitizers. [Evidence](native-audio-manager-backend.md) | Full Windows profile/encoding/stat equivalence (selected installed profile/catalog agreement is AU19), active-map caller ordering, original COM/primary ownership, live thread/timing/audible transitions and injection remain unverified |
| AU18 Native profile API compatibility | Generated ASCII input; PE32 Wine 11.16 reference comparison; offline only | 29 selected string/section/integer calls: 22 exact matches, seven intentional partial-list/numeric policy differences, zero unexpected mismatches; fallback-space and exact section-capacity fixes. [Evidence](../formats/profile-api-comparison.md) | Historical Windows behavior, encoding/registry/duplicates/concurrent updates and live replacement remain unverified; installed selected catalog agreement is AU19 |
| AU19 Installed audio profile/catalog compatibility | Qt-backed installed input and PE32 Wine 11.16 comparison; offline only | 499 exact API matches; identical decoded 412-source/69-group catalogs, 168 resolved members and limit 12; header-comment fix, manifest guards and sanitizers. [Evidence](../formats/installed-audio-profile-comparison.md) | Installed map lists are empty; nonempty classifications, tuning integer conversion, historical Windows/registry/encoding/stat behavior, live caller ordering and replacement remain unverified |
| AU20 Installed native audio manager pipeline | Installed Qt/file/manager/upload/admission/PCM integration; offline only | 995 cases: 918 independent exact PCM previews, 340 one-shot completions, 168 successful group selections, overlap/gain and cleanup/restart; manifest guards, 37 CTests and sanitizers. [Evidence](installed-native-audio-manager.md) | 77 observed path failures include 67 direct logical group names, four quoted-path gaps twice and missing Stream twice; original file-open behavior, nonempty map preload/classification, live timing/ownership/audible replacement remain unverified |
| AU21 Audio filename boundary/native adaptation | Hash-pinned static wrapper flow and explicitly selected native file policy; offline only | Original retains comment-closing apostrophe and passes the path unchanged to its file wrapper; opt-in missing-leaf dequoting loads four existing WAVs, with 926 exact PCM previews, 344 completions, 39 CTests and sanitizers. [Evidence](audio-source-filenames.md) | Literal default and original reconstruction preserved; Stream remains missing, fresh cabinet listing unavailable, historical Windows/CRT/live caller and audible replacement unverified |
| AU22 Native audio catalog preflight | Read-only native startup availability; offline only | 344 file sources/69 groups, all 168 group choices; literal 339 ready sources, compatible 343 with only Stream missing; independent WAV oracle, guarded unchanged inputs, synthetic edge cases and sanitizers. [Evidence](audio-catalog-preflight.md) | Conservative group gate, stable selected assets only; other installations, original caller ordering, permanent preload capacity and live playback remain separate |
| AU23 Qt audio manager session | Recovered/native manager connected to application-owned Qt sink; synthetic output validation only | Negotiated Device clock, stable caller slots, exact queued overlap PCM, missing-source gates, partial discard, map/policy/rate restart, failure unwind and thread confinement; nine scoped CTests and sanitizers. [Evidence](qt-audio-manager-session.md) | Real QAudioSink path implemented; audible device behavior, backend unplug/recovery, listener/camera updates, original timing and live manager routing/replacement unverified |
| AU24 Native menu audio cues | Opt-in native presentation/application integration; synthetic audio only | Semantic mouse/keyboard/page cues, accepted effects volume, shared Device, queue/duplicate cancellation on transitions, close/failure/restart; exact generated PCM, 23 scoped CTests and sanitizers. [Evidence](menu-audio-integration.md) | Cue IDs are explicit native policy; original mappings, physical audio quality/recovery, restart latency, music/persistence and live original audio routing unverified |
| AU25 Native audio output recovery | Default/pinned Qt-device monitoring and application recovery; synthetic validation only | Deferred/coalesced faults, visible retry, availability recovery, retained accepted gain/policy, changed Device clock, silence on reopen and closure cancellation; 24 scoped CTests and three sanitizer fixtures. [Evidence](audio-output-recovery.md) | Physical device notifications/selection, audible recovery, driver failures, latency and long sessions unverified; original game audio routing unchanged |
| AU26 Native menu effects persistence | Application-owned user-scoped QSettings INI and semantic Preferences seeding; offline only | 24 scoped CTests: first-cue exact gain, fresh-process restore, accepted writes, byte-identical Cancel/startup, invalid values, offline acceptance/recovery and failed-write session continuity. [Evidence](menu-audio-preferences.md) | Other preferences/music persistence, original settings import, real user-directory failures, physical playback and live original routing unverified |
| AU27 Native menu music | Explicit local track through application-owned Qt playback adapter, independent saved gain/status; synthetic controller and finite decoder only | 25 scoped CTests, guarded/deferred callbacks, navigation continuity, mute, separate settings, fresh-process restore, failure/closure/destruction, exact generated PCM and missing-file decoder checks, sanitizer/leak fixture. [Evidence](native-menu-music.md) | Audible coexistence, real looping/codecs, hardware recovery/default changes, original music choices and live routing unverified |
| AU28 Native menu music output recovery | Qt default-device monitoring and explicit device binding, application retry/availability recovery; synthetic validation only | 26 scoped CTests, device-ID decisions, deferred/coalesced faults, manual/automatic recovery, retained track/gain, same-turn default switch, stale/close cancellation and two sanitizer/leak fixtures. [Evidence](menu-music-recovery.md) | Physical notifications/binding/reopen, audible coexistence/looping, driver errors/latency and long sessions unmeasured; original music routing unchanged |
| AS01 Loose asset resolution, read-only handles and WAV input | Scoped native backend and WAV loader; six-chunk milestone complete within scope; SPR/ANI consumers recorded separately | Synthetic path/file/lifetime/error tests, 4,834 installed raw files and 356 decoded WAVs; high for bytes/native policies. [Assets](../../assets/README.md), [raw evidence](../formats/asset-file-comparison.md), [WAV evidence](../formats/pcm-wav-loading.md) | Original wrapper compatibility, remaining asset formats, writable state and live integration remain unverified; SPR/ANI have AS03/AS04 evidence |
| AS02 Windows file actions and delegated file access | Static audit only; no new runtime replacement | Pinned clean/No-CD/JPEG imports, IAT references and save temp-path evidence; high within static scope. [Audit](windows-file-api-audit.md) | Recover save/config/profile/listing/metadata/path contracts and DLL/COM loaders; imports are not live call coverage |
| AS03 SPR loading and rendering asset formats | Native version-4 indexed/RGB565 loader and owned mask/origin-aware OpenGL upload; offline only | Reviewed 2026-10-04: 174 installed SPRs decoded (59,407 frames), 91 native OpenGL/original draw and presentation matches; fixture ownership/bounds/limits and ASan/UBSan checks. [Loading](../formats/spr-native-loading.md), [rendering](native-sprite-rendering.md) | Original palette construction, lighting/effects, legacy ANI and live loader integration remain separate; SFT input is tracked in AS15; all seven legacy SPRs now decoded offline with independent complete comparisons; original legacy reader/live equivalence remains unverified; forward ANI and bounded scene work are tracked in AS04 |
| AS04 ANI tables, forward playback and bounded sprite/terrain scenes | Native versions-3/4/5 ANI loader (older records normalized), selected No-CD forward/phase-switch and placement model, numeric group/facing and explicit two-layer and configured mode-one Qt preview with selected original depth/queue ordering and optional SPR bitmask visibility, plus selected ordinary terrain submission and a direct TTD/SPR Qt tile preview and owned MAP grid loading with bounded installed slices plus selected four-orientation camera/world terrain traversal and clipped presentation plus selected recovered camera map binding/viewport/position/scroll setters and owned ordinary-terrain initialization with recovered geometry/surface admission and explicit section-grid placement/rotation and owned authored-region recipe loading/mixed-height assembly and selected single-block candidate/location/seeded selection primitives plus multi-block descriptors/connectors/admission and bounded pruning plus owned placement/count rollback and a selected one-attempt backtracking driver plus two-pass Specific placement/rotation/location fallback plus ten-attempt reset/seed orchestration plus owned recipe/header catalog construction and original connector-threshold policy plus generation from complete available recipe catalogs and successful-assignment conversion to owned ordinary assembly plans plus complete installed generated MAP loading/assembly, geometry initialization and bounded generated Qt/OpenGL scene production plus owned mode-0 palette construction and controlled-light shaded scene drawing and installed global palette configuration and terrain palette-count preferences and owned per-cell fields with controlled source stamps and selected owned creature admission/eight-phase light refresh plus static-object region admission/additive stamps/two-phase publication and combined updater plus owned object-snapshot Qt terrain scenes plus installed effects light-table loading plus selected common effect-record placement; offline only | Reviewed 2026-10-05: 133 files/121,476 record byte matches; 347 forward traces/22,555 states and 2,140 phase-switch traces/70,620 states across 118 assets; 6,097,440 original placement helper matches; 198 layered frames/396 body states/792 layer states, plus 195 body-only frames/390 states; one configured attachment path with 520 original states and 462 complete native frames; 4,096 queue keys/1,285 sorts/164,480 full records; 72 synthetic and 24 installed overlapping queue frames; 8,192 visibility helper/1,024 reverse-pass matches and 50 installed visibility frames per normal/sanitized build; 15,204 terrain producer cases/15,317 records and 32 installed terrain frames/28 hidden decisions per normal/sanitized build; 683 MAP payloads/7,595,200 cells and 15,190,400 selected original cell checks; 56 installed MAP slice frames per normal/sanitized build; 18,428 safe world traversal cases/3,089,824 records across four views (four native-domain rejections), plus 96 installed terrain scene frames per normal/sanitized build (39,368 hidden and 22,762 partially clipped draws across both); 139,264 selected camera setter state comparisons and 16 installed camera traces/32 normal/sanitized images; 2,048 geometry fixtures/387,962 cells and 683 installed maps/7,595,200 cells with complete original geometry-pass byte matches, plus 96 initialized scene images; 8,192 synthetic section-copy fixtures/410,500 cells and 683 installed MAP selections/30,084,800 copied cells across four rotations with complete destination/source-side-effect checks, plus 96 assembled scene images; 40 installed recipe comparisons and 3,136 authored-region placement fixtures/46,656 blocks, two installed original descriptor/admission/placement fixtures/32 blocks, plus 32 authored-region scene images; 262,144 complete-state RNG draw matches, 207,840 single-block admission checks, 4,096 candidate/10,240 location tables, 78,584 original seeded choices and 1,232 installed descriptor matches; 1,736 expanded descriptor matches across all 683 MAPs, 39,344 multi-block-domain admission/carry checks, 2,048 connector lookups, 7,200 candidate/500 location tables, 7,200 seeded choices and 8,192 pruning fixtures; 3,136 placement/7,056 removal and 2,048 pruning-carry matches, plus 3,755 one-attempt solver state matches (1,940 backtracked, 231 multi-block backtracked) including 683 installed single-source catalogs; 10,786 Specific-driver state/warning matches (2,430 multi-block cases; 140 synthetic missing-connector refusals outside original execution), including 6,830 cases from 683 installed headers; 5,718 attempt-reset matches and 5,701 post-CFG generation-loop matches/19,883 attempts (84 retry successes, 1,552 exhausted cases, 241 seed wraps; 17 synthetic rollback-bound refusals), including 1,366 installed-header cases; 2,087 catalog/26,888 expanded-descriptor matches, 291 threshold-quirk cases and selected CFG fields for all 40 recipes (39 available installed catalogs; missing Medieval REGION0/Test inputs refused); 2,204 complete recipe-generation state matches with 1,194 owned plans/15,849 block assignments, 1,010 failed generations and 155 installed successes across 39 available recipes/four seeds (four unavailable Medieval REGION0 cases refused); seven complete installed layouts/130 block assignments and 1,223,200 initialized-cell matches with 112 normal/sanitized generated/authored scene frames across all views and visibility modes, plus ten failure-path checks; expanded coverage of all 39 available recipes/78 zero and wrapping-seed layouts, 1,702 block assignments, 17,987,200 initialized cells and 312 corner-camera frames (13 retry successes, no generation/camera refusals; two unavailable Medieval REGION0 seed cases refused without original execution); 1,028,096 baseline palette WORD matches across eight counts/two formats with separate 32/64-bit native checks, 400 normal/sanitized shaded tile/generated-world frames across all views and three realms, and six controlled-light refusals; 1,068 original global configuration read/admission/setter matches, 4,112,384 palette WORD comparisons across four profiles/eight counts/two formats with 32/64-bit native checks, 192 configured normal/sanitized generated scene frames and four config-request refusals; 2,112 original terrain preference admission/loader-dispatch matches and 240 preference-driven normal/sanitized generated scene frames; 564 original field/control/kernel/stamp fixtures with 32/64-bit native checks and 72 per-cell lit normal/sanitized scene frames matching original world queues and indexed draws; 96 original creature-refresh fixtures/2,304 ticks with 18,398,280 five-buffer byte comparisons and 11,520 state values matching 32/64-bit native and sanitized builds; 32,768 static admission checks, 4,096 additive stamps and 128 original static-refresh fixtures/3,072 ticks (1,536 combined creature/static ticks), 104,160,840 five-buffer bytes and 18,432 static state values matching normal/sanitized 32/64-bit models; 36 original object-snapshot ticks with complete five-buffer/eleven-state matches and 192 normal/sanitized object-driven terrain frames matching original four-view world queues and indexed draws, two default-final-tick frames and 22 pre-output refusals; fixtures and ASan/UBSan. [ANI](../formats/ani-native-loading.md), [controller](animation-forward-contract.md), [direction switches](animation-direction-selection.md), [placement/layers](animation-placement-attachments.md), [configured attachment](native-attachment-recipe.md), [queue](sprite-queue-order.md), [overlap scene](native-sprite-queue-scene.md), [visibility](sprite-visibility.md), [native visibility](native-sprite-visibility-scene.md), [terrain producer](terrain-submission.md), [TTD](../formats/ttd-native-loading.md), [terrain preview](native-terrain-preview.md), [MAP format](../formats/map-native-loading.md), [MAP cell checks](map-grid-loading.md), [MAP slice preview](native-map-terrain-preview.md), [world traversal](terrain-world-traversal.md), [rotated traversal](terrain-traversal-rotations.md), [camera setters](terrain-camera-setters.md), [map geometry](terrain-map-initialization.md), [section assembly](terrain-section-assembly.md), [authored regions](terrain-authored-regions.md), [seeded selection](terrain-region-selection.md), [region constraints](terrain-region-constraints.md), [region solver](terrain-region-solver.md), [Specific placement](terrain-region-specific.md), [generation attempts](terrain-region-generation.md), [region catalog](terrain-region-catalog.md), [generated plans](terrain-generated-plans.md), [generated scenes](native-generated-terrain-scenes.md), [expanded generated coverage](terrain-generated-coverage.md), [baseline palette shading](terrain-palette-shading.md), [configured global lighting](terrain-lighting-config.md), [terrain palette preferences](terrain-palette-preferences.md), [terrain light fields](terrain-light-fields.md), [creature lighting](terrain-creature-lighting.md), [static lighting](terrain-static-lighting.md), [object-driven lighting scenes](terrain-object-lighting-scenes.md); 132 whole-original table loads/35,244 DWORDs and profile-read contracts/11,748 type selections, 89 installed entries and six emitting types, native authored-position lighting composition and 69 normal plus 69 ASan/UBSan tests. [Effect tables](effect-lighting.md); 2,048 original common placements across 128 fixtures, 15,220,736 full cell checks and owned rollback tests; renewed 132 effect-table loads; 70 normal and 70 ASan/UBSan tests. [Effect record placement](effect-record-placement.md); whole effect trajectory step reconstructed and compared separately. [Effect trajectory](effect-trajectory.md); 6,391 whole-original same-cell movement calls/20,842 steps with produced recount coordinates and headless lighting composition. [Effect projection](effect-same-cell-projection.md); 3,232 original membership-on transition calls/10,288 steps, 8,911 owned traced cell changes/5,784 cache advances, complete guarded chain/cell/catalog reference checks and moving-light composition. [Effect cell transitions](effect-cell-transitions.md); TL09 adds 3,072 original column-query/unlink fixtures (6,144 calls) and 3,232 complete-parent calls over zero/nonzero terrain, guarded buffers, signed column layers and suppression conditions, plus strict/sanitized headless lighting and rollback. [Effect cell cleanup](effect-cell-cleanup.md); TL10 adds complete-parent blocked entry/return-1 validation, skipped destination insertion/subcell refresh, committed cleanup/cache/recount and active detached state, plus fresh ordinary/cleanup comparisons and strict/sanitized headless checks. [Blocked effect destinations](effect-blocked-destinations.md); TL11 adds whole-parent height exits with raw parameter/trajectory/counter updates before wrapping or fine/cell/membership publication, first/later lower/upper boundary cases and fresh ordinary/blocked/cleanup reruns with strict/sanitized headless checks. [Effect height exits](effect-height-exits.md); TL12 adds complete-parent disabled-membership motion preserving heads/links/flags and skipping column cleanup, bounded repeated/blocked continuations, current-cell/chain divergence and strict/sanitized admission checks with fresh prior comparisons. [Disabled membership](effect-membership-disabled.md); TL13 adds real 8x8x4 terrain-bit sampling gated by cached subcell coordinates, both membership settings, committed hit state/return 1 before subcell publication and fresh prior-contract reruns with strict/sanitized headless checks. [Effect terrain occupancy](effect-terrain-occupancy.md); TL14 adds complete 004e1200 creature footprint query with signed-coordinate bounds, MSB-first 16x16 rows and raw wrapped-height gating, original/native/sanitized exhaustive fine-coordinate and boundary comparisons, and strict rectangle-oracle units. [Creature occupancy lookup](creature-occupancy-lookup.md); TL15 adds complete 004e1260 fixed footprint initialization with raw height preservation and zero/nonzero templates, guarded original/native/sanitized overwrite and chained query proof. [Creature footprint initializer](creature-footprint-initializer.md); TL16 adds selected whole-parent creature candidates and collision handling, initial/refreshed 27-cell snapshots, creator/status filters, wrapped signed coordinate differences, first-hit ordering, type3 continuation and creator exclusion after refresh, with guarded original/native/sanitized comparisons. [Effect creature collision](effect-creature-collision.md); TL17 adds 8,144 original/native/sanitized calls covering all 27 candidate positions, normal/wrapped/aliased/vertical edge grids, competing hits, type3 mixed exits and caller-authored mutations between calls. Production and prior evidence inputs are unchanged. [Effect creature boundaries](effect-creature-boundaries.md); TL18 adds kind34 creator admission in initial/refreshed candidates for selected effect types, zero metadata threshold, dedicated out-of-initial-range refresh cases and fresh ordinary/boundary reruns. Historical TL16/TL17 source fingerprints are retained and now stale after shared implementation/CMake edits. [Effect kind34 creator](effect-kind34-creator.md) | Original legacy ANI playback, reverse mode, complete named action/attachment producer mapping, gameplay events, opaque metadata, full camera lifecycle, original CFG/file-error recovery and full map lifecycle and entity initialization, complete world admission/entities, remaining terrain producers and visibility activation and live clock/integration remain unverified; local slice layout, explicit world camera configuration and recovered-camera start selection, ordinary-terrain reference/object projection, clipping, authored-only selection when no generation seed is supplied and 49-candidate pruning bound plus typed Specific warnings and missing-connector/rollback bounds, child ticking/admission and restart are preview policies; scene pixels use an independent CPU oracle with original queue orders and selected visibility decisions for overlap fixtures, not original whole-scene rendering; baseline shaded frames additionally match original indexed draws fed recorded native queues with explicit count/levels/powers and chain ordinal zero; captured entity source inputs, type-specific effect setup, remaining effect movement/recount and region input production, special size-one lights, spectator/sentinel admission and remaining lighting lifecycle and non-power-of-two palette counts, effect-chain/ordinal selection and live replacement remain unverified  TL19 recovers whole 004df3a0 trajectory initialization, including its original XY helper, retained state word3 and 579,584 chained steps from 18,112 authored initializations. Installed coordinate/period provenance and effect creation parents remain separate. [Trajectory initializer](effect-trajectory-initializer.md)  TL20 recovers whole 0048a950 motion record setup, original reset/trajectory helpers, startup words, nine pending caches and cell/terrain ordinals. 1,280 guarded records and 20,480 chained steps match; full creation/type dispatch, installed inputs and ownership remain separate. [Motion initializer](effect-motion-initializer.md)  TL21 recovers whole 00489200 animation selection, exact compressed kind dispatch and lazy owned metadata/descriptor branches. All 1,522,626 calls match; full creation dispatch, installed metadata and animation binding remain separate. [Animation selection](effect-animation-selection.md)  TL22 adds owned effect ANI entry/sequence binding to the existing forward player: static caller metadata resolution, 448 whole original bind/start compositions, 28,672 ticks and 29,120 matching states. Full inline caller/creation dispatch, installed assets/metadata and effect drawing remain separate. [Animation binding](effect-animation-binding.md)  AS23 adds native installed effect animation catalog/forward binding: all 222 entries and 13 ANI/SPR pairs, 14,430 native/sanitized states and bounded load refusals. Original collection loader execution and effect rendering/lifecycle remain separate. [Installed effect catalog](effect-animation-catalog.md)  TL24 executes original file-count/metadata producer regions: 131 count windows, 393 metadata windows and 2,076 matching entries, including all 222 installed entries under three fills. Native refusal/trim policies stay distinct; whole loader allocation/file/cache/lifecycle and live remain pending. [Metadata producer](effect-animation-metadata.md) |
| AS05 Persistence/progression input readers | Native bounded container, CFG, typed realm/name and version-20 save readers; offline only | Reviewed 2026-10-04: seven asset CTests, 36 CFG + 36 No-CD save-container synthetic matches, 11 installed encrypted CFG matches, three realm configs/four name files, native save fixtures and ASan/UBSan (leak detection disabled); high within these byte/parser policies. [Loading](../formats/persistence-native-loading.md), [static save layout](../formats/save-game.md) | No original-generated save comparison, writer, world restoration (structural grammar now AS20), live integration or native campaign rules; stricter malformed-input behavior is native policy |
| AS06 WZD wizard-definition input | Native owned text/schema reader and inspector; offline only | Reviewed 2026-10-04: 101 installed files compared field-for-field with Python, nine asset CTests and ASan/UBSan (leak detection disabled); high for observed schema and native parser policies. [WZD](../formats/wzd-native-loading.md) | Original defaults/clamping, context-specific selection, table/resource binding, action execution and live wizard initialization remain unverified; no gameplay application |
| AS07 CUR cursor input | Native indexed 1/8-bpp decoder and inspector with owned palettes/AND/XOR planes/hotspots; offline only | Reviewed 2026-10-04: all 20 installed files/22 images match independent Python decoding; 11 asset CTests and ASan/UBSan (leak detection disabled); high for bytes/native policies. [CUR](../formats/cur-native-loading.md) | Original GDI comparison, colour-XOR presentation, image/semantic selection, Qt/OS cursor application and live integration remain unverified; modern/other CUR encodings explicitly unsupported |
| AS08 PCX indexed-image input | Native owned version 5 single-plane 8-bit RLE decoder and inspector; offline only | Reviewed 2026-10-04: all 124 installed files/52,002,288 pixels match independent Python decoding; 13 asset CTests and both PCX ASan/UBSan tests pass (leak detection disabled); high for observed bytes/native policies. [PCX](../formats/pcx-native-loading.md) | Other PCX modes explicitly unsupported; selected Realm Viewer overlay/mask consumption has UI15 evidence; original caller behavior, other transparency/placement and renderer consumers, and live comparison remain unverified |
| AS09 BMP RGB-image input | Native owned 24-bit BI_RGB decoder and inspector; offline only | Reviewed 2026-10-04: all 48 installed files/4,603,863 pixels match independent Python decoding; 15 asset CTests and both BMP ASan/UBSan tests pass (leak detection disabled); Qt menu consumption now has [UI09 evidence](menu-image-integration.md); high for installed bytes/native policies. [BMP](../formats/bmp-native-loading.md) | Other BMP modes explicitly unsupported; original caller behavior, game transparency/placement, renderer consumption and live comparison remain unverified |
| AS10 JPEG RGB-image input | Owned native asset API with private Qt JPEG codec backend; offline only | Reviewed 2026-10-04: all 752 installed files/126,954,548 pixels match Pillow RGB hashes; 17 asset CTests and both JPEG ASan/UBSan tests pass (prebuilt dependencies uninstrumented, leak detection disabled); Qt menu consumption now has [UI09 evidence](menu-image-integration.md); high for tested corpus/policies. [JPEG](../formats/jpeg-native-loading.md) | Original codec/caller equivalence, EXIF/ICC application policy, renderer consumption and live comparison remain unverified; backend codec support/recovery is not independently reconstructed |
| AS11 MPS map placement input | Original-reader-confirmed version-1 schema and owned native reader/inspector; offline only | Reviewed 2026-10-04: No-CD selected disassembly/read-only Ghidra, all 683 installed files/9,464 records match independent Python decoding, 19 asset CTests and both MPS ASan/UBSan tests pass (leak detection disabled); high for byte layout/named kinds/native policies. [MPS](../formats/mps-native-loading.md), [reader/callers](mps-placement-loading.md) | Kind 6 and complete parameter/catalog meanings, world transforms/admission/entity creation and original live integration remain unverified; unknown values preserved |
| AS12 EVT event-area input | Original-reader/writer-confirmed version-1 schema and owned native reader/inspector; offline only | Reviewed 2026-10-04: selected No-CD disassembly/read-only Ghidra, all 685 installed files/2,020 records match independent Python decoding, 21 asset CTests and both EVT ASan/UBSan tests pass (leak detection disabled); high for boundaries/coordinates/native policies. [EVT](../formats/evt-native-loading.md), [reader/writer/callers](evt-area-loading.md) | Original name rules/encoding, script binding, complete world transforms/containment/trigger execution and live loading remain unverified; raw names and endpoint ordering preserved |
| AS13 TAG sprite-name input | Corpus-confirmed headerless schema and owned native reader/inspector; offline only | Reviewed 2026-10-04: all 85 files/130,891 records match independent decoding and 17 companion SPR name tables/occurrence sequences; 24 asset CTests and both TAG ASan/UBSan tests pass (leak detection disabled); high for tested bytes/correlation/native policies. [TAG](../formats/tag-native-loading.md), [runtime boundary](tag-sprite-tables.md) | No original TAG reader or live consumption identified; editor role is a hypothesis. Headerless input cannot detect whole-record loss/addition; lookup, consumers and live integration remain separate |
| AS14 FP Realm Viewer flag-path input | Original-reader-confirmed version-2 schema and owned native reader/inspector; offline only | Reviewed 2026-10-04: selected No-CD disassembly/read-only Ghidra, all 41 files/79 paths/15,950 points match independent decoding, 26 asset CTests and both FP ASan/UBSan tests pass (leak detection disabled); high for layout/selected static consumers/native policies. [FP](../formats/fp-native-loading.md), [reader/caller](fp-flag-path-loading.md) | Header-point meaning, complete flag reservation/selection/movement/scaling/drawing and native Realm Viewer/live loading remain unverified; inactive metadata and valid stored ranges preserved |
| AS15 SFT font input | Original-reader-confirmed version-3 glyph/profile schema and owned native reader/atlas inspector; offline only | Reviewed 2026-10-04: selected No-CD static evidence; all six fonts/1,205 glyphs/26,459 contour pairs match independent decoding; 29 asset CTests and three SFT/SPR ASan/UBSan checks pass (leak detection disabled). [Loading](../formats/sft-native-loading.md), [reader/consumers](sft-font-loading.md) | Original display palette conversion, stateful text layout/code pages and live replacement remain unvalidated; offline Qt integration is tracked in UI14; version two unsupported |
| AS16 NOD navigation input | Original-reader-confirmed version-1 packed nodes and owned native reader/inspector; offline only | Reviewed 2026-10-04: selected No-CD static reader/lookup; all 683 files/25,769 nodes/463,842 slots match independent decoding and complete byte roundtrips; 31 asset CTests and both NOD ASan/UBSan checks pass (leak detection disabled). [Loading](../formats/nod-native-loading.md), [reader/lookup](nod-node-loading.md) | Slot/tail/trailer meanings, section transforms/registration, graph assembly, pathfinding integration and live replacement remain unvalidated |
| AS17 TXT/WBT input | Owned exact-byte/line reader, typed scroll catalog and read-only installed WBT subset; offline only | Reviewed 2026-10-04: 43 TXT/two WBT inputs match independent parsing and complete line byte reconstruction; 26 scrolls/12 WBT statements; 33 asset CTests and both TXT/WBT ASan/UBSan checks pass (leak detection disabled). [Formats/evidence](../formats/txt-wbt-native-loading.md) | Original parser equivalence, text display/substitution and consumers remain unverified; Grimoire keeps its existing application parser; full WBT language, execution and DAT generation relationship are outside this milestone |
| AS18 DAT AI input | Owned headerless Brain/Experien readers preserving model/state/matrix/vector/parameter bits; offline only | Reviewed 2026-10-04: both installed files match independent decoding and exact byte reconstruction: seven models, 21 layers, 1,512 node records, 108,882 matrix words and 2,500 samples/180,251 values; 35 asset CTests and both DAT ASan/UBSan checks pass (leak detection disabled). [Formats/evidence](../formats/dat-native-loading.md), [reader contracts](dat-ai-loading.md) | Original readers inspected statically, not executed; parameter semantics, training/inference, writing, WBT-generation relationship and live consumers remain unverified; headerless streams cannot detect whole-record loss |
| AS19 Legacy ANI/SPR input | Source ANI v3/v4 expansion into owned 44-byte records and single-palette SPR v2 shorter-header decoding; offline only | Reviewed 2026-10-04: all 136 ANI files/121,941 records match normalized reference hashes; all seven old SPRs/162 frames/278,620 pixels match every palette/metadata/pixel/mask byte; 37 asset CTests and six related ASan/UBSan checks pass (leak detection disabled). [Evidence](../formats/legacy-ani-spr-loading.md) | Original ANI conversion inspected statically, not executed; no original v2 SPR reader identified; names preserve all bytes safely; multi/zero-palette v2 unsupported; original legacy playback/rendering and live integration remain unverified |
| AS20 Saved battle/world structures | Explicit owned-byte/range decoder for version-20 world maps, entities and conditional state; offline only | Reviewed 2026-10-04: 55 static function exports, empty/populated synthetic worlds, both save envelopes, ten selected original creature/missile/effect writer captures accepted at exact extents, 39 asset CTests and four persistence ASan/UBSan checks pass (leak detection disabled). [Grammar](../formats/save-world-native-loading.md), [runtime evidence](save-world-serialization.md) | No original complete save corpus or full original world writer/reader comparison; many fields remain raw; resource/reference reconstruction, compatible writer and live simulation restoration remain unimplemented |
| CF01 Encrypted configuration and lifecycle | Scoped decode/encode and experiment tools; preparation/inspection | Static and documented live precedence; high within findings. [Container](../formats/encrypted-cfg.md), [precedence](../formats/cfg-precedence.md), [writer](../formats/cfg-writer.md) | Config-driven mods are separate from engine replacement; native config manager not recorded |
| TH01 Threading and modern scheduling | Static No-CD message-loop/world-update/pacing recovery; worker architecture proposed only | Reviewed 2026-10-04: high for static dispatch, creature passes and timer separation; live thread ownership unknown. [Threading](threading.md), [world loop](world-tick-loop.md) | Observe thread IDs, cadence, pause/alternate-screen behavior and scheduler budget before changing concurrency or result timing |

## Native menus and original menu observation

Reconciled 2026-10-05 through UI35. Native preview behavior and recovered engine
transitions have separate evidence. Bounded adapters delegate original actions;
original drawing, simulation and unadapted menu paths remain active. Historical
UI20 V4 restrictions do not disable the separate campaign V9–V12 integrations.

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| UI01 Native navigation and result menus | Scoped Main, Quick Battle, campaign/battle Mini Menu, Victory/Defeat and Quick Battle result widgets; standalone previews with semantic actions | Synthetic asset/layout/input/model/navigation checks and installed-asset smoke/visual inspection; high within native preview policies. [Main](main-menu-qt-migration.md), [Quick](quick-battle-qt-menu.md), [Mini](mini-menu-qt.md), [Battle results](battle-results-qt.md), [Quick results](quick-battle-results-qt.md) | Engine data/actions, pause/resume, live screen ownership and transition integration remain unverified; fonts/colors and portrait placeholders lack original visual equivalence |
| UI02 Native Map Selection | Scoped caller-supplied stable-ID list, guarded confirmation and preview return navigation; offline only | Eight targeted Qt checks and installed-asset smoke recorded; selection/refresh/keyboard/transactional rejection fixtures; high within native policies. [Evidence](map-selection-qt.md) | Installed map enumeration, map loading, original selection/return semantics and live battle setup remain unconnected |
| UI03 Main / Quick Battle engine dispatch | Hash-pinned callback/controller recovery and bounded forwarding hook; live observation only | Fourteen original-bytecode PE32 fixture cases and live Main → Quick → Cancel → Main trace (13 records, one observed thread ID); high within selected ABI/transition scope. [Evidence](menu-engine-observation.md) | Other actions, keyboard/focus and longer sessions remain unverified; bounded semantic action integration is now UI17; original logic/drawing retained |
| UI04 Native Multiplayer Game Selection | Caller-supplied stable-ID session list and Join/Cancel preview routes; offline only | Synthetic selection/refresh/keyboard/asset/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](multiplayer-game-selection-qt.md) | Discovery, original session handles, transport/join behavior and engine action adapter remain unconnected; font/control styling approximate |
| UI05 Native Single Player Battle Setup | Configured sliders, supplied player/map model, semantic setup actions and Quick/Map caller navigation; offline only | Synthetic model/input/layout/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](single-player-battle-qt.md) | Original setup field/default/units mapping, wizard catalogs and battle creation remain unconnected; sprites/font/control styling approximate |
| UI06 Native Multiplayer Battle Setup | Separate host/guest layouts, supplied roster/settings, local chat/Ready and Create/Join/Map caller navigation; offline only | Synthetic model/ownership/input/chat/layout/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](multiplayer-lobby-qt.md) | Networking, authoritative membership/settings/readiness, chat delivery and engine action adapter remain unconnected; original sprite/control fidelity and live SFT text equivalence unverified (native fonts: UI14) |
| UI07 Native Region Entry | Supplied region ID/artwork, difficulty/availability and semantic Enter/auxiliary requests; standalone preview only | Synthetic model/input/layout/artwork/transaction/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](region-entry-qt.md) | Preview policies remain distinct; fresh campaign difficulty/Enter/Cancel actions now have UI29/UI30 evidence. Loaded Realm/unlocking, auxiliary lifecycles and original visual/input equivalence remain unvalidated |
| UI08 Native Character Improvement | Supplied stats/cost schedules, configured increments, original BMP stat textures/selected JPEG faces, local budgeted draft edits and Region Entry caller navigation; offline only | Synthetic budget/refund/model/input/layout/transaction/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](character-screen-qt.md) | Original pricing/indexing/refunds, campaign snapshot/persistence and engine action adapter remain unconnected; portrait/bar/font/control fidelity approximate |
| UI09 Native menu BMP/JPEG consumption | All implemented menu backgrounds through native loader APIs; Character stat textures and explicit face selection; offline only | 22 targeted Qt/loader checks, 20 installed preview smoke modes and inspected Character capture; high for input/ownership/rendering policies. [Evidence](menu-image-integration.md) | Other SPR controls remain pending; shared SFT font integration has UI14 evidence; original bar clipping/portrait transparency/placement and live caller equivalence unverified |
| UI10 Native menu SPR consumption | Character adjustment/gem art, Region icons and explicit setup/lobby portrait/colour/boot sprites; offline only | 20 targeted menu/SPR-loader checks, 20 installed menu smoke modes and five inspected captures; high within native rendering policies. [Evidence](menu-sprite-integration.md) | Original file/catalog/state mappings and live callbacks remain unverified; SFT font integration has UI14 evidence; sliders/radios and other menu controls pending |
| UI11 Native Grimoire | Installed eight-chapter catalog/text, contents/entry/page browsing, JPEG/SPR art and Region caller navigation; offline only | 20 targeted Qt checks; complete 148-entry/36-companion-art traversal, four installed smoke checks and inspected captures; high for bounded input/native preview policies. [Evidence](grimoire-qt.md), [format](../formats/grimoire-text.md) | Original knowledge/research/dynamic stats, text flow/fonts, state/file binding and engine action adapter remain unconnected; original screenshot equivalence unverified |
| UI12 Native Spellbox / Portmanteau | Supplied inventory/spell mappings, original BMP/SPR art, local drag assignment/removal and typed preview/loadout intent; Region caller navigation; offline only | 21 targeted Qt checks; installed 23-item/95-talisman art traversal, drag checks, smoke checks and inspected captures; high for bounded native preview policies. [Evidence](spellbox-qt.md), [assets](../formats/spellbox-assets.md) | Standalone preview uses supplied mappings; Quick Battle engine recipes/inventory and action adapter are separately recorded in UI19. Campaign/persistence, exact original control layout and screenshot equivalence remain unverified |
| UI13 Native Realm Viewer | Original map BMP/SPR art, bounded name/FP catalog and caller-supplied availability, typed selected-region intent, Main/Entry/auxiliary return navigation; offline only | 23 targeted Qt checks; all 36 installed Entry/return routes, four direct auxiliary returns, installed smoke checks and inspected three-map captures; high for bounded native preview policies. [Evidence](realm-viewer-qt.md), [assets](../formats/realm-viewer-assets.md) | Original region hit shapes, flags/path animation, ownership/unlocking, campaign snapshots/progression, fonts/placement and engine-thread commands remain unconnected; live screenshot equivalence unverified |
| UI14 Native menu SFT fonts | Shared native SFT-to-Qt outline registration and Heading/Body/Tooltip/Yellow roles across all migrated controls; offline only | 24 targeted Qt checks; synthetic glyph mapping/mask/origin/advance/bounds checks, 21 installed variants at three canvas sizes, editing/tooltips/shared lifetime audit and five inspected captures. [Evidence](menu-font-integration.md) | Original palette/coverage shading, contour kerning, punctuation/code pages, wrapping and live text/screenshot equivalence remain unverified; native spacing/rasterization policy is explicit |
| UI15 Native Realm Viewer shapes/animation | Original PCX border overlays and binary mask hit testing, ANI green flags; unsolicited FP-route figure removed after hands-on feedback; offline only | 25 targeted Qt checks; synthetic shape holes/keying, input/availability, timing/loops, hidden timer and reload rollback checks; all 36 installed regions selectable at three sizes, all eleven flag frames fit, and three inspected map captures. [Evidence](realm-viewer-visuals.md) | Original colour-key callbacks, animation clock/roles, direction/route/slot choice, movement speed and campaign travel/engine commands remain unverified; native preview policies explicit |
| UI16 Native Spell Research | Original Grimoire art/spell prose and Realm Research sprite entry; supplied catalog, filtering/availability and typed owner/spell intents; offline only | 26 targeted Qt checks; synthetic model/input/font/asset rollback and Realm return checks; all 42 installed descriptions traversed at three sizes and inspected capture; shared font audit covers 22 variants. [Evidence](spell-research-qt.md) | No dedicated original research-screen layout/caller contract recovered; knowledge/cost/progression IDs and engine-thread commands remain unconnected; native view is explicit |
| UI17 Live native Main / Quick Battle actions | Opt-in versioned command/state bridge, guarded engine-thread original callbacks, acknowledged screen ownership and session fallback; original logic/drawing retained | Five targeted Qt CTests, original-bytecode PE32 command guard fixture and isolated 45-second Qt-driven Main → Quick → Cancel → Main run with acknowledgements 1/2 and one engine thread; original viewport reattached, direct X11 fallback capture inspected and channel permanently retired; additional Main Quit original-bytecode guards and live Main Quit button/window close from Quick reach normal launcher status 0; high within this scope. [Contract and evidence](menu-action-bridge.md) | Other buttons/data, hardware focus/keys, shutdown during preparation/fallback, longer sessions, OpenGL viewport integration and original drawing suppression remain unverified |
| UI18 Live native Single Player setup / Map / Start | Separate V2 data/action contract, original defaults/player generation/map order, guarded full settings transactions and original control callbacks; Start suspends native commands for original loading or waits for UI19 native spell selection; fresh ready Main/Quick return restores Qt | Eight targeted Qt CTests; pinned PE32 setup/list/rule setter/callback guards (including Lives propagation and removed sliders); bounded native setup Cancel, Map Cancel/OK, rule/player edits, then Start, with engine acknowledgements 1–12; inspected real battlefield and original spell-selection captures; genuine XTest original result Quit returns to original Quick Battle, confirmed by capture and retired-channel screen 22; additional two-battle direct-loading run restores Qt twice, refreshes second setup, and completes original Back/Quit at acknowledgement 16 with launcher status 0; inspected captures and before/after immutable checks; high within this scope. [Contracts/evidence](single-player-menu-bridge.md) | Quick Battle in-battle menus, longer play, hardware input, OpenGL integration and original drawing suppression remain unvalidated; campaign Mini actions are separately scoped in UI31–UI35. Spell selection is UI19; setup edits commit on Map/player/Start actions |
| UI19 Live native pre-battle Spellbox | V3 owner/inventory/recipe/name/talisman snapshots; local draft/reset; whole-loadout guards and original shelf/talisman/OK callbacks on the engine thread; original loading/play and fresh Qt root return; Quick Battle Single Player only | Fourteen selected Qt CTests pass, including V3 malformed pool/request and timer-draft checks; pinned PE32 callback/setter/OK fixture covers rejection, existing-assignment removal, cross-alignment placement, input guards and once-only dispatch; corrected live run selects two ingredients, proves all 63 engine control assignments agree, shows selected spells in inspected battle HUD, restores Qt Quick once and completes normal Back/Quit at acknowledgement 15; before/after 2927 immutable checks; high within bounded scope. [Contract/evidence](spell-selection-menu-bridge.md) | Campaign/multiplayer selection, all recipes/alignments live, actual casting, rebuilt spell-list byte comparison, timer expiry with local draft, native previews/in-battle menus, physical focus/input and drawing suppression remain unvalidated; unsubmitted drafts remain local |
| UI20 Mini Menu bridge preparation | Historical V4 Mini/callback/confirmation preparation; V4 activation OFF; separate campaign V9–V12 adapters are UI31–UI35 | Pinned read-only export, isolated original-bytecode callback/guard and disabled-gate checks, focused V4 wire checks; high for those scopes. Proposed live Quick Battle Escape ingress instead reached Game Over and timed out, matching recovered context exclusion. [Evidence](mini-menu-engine-bridge.md) | Quick Battle V4 ingress and complete pause/timer/audio semantics remain unvalidated; failed Escape probe is retained. Fresh campaign Mini Cancel/Quit/report/Preferences have separate UI31–UI35 evidence; Realm mode 4, loaded campaigns and hardware input remain pending |
| UI21 Quick Battle results engine bridge | V5 engine-formatted display rows and original Continue/Quit dispatch; Qt results ownership and original gameplay/Quick return; context 1 only | Pinned read-only disassembly/vtable/Ghidra, original-bytecode receiver/ABI/effects/guard fixture, focused Qt wire/controller/widget regressions and three bounded live runs including results window-close with exact callback/ack trace and normal exit; high within selected scope. [Contract/evidence](quick-battle-results-engine-bridge.md) | Spectate/multiplayer/campaign, portrait sprites, original font/input equivalence, full victory paths, live nonzero scoring, complete pause semantics and longer play remain unverified; original drawing/tick/results updater retained |
| UI22 Preferences contract recovery | Pinned screen/settings identity, packed fields, original radio/slider units, availability, entry snapshots, OK/Cancel and profile writer; live Qt activation unchanged | Read-only disassembly/Ghidra and isolated original-bytecode enter/slider/button/writer matrix, including rollback, device/MCI branches and ignored write failure; targeted Qt preview regression; high for those scopes. [Contract/evidence](preferences-engine-contract.md) | Main bridge/persistence/display round trips are UI23–UI25 and selected campaign Preferences are UI34/UI35. Physical audio/CD, original write-failure handling, other callers and pause/resolution equivalence beyond those scenarios remain unvalidated |
| UI23 Main Preferences engine bridge | V6 Main-only guarded settings/control snapshot, original preview/OK/Cancel delegation and Qt draft/window-close ownership; original services retained | Pinned original setter/callback and rejection fixture, nine targeted Qt tests, V5 fixture regression and automated bounded Preview/Cancel/OK/reopen/window-close run with exact ACK/dispatch trace, byte-identical Cancel file and edited-key readback; high within scope. [Evidence](preferences-engine-bridge.md), [behavior register](preferences-coverage-register.json) | Main-only scope; selected campaign caller integration is separate UI34/UI35. Physical audio/CD, original write failures, power-loss durability and unsupported callers remain unvalidated; later persistence/display checks are UI24/UI25; no engine replacement |
| UI24 Main Preferences cross-launch persistence | Native store for seven original-accepted Main settings; original writer/callbacks retained, atomic accepted readback and fresh disposable import | Ten targeted Qt checks, three synthetic Python checks and automated two-process Preview/Cancel/OK/reopen/restore/Cancel/Quit; all seven fresh engine/Qt values agree and Cancel preserves store bytes. Original manifests match 2,927 files before/after. [Evidence](preferences-persistence.md) | Other callers, original-menu fallback export, physical audio/CD, actual resolution rebuild and power-loss durability remain unvalidated; no engine replacement |
| UI25 Main Preferences display rebuild validation | Retained original leave/rebuild path, test-only forwarding observation and Qt resolution round trips | Six original leave branch/repeat cases, V6 observer regression, ten targeted Qt checks and automated four-change Main run: exact callback/ACKs, 800×600/640×480 client sizes, cleared flags, restored font modes and reopened settings. [Evidence](preferences-display-rebuild.md) | Gameplay resources validated only by isolated ordering; physical monitor/fullscreen/device failure, pixel/font equivalence and other callers remain unvalidated; no replacement |
| UI26 Campaign menu entry contract | Original Main New Game reset/request, Realm lifecycle prefix, admission and auxiliary/navigation branches; Qt bridge remains pending | 269 isolated original-bytecode cases (7 Main, 25 occupancy, 216 admission, 7 auxiliary, 14 navigation); read-only static export; all 2,927 originals preserved. [Contract/evidence](campaign-menu-engine-contract.md) | Historical recovery stage; fresh ingress, Region Entry actions and campaign Mini follow in UI27–UI35. Full resource/auxiliary lifecycles, loaded Realm, mode-4 return and original/native equivalence or replacement remain unvalidated |
| UI27 Original campaign ingress observation | Opt-in guarded Realm custom-tick forwarding; fresh campaign observer only | Synthetic wrong-slot/bytes/receiver/repeat guards, 300-call deduplication and V6 original callback regression; automated Xvfb/Wine New Game records Realm states 0→1→2 and original Region Entry screen 18. All 2,927 originals preserved. [Evidence](campaign-menu-entry-observation.md) | The observed fresh path admits Region Entry before Realm-map resources initialize; loaded-Realm readiness remains unknown. Fresh Entry/Qt actions follow in UI28–UI30 and gameplay Mini in UI31–UI35; auxiliary lifecycles, mode-4 return and equivalence/replacement remain pending |
| UI28 Region Entry contract and return observation | Caller-dependent actions, deferred world request, opt-in Region Entry tick and Realm resume forwarding; original controls only | 175 isolated original-code cases, synthetic guard/forwarding checks, V6 regression and automated four-difficulty/Cancel run. Fresh Cancel resumes Realm then returns to Main; all 2,927 originals preserved. [Evidence](region-entry-engine-contract.md) | Fresh Qt difficulty/Cancel/Enter now have separate UI29/UI30 evidence; loaded-Realm return and auxiliary destination lifecycles remain pending. Historical source fingerprints retain their staleness; no native equivalence/replacement |
| UI29 Region Entry engine bridge | V7 fresh New Game, Qt difficulty/Cancel dispatch, engine-confirmed Main return and window-close Cancel/Quit | Private original selector/Cancel guards, six targeted Qt checks, V6 Preferences/diagnostic regressions and automated live round trip; original drawing/tick retained. All 2,927 originals preserved. [Evidence](region-entry-engine-bridge.md) | Fresh Celtic region 1 only; Enter/world loading follows in UI30 and gameplay Mini in UI31–UI35. Loaded Realm, other regions/callers, auxiliary lifecycles and equivalence/replacement remain pending; historical fingerprints retained |
| UI30 Fresh Region Entry Enter | V8 Qt difficulty/Enter and original fresh campaign loading handoff | Original callback/admission guards, World forwarding guards, six Qt checks, original-only and Qt Enter to three initialized gameplay ticks, V7 Cancel/Quit regression; all 2,927 originals preserved. [Evidence](region-entry-enter-engine-bridge.md) | Fresh Celtic region 1 only; campaign Mini Cancel/Quit/report/Preferences now have UI31–UI35 evidence. Original loading/draw/simulation retained; loaded Realm/campaigns, other regions, auxiliary lifecycles and equivalence/replacement remain pending |
| UI31 Campaign gameplay Mini Cancel | V9 native campaign Mini after original Escape; Cancel/Escape return to original World | Original-only ingress/two returns, pinned Cancel/forwarding guards, seven Qt checks, both V4 gate fixtures and bounded Qt round trip. All 2,927 originals preserved. [Evidence](campaign-mini-cancel-engine-bridge.md) | Original drawing/simulation retained. Quit/report and selected Preferences follow in UI32–UI35; save/load, Realm mode 4, loaded campaigns, timer/mana/audio pause semantics and equivalence/replacement remain pending; historical hashes preserved |
| UI32 Campaign gameplay Mini Quit | V10 native Quit to original confirmation No/Yes; World resume or original defeat/report/Realm return to Main and normal Main Quit | Pinned original Quit/answer fixtures, eight Qt checks, V9 Cancel regression, original-only and Qt automated No/Yes runs. All 2,927 originals preserved. [Evidence](campaign-mini-quit-engine-bridge.md) | Original confirmation, cleanup, drawing and simulation retained. Native confirmation, auxiliary actions, Realm mode 4, loaded campaigns, pause/helper equivalence and replacement pending; historical hashes retained |
| UI33 Campaign Quit defeat report | V11 native report after observed Quit/Yes, original title/21 labels and original OK; World/Realm return to native Main | Original OK/fade bytecode guard fixture, V10 Quit regression, eleven Qt checks and bounded automated native No/Yes/report/OK/Main Quit run. All 2,927 originals preserved. [Evidence](campaign-defeat-engine-bridge.md) | Native admission/presentation policies separate; other outcomes, scoring/helper/pixel equivalence, native confirmation/Realm and replacement pending; historical hashes retained |
| UI34 Campaign Mini Preferences | V12 original Mini local2 and Preferences parent17/depth6; native preview/Cancel/OK and viewport return policy | Pinned bytecode fixture and six targeted Qt bridge/controller/session/store checks; original files verified before/after. [Evidence](campaign-preferences-engine-bridge.md) | Selected live preview/Cancel/OK/reopen and World returns are separately recorded in UI35. Resolution/game-speed/pause equivalence, loaded campaigns, other callers and replacement remain pending; historical hashes retained |
| UI35 Campaign Preferences live round trip | V12 native Mini/Preferences with original preview, Cancel rollback, OK persistence, reopen and original World resumes | Automated bounded private Wine/Xvfb native workflow, exact callback/ACK/thread/value trace, original profile plus native store, three Preferences returns then normal campaign/Main Quit; six targeted Qt checks. [Evidence](campaign-preferences-live-engine-bridge.md) | Selected effects/dialogue settings; resolution rebuild, game-speed/pause cadence, restart and other callers/equivalence/replacement remain pending; UI34 historical hashes retained |

2026-10-04 hands-on menu corrections (UI07/UI12/UI15): difficulty options now
show explicit empty/filled indicators, selected-row emphasis and full-rectangle
click targets that scale with the canvas. Realm selection no longer starts an
unsupported walking-figure loop. Portmanteau now provides the selected ingredient
and three supplied spell artworks in a header, ingredient pointer/drag artwork,
transient talisman hover previews, assignment artwork and returns to occupied or
empty shelf areas. The original manual (printed page 10) and installed tutorial
confirm the header and drag/create/remove pattern; click-to-carry/cancellation
follows the user's requested mock behavior, not a recovered input contract.
Focused synthetic Realm visual, Region Entry and Spellbox checks pass, including
new scaled hit-area, idle-marker, hover-without-assignment, conserved-copy move,
shelf-return and pointer cleanup cases. Original manifest verifies before/after
reference inspection. No paired original runtime interaction/visual comparison
or live replacement is claimed. Spell recipes, mana rings and campaign travel
remain unresolved; see [Portmanteau evidence](spellbox-qt.md),
[difficulty policy](region-entry-qt.md) and [map visuals](realm-viewer-visuals.md).

UI12 held-drag follow-up: pickup subtracts one displayed shelf copy; ingredient
artwork disappears only when no unassigned copy remains. Release/cancel restores
the displayed copy count according to current assignments.
The shell builds and `qt-spellbox` passes with synthetic held/released shelf-pixel
and unchanged quantity/assignment checks, including stacked copies and a
partially assigned stack. This is native presentation validation;
original input equivalence remains unverified (same Portmanteau evidence).

UI12 talisman pickup follow-up: pressing a filled talisman clears its draft
assignment immediately and carries ingredient artwork. Releasing to the
right-hand panel returns its copy to the shelf; supported talisman drops assign
it there instead. Cancelled dragging returns the removed copy to the shelf.
The shell builds and `qt-spellbox` passes with immediate-clear and physical
press/release shelf-pixel/quantity checks. This remains native policy validation.

## Shared frame, input and media protocol ownership

The 2026-10-04 extraction established `protocols/` ownership of the v1 wire
schemas, generated C/C++ and Python definitions, publication semantics and
compatibility rules. Injected
render/input/media adapters, Qt clients and Python fixture/launcher tooling now
consume those definitions. Wire bytes and existing publication/fallback behavior
are preserved; this extraction does not advance live replacement coverage.

Validation: four independent protocol checks, four focused Qt tests, three
launcher preference tests and the existing synthetic frame, input and media
integration workflows pass. PE32 `.text` sections match the pre-migration build
for live and self-test bridge code. Original-manifest verification passes before
and after validation. Confidence is high within synthetic v1 scope; no new live
game validation. The input timer's literal 50 ms in actively edited `main.cpp`
was deferred at that milestone. Audio v2 and later versioned menu formats were
outside that extraction. The reusable render-command v2 foundation is recorded
separately below; its PE32/Qt integration remains pending. See
[migration evidence](../../protocols/VALIDATION.md) for
commands, report paths, build provenance and limitations.

## Native simulation ownership and checkpoints

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS01 Native world ownership, coarse tick phases and deterministic checkpoints | Bounded owned entity slots/generations, cleanup/release and reference repair, staged tick commands, explicit counters/phases, native-v1 encoder/decoder and POSIX file publication; headless synthetic sandbox only | Initial two normal and two ASan/UBSan CTests pass: native lifecycle/transaction/limits/store checks and independent byte oracle with 200 uninterrupted versus 73+127 fresh-process ticks. Regression checks pass with NS02. Leak detection disabled due to environment tracing. High within tested native contracts. [Evidence](native-world-foundation.md), [implementation](../../game/README.md), [format](../formats/native-world-snapshot.md) | AI, combat/spells, campaign triggers, original-save import/export and live gameplay remain open. Bounded movement/map rebinding now has separate NS02 evidence; complete original tick/scheduler and entity cleanup remain separate |
| NS02 Single-creature native movement and moving checkpoint continuation | Typed actions/route/cursor/queued moves, native navigation interface, bounded frozen adapter using recovered legality/cost/search, native waypoint policy and native-v2 save/map rebind; headless offline only | 22 normal and 22 ASan/UBSan CTests pass, including independent v2 bytes, fresh-process 8 versus 2+6 traces, sixteen-point prefix replanning at 20 versus 8+12 ticks, blocked/seam/zero-distance/release/rollback/map refusal. Fresh original 120,842 movement-helper and 149,070 scalar comparisons pass with manifest preservation. [Evidence](native-creature-movement.md) | One profile/4,096 frozen cells; waypoint timing/goal guards are native policies. Original motion action/fine coordinates, full order eligibility, live agreement, dynamic multi-creature occupancy, scheduler/search-heap continuation and general resources remain open. Next: independently compare original motion/route-consumption transitions |

This is the user-selected native world + deterministic save/restore milestone.
NS01 restores a native lifecycle/idle world; NS02 adds bounded movement and map
rebinding, not a legacy battle or a complete playable simulation. Native
generation identities, transactional commits, waypoint pacing and the save
schema are explicit policies. Static tick/entity findings inform selected
boundaries. Native checks consume only synthetic data; NS02's separate original
helper comparisons have explicit hash/manifest evidence. Existing live gameplay
and balance remain unchanged.

### Native fine motion (NS03)

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS03 Bounded forward sample motion and intra-cell checkpoint continuation | Independent recovered arithmetic model, opt-in native driver, fine XYZ/progress/rate accumulator/sample cursor, route consumption and native-v3 persistence; headless frozen one-creature slice | 455,640 isolated original action transitions and 136 route-consumption/coordinate-snap cases match; 24 normal and 24 ASan/UBSan CTests pass, including independent v3 bytes and fresh-process intra-cell/fractional/cycle/prefix traces, rollback and map refusal. Original manifest preserved (2,927 files). High for the tested arithmetic/consumption contracts; controlled animation/event/environment inputs. [Evidence](native-creature-fine-motion.md) | Full segment setup/speed continuity, original animation event production, terrain height, reverse/special profiles, environment/combat callbacks, dynamic occupancy and live agreement remain open. Next: independently compare initialization and successive route-point speed/animation transitions |

The sample-cycle event stream, route-provided speed and reset at segment/order
boundaries are explicit bounded native policies. NS03 does not complete original
motion or replace live gameplay. V1/v2 checkpoint bytes and their policies remain
compatible; v3 retains only the new owned intra-cell continuation.

### Native planar segment continuity (NS04)

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS04 Planar category-zero setup and successive-segment continuation | Recovered scalar/setup model, opt-in `move-continuous`, owned completed history, separate sample/animation cursors, replay-validated native v4 | 55,296 isolated complete original setup cases; 960,696 motion transitions and 136 original consumption/snap cases; 26 normal and 26 ASan/UBSan CTests including independent v4 bytes, boundary/turn/fractional/prefix restart, refusal and rollback. Original manifest preserved (2,927 files). High for tested setup/arithmetic with controlled environment/animation dependencies. [Evidence](native-creature-segment-continuity.md) | ANI event production still uses a supplied twelve-frame clock; vertical/category-four/reverse/special setup, terrain, original eligibility, dynamic occupancy and live agreement remain open. Next: independently recover and compare animation frame/event production |

Native v1/v2/v3 layouts and policies remain compatible. V4 owns the continuation
needed for the bounded profile; it is not an original save writer or restored
playable world. New-order resets and supplied event timing remain native policies.

### Native ANI-driven motion (NS05)

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS05 Forward ANI event production and owned native continuation | Existing recovered controller composed per movement substep; opt-in `move-ani`, owned ANI bytes/base/controller cursors, replay-validated native v5 and staged resource restoration | 28,800 composed original action/start/tick/restart transitions match; repeated installed ANI comparison matches 347 traces/22,555 states and all 136 owned decodes. 31 normal and 31 ASan/UBSan CTests plus loader fixtures pass, including independent v5 bytes, event/boundary/turn/prefix continuation, source deletion and failed in-place restoration. Original manifest preserved (2,927 files). High for selected forward event-0/event-2 composition. [Evidence](native-ani-motion.md) | Explicit directional base remains caller policy; other gameplay events, original action/config mapping, reverse/special/vertical setup, terrain and live agreement remain open. Next: compare vertical/category-four segment setup and terrain height inputs |

V5 embeds ANI bytes and controller state; prior native checkpoint layouts/pacing
remain compatible. This remains a frozen one-creature headless slice, not a
playable restored world or original-save writer.

### Native terrain-aware motion (NS06; reviewed 2026-10-05)

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS06 Ordinary terrain heights, sloped/vertical/category-four setup and owned continuation | Recovered selected setup/ordinary snap, opt-in `move-terrain`/`move-terrain-ani`, frozen terrain lookup, native v6 with previous edge origin and optional owned ANI; headless only | 55,296 planar regression and 5,760 terrain/vertical/category-four original setup cases, 192 ordinary original snaps, 576,000 composed original ANI/motion transitions; 960,696 arithmetic and 136 consumption regressions. 32 normal and 32 ASan/UBSan CTests, independent v6 bytes and 264 fresh-process restart/trace comparisons, rollback and resource/corruption refusal. Original manifest preserved (2,927 files). High for selected isolated forward contracts; controlled eligibility/environment dependencies. [Evidence](native-terrain-motion.md) | One creature/4,096 frozen cells, terrain offsets -16..16 and height differences -32..32; special height helper, reverse/other categories, complete action/config mapping, installed MAP navigation/entity admission, dynamic occupancy/scheduling and live agreement remain open. Next: a bounded terrain-and-creature presentation using validated movement and explicit resource/facing/depth inputs |

This advances offline reconstruction and native synthetic integration only.
Earlier native checkpoint modes retain their formats and pacing; no gameplay
balance change or live replacement is claimed by NS06. The bounded diagnostic
creature presentation is recorded separately in NS07.

### Native terrain and creature presentation (NS07; reviewed 2026-10-05)

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS07 Bounded movement-checkpoint scene | Standalone Qt/OpenGL app reads NS06 fine positions and owned current/completed ANI displays; explicit diagnostic terrain fixture, recovered body offsets/SPR origins and mixed signed depth queue; step/save/export | 72 independent CPU/OpenGL full-image checks, four projection views and display boundary/refusal checks; 144 installed-SPR full-image comparisons and 12 fresh-process frame/queue/checkpoint continuations in both normal and ASan/UBSan builds; headless fine positions agree. Original manifest preserved (2,927 files). High for bounded native integration. [Evidence](native-world-scene.md) | One creature, 64 body-only visual terrain tiles; diagnostic projection/centering and selected standing-layer display are native policies. No installed MAP navigation, original mixed-scene comparison, camera-relative action selection, shaded/visibility/attachment integration or live replacement. Next: pair real world geometry with validated navigation admission, then dynamic occupancy and multiple-creature scheduling |

### Native installed MAP crop navigation (NS08; reviewed 2026-10-05)

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| NS08 Ordinary installed terrain projected to bounded navigation and matched creature scene | Full-source ordinary geometry derived before cropping, sealed XY ring/all layers, exact projected MAP/TTD bytes in frozen navigation, explicit synthetic one-cell profile and strict visual-resource identity checks; actual terrain definitions/flags plus owned creature display | Six Plains/Forest/Village crops per build: 16,896 original support/validity and 5,434 complete movement-helper comparisons (1,050 accept, 4,384 reject, zero sealed-ring accepts), independent geometry/frozen bytes, selected native route arrival, 288 complete pixel comparisons and 24 fresh-process frame/queue/checkpoint continuations in normal and ASan/UBSan builds. 98 normal CTests and three selected sanitizer tests pass. All 2,927 original files preserved. High within the explicit ordinary projection/profile. [Evidence](native-map-navigation.md) | 4..16 XY, 2..32 layers, 4,096 cells and one creature. Runtime object/reference branches omitted; profile, crop sealing and projection are native policies. No original whole-load/search or mixed-scene equivalence, installed creature configuration, dynamic occupancy, live observation/replacement or lighting integration. Next: recover configured creature/object admission or add separately validated dynamic occupancy; coordinate lighting integration with its existing workstream |

### Configured ground creature movement (NS09; reviewed 2026-10-05)

| Milestone | Implemented/validated scope | Evidence | Remaining boundary |
| --- | --- | --- | --- |
| NS09 Installed ground Redcap profile | Selected CFG fields and recovered clamps, ANI-derived ground banks/maximum, normal-memory asset binding, configured sealed MAP navigation and owned ANI checkpoint creation | Original selected-field/sample/motion comparisons, installed terrain predicates, complete four-view native pixels and process continuation; [scope/evidence](native-creature-profile.md) | One ordinary ground type; runtime entities/dynamic occupancy, complete type construction/actions, flying/swimming, camera-relative action production and live replacement remain separate |

## Gameplay areas without recorded replacements

These entries are baseline gaps, not proof that no research exists. Recovered
movement predicates do not cover the whole creature update or AI system.

| ID | Area | Boundary |
| --- | --- | --- |
| GP01 | Commander orders and general creature AI | Static No-CD command ingress, primary move/target/coordinate requests, partial queues/follow, complete behavior/action resolver tables, autonomous selector priority and staggered route/decision scheduling mapped. Handler bodies/status semantics and live equivalence remain incomplete; no native AI or commander feature replacement. [Evidence](creature-ai-combat-spells.md) |
| GP02 | Combat, damage, targeting and spells | Static No-CD animation-triggered melee/ranged attacks, target scoring, defended versus direct health change, lethal/ongoing damage, cast admission and secondary effect dispatch mapped; selected summon/Cure/Blood Lust/projectile/explosion paths reviewed. [104-ID dispatch inventory](spell-dispatch-inventory.md) records pending effect contracts. Hash-checked 60-range exporter; no independent full combat/spell model, live validation or native replacement. [Evidence](creature-ai-combat-spells.md) |
| GP03 | Per-creature veterancy | No implemented gameplay feature recorded |
| GP04 | Mana generation, spending and economy | No native replacement or economy change recorded |
| GP05 | Simulation clock and update ordering | Static No-CD world-update entry `0x0046afc0`, counter candidate, ordered creature passes and pacing recovered. NS01 adds native coarse tick/transaction/counter orchestration; complete phase bodies, original scheduling and live ownership/timing remain unimplemented/unvalidated. [Evidence](world-tick-loop.md), [native foundation](native-world-foundation.md) |
| GP06 | Campaign, scenario scripting and triggers | Static No-CD realm initialization, configured Celtic/Greek/Medieval chain, selected battle-return region ownership changes and realm-change state mapped; script-state serializer sizes identified; native config/name readers have offline AS05 evidence. Result producers, full trigger semantics and live progression remain unverified; no native replacement. [Evidence](persistence-progression.md) |
| GP07 | Saves and persistent state | Static No-CD named save/load dispatch, version-20 decoded header, outer packing/obfuscation, campaign blocks and optional world serializer dispatch mapped. Destination truncation precedes completion; final move/delete failures are unchecked. Offline native input readers now parse campaign blocks and preserve world bytes (AS05), with explicit structural decoding (AS20). No real-save round trip, writer or live persistence service. [Runtime](persistence-progression.md), [format](../formats/save-game.md) |
| GP08 | Terrain/sprite loading, animation and scene composition | Native SPR/ANI loading, selected forward/phase-switch and placement helpers, bounded group/facing and two-child/configured mode-one preview and selected original depth/sort queue and SPR bitmask visibility pass and selected ordinary terrain producer/direct TTD/SPR tile preview plus owned MAP loading and bounded installed MAP slice preview and selected four-orientation camera/world terrain traversal with clipped presentation and selected recovered camera setters plus ordinary-terrain initialization, geometry/surface admission and explicit section-grid placement/rotation and owned authored-region recipe loading/mixed-height assembly plus selected single-block candidate/location/seeded selection primitives plus multi-block descriptors/connectors/admission and bounded pruning plus owned placement/count rollback and a selected one-attempt backtracking driver plus two-pass Specific placement/rotation/location fallback plus ten-attempt reset/seed orchestration plus owned recipe/header catalog construction and original connector-threshold policy plus generation from complete available recipe catalogs and successful-assignment conversion to owned ordinary assembly plans plus complete installed generated MAP loading/assembly, geometry initialization and bounded generated Qt/OpenGL scene production plus owned mode-0 palettes and controlled-light shaded scene drawing and installed global palette controls and terrain palette-count preference dispatch and owned per-cell light fields/controlled source stamps and selected creature admission/eight-phase work/published/target refresh plus static region admission/additive staging/two-phase publication and combined updater plus owned object-snapshot lit Qt terrain scenes and installed effects light-table loading and selected common effect-record placement and whole effect trajectory stepping plus empty-world same-cell motion/recount and bounded membership-on cell transitions and headless lighting composition from authored state validated offline; captured entity light-source inputs, type-specific effect setup, remaining effect movement/recount and region input production, special size-one lights, spectator/sentinel admission and remaining lighting lifecycle, non-power-of-two palette counts and palette effect/ordinal selection, complete terrain, named actions, attachment lifecycle, full camera lifecycle, original CFG/file-error recovery and full map lifecycle and entity initialization, full world admission/entities, remaining terrain producers and visibility activation and live pipeline remain unimplemented. [Scene](native-animation-scene.md), [selection](animation-direction-selection.md), [placement/layers](animation-placement-attachments.md), [configured attachment](native-attachment-recipe.md), [queue/overlap](native-sprite-queue-scene.md), [visibility](native-sprite-visibility-scene.md), [terrain producer](terrain-submission.md), [terrain preview](native-terrain-preview.md), [MAP slice preview](native-map-terrain-preview.md), [world traversal](terrain-world-traversal.md), [rotated traversal](terrain-traversal-rotations.md), [camera setters](terrain-camera-setters.md), [map geometry](terrain-map-initialization.md), [section assembly](terrain-section-assembly.md), [authored regions](terrain-authored-regions.md), [seeded selection](terrain-region-selection.md), [region constraints](terrain-region-constraints.md), [region solver](terrain-region-solver.md), [Specific placement](terrain-region-specific.md), [generation attempts](terrain-region-generation.md), [region catalog](terrain-region-catalog.md), [generated plans](terrain-generated-plans.md), [generated scenes](native-generated-terrain-scenes.md), [expanded generated coverage](terrain-generated-coverage.md), [baseline palette shading](terrain-palette-shading.md), [configured global lighting](terrain-lighting-config.md), [terrain palette preferences](terrain-palette-preferences.md), [terrain light fields](terrain-light-fields.md), [creature lighting](terrain-creature-lighting.md), [static lighting](terrain-static-lighting.md), [object-driven lighting scenes](terrain-object-lighting-scenes.md), [effect tables](effect-lighting.md), [effect record placement](effect-record-placement.md), [effect trajectory](effect-trajectory.md), [effect projection](effect-same-cell-projection.md), [effect cell transitions](effect-cell-transitions.md), [effect cell cleanup](effect-cell-cleanup.md), [blocked effect destinations](effect-blocked-destinations.md), [effect height exits](effect-height-exits.md), [disabled membership](effect-membership-disabled.md), [effect terrain occupancy](effect-terrain-occupancy.md), [creature occupancy lookup](creature-occupancy-lookup.md), [creature footprint initializer](creature-footprint-initializer.md), [effect creature collision](effect-creature-collision.md), [effect creature boundaries](effect-creature-boundaries.md), [effect kind34 creator](effect-kind34-creator.md)  Whole trajectory initializer now recovered from authored inputs; effect creation parents and installed trajectory inputs remain pending. [Trajectory initializer](effect-trajectory-initializer.md)  Whole motion creation helper recovered from valid authored starts; remaining type-dispatch/animation parents, ballistic update, installed inputs and membership/ownership remain pending. [Motion initializer](effect-motion-initializer.md)  Whole animation selector recovered from owned kind/creature metadata; complete type creation and animation asset/frame/direction binding remain pending. [Animation selection](effect-animation-selection.md)  Owned effect ANI binding and forward controller initialization now scoped; complete dispatcher/installed binding and original effect rendering remain pending. [Animation binding](effect-animation-binding.md)  Installed effect catalog now feeds owned forward playback offline; full dispatcher asset admission, direction variants, property consumers and original effect drawing remain pending. [Installed effect catalog](effect-animation-catalog.md)  Original effect metadata region rules now compared separately; full creation dispatcher, direction variants, property consumers and drawing remain pending. [Metadata producer](effect-animation-metadata.md) |
| GP09 | In-game menus and interface logic | Native menu, setup, auxiliary and result previews retain their independent scopes. Bounded Main/Quick/setup/Spellbox/results actions have UI17–UI21 evidence; Main Preferences recovery, persistence and display checks are UI22–UI25. Fresh campaign ingress/Region Entry and gameplay Mini Cancel/Quit/report/Preferences are UI26–UI35, including automated UI35 World returns. Historical V4 Mini activation remains disabled in UI20; separate campaign V9–V12 adapters delegate original callbacks. Loaded Realm/campaigns, save/load auxiliaries, complete pause/input/pixel equivalence and broad replacement remain open. [Layout inventory](../formats/menu-migration-inventory.md), [engine observation](menu-engine-observation.md), [campaign Preferences](campaign-preferences-live-engine-bridge.md) |
| GP10 | Entity lifetimes and ownership | Static No-CD creature allocation/reset/activation, cleanup versus release, slot reuse, selected reference repair and expiry recovered; secondary missile/effect admission/removal, third map-linked pool and teardown order mapped. NS01 adds a bounded native slot/generation and reference-ownership policy with native checkpoint restoration; no original lifecycle equivalence, live observation or native replacement. Complete death states, backing-array ownership, legacy save/load and reference audit remain open. [Evidence](entity-lifetimes.md), [native foundation](native-world-foundation.md) |

## Remaining work and next milestones

Reconciled 2026-10-05 against the current register and linked evidence. This is a
planning summary of the boundaries above, not new experimental evidence or a
claim of replacement. Existing test/run counts were not rerun for this update.

| Area | Remaining work | Next bounded milestone and acceptance evidence |
| --- | --- | --- |
| Live frame capture/presentation (RE02) | Bounded GDI menu capture and three identical early-startup native frames are recorded; continuous animated menus/gameplay, driver-pixel agreement and readback safety remain unresolved | Validate a sustained menu → map → menu scenario with explicit gaps, failure counts and independent original-driver comparison; preserve the bounded startup and historical readback-failure evidence |
| Native rendering and world scenes (RE01/RE03/AS04/GP08) | Ordinary section rotation/assembly, selected region generation, bounded scene admission and cooperative PE32/Qt ring transport have scoped evidence; object-linked rotation, complete entity/drawing/lighting/camera lifecycles and sustained command routing remain partial | Validate idle retry, explicit shutdown/recovery and streaming limits beyond the bounded ring adapter; then compare changing game frames without promoting synthetic agreement to live replacement |
| Native menus (UI01–UI35/GP09) | Bounded menu integration now extends through UI35; loaded Realm/campaigns, remaining auxiliaries, pause/timer/audio semantics, hardware input and longer sessions remain unvalidated | Validate a selected loaded-campaign or auxiliary lifecycle with engine acknowledgements and observed resume/ownership; retain fresh campaign Mini/Preferences and Quick Battle results as separate successful scopes |
| Audio (AU01–AU28) | Scoped headless PCM core and one live native selection/user audible report are recorded; unsupported pitch/seek/cursor calls, listener/map state, ownership/cadence, transitions and physical recovery remain open | Exercise an enumerated live voice lifecycle with admission, completion, fallback and unsupported-call outcomes; compare audible behavior/timing separately from exact offline PCM |
| Pathfinding (PF01–PF03) | Coherent live world capture, real-map search agreement, refreshed continuation inputs and multi-creature scheduling | Compare fresh and resumed requests in both known contexts: candidates/costs, route bytes, budgets and flags; retain original search until agreement is recorded |
| Input, hosting and media (IN01/IN02/ME01) | Enumerated focus/dialog/resize/detach/exit checks, game controls and full movie/sound playback/skip/return | Record a bounded actual-game interaction/playback protocol with explicit actions and outcomes; keep successful embedding evidence separate from frame-stream presentation |
| Native simulation (GP01/GP02/GP05/GP10/TH01) | Complete AI/combat/spell contracts, entity/reference ownership, world-update implementation and live thread/cadence evidence | Observe tick/thread/pause behavior and validate one selected entity or spell lifecycle independently before changing scheduling or replacing simulation work |
| Campaign, assets and persistence (AS02/AS05/AS06–AS20/CF01/GP06/GP07) | Live caller contracts, resource/reference reconstruction, triggers/progression, compatible save writing and simulation restoration | Obtain a real-save corpus and compare complete original read/write extents, then validate a bounded native round trip; preserve raw/unknown fields and separate structural decoding from restored gameplay |
| Application and protocol boundaries | v1 frame/input/media extraction, audio/menu contracts and reusable render v2 foundation have separate evidence; remaining caller/lifecycle/fallback integration is incomplete | Validate each new action/channel independently with ownership, saturation/backpressure, timeout/failure, shutdown and fallback evidence; keep original work retained until the replacement scope is demonstrated |
| Gameplay enhancements (GP01/GP03/GP04) | Commander orders, per-creature veterancy and more active mana tempo/economy remain unimplemented | Define and test intentional rules separately from recovered baseline behavior after the affected engine contracts are established |

Recommended immediate validation priority is RE02 continuous capture and
presentation. This is a proposed next milestone; the successful Wine-window
hosting run remains useful independently of that unresolved path.

Selected ordinary section copying/rotation and explicit grid assembly already
have [recorded original comparisons and assembled-scene checks](terrain-section-assembly.md):
8,192 synthetic fixtures, 683 installed selections across four rotations and
96 complete assembled images. `RE.terrain_sections` links that historical
comparison; its broader registration remains partial. Later region-generation
research separately covers selected seeded choices and placement drivers.
Object-linked rotation, complete entity admission and live pipeline equivalence
remain outside those results. This documentation review does not rerun them or
refresh their source fingerprints.

## Coverage measurements

No aggregate percentage is assigned. Rows are a starter inventory, not equally
sized units of functionality. Source lines, tests, imports and row counts must
not become a whole-engine percentage.

For functional coverage, use the register's stable behavior IDs and retain
unmapped branches and explicitly pending gaps.
For example, separate audio upload, duplication, play/stop, looping, volume,
pan, frequency, cursor state and device output. Report separate fractions for
implemented, original-equivalent and live-replaced behaviors. Include the
checklist version, exclusions and unknowns. An unweighted fraction describes
that checklist only; it does not measure work remaining or runtime cost.

| Metric | Measurement | Limits |
| --- | --- | --- |
| Replacement call coverage | Native-completed calls / all observed calls within instrumented scope | Observer hooks/display copies excluded; count fallback, unsupported and failed calls separately |
| Native CPU work share | Measured native CPU work / total measured CPU work across included processes | Include bridge overhead; identify unmeasured DLL/driver work; this is not speedup |
| Frame performance | Median/p95 frame time, input latency and stalls in matched baseline/native runs | Same map, actions, resolution, settings, hardware and builds; account for GPU/wait time separately |
| Fidelity | Matching outputs / comparable completed observations | Report mismatches and missing/inconclusive observations separately; never silently omit gaps |

Debugger pauses, snapshot copies and diagnostic I/O distort timings. Use
separate low-overhead runs for performance. Do not sum overlapping thread times
into frame wall time or equate call coverage with time coverage. CPU/GPU
optimization benefits remain hypotheses until matched measurements show them.

Suggested broader scenarios: startup/movie/menu; idle map; individual movement;
simultaneous orders; blocked destination; combat/spells; palette/effect updates;
overlapping sounds; map transition; save/load; clean exit. Complete coverage of
this matrix remains proposed. UI17–UI35 and bounded startup command observation
cover selected paths, not every scenario or a matched performance comparison.
Record actual maps, creatures and settings exercised.

## Binary-to-behavior accounting (2026-10-05)

The [traceability package](coverage/README.md) connects the hash-pinned No-CD
Ghidra discovery inventory to a cross-subsystem code/behavior index. The reviewed
committed register at `62d8ac9` has 364 behavior/policy/gap entries, including all
104 spell configuration IDs, 291 evidence records and 81 bounded scenarios.
It covers pathfinding/motion, audio, assets, rendering/animation, Qt menus/services, runtime adapters,
protocols, native world/tooling and outstanding gameplay contracts. The detailed
Preferences register was consolidated without broadening its live-observation
scope. Selected original comparison reports and native scene reports retain
unchanged historical source fingerprints; code review itself is static evidence.

The [committed-baseline pinned source census](coverage/source-index-nonrender-hook-final-20261005.json)
indexes 1,098 implementation/header/assembly/build/schema/tool/test/web files,
with 865 explicitly linked and 233 still unlinked. Support files and in-progress
work are retained; a file link is not full functional coverage. The audit detects new, removed or
changed source files, mismatched explicit links, and focused-register changes
since consolidation. Census refreshes require review and never refresh original
validation hashes.

Run `python3 tools/audit-re-coverage.py` for current links, independent status
claims and evidence freshness. The committed-baseline audit has no errors and
retains 141 stale evidence records, 6,483 unclassified functions, all 190 unmapped imports,
1,543 unresolved indirect flows and ten incomplete dispatch inventories.
`--require-fresh` rejects stale evidence; these historical warnings do not mean
the original comparisons failed when recorded. Source-census drift is reported
separately. No original comparison, live session or replacement was rerun here.

The binary inventory retains 6,675 discovered functions. Ghidra did not assign
the already-tested motion action `0x005104b0` to a function body; documented
recovered-range links preserve evidence without removing discovery gaps.
Executable gaps can also contain padding/data. Most original functionality still
requires mapping and bounded validation; no whole-engine completion percentage
is inferred from these entries.

The [new generated summary](coverage/summary-ledger-audit-20261005.md) records
the committed-baseline audit. Working-tree renderer drift remains separate.
The [earlier summary](coverage/summary.md),
[initial movement summary](coverage/summary-initial-movement.md),
[initial validation](coverage/validation.json) and
[indexing validation](coverage/code-index-validation-20261005-final.json) remain
historical. The adoption snapshot's 285 entries, six scenarios, 844 files and
39 indexing guard checks describe that earlier review rather than today's tree.

## Deterministic change-accounting gate (2026-10-05)

`tools/check-re-coverage.py` compares reviewed snapshots or actual Git base
commits with current source/test/research bytes, behavior definitions, evidence
and remaining gaps. Exact change receipts declare review scope and pending,
recorded or unnecessary validation. The gate preserves original inventories,
immutable evidence/provenance and past receipts; a census refresh cannot conceal
changes from a Git comparison. Status promotions require fresh execution proof.
The adoption baseline retains existing accounting debt without rerunning original
comparisons or promoting any engine milestone.

The dedicated `Coverage accounting` CI job runs audit/gate guard tests and checks
PR, push and merge-group bases without installed game media or original execution.
Merge rules must require that job for server-side enforcement. Local tooling and
AGENTS.md also require the gate. Commands, receipt schema, adoption handling and
limits are in [the accounting workflow](coverage/README.md#required-change-accounting).
This adds bookkeeping enforcement; scientific scope and test adequacy remain
reviewed research decisions.

## Maintaining the ledger

Update the relevant row and review date when a behavior advances. Link the
implementation, repeatable check and evidence report through its research
document. Record executable/native build hashes, scenario, validation type,
supported scope, fallback behavior and unresolved failures. Keep unsupported
behaviors visible rather than shrinking the denominator.

Promote an optional adapter to live replacement only for paths/scenarios
actually exercised with original work bypassed and observable behavior checked.
Keep fallback as a separate outcome. Passing synthetic tests or staging alone
does not justify promotion. No live replacement percentage is available yet.

Smallest useful next evidence: continuous game-owned frame capture/presentation,
live search sequence agreement, real-game draw operation inventory, media
playback/return validation, live audio field/routing checks, remaining semantic
menu-action integration, and native config/save file caller contracts. Selected animation
direction/placement and bounded attachment contracts already have offline AS04
evidence; complete named actions, reverse mode, attachment lifecycle, entities
and live scene integration remain open. Installed raw asset and offline WAV
input comparisons pass within their recorded scope. Add timing percentages only
after counters and measurement boundaries exist.

### Stationary occupancy predicate (NS10; reviewed 2026-10-05)

Fresh isolated original comparison: 9,509 direct occupancy and 576 integrated
footprint cases pass with immutable-input preservation. [Scope and evidence](native-stationary-occupancy.md).
Opt-in native blocker integration now plans around stationary same-profile
creatures, refuses occupied goals, rechecks edges each tick, and rebuilds owned
occupancy after checkpoint restore. Normal/sanitizer model and process checks
include detour arrival, cleanup/release, stale generations and late/intra-cell
obstructions. Original occupancy production, moving reservations, multiple
moving creature scheduling/presentation and live replacement remain open.

## Direct GPU command replay (2026-10-05)

`NR.gpu-presentation` adds explicit shared-context texture presentation from the
native renderer into Qt. Bounded `--commands` replay displays each PRESENT with
no CPU pixel readback or viewport upload; explicit CHECK diagnostics and image
exports retain readback. Fenced texture leases survive source/renderer release.
416 independent complete-frame comparisons pass across normal/high-DPI and
ordinary/ASan/UBSan builds; the shell replay and GAP refusal pass. Ten existing
renderer/viewport/input regressions pass. [Scope/evidence](opengl-direct-presentation.md).

This is synthetic native preview integration on Mesa/Xvfb. It does not advance
RE02 continuous game capture, live command transport, original rendering
replacement or hardware performance. Historical reports keep their recorded
hashes; edits to shared renderer/host files leave older evidence stale.

### Multiple native movers (NS11; reviewed 2026-10-05)

Opt-in headless native movement now supports up to 32 same-profile drivers with
conservative logical swept reservations, conflict waiting, independent ANI state,
rotating planning priority and a shared 53-expansion allowance. Saved fine edges
rebuild reservations without a wire-format change. Atomic spawn/restore and tick
rollback, six fresh-process continuations per build and 1,000 staggered ANI
per-actor checkpoint comparisons pass. Normal 102-test and focused nine-suite
sanitizer runs pass. [Scope and evidence](native-multi-creature-movement.md).

This advances `NP.multi-movement` only. Original scheduler/occupancy production,
fine-space collision, deadlock resolution, mixed profiles, multiple-creature Qt
presentation and live replacement remain open. Prior NS10 original comparisons
retain their hashes and scope; older shared-source native evidence may be stale.

## Incremental native command consumer (2026-10-05)

`NR.incremental-command-consumer` now has scoped native implementation and
synthetic preview evidence: persistent surface/palette state across batches,
shared decoder/consumer admission and session quotas, explicit Verify versus
Skip diagnostics, END/failure/interruption cleanup and direct shared-texture
PRESENT. Qt file replay feeds up to 32 commands per timer event. The new
[record](opengl-incremental-commands.json) passes 64 partitioned sessions, 708
complete framebuffer comparisons and 60 failure cases in normal and sanitizer
builds, plus production CLI replay. Ordinary readbacks/image uploads are zero.

Live engine transport, fragmented-byte framing, overflow/resynchronization,
original drawing coverage, shadow comparison and live replacement remain pending.
Historical direct-presentation/UI evidence retains its hashes and statuses;
shared command/main/document changes leave affected historical results stale.
Current synthetic consumer evidence does not refresh original/live comparisons.

### Multiple native creature presentation (NS12; reviewed 2026-10-05)

The native diagnostic scene now presents up to 32 same-profile terrain-motion
creatures from NS11 checkpoints, reading each actor's own saved ANI display and
fine position. Draws retain slot/generation identity through mixed terrain depth
sorting and export. Single/stationary/multi policy fingerprints are resolved at
admission; ordinary-map immutable geometry uses its separate raw byte identity.

Normal and ASan/UBSan runs each pass 144 full CPU/OpenGL pixel comparisons and
twelve exact fresh-process JSON/RGB565/PNG/checkpoint continuations, covering two
actors in four views over terrace, slope and vertical fixtures. Installed sprite
reads pass original-manifest verification before and after. [Scope and accepted
evidence](native-multi-world-scene.md). Original multi-entity admission/rendering
equivalence, mixed asset profiles, attachments, shaded/light/visibility integration,
automatic play and live replacement remain open. Historical shared-source evidence
keeps its hashes and may be stale; no recovered original status is promoted.

## Interactive coverage atlas (UI02; reviewed 2026-10-05)

`TOOL.coverage-ui` presents a local read-only register/audit snapshot with
percentage summaries, subsystem drilldowns, partial/missing feature filters,
verbatim scope boundaries, source/test/evidence links, and binary function
search with recovered-range exceptions and direct-entry calls.
[Launch and metric definitions](../../apps/coverage-ui/README.md).

Function-link and associated-body byte percentages measure mapping footprints;
scoped/partial/missing implementation counts use equally weighted behaviors.
Original comparison and live replacement use recovered behaviors only. Native
policies remain distinct. Current, mixed and stale comparison fingerprints do
not extend historical comparison scope; generic next milestones do not infer
unrecorded branch semantics or whole-game completion.

Seven focused semantic/HTTP regressions and fifteen real browser checks cover
percentage denominators, overlaps/interior anchors, data/classification/range
exclusions, evidence freshness, source-access boundaries, filters, detail links,
refresh, pagination and responsive layout. The final record additionally runs
coverage guard suites for web-source census compatibility.
[UI validation only](coverage-ui-validation-final.json). No engine contract,
original execution/comparison, gameplay policy or live replacement is promoted.
Historical UI01 and existing engine evidence retain their original hashes.
The source census now includes HTML/CSS/JS; exact prior non-web censuses remain
readable for Git comparisons while new/changed web files remain gate findings.

## Live bounded native command transport (2026-10-05)

`NR.live-command-transport` connects owned PE32 session records through a separate
versioned append-only mapped channel to fragmented decoding and the persistent
Qt GPU consumer. Writer claims, immutable publication, cancellation, terminal
draining and explicit failure cleanup are native policies. The opt-in
`--native-commands` launcher wiring preserves original drawing and retains the
existing 16-operation observation limit. Qt executes at most 32 commands per poll.

The new [native integration record](opengl-live-command-transport.json) passes
normal/sanitizer fragment checks and eleven exact C writer cases, plus seven
Wine COM-hook sessions: four complete RGB/indexed sessions match every independent
fixture native pixel at PRESENT while the producer runs; three incomplete or
cancelled sessions refuse. Ordinary native readbacks and viewport uploads are zero.
All eight renderer CTests pass. No original artifacts are consumed. Original-game
shadow comparison, unsupported drawing/initialization branches, indefinite-session
checkpoint/resynchronization policy, physical GPU performance and replacement
remain pending. Shared decoder/hook edits leave affected historical evidence
stale; old records and hashes are preserved.

### Coverage atlas direct call boundary (UI03; 2026-10-05)

Inferred/computed call targets remain in the audit; the function detail graph
now lists only direct discovered entry-to-entry calls. A focused inferred-target
fixture would fail if those edges were counted as direct. The seven semantic/HTTP
and fifteen real browser checks pass again against a freshly started server.
[Current UI-only validation](coverage-ui-direct-call-validation.json). UI01/UI02
retain their source hashes and may be stale after the backend/test change; no
engine evidence or stage is refreshed.

The append-only gate now gives an obsolete recorded receipt no coverage credit
when none of its exact behavior contracts matches the current delta; unchanged
files cannot reactivate its stale execution result. The old receipt and hashes
remain intact, and every current file/contract still needs a matching new review.
A focused regression rejects both old-only approval and stale current execution
claims while accepting a new explicit pending review. All 44 gate and 45 discovery
guard tests pass. This supports UI history upkeep without refreshing engine evidence.

### Native scene move controls (NS13; reviewed 2026-10-05)

The diagnostic scene now selects individual available creature drivers and queues
XYZ-cell moves until Step. Full-generation identity is checked before queuing;
cleanup/release/reuse clears stale selection. The widget remains separate from
session orchestration. Save preserves pending orders; application selection and
the interactive body outline are transient.

Normal and ASan/UBSan production-widget/controller checks and exact fresh-process
pending-order continuations pass, plus 105 normal CTests and five focused sanitizer
suites. Inputs are synthetic; [scope and accepted evidence](native-scene-orders.md)
exclude original mappings, full-window mouse picking, automatic playback, faction
permissions, commander/summoned gameplay and live replacement. Older shared-source
scene evidence retains its hashes and may be stale. Committed-history receipts are
reviewed separately, with pending retrospective documentation accounting for two
render-transport prose versions; no past gate or new transport validation is claimed.

## Original-game native command shadow (2026-10-05)

`NR.real-game-command-shadow` adds an isolated, reproducible startup observation
with original drawing retained. The [evidence and procedure](opengl-real-game-shadow.md)
confirm that startup color fills reach the owned tracker but fail the ordered
command channel before any records or native PRESENT. The missing fill route in
`game_session_blit_begin` is the first demonstrated code blocker. Native refusal
cleans resources; subsequent original Locks/blits and owned primary frames continue.

This milestone establishes a real-game integration gap, not pixel equivalence or
a working native game viewport. Implement and independently validate the fill
route next, then repeat the shadow run. Independent driver output comparison,
full sessions, gameplay coverage, continuous ownership and replacement remain
pending. Historical synthetic transport evidence remains unchanged.

### Native scene mouse picking (NS14; reviewed 2026-10-05)

The diagnostic scene now selects foremost visible creature pixels and queues
right-click terrain-parent-cell moves through the existing native Orders path.
Picking uses the presented depth queue, exact decoded sprite coverage and explicit
standing layers; foreground terrain occludes actors and opaque tiles without a
cell block hidden terrain targets. Full-generation identity, bounded fractional
mouse coordinates and Step/Save command semantics are retained.

Normal and ASan/UBSan runs each pass 4,718,592 independent pixel-picking comparisons,
actual Qt mouse events, four-view metadata and refusal cases and an exact fresh
process pending-order continuation. Composition regression adds 144 CPU/OpenGL
frames and twelve exact frame/checkpoint continuations per build, with original
manifest verification around installed sprite reads. 106 normal CTests and six
focused sanitizer suites pass. [Scope and accepted evidence](native-scene-picking.md).
Original rays/input mappings, whole installed-window interaction, group/stop
actions, faction rules, automatic playback, commander gameplay and live replacement
remain open. Historical shared-source evidence retains its hashes and may be stale;
committed intermediate receipts are reviewed separately.

## Ordered native constant fills (2026-10-05)

`NR.owned-session-fill` routes admitted successful full/rectangular fills through
one existing UPDATE using pixels derived from captured dwFillColor. Initial
zero storage is synthetic and has no before-CHECK; there is no fake source ID
or new wire opcode. [Implementation and evidence](opengl-owned-session-fills.md)
cover eight indexed8/RGB16/24/32 fixture sessions and 17 independent full GPU
frame comparisons, explicit native-byte CHECK replay, failed-call retry and
initial partial-fill refusal. Ordinary native reads and viewport uploads remain zero.

The original startup now publishes 15 records (five CREATE/UPDATE/CHECK groups),
then fails with tracked-surface invalidation GAP 3 immediately before dc_acquired.
The existing application GetDC path invalidates ordered ownership; it is the next
boundary. Original Main drawing continues and 2,927 originals verify unchanged.
No real-game native PRESENT or driver-pixel equivalence is established. Shared
hook/fixture edits leave historical evidence stale; old records/hashes are retained.

## NS15 — native stop and cancellation of queued moves

`NP.stop-orders` adds semantic scene controls and native operation 5: selected
full-generation pending move removal without ticking or altering active motion,
and FIFO tick-boundary route cancellation/reset at the committed logical cell.
Pending stops select native checkpoint v7; versions 1–6 retain strict opcode
contracts and unchanged minimum-version output when no stop remains. Native
preview validation uses production widget/controller/session with synthetic owned
navigation, exact pending-stop restart, fine-motion reset, stale/cleanup refusal,
queue budgets and rollback; existing native tests provide regression coverage.
Evidence: `NS15.stop-orders`, [contract](native-stop-orders.md). Historical source
fingerprints are preserved; older shared-source claims may remain stale. Original
stop/input mapping, installed whole-window interaction, group/faction/playback
policies, commander/summoned gameplay and live replacement remain outstanding.

## NS16 — native fixed-rate Play/Pause

`NP.scene-playback` adds Qt-free 10 Hz tick admission, four-tick bounded catch-up
with discarded excess elapsed intervals, fresh resume intervals, paused Step and
semantic Qt scene controls/orchestration. World/pending orders and transient
selection survive pause; Save pauses before modal interaction. Playback timing
is not a checkpoint field and loaded application sessions start paused. Evidence
`NS16.scene-playback` validates owned synthetic virtual-time admission, production
buttons and timer dispatch, deterministic admitted-tick/checkpoint continuation,
error pause and reentrancy; native regressions are separate from original comparison.
[Contract](native-scene-playback.md) retains original pause/cadence, whole installed
window interaction, group/faction/commander gameplay and live replacement gaps.
`MV.pause-clock` stays independently partial without implementation/comparison.
Historical shared-source evidence keeps its fingerprints and can remain stale.

## Major engine track dashboard (UI04)

The local coverage atlas now opens on major engine tracks rather than binary
association breadth (`UI04.dashboard`). The curated capability checklist in
`apps/coverage-ui/tracks.json` groups movement/navigation, native scene controls,
rendering/animation, effects/lighting, audio, assets/persistence, menus/campaign,
and core gameplay/spells; planned commander, veterancy and mana changes remain
separate. All required registered stages and applicable execution evidence must
exist before a milestone counts demonstrated. Partial implementations, missing
mappings, explicit next tasks and historical/current evidence boundaries remain
visible. The 104 spell entries contribute one spell goal. These percentages are
named-roadmap milestones, not estimates of effort or whole-game completion.
Twelve semantic/HTTP tests and twenty desktop/mobile Chromium checks passed for
this UI-only change. No engine implementation/comparison/integration/replacement
status is promoted and earlier evidence fingerprints remain unchanged.

## Ordered application DC checkpoints (2026-10-05)

`NR.owned-session-dc` now suspends recorded native identities while the
application holds its DC. A validated pre-ReleaseDC bitmap becomes a full v1
UPDATE only after original success; failed release remains retryable. Acquisition
generation, bitmap/thread identity, epoch and incomplete-ownership checks remain
explicit. No individual GDI drawing or font replay is implemented.

[Ordered DC evidence](opengl-owned-session-dc.md) records 18 synthetic sessions:
12 complete and six refused, with 38 independent full GPU frame comparisons,
explicit native-byte replay and terminal resource cleanup. Eight legacy DC
bootstrap cases and eight fill regressions (17 GPU comparisons) pass, as do the
incremental/native-channel CTests. These are scoped native integration results,
separate from original pixel equivalence and rendering replacement.

The fresh original startup observation publishes the 400×280 RGB565 DC
checkpoint as UPDATE sequence 16 for existing identity 5. It then exhausts the
4,062-record limit: per-row unlock uploads account for most of 4,039 UPDATEs.
The channel contains 21,576,532 bytes but no PRESENT. Original drawing/menu
continues, and all 2,927 original inputs verify unchanged before and after.
Next: pack admitted unlock rectangles into a single UPDATE while retaining
ownership and bounds, then repeat startup. Complete native original-game frames,
independent driver-pixel comparison, continuous sessions and replacement remain
pending. Historical evidence retains its hashes; older shared-source evidence
may be stale.

## NS17 — actual native window playback and checkpoint validation

`NS17.scene-window` extends native playback/picking/presentation preview evidence
with the production entry-point window, actual Qt mouse/button/modal Save events,
real timer admission, an installed sealed Forest crop and actual Redcap ANI/SPR.
Normal and sanitizer runs each cover four orientations, eight windows, 40 complete
independent CPU/OpenGL frame comparisons and four exact fresh-window checkpoint/
frame continuations. A test-only observer clicks Pause at committed tick boundaries;
original pause latency/input/cadence and live equivalence are not asserted. The
ordinary-map art-to-cell target is corrected from `z+1` to the exported standing
cell’s own layer above zero. Existing diagnostic explicit layers remain unchanged.
The stopped actor remains in simulation/selection but has no displayed ANI body
after cursors are cleared; idle/stopped body retention and original action mapping
are newly confirmed outstanding display work. Historical evidence and hashes remain
retained, including source staleness. [Proof and reproduction](native-scene-window.md).

## Packed ordered unlock updates (2026-10-05)

`NR.owned-session-unlock-packed` serializes each admitted later full or partial
unlock as one existing v1 UPDATE. Full-width data is already contiguous; narrow
regions use temporary tight storage counted with the pending after-snapshot,
partial base and existing ownership reservations. Partial base CHECK,
pre-original-Unlock capture, failure retry and epoch/generation checks remain.
No record/byte/operation limits are raised, and no wire version changes.

[Packed unlock evidence](opengl-owned-session-unlock-packed.md) records 26
fixtures: 17 complete sessions and nine refusals, with 55 independent complete GPU
frame comparisons and per-rectangle native input comparisons. Current DC/fill
regressions pass (18 and eight cases; 38 and 17 GPU comparisons), as do the native
incremental/channel CTests and seven live-transport sessions. Explicit byte-limit
refusals remain; forced scratch allocation/admission failure is untested.

Original startup now publishes 39 records rather than exhausting 4062 records:
six CREATE, 14 UPDATE, 18 CHECK and one BLIT, totaling 24,594,964 bytes. It next
reaches the existing 16-operation cutoff before primary PRESENT; mirror GAP 6 and
mapped GAP 2 retain that incomplete boundary. All 2927 immutable inputs verify
unchanged before/after, and original drawing continues. No first native
original-game frame, independent driver-pixel equivalence, continuous session or
rendering replacement is established. Next: a separately bounded startup
completion policy that admits the first eligible primary presentation while
retaining resource and ownership limits. Historical evidence hashes stay intact.

### NS18 — owned static display after native Stop

The native scene retains its last displayed movement bitmap after Stop clears
movement continuation, at the existing committed-cell Stop position. Snapshot v8
preserves the compact pose; resource rebound validates the directional ANI bitmap
and rejects malformed references transactionally. Reorders use the pose until
an active display takes priority; cleanup/release/reuse clear ownership.
Evidence and pending boundaries are in
[native-idle-display.md](native-idle-display.md) and its JSON record. Original
idle/action sequence selection and initially spawned idle bodies remain pending,
with no original comparison or live replacement promotion. Historical NS17
records remain unchanged and can be source-stale.

### NS19 — explicit initial native creature body

A native spawn initializer owns the first bitmap in movement direction zero
without executing its ANI player, ticking or issuing orders. The new
`spawn-terrain-ani` sandbox command produces an idle zero-tick v8 checkpoint
for immediate native presentation and picking. Static idle ticks, modal Save,
fresh-window first-order continuation and movement/Stop display priority are
validated in the bounded Forest/Redcap slice. Scope, evidence and remaining
original idle/action, animation, initial-facing and full-game admission gaps are
in [native-spawn-display.md](native-spawn-display.md) and its JSON record.
No original comparison, live replacement or gameplay balance promotion is made.

## Bounded startup through first primary presentation

2026-10-05 — `NR.owned-session-startup` retains the ordinary16-operation sample
when PRESENT exists, and otherwise admits first primary PRESENT through64
successful operations with all existing ownership, command and resource limits.
Eight boundary fixtures pass (four complete/four refused); seven live transport
and26 packed unlock regressions and both native consumer CTests pass.
The original game now publishes69 records/39,226,036 bytes, presents one native
GPU frame at23 successful operations and ends cleanly. Mirror/channel match,
zero ordinary native/RGBA readbacks or viewport uploads, all native surfaces
released. Original drawing remains active; immutable2927-file manifest verifies
before/after. Explicit GPU replay passes31 owned CPU CHECKs; independent original
pixels, continuous presentation and replacement remain pending. See
[protocol and evidence](opengl-owned-session-startup.md). Old source fingerprints
remain historical; this does not refresh complete-contract equivalence.

### NS20 — paused-window terrain creature spawning

The native scene accepts terrain-cell selection and a semantic paused Spawn
action. New scene checkpoints admit up to 32 same-profile bodies under the
already versioned multi-creature occupancy policy. Native terrain validation and
transactional occupancy/reservation checks reject unsupported, occupied and
unfinished-edge placement without state or selection mutation. Successful
spawning selects the new body for orders and persists it in the existing v8
checkpoint. Synthetic controls/admission/limit checks and actual Forest/Redcap
window Save/reload/first-order continuation in four views are recorded in
[native-scene-spawning.md](native-scene-spawning.md) and its JSON record.
Original spawn/gameplay admission, other profiles and live replacement remain
pending; historical evidence fingerprints are retained unchanged.

## Bounded successive native primary presentations

2026-10-06 — `NR.owned-session-sequence` requests1..32 primary PRESENTs while
retaining the same native GPU surface history. First PRESENT by64 successful
operations, requested sequence by256; early/short sessions refuseGAP6. Existing
command/byte/ownership/resource caps stay unchanged. Explicit sequences omit
CHECK expected-output diagnostics, preserving all owned render inputs; ordinary
samples retain CHECKs. The initial CHECK-carrying original attempt exhausts64MiB
before a second producer PRESENT; its evidence is preserved separately.
Fresh input-only execution: eight sequence fixtures pass,40 complete-stream
frames plus four prior frames on deliberate refusals; eight default startup
regressions and both native consumer CTests pass. Original rerun presents three
native frames at57 operations,74 records/39,363,820 bytes, clean END, mirror
match, native cleanup and zero ordinary readbacks/uploads. All three frames
have identical early-startup pixels; animation is validated synthetically, not
claimed for this original-game sample. Original2927-file manifest verifies
before/after, original drawing stays active. Sustained reusable transport,
longer menu/gameplay scenarios, independent original-driver comparison and
replacement remain pending. See [scope and records](opengl-owned-session-sequence.md).
Historical shared-source evidence remains independently stale.

## Acknowledged reusable command transport foundation

2026-10-06 — `NR.command-ring` adds version2 single-producer/reader1MiB ring
with release/acquire publication and acknowledgement after a private byte copy.
FULL is nonblocking and changes no published data/counters; acknowledged slots
may be reused. Identity/counter/terminal corruption, cancellation and32-bit
lifetime saturation refuse. Native and ASan/UBSan16-case tests each transfer70MiB
across independent processes; four changing native GPU display frames pass
through production bounded decoder/consumer across wrap and oversized-record
fragmentation, with cleanup and zero ordinary readbacks/uploads. Existing native
incremental/v1 channel CTests and protocol tests pass. This additive foundation
is not connected to PE32 hooks or live original-game rendering. Existing v1
remains active; queues/retry scheduling outside tracker, Qt mapped adapter,
launcher/shutdown/recovery and removal of bounded decoder/archive limits remain
pending. See [wire, limits and execution](../formats/render-command-ring-v2.md).


## Cooperative mapped command ring integration

2026-10-06: `NR.command-ring-adapter` connects the version2 native reader to a
32MiB owned PE32 producer queue, drained nonblocking after tracker release in
bounded1MiB attempts. FULL retains bytes for later callbacks; first failure is
sticky, END waits for publication, cancellation/overflow/interrupted detach
refuse. v1 remains supported. Queue and mapped-ring budgets are separate from
existing owned-pixel/archive limits.

Native and ASan/UBSan11 legacy plus7 queue cases pass, including35,653,675 bytes
across private queue wrap, delayed-reader retries, poisoning and failure paths.
Eight v2 PE32 sequence fixtures validate44 complete independent GPU frames;
eight v1 startup regressions pass. Native incremental/live-channel CTests pass.
Original no-CD40209ca7 startup sends39,363,820 bytes through1MiB ring and completes
three native PRESENTs before producer exit, cleanEND and zero live native surfaces.
Original drawing remains active; the three early-startup pixels are identical.
All2927 immutable files verified before/after. See
[adapter evidence and limits](opengl-command-ring-adapter.md).

Idle retry scheduling and explicit lifecycle shutdown outside DllMain are next.
The decoder/archive64MiB/4096 limits, sustained gameplay, original pixel comparison
and replacement remain outstanding. Shared historical source evidence remains
independently stale; scoped fresh records do not promote unrelated contracts.

## Bounded scene construction and resource admission (WS01; 2026-10-05)

`NP.bounded-scene` now loads the hash-pinned Forest section-01/Redcap
[specification](../formats/bounded-scene-specification.md) through the existing
ordinary crop exporter and native scene preview. Six exact installed resources
are admitted and staged once for navigation and presentation; unknown fields,
unsupported policies/types/bindings and resource drift refuse. The document
records preserved terrain/configuration fields and deliberately omitted world
entities, scripts, resource nodes/economy and resource families.

Fresh current-source Debug binaries pass the map-navigation and creature-profile
CTests; four synthetic admission suites pass. Two installed loads reproduce
geometry, PNG/RGB565 frames and queues; four complete frames match an independent
SPR compositor. The specified actor reaches its terrain-aware destination, and a
changed terrain-sprite hash refuses before output creation. Manifest checks retain
all 2,927 original files. [Execution evidence](bounded-scene-native-20261005.json)
records this native preview scope only. Original whole-world equivalence,
reachability of arbitrary admitted endpoint pairs, scripts/entities/resource
economy, other profiles/maps, portable checkpoint relocation and live replacement
remain pending. A partial directory without `result.json` is not a successful
load. Historical evidence and source hashes are preserved.

## Idle command lifecycle and native sustained decoding

2026-10-06: `NR.command-idle-lifecycle` starts a bounded retry worker from normal
callbacks outside DllMain. `RenderShutdown` stops owned capture under the tracker,
then drains or refuses pending bytes and joins outside tracker/loader lock with
finite deadlines; queue and mapping are released only after join. Startup and
shutdown serialize handle publication. DllMain retains interrupted worker-visible
resources for process cleanup. Calling the export before exit remains an explicit
orchestrator responsibility; it does not uninstall hooks.

`NR.command-streaming` selects bounded-memory decoding for live v2 channels,
removing cumulative64MiB/4096 caps. It bounds fragments, records, live surfaces,
pixels and command batches; monotonically increasing CREATE IDs replace an
unbounded historical ID set.32-bit byte/sequence lifetimes still cannot wrap.
v1/offline replay keep their original limits. The PE32 owned archive/sample is
still bounded independently; sustained original capture/archive/session rotation
and original pixel comparison/replacement remain pending.

Actual PE32 idle, pending-shutdown, cancellation, timeout and detach cases pass
with independently checked owned bytes. Native/sanitized C queue regressions pass.
The native GPU fixture sends74,631,284 bytes/5,077 commands through1MiB ring with
71 FULL retries andfour independently checked complete frames; ASan/UBSan native
GPU repeat and bounded regressions also pass. Qt/Mesa integration disables leak
checking; C queue enables it. Cleanup/readback/
upload counters pass. Regression details are recorded
in [the lifecycle and streaming evidence](opengl-command-streaming-lifecycle.md):
eight v2 cases/44 full frames and eight v1 startup cases/four frames. Final original
startup completes three native PRESENTs/39,363,820 bytes, with original drawing
retained and both2927-file manifest checks passing. Shared historical evidence
retains its source fingerprints; these scoped records do not refresh full contracts.


## Rendering path inventory

2026-10-06: [current drawing inventory](render-drawing-inventory.md) separates
live observations, isolated original execution and static reachability for
copies, fills, locked pixel writes, GDI/text, terrain, sprites, effects, UI and
movies. `RI.*` entries seed the nine partial contracts and preserve unsupported
branches and caller gaps. The retained static report contains selected caller
contexts, discovery owners and unresolved indirect sites for the pinned No-CD
build. Its WORD-copy window remains outside discovered function boundaries.

No new game session or original comparison was run. Existing effect/terrain state
comparisons, native previews, startup command CHECKs and live activity retain
their original scoped evidence and source hashes. No runtime frequency, complete
consumer dispatch, independent driver-pixel parity or live replacement claim is
added. Full UI/HUD and effect raster caller chains remain pending.


## Continuous producer independent of bounded archives

2026-10-06: `NR.continuous-producer` adds explicit v2 continuous owned publication,
with independent live state and optional bounded command archives. Default
bounded sessions retain their limits. Optional archive record/byte exhaustion
ends only the archive with GAP; archive creation/write failure cannot complete
or refuse the continuous transport. Current 32-surface / 16,777,216-pixel session,
64MiB owned snapshot, 32MiB queue, 1MiB ring and finite UINT32 lifetime budgets
remain. Unsupported ownership/drawing and transport failures still refuse.

[Final synthetic execution](opengl-continuous-producer.md) checks eight actual
PE32/Wine-to-native-GPU cases: five complete streams, three intentional refusals
and 352 complete independent frame comparisons. Long sessions exceed 64MiB,
4096 commands, 256 operations and 32 presentations, with 70 changing frames.
Both archive exhaustion modes and archive creation failure leave native END
complete. Explicit shutdown, missing-PRESENT/v1 and surface capacity boundaries
are checked; default sample and idle/queue regressions remain separate evidence.

No original game run or driver-pixel comparison was performed. Current resource
retirement still invalidates the stream; dynamic IDs/releases, continuous mixed
operations, full counter-lifetime exhaustion, better backpressure, automatic
application shutdown, recovery and sustained original gameplay remain pending.
Historical evidence fingerprints are preserved rather than refreshed.

## Continuous resource IDs and final-release retirement

2026-10-06: `NR.continuous-resource-lifetime` separates wire IDs from reusable
session slots. Continuous CREATE IDs increase without reuse; confirmed observed
final Release emits DELETE, reclaims session pixels, and uses existing tracker
cleanup for component snapshots, metadata, aliases and attached-surface links.
Nonfinal/untracked Release preserves live resources. A reused address receives a
fresh ID and layout state, while simultaneous surface/pixel/storage budgets stay
bounded. Default bounded final-release GAP remains unchanged.

[Resource-lifetime contract](opengl-continuous-resource-lifetime.md) distinguishes
admitted retirement from active lock/DC, contention and ambiguous-alias refusals.
The new synthetic PE32/GPU suite checks all slots occupied and repeatedly reclaimed,
more than128 created lifetimes, independent primary recreation, aliases/nonfinal
Release, untracked retirement and cumulative pixel/snapshot-budget reclamation.
Final execution evidence is recorded separately from historical results.

No original game or driver comparison is claimed. UINT32 ID exhaustion is guarded
but not executed. Successful CreateSurface at a still-tracked address, unsupported
shape/restore branches, unobserved implicit destruction, alias saturation and
uncertain ownership retain refusal. Broader updates, improved backpressure,
automatic lifecycle orchestration, recovery and original-game validation remain
separate chunks; historical evidence hashes and statuses remain intact.

Final-source lifetime execution passes nine cases, four complete/five refused
streams, 63 independent complete-frame comparisons, 196 unique churn IDs and
100,666,308 bytes in the pixel-churn case. Fresh continuity regression passes
all eight cases and 352 frames under a new evidence ID. The bounded owned-session
suite passes all ten cases; its legacy record lacks source fingerprints, so no
new current-source equivalence status is promoted from that report.

## Continuous mixed mutations and committed layout replacement

2026-10-06: `NR.continuous-mutations` adds complete writable Unlock replacement
of dimensions/format/masks through ordered DELETE and a fresh monotonic CREATE,
preserving observed COM aliases. Continuous palette changes retain a complete
initial palette, then serialize the minimum changed contiguous RGB range;
flags-only changes never become alpha or redundant palette commands. Bounded
serialization and layout mismatch policy remain unchanged.

[Mutation contract](opengl-continuous-mutations.md) links sustained owned full/
partial writes, constant fills, admitted keyed/unkeyed same-format copies,
observed two-buffer flips, indexed palette changes and validated RGB DIB handoffs.
Independent synthetic engine storage and palettes supply complete GPU frame
comparisons; failed original calls/retries and ownership/layout/flags refusals
are separate cases. Final execution is recorded under new IDs; historical
comparison/integration records and source fingerprints remain intact.

Descriptor-only shape changes, invalid partial bases, Restore, missed operations,
unsupported effects/stretch/clipping/complex flip chains and recovery retain
refusal. No original game, driver equivalence, GDI/text instruction replay or movie
producer is claimed. Backpressure, lifecycle orchestration, recovery and original-
game validation remain separate chunks.

Final-source mutation execution passes all twelve cases: eight valid/four refused
streams and 933 independent complete-frame comparisons. Each mixed stream runs
40 cycles, with 201 RGB or 281 indexed presentations; RGB DIB handoffs repeat
12 times with failed-release retry. Layout replacement covers dimensions, bit
depth and mask-only RGB565-to-RGB555 changes, then RGB24/indexed8 with fresh IDs.
Forty two-entry palette RGB updates and flags-only cache changes are checked.
Fresh lifetime and continuity regressions pass nine/63 and eight/352 cases/frames
under separate new evidence IDs. Production and selftest PE32 builds pass.

## Continuous command backpressure

2026-10-06: `NR.command-backpressure` retains bounded whole-append ownership and
sticky hard overload while adding guarded validation during idle, adaptive worker
retry intervals, pressure diagnostics, and continuous-only no-ACK timeout.
Validated ACK progress refreshes the deadline; fully drained work disarms it.
Cancellation and malformed ACK retain their own reasons; a no-progress open
stream refuses INTERRUPTED. Producer callbacks never wait for the reader, and
terminal publication remains distinct from GPU completion.

[Backpressure policy](opengl-command-backpressure.md) separates raw PE32/queue
ownership tests from independent healthy native GPU frames. Historical source
fingerprints/statuses are retained. No original artifacts/gameplay or semantic
coalescing, dynamic budgets, OS scheduling guarantee, orchestrated shutdown or
recovery is claimed. The completed inventory/first three producer chunks are
committed in 422be2a; subsequent chunks are committed after staged accounting.

### 2026-10-06 continuous startup and orderly exit

`NR.command-orchestration` adds explicit startup readiness and a guarded resolved
main-image ExitProcess import for continuous mode. Shutdown closes capture
admission, emits resource DELETE/END, drains and joins outside DllMain and forwards
original exit. Configured channel failure cannot silently become optional. The
existing Qt consumer owns final drain and cleanup. The
[contract](opengl-command-orchestration.md) separates synthetic integration from
original-game exit branch coverage, forced termination, loader-lock exit and
recovery. Earlier evidence remains historically fingerprinted.
The prior committed range `422be2a..bc29824` has matching exact file and behavior
receipts in [its history review](coverage/committed-history-backpressure-20261006.json).

Final-source orchestration evidence passes eleven PE32 cases / eight complete
GPU frames, eight pressure cases, five idle cases, two continuous GPU cases /72
frames and native/sanitized queue checks. The register links new immutable
records; historical statuses and fingerprints are retained. Original-game exit
coverage and recovery remain pending.

### 2026-10-06 explicit fresh-session recovery

`NR.command-recovery` introduces a PE32-local recovery export for a quiescent
caller after worker shutdown/join and mapping retirement. A distinct exact-size
v2 file with usable filesystem identity and a higher session ID establishes a
new resource namespace; a finite sixteen-file history prevents reopening old
files, aliases or IDs. Old checkpoint/alias/palette state is discarded before
fresh observations. Archives receive distinct suffixes. The
[contract](opengl-command-recovery.md) retains automatic Qt negotiation,
same-context consumer handoff, original-game recovery, unobserved leases,
in-flight callback stress and failure injection as pending boundaries.
Historical evidence/statuses retain their fingerprints. The committed range
`bc29824..4eb3977` has matching exact receipts in
[its history report](coverage/committed-history-orchestration-20261006.json).

Final-source recovery evidence passes nine scenarios /33 same-process sessions /
66 independent GPU frames, with immutable prior terminal files and fresh reset
state. Twenty-five native/sanitized queue cases and eleven writer cases each,
eight pressure cases, five idle cases, eleven startup/exit cases /eight frames,
and two continuous GPU cases /72 frames pass. Production/selftest PE32 builds
pass. The central register links six new records for this scoped native policy;
original comparison and live replacement remain none.

### Qt command recovery orchestration (2026-10-06)

`NR.command-host-recovery` extends explicit producer recovery with a versioned
administrative channel and automatic fresh-file/higher-session handoff in the
same Qt viewport/context. Failed consumer records/textures are discarded;
forwarded input resumes only after matching producer READY and a complete new
PRESENT. Finite reply/frame deadlines, three recoveries, observed callback and
pixel/DC quiescence, permanent cancellation and original-window fallback are
native policies. Loader-lock worker creation/join and original process kills
are excluded. [Contract and boundaries](opengl-command-host-recovery.md).

Synthetic PE32/Qt independent full-frame comparisons, held/concurrent ownership,
refused/invalid/silent/cancelled negotiation, repeated failures, startup-stream
failure and fresh-frame timeout are recorded separately from actual gameplay.
Original game recovery, real-driver equivalence and movie/recovery interaction
remain pending. Shared-source historical evidence stays pinned and may be stale;
this milestone neither refreshes those hashes nor claims original comparison or
live replacement. Committed predecessor receipts are reviewed separately in
`coverage/committed-history-host-recovery-20261006.json`.

Final-source host validation passes 13 scenarios /143 independent GPU frames,
10 native and 10 ASan/UBSan control cases with leak checking, nine invalid launch
configurations, and nine explicit producer API regressions /33 sessions /66
frames. Native/sanitized queue and four Qt/OpenGL regressions pass, along with
production/selftest PE32 and Qt shell builds. New immutable evidence records
retain those bounds; original comparison and live replacement remain none.

### Explicit palette resource identities (2026-10-06)

`NR.palette-resources` adds inner command stream v2 with independent palette IDs,
immutable lifetime generations, shared ordered RGB updates, surface bindings and
verified final-Release retirement. Continuous launch defaults to this profile;
explicit legacy and bounded profiles retain stream v1. Palette and surface
namespaces are qualified by the fresh transport session. Capacity and ambiguity
refuse the stream; shutdown retires surfaces before palettes.
[Contract, evidence scope and gaps](opengl-palette-identities.md).

Final-source synthetic PE32 validation covers 321 independent indexed GPU frames,
40 same-pointer recreations with aliases/nonfinal/final Release, six layout
replacement frames and one unsupported palette-flags refusal frame. Native and
ASan/UBSan consumer runs each pass 21 independent frames and 15 identity/version/
capacity/refusal checks, with zero ordinary readbacks and terminal resources.
Actual-game equivalence, producer counter-exhaustion/conflicting-alias fault
injection, implicit destruction, late attachment and retained checkpoint
recovery remain pending. Historical shared-source evidence stays pinned.

The committed range `529019f..4428c27` was reviewed against exact parent/current
receipts in `coverage/committed-history-palette-resources-20261006.json`. A missing
`protocols/CONTRACTS.md` file receipt was appended retrospectively, independently
of working changes or claims about the prior gate.

### Complete owned checkpoint attachment (2026-10-06)

`NR.command-checkpoint-attachment` adds strict administrative CHECKPOINT=2 and
Qt `attachCheckpoint()` for delayed readers or fresh attachment during drawing.
It preserves complete independently owned pixels, current metadata, observed
aliases and palette lifetime generations, then serializes all resources and an
initial PRESENT into bounded storage before READY. Incremental drawing resumes
in the fresh wire namespace. Unknown/incomplete state, borrowed locks/DCs and
capacity limits refuse; ordinary RECOVER retains its fresh-observation policy.
[Contract and remaining boundaries](opengl-command-checkpoint.md).

Ten synthetic PE32/Qt scenarios pass six fresh checkpoint sessions and 42
independent full native-pixel frame comparisons. RGB32 and indexed/shared/aliased
palette continuations cover partial negative-pitch writes, failed-call retries,
fills, keyed/unkeyed copies, flips and palette RGB/flags updates. Seven distinct
incomplete/borrowed/capacity refusal categories plus uncertain metadata are
covered. Four automatic recovery regressions pass 83 frames; eleven native and
eleven ASan/UBSan control client cases, five protocol checks and three existing
control/OpenGL CTests pass. Production/selftest PE32 and Qt shell builds pass.
Original call/error preservation and immutable retired rings remain explicit.

This is attachment within an already established producer/host launch, not
independent external host takeover. Actual-game checkpoints, 16/24-bit and legacy
palette checkpoint fixtures, concurrent checkpoint callback collision and injected
allocation/worker/map failures remain pending. Shared historical evidence stays
pinned; no original comparison or live replacement status is promoted. The
predecessor `4428c27..ade3e60` was reviewed against exact parent/current receipts
in `coverage/committed-history-checkpoint-20261006.json`.


## Rendering recovery failure validation (2026-10-06)

The broadened administrative checkpoint fixture passed 20 cases, 27 fresh sessions
and 294 independent GPU frame comparisons. RGB16/24/32 and indexed/mixed state,
120 same-pointer palette recreations, repeated overflow/stall/foreign ACK recovery,
and mapping/allocation/worker-start refusal cleanup are covered. Host regressions
passed 83 frame comparisons; native and ASan/UBSan queue/client checks passed.

Two bounded No-CD startup observations requested recovery after original drawing
began. CHECKPOINT was refused at complete-state admission. Ordinary RECOVER reached
READY, then invalidated before any native PRESENT. Both consumers fell back with
zero remaining resources; the original process and main menu remained available.
Original manifest verification passed before/after. Game recovery, gameplay routes,
original pixel equivalence, movies and live replacement remain pending.
See [the validation record](opengl-command-recovery-validation.md); synthetic
success and actual-game fallback are separate milestones. Historical evidence
retains its hashes; shared-source changes do not refresh earlier comparisons.


## Rendering recovery metadata diagnosis (2026-10-06)

Early CHECKPOINT refusal is confirmed: the primary has current 800x600 RGB565
metadata but no owned pixels. Waiting for a complete original primary permits
attachment. Ordinary recovery now retains verified lifetime aliases/properties
while discarding pixels; a one-query canonical-lock/alias-unlock fixture passes
three sessions. Qt polling now consumes its finite byte budget across 64KiB
fragments, retaining the 32-command bound, rather than one fragment per GUI tick.

Fresh validation passed 294 CHECKPOINT pixel comparisons, 20 explicit-recovery
comparisons, 83 host comparisons, native/sanitized GPU budget cases and queue/client
checks. Production PE32/Qt builds and three selected CTests passed. Original-game
checkpoint delivered one native frame before sustained overflow; ordinary recovery
retains aliases but misses required source pixel baselines and falls back. Every
consumer cleaned up, original main menus remained visible and original manifests
passed before/after. First-frame delivery does not establish sustained recovery or
equivalence. See [the diagnosis](opengl-command-recovery-metadata.md).


## Automatic owned-checkpoint recovery (2026-10-06)

Administrative RECOVER now prefers complete independently owned state before
candidate claim, preserving initialized source pixels under fresh wire IDs.
Incomplete state uses fresh observations; borrowed/lifecycle admission and failed
claimed checkpoints refuse. Direct RenderRecover and strict CHECKPOINT remain
separate policies. No finite budget or wire version was relaxed.

Fresh validation passed 16 administrative cases/189 independent GPU comparisons,
20 strict cases/294 comparisons and 4 direct-export cases/20 comparisons, plus
native/sanitized client checks and selected CTests. A static-drawing case restores
a frame without any new original locks; incomplete and worker-fault cases prove
the fallback/refusal boundary. Original-game automatic retries selected complete
checkpoints for sessions 124/125/126 and presented three native frames. Sustained
queue overflow still exhausts recovery; original windows and cleanup remain intact.
Original manifests passed before/after. First-frame recovery is validated within
this bounded observation; sustained gameplay and pixel equivalence remain pending.
See [automatic checkpoint recovery](opengl-automatic-checkpoint-recovery.md).

## Compact continuous owned unlock updates (2026-10-06)

Continuous full writable locks quarantine a matching owned baseline under pending
storage accounting. Successful original Unlock emits the smallest changed native
pixel rectangle, or no UPDATE when storage is unchanged, preserving PRESENT and
checkpoint/lifetime ordering. Initial/missing/changed-layout baselines retain
complete replacement. Partial locks, DC handoffs, fills, finite queue/pixel/surface
budgets and sticky overload refusal retain their existing policies.

Native/sanitizer reconstruction, independent synthetic full-frame continuity and
checkpoint/recovery guards are recorded in the
[delta research](opengl-owned-unlock-deltas.md). Bounded original-game startup
recovery now requires sustained publication in one healthy session. This milestone
does not assert original pixel equivalence, gameplay/movie interaction, indefinite
throughput or live replacement; historical source fingerprints remain historical.

The final original-game checkpoint and automatic-recovery observations presented
208 and 235 native frames respectively, each in one healthy session for 12 seconds,
with zero ordinary readbacks/uploads and complete consumer cleanup. These exceed
the earlier three-frame/retry-exhaustion boundary without enlarging any queue.

## Original campaign routes and independent menu regions (2026-10-06)

The new bounded route harness pairs continuous native rendering with forwarded
original campaign observation and independent X11 client snapshots. Five stable
menu regions match within one RGB channel value, including Region Entry after a
second failed-reader recovery. Campaign entry reaches three original World ticks
and 238 native presentations, then the 33rd tracked surface exceeds the existing
32-surface budget. Complete checkpoint admission refuses, fresh observations hit
untracked copies, and native rendering falls back with zero retained resources
while original World drawing continues.

Movie-enabled startup reaches Main and presents 217 native frames in a healthy
recovered session; actual movie playback has no sample hits in the bounded archive
and remains unverified. Whole-frame/gameplay/movie/driver equivalence and replacement
remain pending. No budgets or production admission rules changed. See the
[route evidence and remaining boundaries](opengl-render-routes.md).

## Original RGB565 keyed wrapper outputs — 2026-10-06

Unchanged no-CD wrapper0x58ca90 ran in a private PE mapping against real Wine
Surface2, retaining436 complete before/after cases, both APIs/WAIT branches,
exact high-bit keys and144 second calls after key mutation. CPU checks compare
all436 outputs; native COPY compares434 successful outputs with868 complete
source/destination checks. Two E_INVALIDARG missing-key Blt calls remain captured
without native HRESULT equivalence. Seven provenance/state/output refusal tests
pass. No live gameplay, Windows-driver, indexed/clipped key, retry/Restore or
replacement claim. Wine experiments are serialized. See
[findings](original-surface-keys.md). The reviewed454c5d5..e6b493a history range
has no missing exact file receipts; current uncommitted route work is separate.

## Original RGB565 fill wrapper outputs — 2026-10-06

Unchanged no-CD wrapper0x58bac0 ran against real Wine Surface2 in the isolated
loader-reserved mapping. Its250 cases cover full/partial/edge/invalid rectangles,
five clipper states and original low16-bit truncation of high32-bit arguments.
CPU admission and complete destination comparisons match145 successes,65
invalid-rectangle failures and40 missing-clip-list failures. Native argument-derived
constant UPDATE regions match145 full destinations; expected after pixels are
CHECK diagnostics only. Eight provenance/state/color/result refusal tests pass.
Error-reporting code executes with bounded MessageBoxA/DestroyWindow observers.
Native fill HRESULTs, original lost/Restore/retry and0x58bc10, unknown initial
storage, indexed/masked formats, live composition and Windows-driver equivalence
remain pending. Keys and fills used serialized Wine sessions and manifest checks
before/after each original experiment. See [findings](original-surface-fills.md).
The ledger retains one keyed-work entry; a duplicate introduced while combining
concurrent ledger edits is removed without changing its evidence or scope.

The shared keyed checker now derives its missing-key Blt error admission from
key/API input state and compares all436 CPU HRESULTs. A fresh native434-copy
regression and eight keyed refusal tests pass; the historical keyed replay record
is retained with its original source hashes. Native keyed HRESULTs remain pending.

## Finite continuous consumer residency (2026-10-06)

Continuous publication now reclaims eligible offscreen wire slots with ordered
DELETE and re-admits complete owned baselines under fresh IDs. The consumer still
permits32 resident surfaces/16,777,216 pixels; the producer's128-entry/64 MiB CPU
store and queue/ring/ownership/retry bounds remain finite. Primary, borrowed and
current copy/flip operands stay pinned. Consumer retirement is separate from
verified application final Release and CPU/alias lifetime. Strict recovery retains
complete CPU validation and aggregate guards, materializing a complete primary
before READY and other verified dependencies when used.

Six cache, 20 checkpoint,16 administrative, nine continuity and nine lifetime cases
pass 1,128 independent complete-frame comparisons. The required original campaign
run stays Active across World entry and a forced failed reader, then publishes 29
more frames over 3.02 seconds in one recovered session. Four stable menu regions
match independently captured X11 pixels within one channel value; original World
and cleanup/manifests remain intact. Whole-World pixels, long action/effect/HUD
scenes, movies, hardware drivers and replacement remain pending. See the
[policy, evidence and finite boundaries](opengl-resource-working-set.md).

### Guided idle World pixels and sustained publication (2026-10-06)

[Independent World observation](opengl-world-observation.md) extends the required
campaign route to 30 seconds after forced reader recovery: 642 total native
frames, 331 additional frames over 32.398 seconds in the same Active session.
Initial guidance, five forest terrain regions and five commander portrait regions
match independent original X11 pixels; four menu comparisons also pass. Explicit
test snapshots are separate from ordinary renderer readbacks/uploads, which stay
zero, and final native resources retire. Original artifacts/preferences are
preserved. The opening dialog advances to Select Zombie Spell: active tutorial
actions, creature animation, spells/effects, other scenes, movies, synchronized
whole-frame/driver equivalence and replacement remain pending. Historical
synthetic fingerprints/statuses are retained rather than refreshed.

## Original opaque RGB565 overlap — 2026-10-06

Unchanged wrapper0x58c360 capture and input-derived CPU comparison now cover same-surface copies, four directions/diagonals, identical/nonoverlapping rectangles and verified Surface1 aliases with ordered clipping and selected errors. See [overlap evidence](original-surface-overlap.md). Native GPU replay, keyed/masked and other formats, original loss/Restore/retry, live replacement and Windows driver equivalence remain pending.

## Native opaque RGB565 overlap replay — 2026-10-06

GPU per-clip-piece snapshots and v3 same-ID replay now independently match all1536 retained original opaque RGB565 result/pixel cases, including16 partial-write errors and768 verified interface aliases normalized to shared logical identity.20 backend policy checks and6 refused streams pass; ordinary backend copies perform zero CPU uploads/native or RGBA readbacks. Eight renderer regression tests pass. Scope remains offline; keyed/masked/other-format overlap, live COM tracking, loss/Restore/retry and Windows equivalence are pending. See [native overlap evidence](original-surface-overlap-native-20261006.json).

## Original surface Restore/retry scheduling — 2026-10-06

4480 unchanged original four-wrapper scripted fault traces compare ordered Restore/key/reload/retry events with an input-derived CPU model. See [recovery evidence](original-surface-retry.md). This is control-flow evidence; actual driver loss/Restore/pixels/palette, asset/sentinel reload and native/live replacement remain pending.

## Bounded surface recovery and native content validity — 2026-10-06

Portable recovered C++ callback scheduling matches4480 original scripted fault traces;1536 budget stops preserve the original event prefix without reporting success. Native invalidateContents policy refuses unknown read/presentation/sampling until each needed region is established by explicit updates/opaque copies, with30 GL tests including test-only recovered fill adapter composition. Eight renderer regressions pass. Real driver loss/Restore/pixels/palette, asset/sentinel reload and live/wire recovery remain pending. See [recovery evidence](original-surface-retry-native-20261006.json).

### Tutorial summon observation (2026-10-06)

Original Zombie selection and right-click summoning are now observed independently
through the original tutorial prompts and `0/15` to `1/15` control count. Identical
metadata observations preserve native generations; 745 independent fixture frames
cover nested metadata, uncertain source copies, mixed mutations and palette/DC
regressions. Active gameplay can still expose successful original copy interleaving
and conservative native refusal. Ordering and animated/full-frame equivalence remain
pending; see [the scoped report](opengl-world-summon-observation.md).

## Standalone real surface driver recovery — 2026-10-06

Two serialized Wine11.16 Surface2 API captures cover16/32 display modes and four requested cap classes: actual IsLost/Lock loss, offscreen Restore success, primary wrong-dimension refusal followed by successful Restore, repeated Restore and source-key retention.2458 ordered rows verify offline;8 provenance/refusal tests pass. No game image/native renderer is executed. Lost COLORFILL returns success, so original-wrapper actual-lost-draw composition remains pending; undefined restored bytes are observations, not portable output. Palette/clipper/flip chains, Windows drivers, reloaders and live recovery remain pending. See [driver findings](surface-driver-restore.md).

### Opt-in copy ordering (2026-10-06)

Continuous producers can opt into bounded scheduling across original Blt/BltFast
calls, admission and commit. Timeout forwarding and reentrant writes retain
uncertain-state refusal. Six fixture cases compare408 complete frames. A strict
original Zombie summon observation remains Active for308 more frames over25,221ms
after forced World recovery, with13 independent stable-region comparisons and
zero terminal resources. This native scheduling policy remains opt-in; repeated
active gameplay, other mutation concurrency and animated/full-frame/driver
equivalence remain pending. See [copy ordering](opengl-copy-order.md).

## Indexed palette retention and explicit surface reload — 2026-10-06

Serialized real Wine11.16 indexed8 API evidence covers GetPalette loss refusal, post-Restore within-run identity/entry retention, shared partial palette updates during loss, explicit full index writes, full palette updates and rebind without index mutation.8233 ordered rows/6656 palette entries verify offline;9 refusal tests pass. Nine native API compositions pass45 unknown guards,18 index comparisons and864 input-derived RGB pixels. No original game/asset reloader is executed and no driver RGB display equivalence is claimed. Original asset/sentinel reloaders, actual-lost-draw wrapper composition and live recovery remain pending. See [palette recovery findings](surface-palette-restore.md).

Final-source repetition confirms the original summon/native control region, then
refuses at497 frames through pixel-tracker contention. Its bounded trace contains
no prepared-copy generation conflict or copy-gate timeout. The earlier passing
strict attempt remains historical; reliable active-gameplay continuation and
tracker ownership across other mutation families remain pending. Final synthetic
ordering/timeout/reentry matrix still passes408 independent frames.

### CPU handoff ordering and tracker provenance (2026-10-06)

Bounded source-tagged tracker diagnostics observe8..15ms copy-commit holds,
exceeding the unchanged8ms admission budget. The historical failed requester is
still unknown. The existing opt-in50ms scheduling gate now covers CPU Lock/Unlock
callbacks as well as copies, retaining borrowed-interval, nested-write and timeout
refusal. Eight independent fixture cases compare411 complete frames. A strict
final-source original summon and30-second post-action observation remain Active
with500 more frames after forced World recovery and17 matched stable
regions; terminal resources are zero. Other mutation concurrency, scene repetition
and animated/full-frame/driver equivalence remain pending. See
[CPU handoff ordering](opengl-cpu-handoff-order.md).

## Original cursor sentinel and bitmap reload — 2026-10-06

1698 unchanged original x86 sentinel/bitmap loader/four-wrapper captures compare20060 ordered scripted CRT/COM/GDI endpoint events offline from12 bitmap inputs, including the installed cursor asset.10 regression tests pass. The -1 route always reloads the fixed global cursor, including twice when both copy receivers recover, and sentinel/DC loader ignore nested/DC/GDI failures within the scripted out-parameter scope. See [cursor reload findings](original-surface-sentinel.md). Submitted DIB buffer hashes are captured; real GDI pixels, native resource routing/conversion, other reload bodies, malformed/second allocation branches and live recovery remain pending. No Wine executes in this chunk.

### DC handoff and flip ordering (2026-10-06)

The existing continuous opt-in scheduling gate now spans GetDC, ReleaseDC and
Flip callbacks alongside copies and CPU handoffs. A timed-out original flip
invalidates both metadata/pixel epochs before and after forwarding, refusing
uncertain buffer history. Budgets remain unchanged and borrowed painting remains
outside callback admission. Eleven isolated committed-renderer fixture cases
compare228 complete frames, with six valid and five explicitly refused sessions.
The final private-installation strict original summon remains Active with
487 more frames over40904ms after forced recovery;17 stable regions
match and terminal resources are zero. Source/preference guards and immutable
manifest checks pass. Workspace-probe records against independent renderer changes are retained
separately. Palette/property/lifetime concurrency, DC/flip-specific reentry and
borrowed intervals, repeated gameplay and animated/full-frame/driver equivalence
remain pending. See [DC and flip ordering](opengl-dc-flip-order.md).

## Native global cursor reload and independent GDI pixels — 2026-10-06

Explicit global cursor routing and bounded original sequential BMP input parsing match1456 original dispatch projections and12 file result/submitted buffer cases, with813 bounded prefix stops. One reserved Wine11.16 GDI run captures18 standalone RGB32 DIBSection raster outputs from original input buffers;336732 RGB pixels compare offline and6 mutation tests pass.36 nativeRGB24/32 reload/read/present cases match1346928 captured RGB pixels,12 scripted recovery compositions preserve triggering-surface unknown contents, and137 refusals pass.4480 legacy callback schedules/1536 budget stops,30 restoration checks and8 renderer regressions freshly pass. Historical evidence hashes, including initialRGB32 test provenance, are retained. Headless native cursor composition is recorded separately from pending DirectDraw DC/other formats/loss/live/wire recovery. See [native reload findings](native-cursor-reload.md).

### Palette and property callback admission (2026-10-06)

The existing continuous opt-in gate now spans palette capabilities/entry reads,
bindings, updates and Initialize, plus surface source-key/clipper setters.
Timeout forwarding invalidates metadata/pixel epochs before and after originals;
shared palette uncertainty refuses rather than guesses dependent state. Fourteen
independent cases compare554 complete frames, including setter retries, explicit
palette recreation, three distinguishable indexed copy/color phases, timeout
refusal and same-thread source-key generation invalidation. The strict original
Zombie summon remains Active with487 more frames over40721ms after
forced recovery;17 stable regions match and terminal resources retire to zero.
Source/preference guards and immutable manifest checks pass. Dedicated getter/
binding/Initialize concurrency, nested palette updates, creation/aliases/final
release and other metadata callbacks remain pending; see
[palette/property admission](opengl-palette-property-order.md).

## Native Surface2 DC formats and RGB565 colors — 2026-10-06

Two serialized reserved Wine11.16 runs preserve72 standalone Surface2 GetDC/GDI/ReleaseDC cases each, from original submitted inputs, with native words/admission/selected bitmap/output handles and216 sample/full DC GetPixel outputs. Immutable manifests pass before/after. Full offline comparison matches1346928 native and1346928 DC RGB pixels, all32/64/32 RGB565 channel levels; prior floor expansion differs89860 pixels. Native RGB565 reload and shared presentation bit replication now match2693856 independent pixel comparisons in72 cases,24 scripted cursor compositions/253 refusals;1456 original dispatch projections/12 submitted buffers/813 bounded stops freshly pass. RGB24/32 compatibility36cases/12compositions/137refusals,9 renderer/2 sprite tests,104 viewport frames at scales1/1.5 and10 offline mutation tests pass. Historical evidence remains unchanged. Native DC admission, indexed/DC clipping, real loss and live/wire remain pending, including separate runtime CPU floor-color alignment. See [DC findings](surface-dib-ddraw.md).

### Resource lifetime callback admission (2026-10-06)

Continuous opt-in admission now spans QueryInterface, CreateSurface/CreatePalette
and surface/palette Release through original and verified lifetime processing.
Timeouts invalidate metadata/pixels and schedule alias reset on both sides of
forwarding; uncertain pointer reuse never retains old provenance. Fifteen cases
compare72 complete frames, including creation/QI/release worker waits/timeouts,
failed creation/retry and descriptor poisoning, pointer reuse and borrowed
retirement refusal. The committed-renderer shared palette/drawing regression
compares545 frames. The private strict original summon remains Active with
487 more frames over40296ms after forced recovery;17 regions match
and terminal resources retire to zero. Source/preference and immutable-manifest
guards pass. Historical/rejected workspace records remain unchanged. Dedicated
palette lifetime races, nested/failed QI/lifetimes, DirectDrawCreate/startup, other
metadata callbacks and unseen-interface/implicit destruction recovery remain
pending; see [lifetime admission](opengl-lifetime-order.md).

## Native indexed bitmap palettes and separate DC clipping — 2026-10-06

Two serialized reserved Wine11.16 runs preserve360 retained-original-input and150 owned identity/duplicate/bin-boundary Surface2 DC cases; manifests pass before/after. Attached DirectDraw clippers do not constrain these GetDC writes, while explicit GDI regions apply their union. Installed palette/index translation, identical full8-bit index preservation and24-bit5-bit-bin-center conversion compare450cases/21600 native words/21600 DC RGB values plus clipped samples offline;60 absent-palette conversions remain observation-only. Native450cases match43200 independent pixels with13995 defined checks/9510 refusals; partial palette updates/storage ownership and9renderer/15offline mutation tests pass. RGB565/32 andRGB24/32 compatibility72/36cases,2693856/1346928 pixels and24/12 scripted cursor compositions freshly pass;1456 original dispatch projections/12 exact buffers/813 bounded stops match. Initial300-case provenance remains historical. Native COM DC/lease reuse/default/flag/shared palette, arbitrary geometry/indexed original cursor/real loss/Windows/live/wire remain pending. See [indexed/DC findings](surface-dc-palette.md).

### Surface metadata and unsupported mutation callback admission (2026-10-06)

Continuous opt-in callback admission now spans surface descriptions and attached
surface queries, attachment add/delete, Restore and BltBatch. Timeout forwarding
invalidates metadata/pixels on both sides of original-once execution. Admitted
failed originals retain state; successful unsupported mutations still invalidate
rather than invent restored/batch pixels. Twelve failure/retry worker schedules
pass18 complete frame comparisons with two valid/ten refused sessions. Ordered
unsupported cases include a consumer drain window before successful invalidation.
The nine-case shared palette/drawing regression passes745 complete frames.
Production PE32 build and exact intervening committed-history receipts pass.
Original live/driver comparison of this extension, startup admission, borrowed
interval ownership, attachment recreation/stale interfaces and unsupported pixel
reconstruction remain pending. See [metadata admission](opengl-metadata-order.md).

## Default palette context and repeated bitmap DC snapshots — 2026-10-06

One serialized reserved Wine11.16 capture192cases/576leases retains separate preflight default palette input, before/draw tables, normalized DC identity/clip boxes and complete native/DC RGB frames. Manifests pass before/after; a failed compilation attempt produced no retained evidence. Offline27648 native words/27648 RGB values/258048 entries compare;228 indexed leases use supplied default context and9 mutation tests pass. Each new lease resets application clips/snapshots binding/default, while in-lease binding updates defer and direct DC edits remain private. New owned BitmapDcState/raster adapter matches55296 independent pixels/258048 entries with1928 guard/validity refusals and no surfaces; renderer primitives are unchanged. Default context is explicit environment input, not a portable Windows palette formula. Real COMDC/global lock/busy, shared/logical/flag/other GDI state, original loss/Windows/live/wire remain pending; earlier absent-palette evidence remains historical. See [DC reuse findings](surface-dc-reuse.md).

### DirectDraw factory startup admission (2026-10-06)

Continuous opt-in drawing admission now spans DirectDrawCreate startup, original
factory execution and returned-interface installation. Timed-out originals still
forward once while resetting uncertain alias/metadata state and leaving unseen
interfaces uninstalled. Seven first/delayed/failure/null/nested/worker schedules
pass13 complete frames with six valid/one refused session. The fixture installs
no test hooks before initial creation;108-byte descriptors and pointer Unlock
exercise DirectDraw1. Factory startup publishes no incremental packets before
complete initial pixels and does not reset existing resource/session identity.
Lifecycle11-case/8-frame and late checkpoint6-case/186-frame regressions pass,
including incomplete/active Lock/DC refusal. Production/SELFTEST DLLs build.
Original driver/startup comparison, alternative factory entry points, external
startup/shutdown/recovery races, unobserved interface recovery and borrowed
interval ownership remain pending; see [startup admission](opengl-draw-startup-order.md).

### Startup/shutdown transition ownership (2026-10-06)

Startup/shutdown own lifecycle serialization across complete state transitions,
so a losing call cannot finish or disable capture before obtaining ownership.
Ordered-mode shutdown waits for a short admitted callback to commit, while
50ms admission timeout or pre-completion same-thread reentry refuses without
invalidating publication. Completed shutdown returns its cached result before
callback quiescence; original-only later drawing cannot restart the stream.
Six worker/guard/reentrant/completed fixtures pass12 complete frames, including
opt-in on/off transition contention. Fresh lifecycle11-case/8-frame, factory
7-case/13-frame and late checkpoint6-case/186-frame regressions pass. Production/
SELFTEST DLLs build. External recovery collisions, simultaneous long joins,
tracker closure contention, abandoned owners, administrative worker shutdown and
original game/driver/full-frame equivalence remain pending; see
[lifecycle race findings](opengl-lifecycle-races.md).

## Surface lock/DC admission — 2026-10-06

One serialized Wine capture96 standalone Surface2 sequences spans indexed8/RGB565/RGB32 and system/default offscreen caps. Offline and native state replay compare independently captured HRESULTs/normalized output descriptors and readable unchanged final words; terminal busy cases supply no invented bytes and unstable failure caps words are excluded. Mutation checks verify corruption detection. Native tokens/storage bounds/single-thread state are intentional policies; DC raster association/drawing while borrowed, aliases/other flags/rectangles/cross-thread/original game/loss/Windows/live remain pending. Manifests verified before/after; initial sandbox socket and terminal-success-assumption failures yielded no evidence.96cases/1182transitions/8664stable descriptor fields/4032readable words compare with11offline tests/10native guards;84unstable caps words excluded and12terminal busy cases have no final frame. See [lock/DC findings](surface-access.md).

### Recovery overlapping lifecycle transitions (2026-10-06)

Recovery now shares scoped lifecycle ownership through admission, candidate claim,
reset/publication/worker launch and cleanup. Three actual startup/shutdown/recovery
ownership barriers verify all competing entry points refuse before mutation, both
candidate channels remain byte-for-byte unchanged, and retry establishes distinct
sessions with complete independently generated pixels. Three cases compare12
frames in six sessions. Fresh recovery10-case/72-frame, lifecycle6-case/12-frame and
administrative checkpoint6-case/186-frame regressions pass, including alias/repeat/
finite session budgets and failure/borrowed ownership guards. Production/SELFTEST
DLLs build; test pause exports/waits are excluded from production. Drawing during
direct recovery, deeper-stage/repeated collisions, abandoned owners, long/failed
joins, administrative worker shutdown and original-driver/full-frame equivalence
remain pending; see [recovery overlap findings](opengl-recovery-races.md).

## Integrated owned surface operations — 2026-10-06

One native SurfaceBackend associates storage/admission/palettes/clipping/DC reload and RGB565 fills/exact-key copies.2630original/driver HRESULTs/299808pixels/258048palette entries/8664stable descriptors compare retained independent outputs, including105fill/2keyed failures, same-ID key/DC continuations and39native guards.84unstable caps excluded;12poisoned access cases release whole owners. Historical reports stay unchanged; this is fresh native reuse, not new original execution. Keyed overlap/borrowed-fill and new combinations/sharedCOM identity/recovery/other formats/live remain pending. See [owned backend](native-surface-backend.md).

### Drawing during direct recovery (2026-10-06)

Continuous direct recovery now obtains bounded callback exclusion before opening
a candidate, regardless of administrative control or ordered drawing configuration.
Original-once timeout forwarding invalidates alias/metadata/pixel provenance before
and after the original. A fixed64-slot opaque ownership table pins bypassed Lock/DC
through successful exact original return; failed DC return retains ownership.
Before-claim collision preserves an unused candidate; after-claim collision fails
and spends it, keeping worker-visible storage until normal shutdown/new-file retry.
Ten new schedules pass40 independent complete native frames. Fresh full recovery,
transition, lifecycle, administrative checkpoint and orchestration matrices pass;
see [drawing/recovery scope and evidence](opengl-drawing-recovery.md).
Unobserved alias/borrow destruction, opaque table exhaustion, abandoned owners,
administrative worker shutdown/unloading and original game/driver scheduling and
whole-frame comparison remain pending. Historical evidence/statuses are retained.

## Integrated owned surface operations — 2026-10-06

One native SurfaceBackend associates storage/admission/palettes/clipping/DC reload and RGB565 fills/exact-key copies.2630original/driver HRESULTs/299808pixels/258048palette entries/8664stable descriptors compare retained independent outputs, including105fill/2keyed failures, same-ID key/DC continuations and39native guards.84unstable caps excluded;12poisoned access cases release whole owners. Historical reports stay unchanged; this is fresh native reuse, not new original execution. Keyed overlap/borrowed-fill and new combinations/sharedCOM identity/recovery/other formats/live remain pending. See [owned backend](native-surface-backend.md).

## Display-only fullscreen and scaling — 2026-10-06

Qt OpenGL presentation now offers sharp/smooth aspect fit, physical-pixel integer
enlargement with small-window fallback, and F11 fullscreen with chrome/geometry/
maximized-state restoration. Launch controls stay available while waiting for a
frame. Source textures, logical input coordinates, engine display preferences,
simulation and field of view remain unchanged. Synthetic Xvfb/Mesa at DPI scales
1 and 1.5 passes 240 independent complete framebuffers plus targeted XCB input,
CPU-upload/GPU-lease regressions, all fullscreen CLI scaling modes and incompatible
option refusal. Display policy adds zero renderer readbacks or GPU-frame uploads;
smooth interpolation runs only in the viewport shader. Original-game interaction,
other display backends/scales and hardware performance remain pending. Historical
shared-source evidence retains its hashes/status; this fresh native integration
promotes only scoped `HOST.presentation`, not original equivalence or replacement.
See [policy and evidence](viewport-presentation.md).

The presentation toolbar now has an explicit Fullscreen / Exit fullscreen push
button before scaling. Fresh normal/high-DPI button clicks, checked/label state
synchronization with F11, geometry restoration and the existing presentation/input
matrix pass; see `viewport-fullscreen-button-20261006.json`. Earlier evidence is
preserved; original-game/hardware boundaries remain pending.

## Combined owned surface operations — 2026-10-06

Chunk1 is implemented within the measured operation scopes:5114draw HRESULTs/538272pixels,258048palette entries/8664stable descriptors,576DC phases/1182access transitions and40native guards.2304new unchanged-original keyed overlap calls establish forward row-major current-storage reads;180standalone driver calls establish leased COLORFILL success and borrowed empty-Blt success. Native CPU snapshots remain coherent for12held-destination fills. GPU copies use no pixel uploads/readbacks; keyed self-overlap has an intentional4096texel pre-write admission cap. Both captures serialize Wine and verify immutable manifests before/after. Six keyed-overlap and eight borrowed-draw mutation tests pass, plus all14renderer CTests in an isolated committed baseline with exact owned changes. Final fresh evidence supersedes validation of shared sources; preliminary/base evidence hashes remain historical. Shared palette/COM ownership, flip/loss/Restore/retry, additional formats/pitch, uncaptured precedence/negative-debt raster and live/wire/Windows equivalence remain pending; the whole milestone is not complete. See [owned backend](native-surface-backend.md).

Standalone command-replay test hosting now links the fullscreen presentation
controller, fixing the default launcher all-target linker failure. The full Qt
build succeeds; synthetic standalone GPU replay and native-command shell startup
pass without running the original game. Full presentation regression evidence is
retained separately in `viewport-presentation-link-fix-20261006.json`; original
comparison/live replacement statuses remain unchanged.

### Terminal application shutdown ownership (2026-10-06)

RenderStop permanently retires the continuous producer launch, separately from
recoverable RenderShutdown. Qt semantic stop and actual PE32 ExitProcess now
coordinate administrative/publication workers, callback and borrowed ownership,
bounded cleanup/END draining through owned-copy ACK, verified joins and independent
CPU/transport retirement. Failed joins keep storage; successful borrow return
permits retry, cancellation/reader loss/timeout refuse clean completion, repeated
stop cannot revive a session, and post-stop callbacks remain original-only.
Eleven synthetic application-stop cases compare22 complete frames, including real
active checkpoint recovery and failed DC return, plus fresh regression matrices.
The DLL is pinned before installing callbacks. Dynamic unload remains unsupported;
loader detach signals only, with OS reclamation of refused ownership. Separate
frame/input/media and diagnostic mappings remain process-owned. Original-game and
hardware-driver shutdown, abandoned callbacks/borrows, opaque capacity/alias
uncertainty and arbitrary hostile-peer mutation schedules remain pending.
See [ownership, phase bounds and evidence](opengl-application-shutdown.md).

Fullscreen readiness correction (2026-10-06): native-command context startup
shows an empty GL widget. A reproduced visibility-only chrome check hid launch
controls before a usable frame existed. Chrome now waits for an initialized,
nonempty, error-free frame and returns when that frame clears; layout changes
and redraws are deferred. The full launcher build, 34 actual composited-window
checks with retained/changing GPU frames and 240 framebuffer comparisons pass
at scales 1 and 1.5 on a 1920×1440 Xvfb/Mesa screen. Evidence:
`viewport-fullscreen-ready-20261006.json`. Original-game and user-driver
fullscreen behavior remain pending; historical results keep their original hashes.

## Continuous native-command launch default — 2026-10-06

2026-10-07 selection update: ordinary Qt OpenGL game launches now default to
native command presentation, with `--frame-readback` selecting the earlier
original-renderer mirror. Explicit capture/readback diagnostics and Wine-window
embedding retain their existing paths. Existing continuous-v2 and terminal
fallback policies apply. The Launch-button harness exercises native selection
without the old opt-in flag. This application default does not force NVIDIA,
bypass original drawing or establish original/live equivalence; user-driver,
movies and sustained gameplay validation remain pending.
Fresh synthetic evidence `native-render-selection-default-20261007.json` passes
ordinary native launch, matching child channels, 4,202-command drain and retained
visible normal/fullscreen/fractional-DPI frames, plus explicit native continuous
and bounded modes. The shell builds and frame-readback/embedding CLI smoke and
conflicting-selection checks pass. Historical shared-source results remain
historical; no status promotion or original-driver equivalence is asserted.

The three latest completed native-command captures used bounded v1 and each
recorded exactly one entirely black 800×600 RGB565 startup PRESENT before END.
The supplied launch instructions omitted the continuous setting used in the
earlier run. Qt `--native-commands` now defaults to continuous v2 and passes a
matching child environment/control channel; explicit `MNM_RENDER_CONTINUOUS=0`
retains bounded diagnostics. Direct producer/experiment defaults stay bounded.
Actual-shell synthetic Launch-button tests drain 4,202 commands and retain
visible composed frames in normal/fullscreen/fractional-DPI windows; the fresh
240 framebuffer/34 window/input/CLI matrix also passes. This validates application
policy only; original-game retry, hardware, movies, sustained gameplay and native
replacement remain pending. See [findings and boundaries](native-command-launch.md)
and `native-command-default-20261006.json`.

## Owned surface ownership and recovery — 2026-10-06

Chunk2 is implemented within the offline scopes in [ownership and recovery](native-surface-ownership-recovery.md): canonical aliases, caller-released retained/shared palettes, detach/rebind, attached-back reacquisition/retirement, and two-buffer storage/validity/CPU-DC lease swaps. A serialized standalone Wine capture records5647rows/11flips, including busy after swap and object-local cachedDC return failures/flipback recovery; no original game instructions run. Fresh native replay compares2128ownershipwords,6656palette entries/864definedindexbytes/66loss-palette statuses.4480retained original fault traces and1536bounded prefixes freshly match the existing scheduler, separately from16native wrapper/reloader compositions/432input pixels and supplied globalcursor BMP/DC reload. Restore never invents content. Existing5114draw results/538272pixels/258048entries/8664stable descriptors and all15renderer CTests pass in isolated ef5b5a4 baseline with exact own changes;11oracle tests pass. Native counts/budgets/defaults/lost-wrapper fill admission are explicit policies; initial reports/hashes remain historical. Formats/pitch, flagged palettes, longchains, uncaptured interleavings/negative-debt raster, physical original lost-draw/asset internals and live/wire/Windows remain pending; the full surface milestone is not complete.

### Native command refusal and ordered startup (2026-10-06)

The continuous user run `run-b_9tbgld` failed with producer GAP after an8ms
tracker timeout and incomplete snapshot history; both recovered streams also
refused. This is separate from bounded black startup or fullscreen presentation.
Identity-checked native reader diagnostics now preserve the session and terminal
reason; five known/unknown/malformed refusal cases and native pixel/streaming
regressions pass. The actual Qt Launch-button fixture passes all four launch
mode/window/DPI cases.

A fresh bounded original v2 startup completes one native PRESENT with original
drawing retained. Software rendering and skipped movies alone did not complete
the early-checkpoint recovery attempt; that failure is retained. With the
existing explicit `MNM_RENDER_ORDERED_COPIES=1` drawing/CPU handoff policy, a fresh
isolated software-rendered campaign passes328 frames, stable menu region
comparisons, forced region/World recovery, and27 further World frames over3106ms.
All original manifests verify before/after. This intentional scheduling policy
remains opt-in; user hardware/movie playback, unrestricted gameplay, synchronized
full-frame comparison and replacement remain pending. See
[native refusal diagnosis](native-command-refusal.md) and its four immutable
execution/inspection records. Historical fingerprints and statuses are retained.
Committed accounting range `2f95b25..b48b2a9` has no unresolved exact receipt gaps.

## Original display snapshot to native resources — 2026-10-06

`NR.scene-snapshot` observes the pinned original queue consumer at `0x005002a0`
without replacing simulation or drawing. Its bounded immutable v1 protocol owns
ordered display records and normalized frame identities; snapshot-local tokens
replace original pointers. The host adapter resolves explicit pinned SPR assets
through native storage/resource services and renders supported records using
`SceneRenderer`. Readable frame admission, malformed/truncated wire refusal,
alias ambiguity, changed-asset detection, ordering, hidden draws and terminal
surface ownership are tested. The PE32 fixture also checks original forwarding,
return/LastError, sample bounds, x87/XMM/MXCSR state and unchanged queue inputs.

A fresh original Single Player battle produced four complete 800x600 snapshots.
Native replay resolves every supported frame; independent Python SPR decoding
checks all native RGB565 pixels and byte-exact observed frame identities. This
is native diagnostic-policy comparison, not original shaded-frame equivalence.
Strict replay refuses the observed special modes/sentinel adjustments; partial
preview requires explicit `--supported-only`, and embedded-palette replay always
requires `--unshaded`. Shade parameters and omitted record indices remain in
reports. Continuous host streaming, original palette remaps/effects, background,
HUD and live original-renderer replacement remain pending. Existing historical
source fingerprints are preserved. [Scope and reproduction](native-scene-snapshot.md)
and [wire admission](../formats/scene-snapshot-v1.md) document the measured limits.
The source census now includes `compat/` so the host adapter is accounted for.

### Sustained publication and bounded unlock partitions (2026-10-06)

Fresh optimized original World/tutorial summon observation extends the prior
short qualification to60 seconds after the summon. The baseline stays healthy
beyond old frame/operation/record/archive limits; measured UPDATE pixel traffic
includes70.6% unchanged storage in the retained World archive. Existing full
Unlock compaction now chooses at most32 disjoint tight cell rectangles when
pixel plus envelope bytes beat one bounding UPDATE. Finite scratch/CPU/queue/ring,
resource and byte/session budgets are unchanged. Independent native/sanitizer
reconstruction and real PE32 native-byte/GPU fixture checks accompany fresh
continuous/archive, partial unlock and backpressure regressions and strict live
original stable-region comparison. Legacy partial fixtures explicitly call
RenderShutdown before ExitProcess; DLL detach no longer owns END. Whole animated
frames, longer multi-action/battle/movie/hardware scenes, default activation,
original shutdown and live replacement remain pending. See
[measurement, final result and exact limitations](opengl-sustained-publication.md).

Final strict live result:27/27 independent stable regions match,1,139 total
native frames,837 further presentations over71.005 seconds since World recovery,
no unplanned recovery and zero ordinary readbacks/uploads. Recovered World wire
traffic falls from571,708,670 to220,296,440 bytes (61.47%) at similar observed
pacing. Weighted polling removes the split-only RegionEntry lag; that intermediate
25/27 run and its near-full ring samples remain recorded rather than discarded.
The default renderer/producer remains opt-in, without whole-frame or replacement
equivalence. Committed b48b2a9 ownership code is included in the final snapshot;
independent pending presentation/resource/scene workspace changes are excluded.

## Native surface formats and pitch — 2026-10-06

[Chunk3](native-surface-formats-pitch.md) implements indexed8/RGB555/RGB565/RGB24/RGB32 same-format fills/keys/copies/clips/overlap and explicit owned signed byte rows. Standalone serialized Wine capture records3500cases:1400admitteddraws/2100rejectedsetups, complete nativewords/descriptors/keyoutputs/callerpadding/guards. LPSURFACE creation rejects; Surface3 positivepadded indexed/16/32 and tightRGB32 bindings admit; negative and sampled RGB24 memory bindings reject. Native signed imports copy caller bytes and compare2800additional variants against recorded logicaloutputs, separately from driver acceptance. Active-mask fill/key semantics retain installed highkeybits and copiedunusedbits; descriptor keyflags/range and storage-following pitch are explicit. Fresh667800nativeword/descriptor/presentation values,2150400packedbytes,27259checks/60refusals,13oracle tests and all16renderer cases pass across latest suite/serial Xvfb cleanup rechecks; coverage audit/gate suites pass. Existing5114drawresults/538272pixels/258048entries/8664descriptors and11flips/4480originalretrytraces/1536boundedstops freshly compare. Tested tree is isolated fc65636 plus ownchanges; foreign shared-worktree changes excluded. Game captures establishRGB565; static setup8/non8/WORDcreation/pitchLock boundaries do not prove whole requiredformat reachability. Presentation/signed layouts/defaultpitch are native policies; requiredcaller/descriptor census, remaining flags/uncaptured recovery/DC/asset branches and live/wire/Windows are acceptance boundaries. The full surface milestone remains incomplete.

The exact selected commit candidate based on `fc65636` passes fresh synthetic
240 complete framebuffer and34 visible-window comparisons, logical input and
replay checks, seven identity/reason refusal cases, native channel/streaming
regressions and four actual-shell mode/window/DPI launch cases. The shell and
standalone replay target build in Release. The immutable record is
`fullscreen-native-commit-validation-20261007.json`; no new original execution,
hardware performance or input-latency result is asserted for this commit.

After the committed surface-format update (`475019f`), the same selected Release
build and synthetic matrix were rerun successfully. Fresh immutable evidence is
`fullscreen-native-commit-final-validation-20261007.json`; prior records retain
their original hashes. This rerun retains the same hardware/live/latency limits.

## Native semantic visual resources and bounded upload ownership — 2026-10-06

A reusable native manager now binds immutable canonical creature/terrain/effect/UI
IDs to explicit SPR/ANI/TTD/BMP/PCX/JPEG recipes, owns complete decoded resources
and enforces binding/count/resident-capacity budgets. Explicit retirement keeps
IDs stable and assigns fresh reload revisions. A separate GPU cache uses frame/
colour-value/revision keys, LRU eviction and shared renderer budgets, preserves
SPR coverage/origins, and supplies an offline recipe preview. Fourteen synthetic
and dependency checks pass, including complete expected pixels, cache reuse,
palette/revision changes, clipping, refusal and zero terminal ownership. Installed
Redcap frame-zero smoke and immutable manifests pass before/after. Complete
installed recipe population, terrain field/action/lighting/effect admission, scene
snapshot/wire and Qt-shell/live replacement remain pending. This is native policy
and integration evidence, with no new original comparison or gameplay change.
See [resource ownership contract and evidence](native-resource-manager.md).

## Shared native resource-backed scene service — 2026-10-06

Step 2 routes sprite/layer and world previews through semantic resource IDs and
one persistent native background/canvas/upload service. Ordered display records
own optional colour tables; existing ANI clocks, attachment/visibility, camera
projection, exact signed sort and picking identity remain in their adapters.
World recipes own checkpoint ANI bytes with separately bounded encoded storage.
Synthetic tests exercise complete pixels, zero-upload/readback resident draws,
whole-list refusal preserving completed frames, GPU failure blocking partial
presentation, retry and terminal surface cleanup. Installed checks pass195 sprite
frames/390 original controller selections,198 layered frames,144 world pixels/
12 continuations and24 actual Qt windows/128 pixels/16 fresh-window continuations.
New evidence preserves all historical report hashes; broader original contract
equivalence is not reasserted for shared-source changes. Live original scene
snapshots/resource mapping are step 3; complete lighting/effects/action admission,
camera equivalence and rendering replacement remain pending. No gameplay balance
changes. See [scope and reproduction](native-shared-scene-renderer.md).

## Original display snapshot to native resources — 2026-10-06

`NR.scene-snapshot` observes the pinned original queue consumer at `0x005002a0`
without replacing simulation or drawing. Its bounded immutable v1 protocol owns
ordered display records and normalized frame identities; snapshot-local tokens
replace original pointers. The host adapter resolves explicit pinned SPR assets
through native storage/resource services and renders supported records using
`SceneRenderer`. Readable frame admission, malformed/truncated wire refusal,
alias ambiguity, changed-asset detection, ordering, hidden draws and terminal
surface ownership are tested. The PE32 fixture also checks original forwarding,
return/LastError, sample bounds, x87/XMM/MXCSR state and unchanged queue inputs.

A fresh original Single Player battle produced four complete 800x600 snapshots.
Native replay resolves every supported frame; independent Python SPR decoding
checks all native RGB565 pixels and byte-exact observed frame identities. This
is native diagnostic-policy comparison, not original shaded-frame equivalence.
Strict replay refuses the observed special modes/sentinel adjustments; partial
preview requires explicit `--supported-only`, and embedded-palette replay always
requires `--unshaded`. Shade parameters and omitted record indices remain in
reports. Continuous host streaming, original palette remaps/effects, background,
HUD and live original-renderer replacement remain pending. Existing historical
source fingerprints are preserved. [Scope and reproduction](native-scene-snapshot.md)
and [wire admission](../formats/scene-snapshot-v1.md) document the measured limits.
The source census now includes `compat/` so the host adapter is accounted for.

### Isolated native scene commit validation (2026-10-06)

Fresh hash-bound commit records validate14 resource checks, seven migrated
preview checks, three snapshot CTests, the PE32 forwarding fixture and four
independent synthetic pixel compositions against HEAD plus native asset/scene
changes. One initial OpenGL test timeout is retained separately; the full
seven-test retry passes. Earlier installed/captured replay evidence and its
source hashes remain historical when excluded shared build changes differ.
Original queue observation remains independently recorded; new synthetic
validation does not establish shaded output equivalence or live replacement.
Unrelated Qt presentation, command producer and surface-format work is excluded.

### Native scene validation after the format/pitch commit (2026-10-06)

Fresh hash-bound commit records validate14 resource checks, seven migrated
preview checks, three snapshot CTests, the PE32 forwarding fixture and four
independent synthetic pixel compositions against HEAD plus native asset/scene
changes. The earlier OpenGL timeout and successful retry remain historical;
the final renderer profile passes the complete suites. Earlier installed/captured replay evidence and its
source hashes remain historical when excluded shared build changes differ.
Original queue observation remains independently recorded; new synthetic
validation does not establish shaded output equivalence or live replacement.
Unrelated Qt presentation, command producer and surface-format work is excluded.

### Final native scene commit profile (2026-10-06)

Fresh hash-bound commit records validate14 resource checks, seven migrated
preview checks, three snapshot CTests, the PE32 forwarding fixture and four
independent synthetic pixel compositions against HEAD plus native asset/scene
changes. The earlier OpenGL timeout and successful retry remain historical;
the final renderer profile passes the complete suites. Earlier installed/captured replay evidence and its
source hashes remain historical when excluded shared build changes differ.
Original queue observation remains independently recorded; new synthetic
validation does not establish shaded output equivalence or live replacement.
Unrelated Qt presentation, command producer and surface-format work is excluded.

### Input after native command refusal (2026-10-07)

`HOST.command-input-fallback` adds original-window attachment and direct input
after terminal native command failure. The supplied `run-f619ca_i` headers show
GAP in the initial session and all three retries; the user reports lag only
after refusal. Two actual-shell synthetic windowed/fullscreen fixtures exhaust
three retries, attach an independent X11 client, clear forwarded input and
deliver direct mouse/key events. The full Qt build and continuous/bounded launch
regression pass. [Evidence and remaining boundaries](native-command-input-fallback.md)
keep user-driver/game input latency, native media interactions, missing/ambiguous
window execution and producer history repair separate. No original comparisons,
engine behavior or live replacement statuses are promoted. Shared historical
main/session source evidence retains its hashes and may now be stale.

## Native command responsiveness — 2026-10-07

The command reader queues display updates instead of synchronously repainting
inside every PRESENT; the shell continues known backlog through bounded Qt
callbacks, yielding between batches. Every wire command/resource transition
remains ordered, with existing byte/work limits. Synthetic validation passes70
changing PRESENTs in three polls with zero synchronous reader Paint events and
an event delivered between polls; the old implementation fails the new guard.
The last complete pixels survive DELETE/END. Existing fragmentation/checkpoint/
failure/5,077-command streaming tests, targeted input, two actual-shell4,202-record
final-frame fixtures and normal/fullscreen GAP retry/direct-input fallback pass.
No ordinary readbacks/uploads are added. Original game/user-driver latency,
producer contention/GAP repair and live replacement remain pending. Historical
source hashes/statuses remain unchanged. See [policy and evidence](native-command-responsiveness.md).

Performance observation (2026-10-07): two bounded original-game startup/Main-menu
sessions using the real native command reader and viewport under Xvfb/llvmpipe
delivered approximately 19.5/19.6 captured PRESENT intervals/sec. Busy polls were
about 3 ms median and 8 ms p95; about 94% of consumer CPU samples were in Mesa
graphics/JIT code. No measured producer refusal or ordinary native/RGBA readback
or viewport upload occurred. Hardware acceleration is the first diagnostic step;
desktop GPU, battles, producer CPU and end-to-end latency remain unmeasured.
Original drawing remains active. [Immutable timings, provenance and reproduction](native-render-profile-20261007.json)
are linked to the responsiveness, live transport and partitioned-update behaviors
without status promotion or equivalence/replacement claims.

## Headless GPU rendering baseline — 2026-10-07

A profiling-only EGL device adapter now replays the exact retained 15-frame
startup prefix through the unchanged native decoder, consumer, renderer and
shared GPU leases without a desktop. Twelve measured replays per
hardware/software and serial/throughput combination follow two warmups.
Independent RGB565/keyed-copy/update final pixels and complete capture final
pixels match across NVIDIA TITAN RTX and llvmpipe; cleanup retires all native
surfaces and renderer diagnostic readback counters stay zero during timing.

NVIDIA queued completed-output throughput is 96.3 frames/sec median. After the
initial checkpoint, serial frame completion is 10.66 ms median, 13.83 ms p95 and
18.75 ms maximum; remaining GPU wait after submission is 0.033 ms median. Matching
software rendering reaches 132.9 frames/sec, with 7.74 ms median completion.
Approximately 70% of sampled CPU time is under OpenGL context switching and 84%
under surface update (inclusive shares overlap). Each replay executes 3,987
commands including 3,898 uploads; the native update enters/restores its context
per call. Context batching remains an unmeasured optimization candidate.

This qualifies `NR.headless-command-profile` as a scoped headless diagnostic
policy, without changing existing renderer or original comparison/replacement
statuses. GPU timeline intervals include host feeding and synchronization, not
pure shader busy time. These unpaced prefix results are separate from the earlier
Xvfb live startup observation. Desktop GLX, battles, producer/IPC queue age,
actual Qt widgets, compositor/scanout and input-to-screen latency remain pending.
See [tool, reproduction and timing boundaries](../../tools/native-render-profile/README.md)
and the [immutable source-bound execution](native-render-headless-profile-20261007.json).

## Bounded renderer context batching and GL state reuse — 2026-10-07

Native policy `NR.command-context-batching` retains the dedicated renderer context
across consecutive non-PRESENT commands in each already bounded consumer submit.
PRESENT callbacks run with the caller restored. Private FBO attachment/viewport,
shader/VAO/raster state and uniform setup are reused; texture deletion invalidates
the attachment cache. Release ordinary GL errors are checked at batch boundaries,
while resource allocation, explicit readback and Debug checks remain immediate.
FIFO commands, per-command input admission, poll/work quotas, resource ownership,
producer ACKs and GPU lease fences are unchanged.

[Fresh before/after headless evidence](native-render-batching-profile-20261007.json)
uses the same 15-frame/3,987-command/3,898-upload RGB565 startup prefix on the TITAN
RTX. Median completion falls from 10.785 to 0.774 ms, p95 from 18.031 to 1.161 ms;
queued completed throughput rises from 96.9 to 1,061.6 frames/sec. All hardware/
software/mode final pixels match the immutable earlier baseline; independent
keyed-copy/update fixture pixels match. This validates the combined optimization
for finite EGL workload capacity, not desktop GLX, live FPS or input latency.

[Final native regressions](native-render-batching-regressions-20261007.json) pass
16 Release CTests, the Debug consumer test, 708 combined full-frame checks and
context nesting/restoration/error guards. Qt timer replay Skip/Verify and failure
cases pass; ordinary readbacks and viewport image uploads remain zero. The actual
Release shell is rebuilt and its [synthetic CLI checks](native-render-batching-qt-cli-20261007.json)
pass. No original media or executable is consumed by this increment.

Shared `blit.cpp`/`blit.hpp`/`command_consumer.cpp` and consumer-test edits leave
older linked evidence historical/stale; retained records and source hashes are
not rewritten. New regressions cover only their named retained fixtures. Other
original comparisons, sanitizers, hardware widgets, live producer/IPC, battles,
loss/recovery, compositor/vsync/scanout and whole-engine replacement remain
pending, with no gameplay change or original equivalence promotion. The previously
reviewed committed range still ends at unchanged HEAD `32db1b85`; the retained
history audit remains `working/native-render-profile-20261007/history-review.json`.

The [renewed hosting guards](native-render-batching-hosting-20261007.json) retain
current synthetic validation for `HOST.native-command-responsiveness` and
`HOST.command-input-fallback`: 70 queued presentations/three bounded polls,
intervening input, complete >4096-record final-frame drain, high DPI/fullscreen
and terminal GAP direct-input fallback. Earlier shared-source synthetic evidence
is preserved as historical, with no assertion of original or hardware latency.

2026-10-07 fullscreen shortcut: `HOST.presentation` reserves F9 for entry/exit
(previously F11). Existing synthetic presentation tests cover the new key,
repeat suppression, chrome and geometry restoration. This is native display
policy; original-game interaction and physical desktop input remain pending.
Historical F11 evidence is retained unchanged.

## Complete native World canvas comparison — 2026-10-07

`RS.world-frame`, `RS.world-composition` and `NR.world-admission` now own
[effective World input capture and native composition](native-world-frame-rendering.md).
The original consumer remains active; selected primitive entry/return hooks
export pointer-free owned SPR requests, actual selected colours/offsets and
effective clips, with separate original canvas oracles. Native strict resource
binding uses 243 normalized frame identities in eight pinned installed SPR files.
No omitted requests, original destination inputs or unshaded approximation
supply the native result.

Four complete 800x600 consumer canvases/1,920,000 pixels match both independently
executed original raster routines and the captured live original output exactly.
Each contains 1298 ordered requests: 1259 copies, 35 displacement requests and one
each half, quarter-source, quarter-destination and projected shadow. Eighteen
additional six-mode clipped fixtures match 6912 complete pixels. Four native
wire/resource/cache/scene CTests pass, including opaque zero, actual palette
aliases, malformed/truncated input, atomic refusal and surface cleanup. Native
integer OpenGL composition uses owned zero initial storage and runs on llvmpipe;
one readback per frame is deliberate evidence export, not a live producer path.

The [fresh comparison](native-world-frame-comparison-anchored-20261007.json)
also pins the original reference's entry bytes. All
[16 shared renderer regressions](native-world-frame-renderer-regressions-20261007.json)
pass, including native sprite frames, persistent surfaces, palettes, command
ring/replay/consumer and capture replay. Earlier comparison reports remain
immutable; new anchored results use new evidence IDs.

This advances bounded DRAW-02 comparison and the pixel composition part of
DRAW-03. Integration is headless and replacement is none. Continuous immutable
frame publication, Qt live World presentation, camera/active battle actions,
consumer state/readback retention and bypass, later HUD/minimap/text/cursor,
movies/transitions/recovery/full shutdown and physical-driver evidence remain
pending. Original gameplay balance and simulation timing are unchanged.

The first incorrect terrain colour-table interpretation and an intermediate
trace omitting internal horizontal clipping are retained failed runs, not
comparison passes. The latter's 32,948 border-pixel mismatch identified a concrete
missing branch. Current observation/comparison reports preserve pinned build,
source/input hashes and original-manifest verification before/after. The original
copy reference's `0x57e1b0` entry remains outside discovered Ghidra functions;
only its verified eight-byte entry anchor is explicitly registered, without
inventing a complete function boundary.

Earlier shared renderer/observer/staging evidence keeps its recorded hashes and
historical statuses. Source-only changes leave those comparisons stale; this
increment's finite World and synthetic regression checks do not renew the whole
command-stream, UI, recovery or previous live-observer corpus. HEAD remains
`c0fb4bf21064962a491ca8d04ccf679e3bb34a78`; the preceding exact committed-history
review is retained at `working/native-word-mvp/history-review.json` with no
intervening commits. New execution/accounting reports are retained under
`working/tests/world-frames/`.

## User native tracker refusal investigation — 2026-10-07

[Exact-source investigation](native-tracker-investigation.md) retains the
run-tvc6w3zj negative observation under a new evidence ID. Both original/retry
rings refuse GAP; retry acknowledges zero bytes. All 61 captured runtime C/header
hashes match. Wait tags implicate post-blit commit competing with pre-Unlock
capture, then successful-Lock admission competing with post-blit commit.
Recorded holds of 9–16 ms exceed the 8 ms admission budget. Remote samples can
race ownership turnover and include scheduler pauses; CPU/disk/driver causality
remains unproven. Native mode still publishes an RGBA mirror under the tracker;
its contribution is a performance hypothesis, not measured attribution.

Fresh four synthetic ordering/timeout cases pass six complete-frame comparisons.
Paired isolated original Main-menu runs with movies configured enabled both
retain Active native publication after deliberate reader cancellation/recovery:
212 frames/55,035 commands without ordering, 215/55,561 with ordering. Neither
reproduces incidental GAP; sampled movie playback is unconfirmed. All 2,927
original inputs verify before/after both runs. This validates bounded observations
and synthetic policy only. Ordered callbacks remain opt-in; no default, engine
code, wait budget, status promotion or original replacement changes. User NVIDIA
reproduction, callback-stage timing, active gameplay and full-frame equivalence
remain pending. Prior reports and hashes remain untouched.

## 2026-10-07 continuous native World shadow presentation

`NR.world-channel` and `NR.world-live-view` add the versioned two-slot owned
World delivery channel, bounded native v4 sprite catalogue, lazy pinned asset
binding and a standalone Qt GPU viewer. The producer drops publication attempts
without waiting for the host; the reader consumes complete owned requests and
supersedes older completed packets. Rendering proceeds in32-request batches;
partial frames are never presented. Failure, resize and close release the viewport
lease before scene storage. The opt-in launcher is `tools/run-native-world.py`;
menus and battle input remain in the original window, and original drawing runs.

Current execution records65 passing native CTests,18 independent six-mode
original/native composition fixtures and4 focused World/scene CTests. The current
ordinary live run presents24 owned native World frames with0 CPU pixel readbacks,
0 viewport image uploads and0 remaining surfaces;53 publication attempts are dropped
and23 older completed packets superseded. These results validate native delivery
and presentation, not all original World outputs, physical GPU performance or
whole-scene replacement. Cold asset admission remains synchronous (about1.2 s
worst poll), and sustained resource retirement/public launcher/input/HUD/transition
lifecycles remain pending. Reports are linked through the central register and
`native-world-live-*`, `native-world-primitives-current-*` evidence records.

`RS.world-initial-state` records a newly explicit baseline boundary. A retained
live frame has7 nonzero pixels where both current native rendering and private
unmodified original raster execution from zero initial canvas produce zero;
all480000 native/private-original pixels match. Live failures with13 and11 pixels
are also retained. Earlier pre-batching24-frame live equality remains historical
at its original hashes. Retained previous canvas pixels are a hypothesis; initial
canvas ownership/clear/reuse and an untraced writer require observation. Native
rendering is never seeded from the original oracle. Whole-consumer bypass and
unqualified live baseline equivalence remain unavailable. See
`research/runtime/native-world-live-background-gap.md` for the evidence boundary.

No commits landed after the retained c0 fb4 bf committed-history review. Historical
source/evidence hashes and prior receipts remain preserved. Current source-only
staleness is acknowledged separately from the new native and original execution
results; prior unrelated milestones are not renewed by these tests.

## Native canvas history and confirmed retained pixels — 2026-10-07

`NR.scene-history` adds an owned GPU canvas that retains complete native pixels
across contiguous source frames. It starts only from an explicit native
background, checks logical canvas identity and actual source sequence, refuses
gaps/regressions/partial frames, and requires a native reset after execution
failure. The offline `mnm-world-history-preview` accepts pinned closed request
timelines with no original destination pixels. Eight frames across all six
composition modes match private unmodified original execution on all 3,072
pixels. Native initialization in this comparison is explicitly zero. All 66
native renderer/channel/Qt/history regression CTests pass.

The current ordinary viewer presents 24 complete native frames with zero CPU
pixel readbacks, zero viewport image uploads and zero remaining surfaces.
Its 212 dropped publication attempts and 23 superseded packets preserve the
need for complete source-history accounting. The run uses the explicit stateless
native background; it does not establish live retained-history equivalence.

`RS.world-initial-state` now has confirmed retained pixels for a separate
four-frame failing capture: each frame's 15 differences already exist before
the consumer and survive unchanged. Private original replay from that
diagnostic before-canvas matches all 1,920,000 actual output pixels. Native and
private original replay from zero also agree, so the selected discrepancy
comes from the earlier canvas state. Original before pixels are supplied only
to the private original reference and never to native rendering. Another fresh
four-frame capture matches from zero on all 1,920,000 pixels; positive and
failing cases remain separate evidence. The first startup queue already has
365,306 nonzero pixels. Eight bounded pointer-value records do not establish
allocation generations or ownership. Startup diagnostics retain the lazy wave
initialization refusal and unadmitted draw-kind refusals, with original work
forwarded.

This is native history implementation and scoped original comparison, not
interactive history integration or original startup recovery. The continuous
viewer still starts each admitted frame from its explicit native background.
Publication drops and superseding require reconstruction of every intervening
queue before retained live native history is sound. Startup producers, clear/
reuse ownership, remaining draw kinds, HUD/window composition and whole-consumer
bypass remain pending. See [the bounded results and reproduction](native-world-canvas-history.md).
Historical hashes/receipts remain intact. HEAD is still c0fb4bf; no intervening
commits require a new committed-history audit. New census and exact reviews
account for this increment and newly stale shared-source evidence separately.

## Renderer commit split and current evidence, 2026-10-07

The portable word rasterizer and native World composition/frame/history core are
committed independently with partial intermediate registrations. The integration
change retains all original report hashes and append-only reviews. Original/live
World reports preceding the capability-refusal edits are historical; they do not
prove current live equivalence. Current synthetic regression and isolated-original
reruns are registered under new evidence IDs. The additional standalone World
producer refusal self-test is preserved as pending execution/build integration.
Original startup ownership, missed-queue history, whole-dispatch/HUD/input coverage
and complete raster bypass remain pending. See
[commit boundaries](native-renderer-commit-boundaries-20261007.md).

## Normal World shadow refusal and recovery — 2026-10-07

The retained manual launch failed with producer reason 8 before any native
presentation. Its exact queue kind and user scenario remain unknown. New native
policy `NR.world-shadow-refusal` keeps normal shadow sessions active on capture
reasons 7/8: the producer publishes a strict diagnostic header with no partial
requests or oracle, the host clears native presentation and shows a waiting
message, and a later complete supported frame can resume drawing. Verification
and malformed/structural/native admission failures retain fatal behavior.

The dedicated actual PE32 producer now builds and passes normal/verification
Wine tests, including partial-payload removal, recovery publication and fatal
structural failure. Three focused native CTests and all 66 current Debug
renderer/channel/Qt/history CTests pass. An original Quick Battle startup with
skip 0/interval 1 consumes one refusal each for reasons 7 and 8, then presents
24 native frames while original drawing/input remain active. Normal mode has
zero native pixel readbacks, zero viewport image uploads and zero remaining
surfaces. The 243 drops and 23 superseded packets preclude complete history;
the two refusal counts cover consumed packets only. Original manifests verify
before and after this experiment.

Renewed isolated comparisons retain the eight-frame/six-mode native history
match on all 3,072 pixels and the separate four-frame initial-state gap:
15 unchanged prior pixels per frame, with private-original diagnostic before
replay matching 1,920,000 live output pixels. Native initialization remains
independent; unsupported original drawing, startup ownership, complete queue
delivery, manual launcher routes, HUD/input and full replacement remain pending.
The new immutable reports and scenarios are linked in the register; older hashes
and evidence are preserved. See [scope and reproduction](native-world-shadow-refusal.md).

Committed-history review covers c0fb4bf through eadb973 (two new commits), with
exact parent/current SHA-256 receipts and affected behavior/evidence links checked,
zero missing receipts, and no retrospective execution or gate claim. The retained
report is `coverage/committed-history-world-refusal-20261007.json`. Uncommitted
refusal work and shared historical staleness receive separate exact reviews.

## World shadow recovery tests after the commit split — 2026-10-08

The concurrent split finished with commit dd3205a. Its two new exact receipts,
four source-bound execution records, three scenarios and behavior links are
preserved alongside the current increment. A further committed-history review
covers c0fb4bf..dd3205a: all three commits have matching parent/current file
SHA-256 receipts and valid implementation/test/evidence links, with no missing
receipts. This is retrospective accounting, not a past gate or execution claim;
see `coverage/committed-history-world-refusal-20261008.json`.

The split restored the older Qt test file. The 2026-10-07 producer test remained
valid, but that earlier native CTest run did not execute the dedicated refusal/
recovery/bounded-diagnostic assertions described in its broad scope. Its hashes
and result are retained with an explicit retrospective scope limitation. The
restored Qt checks now execute initial kind refusal, native recovery, wave lease
clearing, 100 repeated kind refusals, a second native recovery, fatal malformed
partial refusal and strict verification. They assert 102 consumed refusals,
64 retained records, per-reason counts, zero normal readbacks/uploads and final
surface cleanup. The actual PE32 producer and all three focused native CTests
pass again, and the complete 66-test native regression suite passes with this
test source. New immutable evidence IDs carry these current executions.

The earlier source-bound original startup recovery and isolated history/retained
initial-state comparison remain fresh: their sources did not change during this
test restoration. The unsupported paths, original startup ownership, complete
queue history, public manual scenario and full replacement boundaries remain
pending. Coverage is checked against dd3205a; historical shared-source staleness
and earlier evidence fingerprints remain visible.


## Coverage audit performance — 2026-10-08

The offline audit reuses validated paths and SHA-256 results within each call
and indexes half-open function-body ranges. Nested/fragmented overlaps retain
inventory ownership order. Each audit starts with empty caches, including
staged and Git-base checks; source edits, evidence tampering, missing files and
symlink changes are rechecked on the next call. No persistent cache or gate
exemption is introduced.

All 51 audit and 44 gate guard tests pass. Two same-tree comparisons against the
prior audit produced identical full audit and staged/HEAD gate reports. Median
local elapsed time fell from 3.65 to 0.77 seconds for the audit and from 11.72 to
6.03 seconds for the staged gate. Timings include both staged/base audits and
materialization for the gate; they are host-specific measurements, not a
whole-turn speed guarantee. The immutable report is
`coverage/audit-performance-20261008.json`. Original game artifacts were not
consumed, historical fingerprints/statuses remain unchanged, and no engine
comparison, integration or live replacement milestone is advanced.

Committed-history review covers dd3205a..e3d3c69, with exact parent/current file
and affected behavior-contract receipts and committed reference/evidence links
checked; there are no missing receipts. See
`coverage/committed-history-audit-performance-20261008.json`. This is separate
from uncommitted performance work and asserts no retrospective gate/execution
result. Concurrent uncommitted canvas-startup work is excluded from this change
and its staged census; its owner retains its accounting and validation scope.


## Permanent native rendering web focus — 2026-10-08

The local Coverage Atlas now opens on Native rendering and retains it as the
first sidebar entry (`#view=rendering`). Its refreshed rendering checklist links
World raster comparison, owned canvas history, continuous shadow presentation,
capability refusal recovery and selected direct-word bypass. The initial-canvas
producer/ownership task and authoritative DRAW-01..07 release gates remain
visible; original World drawing is retained and complete replacement is open.
Milestones derive independent stages and evidence freshness from the register;
curated next-task wording requires review as research advances.

`UI05.native-rendering` records 13 semantic/HTTP tests and 27 Chromium interaction
checks, including restored bookmarks, contract/evidence details, rendering gaps,
existing coverage views and desktop/mobile layout. Screenshots and results are
retained under `working/coverage-ui-native-rendering-20261008/browser-v4`. This
validates only the UI; historical evidence hashes and engine stages are preserved.

Committed history e3d3c69..8e43ed5 was checked against exact parent/current
receipts with no unresolved file transitions; see
`coverage/committed-history-native-render-ui-20261008.json`. The reviewed census
updates only this UI's sources, retaining the prior reviewed entries elsewhere.
Concurrent uncommitted canvas-startup work remains outside this increment's
review and validation; its census drift and historical staleness remain visible.


## Canvas startup before first World draw — 2026-10-08

The opt-in startup observer now traces selected create/release/lock/bind/clip,
fill/copy/JPEG and sprite paths before the first World queue. A pinned original
800×600 Quick Battle map-2 capture records 8,407 events: wrapper `0x6a49e0` is
created once, receives one initial zero-fill request, is never released during
the observed prefix, and returns the same pixel pointer on 686 locks. Menu/loading
work reuses it before World. The final copy, lock, bind and first queue entry share
the same fingerprint with 365,306 nonzero pixels. There is no selected clear
between that startup initialization and first World. The initial fill lacks a
prior lock sample, so pixel completion is not asserted.

`RS.canvas-create-lock`, `RS.canvas-bind-clip`, `RS.canvas-startup-clear` and
`RS.canvas-preworld-reuse` separate these recovered partial boundaries. The
existing initial-state behavior keeps its historical comparison and native
history milestones; this observation advances no equivalence or replacement
claim. The alternate nonzero `0x6e1f68` path, physical allocation generations,
complete upstream per-pixel producers, indirect writers, lost-surface/failure
branches, alternate modes/movies and later transitions remain pending. Native
rendering receives no original initial pixels and retains its explicit reset policy.

Evidence and reproduction are in [canvas startup](native-world-canvas-startup.md)
and [diagnostic format](../formats/canvas-startup-diagnostics.md), with immutable
static/live/synthetic reports `native-world-canvas-startup*-20261008.json`. The
actual PE32 forwarding fixture checks all twelve entries, nested JPEG/lock call
relocation, registers/EFLAGS/x87/XMM/LastError, all-entry byte mismatch refusal,
post-boundary forwarding, and eleven malformed diagnostic refusals. Original
manifest verification passed before/after captures and disassembly exports.
Shared observer/build/capture source edits leave older evidence fingerprints
historical; unrelated subsystem comparisons are not rerun or refreshed.


## Opt-in coverage UI IPv4 binding — 2026-10-08

`serve-coverage-ui.py --host 0.0.0.0 --no-open` now binds all IPv4
interfaces; the default remains127.0.0.1. Numeric IPv4 URL hosts are accepted
for wildcard binding, while specific binding admits its selected address and
loopback aliases. Refresh rejects nonmatching origins; arbitrary DNS authorities,
wrong ports and malformed hosts remain rejected. Registered read-only source
and research links have no authentication and become reachable with LAN binding.

`UI06.lan-binding` records15 semantic/HTTP tests, including a synthetic wildcard
listener and LAN headers, and27 actual Chromium checks on a loopback-only server.
The live wildcard smoke test carrying repository payload was denied by automatic
approval review pending explicit LAN exposure approval; physical remote-client
reachability remains untested. Earlier UI05 fingerprints are retained as history.
No engine status, comparison or replacement stage is advanced. No commits landed
after8e43ed5; the preceding committed-history review remains applicable.

The existing disabled-trace scene snapshot suite also completed successfully
(three CTests plus PE32 forwarding and native fixture/admission checks), retained
as `native-world-canvas-startup-default-regressions-20261008.json`. Subsequent
concurrent resource edits leave its affected source fingerprints historical.
The startup observation itself has a separate exact scope/scenario binding and
captured build provenance in `native-world-canvas-startup-bound-20261008.json`;
this binds the retained run and does not rerun or refresh old evidence.

Committed-history review covers dd3205a..8e43ed5: 37 exact parent/current file
transitions have preserved matching receipts, with no missing transitions.
See `coverage/committed-history-canvas-startup-20261008.json`; this does not assert
a retrospective gate pass or new execution. The source census acknowledges the
canvas observer/decoder/forwarding files and explicit dependency links. Concurrent
UI, asynchronous resource and coverage-tooling edits keep their separate owners,
receipts and validation boundaries; indexing their current paths alone does not
validate their changed engine behavior.

### Every bounded startup queue and actual unsupported kinds (2026-10-08)

`RS.world-startup-queues` records raw queue rows before sampling/admission.
Canvas startup and finite refusal captures automatically journal a prefix and
require zero skipped queues and interval one; `--startup-queues` extends the
bounded prefix up to 256 entries. Empty/invalid/unselected entries retain their
own diagnostics. Allocation or output omissions fail prefix validation.

The new original run retains all first 16 queues and all 21,888 rows. Actual
observer-unsupported kinds are 20 in queues 3/4, 17 in 5/6 and 16 in 7/8,
one row per queue. Queue 1 separately refuses wave state (reason 7); kinds are
reported independently of raster refusal reasons. All raw rows and exact
queue/sample correlations are retained. The synthetic PE32 run validates
eight entries, empty/unreadable/over-capacity/unknown-view inputs, all kinds
0..40 and signed extremes, capture beyond the scene budget, exact cap,
EAX/LastError forwarding and 13 malformed/missing-prefix refusals.

See `native-world-startup-queues.md`, immutable live/test records dated20261008,
and `formats/startup-queue-diagnostics.md`. This promotes only the new scoped
live observation. Original drawing remains active; numeric kinds16/17/20 do not
establish their raster semantics. Full-frame equivalence/replacement and
concurrent/reentrant producers remain pending. Prior evidence keeps its source
hashes; shared observer/build/capture dependency changes leave those results
historical. Concurrent resource/UI work retains separate accounting.

### Worker sprite planes and GPU admission (2026-10-08)

Native World now requests one worker-owned colour/coverage or projected-shadow
variant at each ordered GPU cache miss. GUI uploads resume through full-width
rows with 256 KiB and 32-draw limits plus a cooperative 2 ms elapsed check,
retaining the previous complete GPU lease. Shape keys cache projected shadows.
The additional immutable worker decoded store is bounded at 64 resources/128 MiB;
it does not reserve total RSS. Background creation, preflight, individual driver
calls and long-session CPU retirement remain pending. See
[native upload preparation](native-world-upload-preparation.md). Synthetic
execution, original comparison and live replacement remain separate milestones.

The [new synthetic report](native-world-upload-preparation-20261008.json) records
69 passing CTests: 1 MiB in 128 ticks at 8192 bytes/tick, independently expected
pixels, cache-pressure order and warm shape/value reuse. Held worker preparation
retained the prior GPU frame and delivered 46 heartbeat callbacks/50 ms; close
completed in 3 ms with zero remaining surfaces. This validates the synthetic
native policy only. Historical live/original evidence remains unchanged and does
not establish current-code equivalence or installed-session latency.

The [final progress execution](native-world-upload-preparation-progress-20261008.json)
again passes all 69 tests after ensuring each tick attempts one step even when
context setup consumes the time allowance. Tiny 1 microsecond budgets cannot
strand ready drawing or pending row uploads. Final heartbeat remains 46/50 ms;
close takes 2 ms. The preceding upload result is retained as historical evidence.


### Every completed native startup World canvas (2026-10-08)

`RS.world-startup-wave` recovers selected kinds 16/17/20 and observes the original
lazy displacement initializer after its first primitive returns.
`RS.world-startup-palette` corrects the clipped fallback's actual shade-minus127
palette: only explicit black producers synthesize zero. The first negative
comparison remains immutable. Effective-mode unsupported kinds and default-policy
refusals are now reported separately; lifetime diagnostics stop before hashing.

`NR.world-startup-sequence` reproduces every queue in a bounded 16-queue chain
using native word-zero creation/reset, binding/clip and retained completed native
pixels. All 16 completed 800×600 canvases match actual original completions and the
independent original same-zero-state chain: 7,680,000 pixels, zero mismatches. Every
private original replay initialized from diagnostic before-state also matches.
Active wave return ABI, synthetic shade-minus127 palette capture, World/history
regressions and default/startup-mode queue diagnostics pass. No original destination
pixels are native inputs. See `native-startup-sequence.md` and immutable reports.

Original loading/menu and intervening HUD producers remain unreproduced. Actual
entry states differ by 365,338 pixels initially and 27,473..27,537 between queues;
the selected World completions overwrite these differences. Original rendering
stays active; native integration is headless replay, not live history presentation,
whole-startup equivalence or bypass. Separate policies and historical statuses
remain independent; shared-source older evidence remains stale.

## Closed canvas producer replay (2026-10-08)

`RS.canvas-producer-sequence` and `NR.canvas-producer-replay` now have a finite
original/Qt comparison path from800x600 Quick Battle menus and loading through
all16 contiguous startup consumer returns. The49 signature-checked hooks
record create/release/bind/clip/fill/copy, encoded JPEG/BMP/PCX, tinted fonts,
selected raster/blend/wave/shadow requests, panels, packed-word transition fade,
minimap source-cell colors/visibility and source-derived marker/outline points.
The live run matched all1,062 sampled completions /474,236,912 RGB565 pixels
with zero differences. Raw queues retain actual unsupported kinds20 in3–4,
17 in5–6 and16 in7–8, including their ordinals/coordinates. No queue is skipped.
Evidence: `native-canvas-producers-live-20261008.json`, independent source-only
replay `native-canvas-producers-comparison-20261008.json` and forwarding
`native-canvas-producers-forwarding-20261008.json`; see
[native-canvas-producers.md](native-canvas-producers.md).

Native producer storage begins undefined and retains logical generations;
original completion bytes remain separate oracle files. The experimental Qt
consumer composes in native CPU storage, mirrors completed canvases to retained
GPU surfaces and checks exact readbacks before presentation. Original rendering
stays active; this validates bounded replay/observation, not live replacement
or an all-operations GPU compositor. The independent Worldv2 zero-history
renderer retains its own seed/integration boundary. Native generation of
minimap source geometry/visibility, front-buffer/driver behavior, unsampled
outputs, allocation/restore/error paths, format changes, stretched copies,
minimap rotations and unobserved producers remain separate gaps. Historical
negative results and their source fingerprints are preserved. Three copy-entry
exceptions cover only signature-pinned16-byte entries executed with private COM
adapters; inferred whole-function coverage and original inventory unknowns remain.

## Reconstructed native producer history to World and bounded bypass (2026-10-08)

`NR.world-producer-handoff` now hands fully defined native startup/HUD generations
to the existing World GPU SceneRenderer at each contiguous queue entry and
commits checked GPU World completions back to native producer storage. It preserves
bindings/clips and intervening native producer writes; no zero or original
destination seed is used. Initial original-active execution matches all16 World
returns and1,062 observed canvas checkpoints /474,236,912 RGB565 pixels exactly.

`RS.world-producer-bypass` records a separate scoped live replacement: eight outer
indexed-copy59521a bodies in queue16 are skipped and correlated native GPU words
are written to the already locked original canvas. Every reply matches both
independent original prefix execution and the exact suppressed original entry
from its reconstructed native before-state /3,840,000 pixels. The frozen run
also matches16 independent original World completions /7,680,000 pixels and
1,064 mixed destination observations /474,262,832 pixels. Mixed observations
alone cannot establish bypass equivalence. Synthetic forwarding/state/refusal
and native handoff/HUD-retention tests pass. See
[native-world-producer-handoff.md](native-world-producer-handoff.md) and its
new immutable live, comparison, forwarding, bypass and negative evidence.

Native policy and recovered bypass retain independent implementation/comparison/
integration/replacement statuses. Only the eight selected59521a bodies have a
live replacement claim;5947b2 is synthetic-only. Full consumer bypass, other
producer takeover, driver/error paths, native minimap geometry/visibility and
native simulation remain gaps. The default independent Worldv2 viewer keeps
its zero-history policy. Prior evidence/source hashes and discovery unknowns
remain unchanged; historical shared-source validation can be stale. HEAD remains
63f32b1 after the previously retained8e43ed5..63f32b1 exact committed-history review.


The GPU dependency refresh pins119 actual compiler/build/runtime/validation files,
including the shared sprite atlas. New evidence separately revalidates the prior
eight bypass requests offline and a current one-draw live replacement. Current
live and frozen replay both match1,062 completion observations /474,236,912 pixels;
all16 independent original World completions match, and the selected59521a reply
matches both exact original entry and prefix over480,000 pixels. No original
pixels seed native history; cleanup leaves no native surfaces. The verified live
binary is hash-bound to its frozen build. Earlier source/evidence fingerprints
remain historical. Two incomplete eight-limit captures and an initial static
index provenance audit refusal are retained separately. See the current GPU
refresh evidence in [native-world-producer-handoff.md](native-world-producer-handoff.md).
The repository-wide gate may still expose shared historical scope/freshness debt;
this narrow replacement does not promote those other contracts.


### Live startup kind-refusal diagnostics (2026-10-08)

`NR.world-kind-diagnostic` connects the bounded raw queue journal to the live
startup launcher and stopped failure attribution. The original channel/native
admission stay strict; absent or inconsistent raw rows never acquire a guessed
kind. The earlier manual queue-13 failure is still unassigned. Saved map/item
settings differ from the automated fixture; map1/no-items normal live16 and
map1/item15 finite16 both complete without unsupported kinds. A frozen pointed
map1/item15 256-queue capture records kind8 beginning queue133;
`RS.world-kind8-virtual` separately records the pinned first-table object virtual
call, with concrete class/method and full mode semantics pending.
`RS.world-dispatch-targets` inventories numeric targets without expanding raster
admission. See [kind diagnostics](native-world-kind-diagnostics.md). GPU atlas
and palette reuse remains the next performance work; the current 32-frame/64-
surface cache bound prevents solving churn by increasing frame retention alone.
The staged-source symlink failure was restored exactly and is covered by new
guards; original media is unchanged. Historical fingerprints remain intact.

### 2026-10-08: bounded sprite atlas and independent shader palette reuse

Native World now enables NR.sprite-atlas: bounded packed index/RGB565/coverage
pages, owned palette values independent of indexed frame keys, shape-aware shadows,
row transfer admission and page LRU/revision/lifecycle cleanup. Existing per-frame
clients remain available for comparison; no original draw-kind admission changes.
The prospective source-bound final run-t7x04nmz passed all 70 native CTests, including
GUI heartbeat/cancellation and large/invalid transfer admission. Two passes of the
same 16 captured queues produced identical native pixels: 43,988 expanded uploads
per pass versus 500 atlas cold and zero warm, one two-plane page/250 entries and
zero normal native readbacks. Debug llvmpipe frame submission averaged 528.4/526.6 ms
expanded cold/warm versus 238.5/205.7 ms atlas. Explicit comparison readbacks remain
test-only. Originals verified 2,927 files before/after. Native storage policy is
scoped/headless; original comparison and replacement remain none.

See [atlas policy and boundaries](native-sprite-atlas.md) and immutable
[native execution record](native-sprite-atlas-validation-20261008.json). Historical
renderer/preparation/handoff evidence retains old hashes and is source-stale where
shared code changed. User live startup timing, initial-canvas equivalence, unsupported
kind 8, context loss, page fragmentation and destination snapshot reuse remain
separate. No source census refresh or native-path equality claims original equivalence.
HEAD 63f32b1 is unchanged since the retained committed-history review; no new commit
range, staging or commit occurred in this work.

### Selected kind8 particle primitives (2026-10-08)

- Native World now conditionally admits six checked kind8vtable/method pairs:
 5c7420/54ab00,5c73f0and5c7408/546540,5c7494and5c74b0/54ab60,
 5c74cc/54ac00. RGB565first dispatch route and inactive auxiliary markers
 only; unknown classes, alternate formats/routes retain complete refusal.
 Original object/gating/flag execution remains active.
- Pointer-free operation6captures saturating additive rectangles; operation7
 captures bounded64x64uniform/plane RGB565colour rectangles and four selected
 composition modes. The native worker and scene carry them in sprite order
 without asset binding. Colour planes have bounded transient uploads; their
 bytes are outside SPRupload counters. Additive draws freeze the owned GPU
 destination without normal CPUreadback. This is rendering, not simulation.
- Fresh prospective comparison/headless evidence: [validation](native-world-kind8-validation-final-20261008.json),
 17synthetic cases1048960pixels plus one1497-request/480000pixel captured
 queue. All native pixels match independent original execution from the same
 independently constructed base; owner/player and flag-only methods and both
 active primitive hook ABIs pass. The separate original-before diagnostic
 replay exactly reproduces live after; native zero differs from that live
 canvas. Startup/HUD/retained initialization parity remains separate.
- [Frozen discovery](native-world-kind8-discovery-20261008.json) preserves
 57298read-only rows across236queues and five class pairs, original manifest
 verification and frozen source/claim provenance. The earlier432-row
 5c7420capture remains historical underworking/tests/kind8/run-le6ejqbj.
 Recovered wrapper range exceptions retain missing discovery-body boundaries;
 six vtables map only draw slot3and retain all other slots unknown.
- RS.world-kind8-additive/colour/noop now have scoped implementation, recorded
 isolated original comparison and headless integration. Current-code live
 native GUI class coverage, other virtual classes, RGB555, other dispatch
 routes, object writers/state and original replacement remain pending.
 Earlier atlas/world/producer evidence keeps its hashes and may be source-stale
 after shared renderer, protocol and observer edits.

- Final class/raster evidence uses the actual native library and active raster
 ABI dependency closure, with unrelated producer objects compiled only as
 smoke checks. Earlier broader source-bound results remain historical.
 Read-only PE32diagnostics pass in a frozen snapshot (15malformed refusals,
 unreadable object/vtable, truncation and original forwarding).
 [Current native regressions](native-sprite-atlas-kind8-regressions-20261008.json)
 retain all70CTestpasses and renew atlas headless validation after the shared
 shader change; earlier measured captured timings/uploads are not rerun claims.


## Complete World raster queue replacement (2026-10-08)

`RS.world-raster-queue-replacement` separates complete raster-body suppression
from original World traversal. Caller fallback/store semantics consume low16AX,
so the adapter recovers that return rather than preserving incomingEAX. Black
and destination-displacement entries return1 on horizontal refusal; sampled
indexed-copy entries clip internally and return0. Thirteen entries have explicit
callee cleanup, caller signatures and native V2 source/reply admission. Unknown
inside-World routes remain refusals.

One complete live queue16 replaces1521actual original raster bodies. Every
native intermediate andAX matches its precise original entry over730080000pixels;
all16independent original World returns and1062observed canvas checkpoints also
match. These are separate from observations containing native contributions.
Native inputs remain reconstructed startup/HUD history and owned source data.

The first whole-prefix attempt refused a backend8HUD raster after World queue1:
selection incorrectly persisted beyond World return. Actual entry/return lifetime
now bounds suppression, and the following original HUD raster is covered by a
body-counter regression. All13synthetic body skips,49forwards and strict
identity/checksum refusals pass. The current frozen125-source preflight matches
1817canonical queue16 intermediates/AX,1062checkpoints and16World returns.
The current complete prefix now passes: all 16 queues replace 36,556 raster
bodies. Independent actual-entry execution matches every native canvas and AX
result over 17,546,880,000 pixels; all 16 final World canvases, 1,062 checkpoints
and the full before/reply continuity chain also match. Four focused native tests
pass, all 125 source fingerprints remain stable, and the original manifests
verify 2,927 unchanged files before/after. See the immutable
[prefix replacement](native-world-raster-prefix16-boundary-replacement-20261008.json)
and [exact comparison](native-world-raster-prefix16-boundary-comparison-20261008.json).

Original traversal, simulation, menu/HUD raster work outside selected queues,
palette preparation and non-drawing lazy wave initialization remain active.
No full dispatcher, general driver/error-path, all-format or real-time claim is
made. Historical evidence retains its hashes; shared source changes leave older
contracts pending/stale without their own domain-specific reruns. See
[native-world-raster-queue.md](native-world-raster-queue.md) and its immutable
caller, ABI, preflight, queue16 comparison/replacement and refusal evidence.

### Native rendering commit readiness, 2026-10-08

The first accumulated rendering commit retains previous results and stage values in
`native-rendering-commit-readiness-20261008.json` and each affected new behavior's
`historical_status`. Twelve newly registered startup/producer behaviors have no
current source/scope-bound execution support after shared-file and contract edits.
Their present implementation is partial; current comparison, integration and
replacement remain pending. This does not change any already committed historical
stage or immutable evidence, nor assert that earlier comparisons failed. Fresh
preparation, atlas, refusal diagnostics and selected kind8 results are registered
separately. Whole contiguous World replacement remains a separate milestone: the
first prefix attempt completed queue1 but refused a HUD draw after World return.
The boundary fix requires a fresh complete-prefix run and independent comparison.

Commit verification adds immutable prospective results
`native-world-preparation-commit-20261008.json` (4 focused tests),
`native-world-kind-diagnostic-commit-20261008.json` (16 diagnostic fixtures),
and `native-sprite-atlas-commit-regressions-20261008.json` (70 native CTests).
Selected kind8 original/native final proof remains current; no new pixel or
performance claim is inferred from the general regression run. Coverage guard
suites pass 60 audit tests and 51 gate tests. The latest committed range
`8e43ed5..63f32b1` has exact existing file/behavior receipts, retained in
`coverage/committed-history-native-rendering-commit-20261008.json`.

### Complete-prefix evidence and remaining scope (2026-10-08)

The successful current complete-prefix result supersedes the pending retry noted
in the earlier commit-readiness snapshot. `RS.world-raster-queue-replacement`
now has scoped implementation, recorded original comparison, live-equivalence
integration and scoped live replacement. Understanding remains partial. Original
traversal/simulation, menu/HUD bodies outside World, palette preparation and one
non-drawing lazy wave initialization remain active. Three admitted entries were
unobserved in live suppression; vertically top-clipped displacement table phase,
other formats/callers/errors and volatile/scratch-global equality remain unverified.
No whole-dispatcher, native simulation or real-time performance claim is made.

The newly landed `63f32b1..e94f344` range was reviewed before final accounting.
The [retained committed-history report](coverage/committed-history-world-raster-prefix-20261008.json)
finds exact receipt chains for 436 source/link and 138 behavior transitions,
preserves prior receipt objects and lists no unresolved transitions. Historical
accounting is separate from this new uncommitted execution evidence and does not
assert a retrospective gate pass. Historical evidence retains its original
fingerprints and separate current-readiness limitations.


### Complete startup raster prefix repeated, 2026-10-08

An additional source/scope-bound run validates 33,920 admitted outer raster
replacements across all 16 contiguous startup World queues. Every native
intermediate canvas and low16AX matches its actual original entry over
16,281,600,000 pixels, with continuous native before/reply state. All 16 final
World canvases and 1,062 observed checkpoints also match. No original destination
pixels seed native work. Actual and frozen builds declare all 125 reviewed
sources and 76 normalized compiler inputs; four focused CTests pass, sources and
captured inputs remain stable, and 2,927 original files verify before/after.

This repeat supports the same bounded `RS.world-raster-queue-replacement`
claim. The separate 36,556-call run, historical stage values, wider pending
startup/HUD contracts and existing clipping/caller/global limitations remain
unchanged. Traversal, simulation, menu/HUD outside World, palette setup and
non-drawing lazy wave preparation still run originally. Three admitted entries
remain unsampled by these live runs. Queue-boundary batching is the next
performance step. See [the repeat replacement evidence](native-world-raster-prefix16-replacement-20261008.json)
and [the updated contract](native-world-raster-queue.md).

### Manual startup shadow gap, 2026-10-08

The public `run-native-world.py --startup-history --verify` launcher still selects
the World-only retained preview. In manual run `run-l5v_hzru`, queue1matches and
queue2refuses8pixels. Independent original replay of both retained packets from
zero/its prior completion exactly reproduces both native hashes; queue2still
differs from live original at the same8pixels. The writer/timing or omitted
context is not identified. The two-packet diagnostic does not renew full-prefix
or public-launcher parity, and existing statuses remain unchanged. Original
manifests pass2,927files before/after. See the [manual diagnostic](native-world-manual-history-gap-20261008.json)
and [integration boundary](native-world-live-history.md). This is separate from
the successful producer-history/raster replacement proofs.

Public native startup launcher integration: `--startup-history [--verify]` now
selects the bounded automatic original Quick Battle map2/zero-items menu route
and retained source-only CPU/GPU producer/World handoff. Every completed canvas
is compared, including menu/loading/HUD producers; all first16raw startup queues
and actual unsupported World-observer draw kinds are retained. Ordinary
continuous launching remains interactive. `NR.world-startup-launcher` is a
native launcher policy, separate from raster replacement and World-only zero
history. Manual producer-menu input remains pending: `run-ekeoh6y5` hit the1GiB
oracle budget before World entry and had duplicate/noncontiguous sequences; its
immutable diagnostic preserves the failure without thread attribution or
original-equivalence claims. The former manual World-only eight-pixel discrepancy
also remains preserved. A narrow RGB565 wire-container conversion fixes the
concurrently developed batch branch's build; this launcher does not enable it.

Current public-launcher validation: `NR.world-startup-launcher.verified-20261008`
records1062original-completion comparisons/474236912pixels, all16startup queues,
176unchanged prospective sources and74declared compiler dependencies. Every
comparison matches, CPU/GPU checks pass, no original destination pixels seed
native state, no original work is bypassed, and2927original files verify
before/after. The earlier normal result remains historical after shared hook/test
edits; fingerprints are preserved. Four full-register audit errors concern the
other task's planned batch ABI/startup scenarios without evidence. Owned launcher
accounting is also retained separately with those exact omissions disclosed;
no whole-repository gate pass is asserted while that work is incomplete.

Public launcher portability correction: the user failure `run-scltemki`
closed16World returns with938matching native/original completion comparisons
but failed only on the diagnostic ImageMagick root screenshot.
`run-_g_wjtth` exhausted the1GiB diagnostic oracle budget before World entry.
The public startup route now skips desktop screenshots and explicitly requests
3072MiB of original-oracle storage; ordinary research captures retain1024MiB.
Record/stream/oracle-count limits,180-second worker deadline, contiguous queues,
complete output identity/hash/GPU checks and exact every-canvas comparisons stay
enforced. This is native diagnostic policy, not recovered original-engine
budget behavior. The49-entry actual PE32 synthetic fixture passes, including
old-ceiling crossing, exact configured-ceiling acceptance, three budget refusals,
underflow protection and four invalid-environment refusals before hook mutation.
No original equivalence is claimed by those synthetic tests. The first broken-
screenshot live regression matched1062canvases but correctly refused after a
concurrent consumer edit; its scope/source stability was not promoted.
Historical evidence fingerprints remain intact; unrelated batch startup
validation remains with its existing pending scenario.

Fresh portability regression `NR.world-startup-launcher.portability-20261008`
completes16World queues and1062matching canvases/474236912pixels
through the public verified command with a deliberately failing ImageMagick
`import` executable first on PATH. That utility is never called. All176
prospective sources stay stable and all74normalized compiler dependencies
are declared;2927original files verify before/after. Exact raw startup rows
and all completion comparisons remain required. The synthetic budget fixture
proves the strict ceilings independently; this live run does not exhaust3GiB
or assert broader menu-route/whole-engine equivalence. Historical source-only
results retain their original scope and hashes.

Final launcher portability audit: `audit-portability-current.json` records zero
full-register audit errors after the separate batch owner supplied its pending
pilot metadata. Earlier failed audit reports remain historical. This launcher
comparison does not validate that separate batch replacement. Exact source
census and review receipts accompany the launcher changes; no staged files
or original artifacts are altered.


### Guarded World raster queue batching, V7 (2026-10-08)

`RS.world-raster-batch-return` and `NR.world-raster-batch-transport` are separate
recovered-boundary and intentional-native-policy contracts. A mandatory original
destination access guard begins after entry observers finish and before original
traversal; unexpected reads/writes/source aliases/producers/callers refuse.
Producer records use a private heap so observer allocations cannot touch
protected original allocation metadata. Prescribed low16AX/callee cleanup is
immediate, while owned raster sources queue for one GPU completion. The return
verifies/restores protection, validates one correlated native canvas and writes
it before the checkpoint/HUD boundary. V1/V2 remain independent diagnostic modes.

An independent queue1 pilot matches1,839actual original raster canvases/AX. The
current complete prefix then matches all23,480admitted calls/AX over
11,270,400,000pixels, all16native CPU/GPU finals/live replies and1,062observed
checkpoints. No original destination pixels seed native rendering. There are
exactly16World GPU readbacks/writebacks/queue handshakes and zero per-raster
handshakes; other diagnostic checkpoint readbacks remain. Reply pixel traffic
is15,360,000bytes instead of22,540,800,000bytes at per-raster frequency.
The diagnostic live capture's39.27seconds includes startup, and does not establish
real-time frame latency. All133declared sources/captured inputs are stable;
80compiler inputs are covered. Five focused CTests and PE32 synthetic
13-skip/49-forward/state/guard/malformed-reply fixtures pass. Original manifests
verify2,927unchanged files before/after both live capture and comparison.

Understanding remains partial. The recovered return contract has scoped
implementation, recorded original comparison, live-equivalence integration and
scoped live replacement. The intentional transport has scoped implementation
and live-equivalence integration; it claims no recovered original batching
mechanism or original-policy replacement. Ten of13admitted entries are observed; three others
remain synthetic-only. Original traversal/simulation, outside-World menu/HUD and
non-drawing lazy wave preparation remain active. Other formats/callers,
top-clipped displacement, volatile/scratch-global equality, general gameplay,
public-launcher batch integration and real-time performance stay open. Earlier
scalar/startup/HUD/launcher evidence and committed historical stages retain
their hashes and domain-specific freshness limitations; this proof renews only
the two prospectively declared batching contracts.

The initial live guard ordering and later dependency-path/RGB565 proof-writer
refusals are preserved as failures, rather than as positive original evidence.
See [batch boundary and reproducible commands](native-world-raster-batch.md),
[the exact comparison](native-world-raster-batch-prefix16-comparison-v7-20261008.json)
and [combined replacement](native-world-raster-batch-prefix16-replacement-v7-20261008.json).


A separate fresh public startup launcher regression uses its own prospective
`NR.world-startup-launcher` declaration after these shared-source edits.
`--startup-history --verify` still completes16World queues and1062producer
checkpoints with zero original/native mismatches; all176sources remain stable.
A deliberately failing screenshot utility is never invoked. This is the
existing automatic shadow/history route with original drawing retained, not
batch selection or manual interactive validation. See
[the regression](native-world-raster-batch-launcher-regression-v7-20261008.json).

The concurrently landed implementation/launcher commit `b4d5007..e25290c` was
reviewed before completing this coverage update. The
[committed-history report](coverage/committed-history-world-raster-batch-v7-20261008.json)
finds exact existing receipt chains for219source/link and126behavior transitions
across66committed files, preserves prior journal entries and lists no unresolved
receipt/immutable-history gaps. This accounting does not assert a past gate
pass; current full-prefix batching and public-launcher regression are separate
new execution evidence.


## Independent four-orientation minimap terrain — 2026-10-09

`RS.minimap-terrain-raster` adds the standalone Qt-independent
`renderer/minimap` owned RGB565 pixel writer. Four orientation branches, wrapped
in-grid centres, source colours and retained auxiliary fog pixels match the
actual pinned original553a40 routine across1,360cases/35,486,800destination
words, including untouched pixels and positive padded rows. Normal and
ASan/UBSan output is identical;15native atomic refusals pass. Original private
fixtures check object+ff=-2 as its only object mutation, one auxiliary unlock,
and unchanged source descriptors/auxiliary storage. All declared sources stay
stable and2,927original files verify before/after.

Understanding/implementation/original comparison are scoped to this pixel
contract; integration/replacement remain none. The native service does not
implement original Lock/Unlock or object invalidation, and the live producer
wire still admits orientation0. Odd widths, unnormalized centres, negative
pitch, partial clipping, actual driver/error paths, marker/camera-outline
generation, changing HUD/tooltip inputs and drawing suppression remain pending.
No launcher or batching implementation changes are part of this increment.
See [contract and reproduction](native-minimap-terrain.md) and
[immutable comparison](native-minimap-terrain-comparison-20261009.json).

The reviewed committed-history range remains `b4d5007..e25290c`, with no new
commits since the retained batch-v7 report. Independent interactive batch and
profiling workspace changes are outside this minimap execution declaration;
their uncompleted coverage metadata is reported separately by the whole-tree
audit/gate. Census refresh records current links only, not validation of that
concurrent work. Historical evidence fingerprints are preserved.


### 2026-10-09 — bounded interactive batch launcher and consumer timings

The opt-in public `--world-batch` route now stages source-only menu/HUD producer
history from process startup, leaves menus and battle input in the original
window, replaces only the first sixteen guarded World queues, and ends the
isolated session at that bound. `--startup-history` selects the automatic map2,
zero-items route separately. Window closure before completion is cancellation,
not successful validation. Default World-channel shadow and finite startup shadow
routes remain available. Full manual menu journeys, original-window cancellation
and unbounded gameplay remain pending boundaries.

Two prospective-source-bound automatic runs completed sixteen batches and all
checkpoint comparisons. Debug/software-OpenGL measurements show warm resource
preparation medians276–349ms, host GPU submission260–298ms, CPU reference
composition140–175ms and World readback1.27–1.30ms. Session checkpoint diagnostics
cost11.08–11.47seconds across menus/loading/startup. These are native consumer
measurements, not original simulation or hardware-general frame latency.
See [interactive batch scope and measurements](native-world-interactive-batch.md).
Independent exact-original intermediate/AX revalidation and real native-window
cancellation are separate evidence steps; old batch/startup evidence is retained
with its recorded hashes and does not become current merely through these edits.


Fresh instrumented source-only comparison matched29,323 actual admitted raster
intermediates/AX returns,16 final World canvases and1,062 checkpoints, with five
native checks passing. `RS.world-raster-batch.instrumented-replacement-20261009`
preserves the133-source batch contract closure and80 compiled dependencies. Raw
execution retains the larger180-source launcher closure; final launcher-only
refusal/cancellation changes require their own fresh public-route regressions.


Final public automatic completion/profiling and real native-window cancellation
passed after preserving original producer refusal/guard diagnostics across window
closure. `NR.world-interactive-batch.complete-20261009` and
`NR.world-interactive-batch.cancel-20261009` provide fresh180-source launcher
policy evidence; the profiling policy links only the automatic completion result.
Three synthetic launcher tests and the retained startup-shadow synthetic checks
pass. Original-window cancellation and a completed manual menu journey remain
explicit gaps; no continuous gameplay replacement or hardware-general frame-rate
milestone is asserted.

## Native World resource and submission reuse (2026-10-09)

`NR.world-resource-reuse` separates this intentional native optimization from the
recovered raster contracts. Chosen decoded visuals are verified once per resident
resource revision; normalized input identity and palette ambiguity remain checked
on every resolution. Reload/adoption rechecks and failed verification remains
uncached. Guarded World rendering uses the existing bounded indexed atlas and one
outer synchronous GL batch, retaining ordered draws, independent CPU composition,
GPU equality checks and canvas guards. Synthetic invalidation/ambiguity/context
checks, frozen prechange/current owned-source replay, public first16 guarded live
completion and independent original intermediate/AX comparison are separate
validation steps. Older shared-source results retain their historical hashes;
current-code support requires the new executions. No unbounded gameplay, original
simulation replacement, shader formula change or hardware-general latency claim.

### Native minimap overlays: bounded original comparison (2026-10-09)

Owned Qt-independent cell/creature markers and selected camera-corner outlines
now have separate behavior IDs (`RS.minimap-cell-marker`,
`RS.minimap-creature-markers`, `RS.minimap-camera-corners`) and source/scope-bound
isolated-original evidence. [Overlay contract and evidence](native-minimap-overlays.md)
records all four rotations and RGB565/RGB555: 4,752 cases and 53,312,088 complete
destination words match normal and sanitized native executions, including cell
return offsets, creature dimension refresh and camera origin changes. Flash
rings, fog gates, overlapping lists and viewport division boundaries are covered;
16 atomic refusals pass and immutable original files verify before/after.

This is an offline foundation. No live integration or original drawing bypass
is claimed. Camera outer borders, subsequent nonempty marker-list composition,
source camera directions/visibility, driver locks and general caller machine
state remain pending. Terrain validation remains separate and its historical
source fingerprints are preserved. Next integration must transport owned inputs
and apply the observed object/result effects before considering replacement.

The frozen resource-reuse replay now matches all1,062 checkpoints with five
native checks passing. Warm preparation/submission fell310.87→137.93ms and
268.58→67.88ms respectively in the same-input Debug/softwareGL comparison.
`NR.world-resource-reuse.replay-20261009` retains the252-upload/zero-eviction
reuse evidence. A separate public capture completed16 guarded writebacks and
1,058 checkpoints; `NR.world-resource-reuse.live-20261009` supports intentional
native integration, while exact-original intermediate/AX revalidation remains
a separate evidence step. The prechange source patch reconstructs all133
baseline source hashes in an isolated copy; historical artifacts remain intact.

`RS.world-raster-batch.resource-reuse-replacement-20261009` now matches23,843
precise original raster intermediates and low16AX returns,16 final GPU/live
World canvases and1,058 checkpoints with five native tests and80 declared compiled
dependencies. Existing recovered/native batch scopes have fresh current-code
support; `NR.world-resource-reuse` retains original comparison/replacement `none`
and separate live-observation support. Historical evidence and the explicit
manual/all-map/unbounded boundaries remain intact. The newly landed minimap commit
was reviewed separately over `e25290c..8dce3ac`; all24 committed file transitions
have exact receipt chains. Its accounting review does not assert new execution.
