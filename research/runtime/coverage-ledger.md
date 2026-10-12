# Engine modernization coverage ledger

## Other pixel producers: byte text and live glyph shadow — 2026-10-10

`RS.font-byte-consumer` now has isolated-original proof for contour-based byte
spacing, punctuation modes, signed wrapping arithmetic, cursor/profile updates
and admitted single-byte draw chains:3776 cases,64793 operations and59424768
RGB565 WORDs match unchanged NoCD code. `NR.font-byte-producer` owns decoded
SFT data and stages state until atomic raster admission; six refusals and
ASan/UBSan pass. [Scope and reproduction](font-byte-producer.md). Higher string
layout, reset/lifecycle, reader/palette initialization and original failure/ABI
behavior remain outside this increment.

`NR.font-glyph-live-shadow` supplies fresh current-source evidence through the
existing original-active producer pipeline:1935 glyph calls,941 completed
canvases/416635312 pixels, four startup World returns, no differences or producer
failures, and all GPU mirrors equal. [Live scope and reproduction](font-glyph-live-shadow.md).
Native destinations come from owned source history; original completed pixels
are external comparison oracles. The new byte-state adapter remains headless,
and original text drawing stays active. Per-glyph live comparison and text
takeover are separate pending milestones. Broader historical canvas/World
contracts retain their existing statuses and hashes. Committed1fc0b3e..0191a55
review covers15 exact file transitions with zero missing receipts; this records
history accounting without asserting new execution. The concurrent clipped-word
commit0191a55..d6e32e9 was also checked:26 exact transitions, zero missing
receipts. Its own validation remains separate from the text increment.

## Other pixel producers: tinted glyph increment — 2026-10-10

The replacement plan now prioritizes other pixel producers: text, source images,
HUD/minimap/cursor, remaining CPU writers and enabled movies/transitions.
`RS.font-glyph-raster` has fresh dedicated isolated-original RGB565 pixel proof:
720 synthetic cases plus3615 placements of all1205 glyphs in six installed fonts,
4335 whole canvases/3420849 WORDs, zero differences and original source/padding
guards intact. `NR.font-glyph-admission` separately records eight atomic refusals
and transparent/horizontal no-ops in Debug and ASan/UBSan. Native admission now
preflights indices and visible destination reads before mutation and rejects
coverage greater than one. This is an intentional safety policy, not recovered
failure equivalence. [Scope, evidence and reproduction](font-glyph-raster.md).
Text layout/caller state, RGB555, coverage initialization, general ABI and live
shadow/bypass remain open. Original text drawing stays active. Historical shared
canvas/World/minimap evidence retains its hashes and pending current validation.
Committed `ae448ba..d412789` review covers27 exact transitions with zero gaps;
retrospective accounting does not assert new execution or a past gate pass.

## Drawing-family regression follow-up — 2026-10-10

The [reproducible family runner and results](drawing-family-validation.md) execute
selected checks across all nine inventoried families:16 renderer CTests,
10 PE32 drawing/recovery cases with40 complete frame comparisons,
15,204 terrain-submission cases,29,120 effect ANI states, native menu/media
fixtures and a fresh direct-word startup takeover with15,104 bypassed draws.
Eight retained takeover canvases/workspaces independently match original code.
The broad suite's stale sprite presentation expectation is retained as a
negative result; correcting it to the already independently measured RGB565
bit-replication policy yields91 exact original/native/presentation samples,
plus passing sprite guards and exhaustive normal/high-DPI presentation checks.
The executions retain distinct source versions; no old report hash is refreshed.
The reporter also incorporates the now-committed finite World queues1..32 proof.
Effects-to-pixel attribution, active GDI/text, unidentified CPU writers, enabled
movies/World return and full sustained replacement remain open. Engine statuses
and the2/9 drawing-family bypass count are not promoted by these regressions.

## Rendering binary/API support accounting — 2026-10-10

The [reviewed crosswalk and reproducible reporter](rendering-coverage.md)
reconcile twelve required surface-operation categories, four conditional backlog
categories and nine inventoried drawing families with scoped native code and
selected immutable original/driver comparison and bypass evidence. Historical
subset support is reported separately from source/scope/scenario-bound current
validation; categories share implementations and are not completed categories
or effort weights. The first three selected World executions retain individual
10/13,10/13 and9/13 exercised-entry counts and queues1..16 scope. The later
independently compared queues1..32 execution is now separately selected. No old hashes,
engine statuses or replacement scope are promoted by this accounting change.
The initial in-progress queue32 scenario/census limitations suppressed current
readiness; subsequent immutable queue32 records reconcile those particular gaps. Full supported-session
and launcher gates remain under the authoritative replacement plan.

## Native gameplay follow-up checkpoint — 2026-10-10

[Extended gameplay research](native-campaign-extended-gameplay.md) preserves
three new negative journeys: Apprentice idle fixed probes, subsequent successful
movement/player damage followed by wizard death, and highest-difficulty lethal
combat with a busy original-frame capture failure. A longer portrait run defeated
the first enemy before later enemies killed Cornelius and the old harness waited
after original World returned to Main. These are failures, not overall passes.

`HOST.native-campaign-movement-probes` seeds bounded measured navigation with
identity/delta guards. `HOST.native-campaign-spell-observation` seeds a separate
guarded original cast/impact trace and successful-summon mana-debit validation.
`HOST.native-campaign-capture-retry` seeds bounded UI-poll acquisition and prompt
unexpected-exit failure.25 offline validator tests pass; build/admission and fresh
end-to-end validation are recorded separately. New statuses do not claim live
equivalence or replacement. Existing source-bound portrait evidence retains its
historical hashes; changed dependencies require another full live run.

The final no-draw-skip policy has an owned20-second CPU profile with6,864 samples
and no sample loss. Hardware validation is unavailable (`/dev/dri` absent).
The optional spell-case route now distinguishes bounded UI refusals and player
Fireball defended damage. Earlier assumed Cure attempts failed: the green
starting spell is Fireball71; historical reports and traces are preserved. The [fresh bounded spell-case run](native-campaign-spell-cases-passed-20261010.json)
passes original Fireball110→90 damage,10 Zombie melee hits plus Cornelius7→0
finish, two UI refusals, independent movement, native Mini/resume and strict rate
floors (combat36.04/27.34, resumed56.97/37.05native/paint FPS). The [combined long run](native-campaign-fireball-portrait-passed-20261010.json)
also passes22 portrait cycles over61.48seconds, independent stable face pixels
and60seconds resumed input. Ordinary retreat leaves the wizard alive at126HP. Healing/refunds, other maps, timing equivalence,
native GDI and complete drawing replacement remain pending. The exact committed
range `ab40f5a..754f961` contains two reviewed file transitions and no gaps.

[Native normal Quit](native-campaign-normal-quit-passed-20261010.json) now passes
original No/World resume, repeated Yes/native defeat Continue21/Main/Main Quit4,
with original launcher/native shell exit0 and strict gameplay FPS floors.
`HOST.native-campaign-dialogue-readiness` accounts for delayed Hermes prompts;
one early blank frame was insufficient. Adept cleared two actual prompts and
summoned but its old fixed ranged targets failed; revised full Adept validation
remains pending. Confirmation still uses original pixels in native commands,
with native Qt Mini/report/Main; native modal widgets and pause equivalence
remain separate gaps. The reviewed range `b50f128..fd3d55e` has11 exact file
transitions and zero unresolved receipt gaps.

The [complete live drawing replacement plan](../../docs/live-drawing-replacement.md)
is the authoritative source for drawing takeover scope, implementation order,
remaining dependencies and complete launcher-mode criteria. This ledger and the
coverage register retain achieved milestones and their evidence. Current default
launches retain original drawing through native capture/replay presentation.

## Public native campaign test — 2026-10-09

The [campaign smoke test](../../tests/native-campaign-smoke.md) makes the requested
native-menu plus native-command campaign flow an explicit failing integration
check under `HOST.native-command-launch`. It uses the public launcher and ordinary
menu features, with semantic Qt clicks, independent engine observations and native
completed-frame counters. The current shell rejects the combined flags before game
launch ([recorded admission failure](native-campaign-smoke-admission-20261009.json));
automation is compiled but the actual campaign sequence remains pending.
This adds no menu/rendering integration, startup-race fix, movement validation or
complete drawing replacement. Source-only instrumentation leaves historical
shared-shell evidence at its original hashes; it does not renew those claims.

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
verify 2,927unchanged files before/after both live capture and comparison.

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

## Bounded World frame identity cache (2026-10-09)

`NR.world-frame-identity-cache` shares closed validated normalized identities
between catalogue lookup and binding. Exact indexed-tag/owned-byte keys use a
4,096-entry/16 MiB encoded-capacity LRU; object/map/list/token overhead is additional.
Borrowed storage is copied, invalid input cannot enter, oversized valid frames
bypass caching, token values survive eviction and primitives retain order.
Resource loading, palette ambiguity and revision visual checks remain mandatory.

The [native replay](native-world-frame-identity-cache-native-20261009.json) passes
five native tests and all 1,058 same-owned-input checkpoints versus the
prechange consumer. Warm Debug/software GL preparation falls from 116.365 ms
to 7.185 ms; the final queue uses 1,154 hits and zero hashes. Resident
encoded capacity is 516,720 bytes across 274 identities. Synthetic
mutation, bounds, eviction, bypass, malformed, unmapped, reload/adoption and palette
checks pass independently. Every compiled consumer dependency is declared and
matches the frozen build/current source; fixture stream/done and asset hashes are
verified independently of capture decoder changes. Original manifests pass
before/after. Native policies retain original comparison/replacement `none`;
this result supports scoped implementation and headless integration. Full public
hook lifetime and precise original replacement equivalence remain separate
current-validation boundaries. No gameplay or hardware-general performance claim.

Work was validated in an isolated checkout based on `b70c9e6`, excluding concurrent
minimap edits and staged work. The history report
`coverage/committed-history-world-frame-identity-cache-base-20261009.json` reviews
93 committed file transitions across `8dce3ac..60f6b3d` and `60f6b3d..b70c9e6`;
all have exact receipt chains, separately from current execution. Historical
reports, hashes and receipts remain unchanged.

### Original-active owned minimap journal (2026-10-09)

`NR.minimap-owned-observation` adds opt-in producer journal V2 with owned raw
terrain, palette/cell, visibility/creature and camera-corner inputs. Native replay
uses its own prior auxiliary canvas and checks selected original return/object
results; no original completion pixels initialize native storage. All52 observer
entries forward original execution. Synthetic four-view/two-format adapter cases
and malformed/result refusals are separate from live observation.

The preserved [startup report](native-minimap-live-startup-20261009.json) records
1062/1062 equal full-canvas checkpoints (474236912 compared pixels), orientation0,
center68/6, fog1, flash0/1 and RGB565. This historical report predates the optional
paced motion fixture; its hashes and prospective claims remain unchanged.
Original manifest verification passed before/after. Original camera/visibility
generation, outer-border drawing, unsupported row aliases, general driver/caller
ABI and live drawing replacement remain outstanding. The existing standalone
recovered minimap behaviors retain separate isolated-original evidence and do not
acquire blanket live integration status from this native observation policy.

`NR.minimap-input-fixture` separately records private-Xvfb XTest rotation/pan
controls and explicit250ms World-return pacing. `INPUT.camera-minimap-orientation-update`
seeds the selected original update/setter trace; complete input dispatch and
unassigned handler boundaries remain unresolved. See
[contract and reproduction](native-minimap-live.md) and
[V2 wire contract](../formats/canvas-producers-v2.md).

The final [motion report](native-minimap-live-motion-20261009.json) passes
1062/1062 exact checkpoints and474236912 compared pixels with zero mismatches.
Actual stored views0/1/2/3, seven centers and same-orientation panning are
observed; fog1, flash0/1, RGB565 and60 creature entries remain explicitly bounded.
The caller selects outline entries0/2/0/1 for these views, so selected entry
identity is carried independently. Entry5532f0 has no live call in this trace.
The24 adapter cases and36 refusal checks pass, with52 forwarded entries and
five invalid fixture configurations refused before mutation. Both native policies
now have scoped implementation/live-observation integration; comparison and
replacement remain `none`. The interactive V2 Qt tail consumer and unobserved
border/driver/ABI states remain pending. Original manifest verification passes
before/after; historical reports and source hashes are preserved.

### Remote pull reconciliation (2026-10-09)

Fast-forwarded local `60f6b3d` to remote `b70c9e6`, restoring the prior
uncommitted work from a retained Git stash. Coverage register collections were
merged by stable ID and review receipts retained verbatim from both branches.
The combined source census is `coverage/source-index-pull-reconcile-20261009.json`;
incoming committed file hashes are reviewed separately in
`coverage/committed-history-pull-reconcile-20261009.json`.

Offline campaign runner (5 tests) and World batch launcher (3 tests) pass.
Combined-tree audit report is retained under
`working/tests/coverage-audit-pull-reconcile-20261009.json`. It reports the
pre-existing restored local scenario `minimap-owned-live-motion16-20261009`
referencing missing evidence `NR.minimap-owned-observation.motion-20261009`
and its resulting level mismatch. That unfinished local evidence link remains
pending; no result was invented or historical fingerprint refreshed. No engine
comparison, integration or replacement milestone is promoted by this pull.

### Bounded public V2 minimap viewer (2026-10-10)

`NR.minimap-interactive-presentation` exposes original-active owned minimap
composition via `run-native-world.py --startup-history --minimap-owned`.
The Qt tail validates its envelope before payload reads; V1 retains128MiB,
V2 admits512MiB/262144records and assets retain128MiB. Optional
`--minimap-motion` uses the existing private-Xvfb completed-return fixture.

[Fresh public-launcher evidence](native-minimap-interactive-20261010.json)
records two16queue sessions with1062/1062 equal checkpoints each and zero pixel
mismatches. Motion observes all four stored views and seven centers; its
262822134byte stream exercises the former Qt limit. Unpaced startup separately
measures native diagnostic costs. Five Qt malformed/oversized tails refuse
before applying records. All84compiled repository dependencies are declared,
sources remain stable and original manifests verify before/after.

Scoped implementation/live-observation integration advances only this native
viewer policy. Comparison/replacement remain none; original rendering stays
active. Shared World/batch/launcher historical evidence retains its hashes and
requires separate current-contract reruns. Unbounded/manual gameplay, complete
HUD, borders, source generation, driver/ABI and original minimap suppression
remain pending. See [scope and diagnostic costs](native-minimap-interactive.md).


### Native World CPU composition reuse (2026-10-10)

`NR.world-cpu-composition-reuse` introduces an exact owned-byte/indexed-tag decoded
SPR LRU (4096 frames/64MiB resident encoded+decoded vector capacity, with object,
transient and externally retained handle storage additional). Per-draw palette
and effect tables remain independent. RGB565 blends sample only their own
destination; nonnegative displacement with left-to-right traversal samples
untouched pixels, preserving pre-draw semantics without whole-canvas/mask copies.
Undefined pixels, malformed frames, invalid offsets, clipping and CPU/GPU/AX
guards remain admitted/refused as before.

[Current bounded native result](native-world-cpu-composition-native-20261010.json)
passes six native tests, all1058 same-input prechange checkpoints and independent
historical capture checkpoints, and all23843 precise original intermediates/low16
AX in16 queues. Prospective claims bind100 source/build/test/helper inputs; actual
consumer compiler dependencies and the three local original-reference includes
are reviewed. Sources, frozen sources and owned inputs stay stable; original
manifests pass before/after. Warm median CPU composition falls141.556ms
to78.498ms (44.55%) in Debug/software GL. This is a
diagnostic queue measurement, not hardware-general FPS or original simulation cost.

The final replay incorporates concurrent minimap integration commit`b0bd799`;
the earlier successful [pre-integration result](native-world-cpu-composition-preminimap-20261010.json)
retains its hashes separately. The committed-history report reviews9c64221..b0bd799
(46 file transitions/83 exact behavior-accounting transitions) with no missing
receipt chains; uncommitted main-checkout work is excluded. No historical gate
pass or renewed minimap validation is inferred.

Only scoped implementation/headless integration is promoted for the new native
policy. Original comparison/replacement remain none for that policy. The pinned
original execution supports these closed CPU inputs offline; the historical
capture supplies provenance, not fresh live-hook, caller ABI or writeback evidence.
Shared-file historical renderer/producer/minimap/live results retain their source
hashes and become stale where affected. Current V2 motion/interactive minimap,
unobserved original branches and unbounded/all-map gameplay remain separate
validation boundaries. [Contract and reproduction](native-world-cpu-composition.md).

### Existing workspace accounting committed (2026-10-10)

The retained World frame identity live/replay/original comparison records,
World/minimap source censuses and earlier pull/history reconciliation reports
are reviewed together against main checkout `48c2638`. Evidence bytes and
previous review receipts remain unchanged; earlier reports describe their
original execution sources, not a new run against the later CPU composition
implementation. The earlier pull note's missing minimap motion evidence is
present in the current register; the combined audit now has no errors. Historical
staleness and changed-scope validation remain explicit pending boundaries.

The pending edits to the historical minimap terrain census are preserved under
`coverage/source-index-minimap-terrain-workspace-preserved-20261010.json`; the
historical snapshot retains its committed bytes. The current reviewed source
index is `coverage/source-index-existing-work-reviewed-20261010.json`.
Committed history `b0bd799..48c2638` contains14 reviewed file transitions with
zero missing exact receipts, recorded in
`coverage/committed-history-existing-work-20261010.json`. This is retrospective
accounting, not new runtime evidence or a claim about past gate execution.
No engine source, game balance or original artifacts change in this commit.

### Combined native campaign route (2026-10-09)

The historical public admission failure is retained. The new isolated branch
implements a one-process native-menu/native-command route with dual hash-checked
DLL staging, native input ownership and versioned channels. Behavior
`HOST.native-campaign-session` tracks validation independently from original
simulation/drawing replacement; a live scenario will be linked with its first
executed result. Live click-through, gameplay stress and
performance remain pending at this implementation checkpoint.

The first combined live observation is retained as
`native-campaign-premature-escape-20261009.json`: the initial driver confused
Region Entry transition presentations with gameplay and sent Escape before
any completed World tick, opening an unsupported Realm-context Mini instead
of campaign Mini. The test now waits for independent original World readiness.
Native viewport initialization and software-Xvfb backend selection are explicit.
Physical gameplay input, repeated native menu returns and separate native/Qt
paint throughput checks are implemented; positive live validation remains
pending. Historical source hashes and statuses are retained.

The corrected World-ready smoke now completes native Main/Region/Mini, original
World updates and Cancel/resume with native GPU command presentation. Historical
positive smoke: 88 command presentations/56 paints, zero fallback/recovery.
Active Apprentice stress completes four physical-input phases and three native
Mini returns but fails the fixed first-camera 10 FPS window floor (about 9 FPS).
Initiate tutorial input gating and original desktop screenshot occlusion were
also observed separately. Ordered-copy scheduling avoids the captured GAP;
native desktop lowering restores presentation visibility. Optimized builds and
single-process immutable manifest verification improve launch/test overhead.

RGB565 constant-shift presentation preserves all 65,536 colors at scales1/1.5,
but its gameplay rerun still fails the same floor; do not call it a performance
fix. Direct opaque GPU copies preserve five backend/format/overlap/ownership/
blit/surface regression checks and the exhaustive GPU viewport test. Another
live run is in progress at this checkpoint. Profiles attribute native CPU time
primarily to software Mesa JIT; hardware, synchronized pixel equivalence,
movement/combat outcomes, other maps and complete drawing replacement remain
pending. Historical evidence fingerprints are preserved. The committed range
b70c9e6..6f7d9ab has26 exact hash receipt transitions, no unresolved gaps, in its
retrospective report; this review does not assert new execution or past gates.

Final scope-bound Apprentice live observation passes four 40-second physical
camera/rotation/scene-HUD/spell-creature-selection/target-attempt phases and three
native Mini returns. 9,621 native frames / 5,862 visible paints, zero fallback/recovery;
51–55 native/about 33paint FPS by phase,worst 1-second 14 FPS. Immutable before/after
verification and stable source hashes pass. `HOST.native-campaign-session` now
has scoped implementation and live-observation integration support; original
comparison/replacement remain none. Paired CPU profiles and a diagnostic final
frame are retained. Spell casting/summoning/combat outcomes, synchronized visual
equivalence, hardware/other-map/unbounded validation and full drawing replacement
remain outstanding; inspected mana is 0/15. Timing failures remain negative,
with no causal speedup or60 visible-FPS claim.

Retrospective review through 3fbddc0 checks 55 committed file transitions.
Two diagnostic profile text files initially lacked exact receipts; explicit
retrospective addition hashes reconcile them without modifying the original
files. Both the report with these gaps and its resolved continuation are retained.
The continuation has no unresolved gaps and makes no past gate/validation claim.

Final default Initiate smoke also passes on the final source: 89 native frames/
56 visible paints, zero recovery/fallback, three initialized World completions
and one same-thread Cancel/resume. Extended tutorial input remains separate.
Retrospective review through 0857efe checks 73 committed file transitions with
exact receipts and no unresolved gaps; prior gap/reconciliation reports remain.

## Native campaign successful casting/combat checkpoint (2026-10-10)

`HOST.native-campaign-casting-combat` seeds a bounded public native-menu/native-viewport journey with passive original creature and melee-return observations. Two retained negative runs distinguish count OCR failure from a confirmed original Zombie summon with independent native/original `1/15` readings but unverified movement/combat. Actual original enemy HP loss inside a player Zombie melee call remains required; scripted damage cannot pass. No simulation, damage formula, balance or drawing replacement is claimed. The existing strict native/paint throughput floors remain active. See [observation boundary and remaining gaps](native-campaign-casting-combat.md). The committed range `0857efe..3f106d9` has six exact file transitions reviewed with no missing receipts; current uncommitted changes remain separate.

## Native campaign successful casting/combat validation (2026-10-10)

`HOST.native-campaign-casting-combat` now has scoped implementation and live observation from the source-stable highest-difficulty native-menu/native-command journey: two physical Zombie summons, independent native/original `0/15 → 1/15` readings, 14 original Zombie enemy melee HP losses, and Cornelius's original `2 → 0` finishing hit on that same enemy. Native Mini Cancel/resume, 5,798 native frames/3,958 visible paints, zero recovery/fallback and unchanged 20-FPS average/10-FPS window floors pass on private four-worker software Mesa. Original simulation/drawing remain active; comparison and replacement remain none. See [retained result and exact boundaries](native-campaign-casting-combat.md). Eight negative runs preserve OCR, movement/placement, slot reuse, strict timing and finishing-actor findings; repeated post-hit recentering still approaches 10 FPS and remains an open performance branch. The first default-worker and four-worker profiles are retained without a deterministic hardware-speedup claim. The committed range `3f106d9..52dcdf8` has 17 exact reviewed file transitions and zero unresolved receipt gaps. Shared historical evidence remains unchanged/stale after these source edits; only this new scoped journey has fresh execution support.

### Native portrait rendering and draw cadence — 2026-10-10

The [portrait investigation](native-portrait-rendering.md) restores repeated
portrait recentering during successful casting/combat and adds a full minute
after victory. Independent native/original stable face pixels agree within
one channel value. Per-output damage envelopes avoid whole-frame conversion
for small HUD/cursor/portrait updates; first allocation, palettes and swaps
retain full conversion. The renderer-only negative still fails fixed 10 FPS
windows and is preserved. Native staged DEBUG draw skips are now disabled
in both plaintext and encrypted configurations, hash-checked before launch;
selected game speed and original pacing instructions remain unchanged. This
is an intentional native presentation policy, not recovered timing equivalence.

The [fresh passing run](native-portrait-passed-20261010.json) records 22
post-combat cycles/61.18 seconds, 45.52 native/32.83 visible FPS and worst
16.53 FPS within the portrait phase. Whole cast/combat/portrait and resumed
phases pass 20 FPS averages/10 FPS windows/two-second stall gates:
41.39/30.44 and 58.56/38.66, worst 16.41/17.04. 9,443 frames/6,608 paints,
zero fallback/recovery, successful Zombie summons, 12 original Zombie damage
calls and player wizard 8→0 lethal melee on the same enemy. Conversion work
is 61.51% lower than converting every whole frame in this command sequence.
Independent GPU/CPU pixel fixtures, 354 incremental frame comparisons,
14 evidence guards and 3 config tests pass. Original manifests verify 2,927
files before/after. This closes the bounded portrait-recentering performance
gap; prior failures remain historical. Other map/hardware/full-World/effect
pixels, precise timing equivalence and complete drawing/simulation replacement
remain pending.

Committed-history review 52dcdf8..5c086ce checks 23 exact file transitions
against retained receipts with zero gaps; it asserts neither a past gate pass
nor new original validation.

### Native healing loadout and health observation — 2026-10-10

The isolated native Quick Battle map2 route now assigns original-offered Cure41
and Zombie14 through native setup/Portmanteau controls. Physical orders produce
original human injury and Cure277→377, matching original caster mana debit and
same-thread/source/owner/target/caller. Native/paint averages40.81/30.15FPS and
worst16.38FPS pass the unchanged20/10FPS/two-second gates; zero fallback/recovery,
independent images and original manifests pass. V3 signed-health forwarding
preserves original arguments/return/LastError; V1/V2 readers remain supported.
The wrong-HUD and delayed-effect interval failures retain immutable reports/raw
traces. See [extended findings](native-campaign-extended-gameplay.md).
This is bounded healing with private cleanup, not complete Quick Battle or
cleansing/clamp/refund/timing/exception/hardware/replacement equivalence. Shared
campaign/portrait scopes await the final combined rerun after these source edits.
History `fd3d55e..0c935ad` has29 exact committed-file transitions and zero missing
receipts; it asserts no past gate pass or new validation.

### Final stationary portrait and smoke cleanup (2026-10-10)

The final stationary portrait workload fails the unchanged20FPS average in two
complete casting/combat/normal-Quit workflows (16.7121/16.7054FPS). Exact owned
CPU sampling locates original clock busy waiting; no native renderer speedup
claim follows. Sleep1 yields expose native cross-thread history GAP; Sleep0
still gives16.6946FPS and a later original wizard death after random resumed
orders. The experimental yield is disabled by default and stage7 native Main
now fails promptly. These failures and historical reports remain intact; no
pacer integration promotion is made. A source-bound mocked public cleanup
fixture passes after malformed auxiliary JSON, proving exact owned stop/prefix
and manifest/report continuation only. The committed range0c935ad..2ab21f4
has33 exact reviewed transitions and zero missing receipt gaps.

### Native precise real uptime clock fixture (2026-10-10)

`HOST.native-precise-uptime-clock` is a separate intentional hosting policy:
guarded original GetTickCount import reads use actual loaded WinMM timeGetTime,
with32ms shared-epoch admission and original entry/exit LastError preserved.
The1024-sample standalone Wine fixture proves1/2ms versus16/17ms host clock
steps and0..16ms epoch difference; no original game timing equivalence follows.
Game-speed preference, wait target and original wait instructions are retained.
Original gameplay/physics, full timers and other scheduling modes remain open.
The reviewed committed range2ab21f4..e5b4180 has51 exact transitions and zero
missing receipt gaps. Live portrait/casting/resume/normal Quit is being checked
separately with prospective source and scope bindings.

The precise-clock combined workflow now passes from native New Game to original
normal Quit: genuine Fireball100to80, player Zombie melee and same-enemy lethal,
blocked casts,25stationary portrait cycles61.296sec (56.477interior20.0081FPS,
face error1),60.431sec resumed camera/HUD/wheel, MiniCancel and QuitNo/Yes/Main.
It retains8642native frames/6499paints, zero recovery/fallback, manifests and
stable prospective sources. `HOST.native-precise-uptime-clock` gains scoped/live
observation support; no timing/physics or replacement equivalence. This result
has little margin above the20FPS portrait floor and does not imply hardware or
long-session performance. Cure and generic repeated menu stress remain separate.

Fresh final-source native Quick Battle Cure also passes: original397to400
health, Cure41 delta100 and actual caster mana debit from the reviewed caller;
native41.5934/paint30.6934FPS,worst19.0909FPS, zero recovery/fallback and stable
prospective sources/manifests. The selected clamp is observed without a full
formula/cleansing/refund claim. Original277to377 evidence stays historical.

### Final repeated native Mini Cancel stress (2026-10-10)

The source-bound Apprentice repeat smoke passes four43.2second ordinary physical
input phases and three independently observed original MiniCancel/World resumes:
10774native frames/7168paints, zero recovery/fallback, native phase averages
57.5568/65.4852/56.9435/62.2264FPS and painted39.4514/42.4533/39.4551/41.8216FPS,
worst22.1198FPS. Sources/manifests and protocolV12 remain verified. This renews
only the bounded host session; images are unsynchronized diagnostics and there
is no new movement/combat/pixel or full engine replacement equivalence claim.
Together with fresh portrait/casting/blocked-spell/Quit and separate Cure runs,
the outlined bounded smoke cases now have final-source execution evidence.
The commit rangee5b4180..8e37e6b has38 exact reviewed transitions and zero gaps.

- 2026-10-10 crowded native Quick Battle next step: registered
  `HOST.native-crowded-battle-stress` and the native map2 scenario before live
  execution. Uses ordinary native setup controls (mana200/health800/control30),
  original simulation, physical summon/Cure orders and180..240seconds of passive
  population/combat and strict native/visible rendering checks. Live validation
  passes in `native-crowded-passed-20261010.json`:182.34seconds,34 actors peak,
  eight actual summons,213 player melee damage returns, successful Cure,
  native29.1996/visible25.3715FPS,worst19.8556 andzero fallback/recovery.
  Existing smoke results retain historical hashes.
  Reviewed committed history8e37e6b..7f361b0:17 exact file transitions,zero gaps;
  retrospective accounting makes no past gate-pass or new execution claim.

- 2026-10-10 surface-history follow-up: fresh prospective source-bound
  `NR.copy-order.current-stress-20261010` reruns twelve actual PE32/GPU cases.
 418 complete native pixel comparisons,seven valid END/ACK sessions,five expected
  GAP refusals across copy/lock/DC/flip timeouts and reentrant source writes.
  No new renderer defect or algorithm change; actual original driver scheduling
  and the historical Sleep1-related gameplay GAP remain separate boundaries.
  Reviewed7f361b0..0df3a72:27 exact committed file transitions,zero receipt gaps;
  retrospective review is not new behavior validation or a claim of a past gate.

### Merged native campaign baseline (2026-10-10)

Native campaign branch07c2430 is integrated with master8c3da6a, preserving both
stable-ID registers, historical evidence and append-only exact receipts. Reviewed
48c2638..8c3da6a and0df3a72..07c2430:82 parent/current source transitions with
exact receipts, zero unresolved history gaps. The incoming register is retained
separately before integration; new census records the combined source review.

Fresh prospective merged-code runs pass native campaign casting/melee/ranged
combat, invalid-target/insufficient-mana cases, movement, portrait face comparison
(maximum channel error1), mini-menu/resume and normal quit; ordinary native
Quick Battle Cure, cleanup failure-path fixture and real WinMM clock guard pass.
Reports and observation wires use new `HOST.native-integration.*-20261010`
evidence IDs. Original simulation and drawing remain active in these runs.

Two crowded workload failures are preserved separately: only9.601dense seconds
in the first run and only five confirmed summons after eight clicks in the
second. The smoke input now varies physical targets until eight original
successful mana-debited casts (maximum24 attempts), calibrates movement even
when injured, and follows the observed living squad using ordinary ground orders.
No workload/FPS/health/mana rule or admission floor is weakened. The fresh
three-minute pass observes25actors peak,10near wizard,108.579dense seconds,
eight player summons,39original player melee damage events and Cure638→738.
Native39.563FPS/visible29.245FPS average,19.724worst sample, no fallback/recovery.
This is spatial workload evidence, not a visible-pixel population count. The
portrait phase remains close to its20FPS floor; software-Mesa evidence does not
establish hardware performance or full animation/physics equivalence.

Shared World/minimap/surface-history comparisons retain their historical hashes
and explicit current-source limitations. Guarded World replacement beyond the
first16 queues and independent current-code raster comparison remain next.

### Guarded World prefix32 and owned source reuse (2026-10-10)

The finite Quick Battle map2/zero-items World route now admits an explicit32
queue prefix through MNMPRO03; legacy16-queue envelopes and defaults remain.
Full source definitions and shared immutable references preserve canonical packet
hashes and original caller identity. Bounds remain128MiB journal,131072records,
2048definitions and16MiB owned sources; missing/mismatched/over-budget references
refuse. The original32 prototype hit128MiB after25queues. That failure and an
independent parser refusal before definition support retain separate new negative
records and historical source hashes.

Fresh `NR.world32.live-20261010` and `NR.world32.replacement-20261010` bind
32protected native writebacks/readbacks to separate unmodified PE32 comparisons:
63124precise raster canvases and low16AX returns,30299520000pixels,
1224checkpoint canvases and32live final canvases all match. Five frozen native
fixtures pass,86compiled dependencies are declared, input/source hashes remain
stable and original manifests pass before/after. Source-only CPU/GPU parity,
exact original comparison and destination guards remain mandatory. New native
prefix policy has scoped implementation/live equivalence; it is not an original
function or broad replacement claim. Older evidence fingerprints are retained.

The RelWithDebInfo software-Mesa run uses12.87MiB journal and2016definitions
(3.94MiB). Median warm CPU11.11ms, GPU submit63.24ms, guarded queue223.83ms.
This fails real-time performance expectations even though finite correctness
passes. Continuous rolling history, GPU batching, normal-mode/camera/combat
profiling and hardware validation remain outstanding. Native campaign smoke FPS
belongs to its separate original-raster-active presentation route.

Reviewed first-parent integration8c3da6a..8610a25:347exact file transitions,
zero unresolved receipt gaps after23retrospective binary artifact receipts.
These receipts assert provenance, not new execution or a past gate pass.
Concurrent rendering-accounting edits are excluded from this commit projection.

### Direct-word clipped pixels — 2026-10-10

`RS.word-clipping` records 960 native GPU clipping cases and 1,920 independent
original-backend executions matching 447,330 complete canvas pixels per path.
Exact/partial edges, two corners, hidden sprites, signed origins, masks, opaque
zero, row padding and two alignments pass on software Mesa. The prospective
source-bound result validates a bounded pixel contract; no runtime hooks or live
admission were changed. [Research and limitations](word-sprite-clipping.md)
retain the initial harness failure separately. Clipped scratch/argument state,
ABI, other clip globals, installed assets and live replacement remain pending.

### Bounded World throughput implementation (2026-10-10)

`NR.world-batch-throughput` separates bounded native optimization from recovered
baseline and broader interactive frame rate. Regional reusable R32UI snapshots,
ordered512-instance copies with64 owned palettes, at most512 atlas validity
proofs, retained exact frame identity, private64KiB V3 journal/barrier flushes,
tight original checkpoint writes, direct RGB565/wire packing, deferred raster
JSON and bounded4096-record drain preserve existing pixel/return/guard checks.
Optional same-format copy-image snapshots have a3.3 framebuffer-copy fallback.
Closed DONE replay skips live reply polling while retaining marker/proof checks.

Six assertion-enabled Debug native fixtures pass, including overlapping ordered
palettes, scalar RGB565 blend/displacement/additive pixels, regional warm scratch,
atlas validity holes/eviction/invalidation, malformed closed-frame bounds and
resource revisions. PE32 hook fixtures pass buffer boundary/exact bytes/flush
failure refusal alongside register/flags/x87/SSE/LastError and guard checks.
The first copy-image prototype failed the atlas scalar comparison; its destination
framebuffer binding is corrected and the same fixture passes. The3.3 fallback
also passes. Whole original every-intermediate/AX revalidation remains pending.

A prospective warm50ms native-work gate failed at51.418ms despite32 guarded
correctness completions; the new negative evidence preserves executed source
and contract hashes. Do not infer original equivalence or frame rate from that
run. Fresh same-input optimized benchmark and protected live32 execution pass. Native
policy implementation is scoped/headless; exact original comparison is pending.
Eight workers for both baseline/current: native work68.36→40.51ms, GPU host
49.28→23.98ms, diagnostic queue201.16→55.18ms;1224checkpoint canvases match.
Separate live32 workload: native work29.11ms, diagnostic queue66.28ms; all32
guarded writebacks complete under unchanged50/100ms limits. Four-worker budget
misses remain recorded, and no hardware/full-game cadence equivalence is claimed.
Budgets are50ms native work including CPU reference, and100ms diagnostic queue
wall time including capture/transport overhead; full simulation cadence,
continuous rolling native World and hardware remain outstanding. Earlier
performance evidence is historical, with its original source hashes retained.

Reviewed first-parent8610a25..3687a38:35 exact parent/current file transitions,
zero unresolved receipt gaps. Also reviewed3687a38..f60b55c:52 exact transitions,
zero unresolved receipt gaps. This is committed-history provenance, not new
validation or a past gate pass. Concurrent edits remain outside this projection.

### Optimized World32 original revalidation (2026-10-10)

`NR.world-throughput.replacement-20261010` pairs the protected optimized live
capture with a frozen Debug source-only native proof and actual unmodified
nocd40209ca7 backend execution. Every49113 precise raster canvas and low16AX
return matches:23574240000pixels,1222checkpoint canvases and32live finals.
Native reads no original oracle; source/input hashes, compiler dependency closure
and original manifests before/after pass. Evidence binds exact source/contract/
scenario versions and preserves historical records, including both four-worker
performance failures and the corrected copy-image framebuffer branch.

Freshness is restored only for RS.world-raster-batch-return,
NR.world-raster-batch-transport, NR.world-resource-reuse,
NR.world-batch-extended-prefix and the new native throughput policy. The latter
has scoped implementation/live equivalence, with comparison/replacement none
because buffering/batching/budgets are intentional policies rather than recovered
original functions. Wider historical contracts retain explicit pending validation.
The frozen Debug original FIFO proof is correctness evidence, not a timing run.

The controlled same-input optimized benchmark remains68.36→40.51ms native work,
49.28→23.98ms GPU host submission and201.16→55.18ms diagnostic queue, using
same eight Mesa workers and1224identical checkpoints. Separate protected live32
workload passes29.11/66.28ms warm native-work/diagnostic medians.50/100ms budgets
remain enforced. Full simulation cadence, rolling native World, sustained
interactive gameplay and hardware validation remain outstanding. These are
separate from the original-raster-active campaign smoke and from the diagnostic
capture ceiling. Reviewed f60b55c..01e160a:29 exact transitions, zero receipt gaps.

### Direct-word caller state and compatibility — 2026-10-10

`RS.word-backend-state` adds 4,448 independent original/model and production-entry
comparisons for complete workspace, argument mutation, integer/stack/defined
flags and selected masked floating state. All pass; the actual native workspace
helper matches 768 admitted cases. Original comparison refutes the old scalar
workspace9 zeroing in 120 cases. The existing route now preserves its trailing
transparent count and clears admitted x87 C1 as original execution does.

`RS.word-route` retains strict unclipped admission. Fresh shadow compares 12,625
admitted calls without mismatches; fresh takeover bypasses 12,608 calls while
705 unsupported requests execute original bodies. Eight retained canvases /
1,070,400 complete pixels and workspace match independent original execution.
[Research, fixture scope and exclusions](word-backend-state.md) keep model safety
policy, host entry testing and finite live proof separate. Clipped native raster
admission, unmasked floating exceptions, Win32 LastError/reentry stress and
long-session/physical-driver coverage remain pending.

### Native CPU direct-word clipping — 2026-10-10

The separate portable CPU clipping service now implements half-open run
intersection and full-frame preflight, including hidden rows. Fresh independent
original comparison passed 4,448 cases over both pinned backends with all 16
workspace words, arguments, integer/stack/defined flags, selected masked floating
state, complete canvas/source guards and pixels matching. The host clipped
adapter actually handled all 4,400 positive requests; 48 empty requests forwarded
for original floating handling. ASan/UBSan boundary checks passed 288 manually
expanded placements, 43 atomic refusals and 3 empty no-ops; freestanding PE32 core
compilation passed. The initial null-output failure and corrected prospective
rerun are separately preserved.

This establishes bounded CPU clipping plus recovered state composition in an
isolated host adapter, not clipped live admission. The existing live hook still
forwards clipped requests. Win32 access/LastError/reentry, unmasked/full-stack/
exception floating behavior, installed-asset generality and sustained live
validation remain open. See [CPU clipping evidence](word-cpu-clipping.md) and
`RS.word-cpu-clipping` / `NR.word-cpu-clipping-admission`.

## Offline renderer robustness increment (2026-10-10)

`NR.offline-render-soak` now has fresh headless integration evidence:20 complete saved World32 sessions in one renderer,640 CPU/GPU completions,24,480 checkpoint assertions and28,000 seeded adversarial draws. Three assertion-enabled Debug refusal/cache/atlas fixtures pass. Warm native work39.22ms median/72.71ms p95/116.15ms maximum and10.87MiB retained growth pass declared host budgets; every session retires GPU surfaces and scratch storage stays fixed. Cold first-queue preparation remains444.20ms median; no full-game cadence, fresh original comparison, rolling delivery or live replacement claim. See [offline soak scope](native-render-offline-soak.md). The committed range01e160a..0415793 has50 exact reviewed transitions with no gaps.

The offline soak follow-up also passes20 sessions on the GL3.3 framebuffer-copy fallback with a second seed:38.41ms median/71.20ms p95/92.42ms maximum and12.00MiB retained growth. Across both positive runs:1,280 native World completions,48,960 checkpoint comparisons,56,000 seeded mixed draws and40 clean teardowns. A separate intentionally impossible0.001ms budget fails with exit1 despite correct pixels, confirming slow work is rejected. The reviewed0415793..50bec93 range has10 exact transitions and no gaps; source bytes and the first soak census remain unchanged.

### Opt-in live CPU clipping shadow — 2026-10-10

`RS.word-clip-shadow` now connects the CPU core and recovered workspace model to
the actual opt-in route and production entry. Native draws into a guarded copy;
original drawing executes once and owns the engine pixels/workspace. Fresh
isolated execution passes 4,448 selected-state/pixel comparisons, 22,240 forced
fallback checks and original-once/LastError checks. Empty requests forward.

Software Wine capture with a repeated map-dialog round trip and four World
observations retains eight clipped menu/banner requests, all on forward backend
0x596cb8 and one 208×111 asset. Independent original/native replay matches all
workspace words and 1,920,000 complete canvas WORDs. Live mismatches, errors,
stopped state and bypasses are zero; 21,607 calls still forward. Live scalar and
World admission, refusal-class inventory, auxiliary/indexed layouts, broad assets,
full floating/Win32 behavior and sustained sessions remain open. There is no
clipped takeover. See [scope and results](word-clip-shadow.md).

Current scoped implementation/comparison/live-equivalence belongs only to the
new shadow contract. Shared model/build/launcher/capture source changes leave
older evidence historical and stale; their hashes/statuses are preserved.
Initial host alignment failure and short/GL-startup captures remain retained
diagnostics. The committed range 0415793..652c01d has 19 exact reviewed file
transitions, including append-only journals, with no unresolved receipt gaps.

### Pinned native World admission and cold preparation (2026-10-10)

`NR.world-prepared-admission` has fresh scoped/headless saved-input evidence:12 retained-renderer sessions,384 CPU/GPU completions and14688 checkpoint assertions, plus three Debug fixtures. Owned pinned preparations avoid repeated SPR decode; on-demand revision-bound visual indexing and reusable per-thread SHA contexts reduce cold queues from historical444.20ms median to118.98ms median/138.89ms maximum. Warm median38.83/p9547.28/max70.48ms and11.96MiB retained growth pass unchanged warm/memory limits. Catalogue initialization is reported separately at539.99ms median. Three rejected exploratory budgets and the unsuccessful two-worker memory/latency trial remain recorded; the final implementation prepares sequentially. This is no full-game cadence, original comparison or live replacement claim. See [admission evidence and limitations](native-world-prepared-admission.md). Committed50bec93..c9f99e8 accounting reviewed34 exact file transitions with no gaps. Older evidence touching changed shared source stays historical.

### Ordered source-only rolling World foundation (2026-10-10)

`NR.world-rolling-delivery` has scoped/headless frozen-source evidence for a separate two-slot ordered64-bit source/completion channel, persistent producer/GPU/resource history, bounded packet-local source definitions and a latest exact-byte decoded packet cache. Default and GL3.3 fallback each pass512 queues, all1224 saved-prefix checkpoint hashes and512 native CPU/GPU completions/ordered replies;480 continuation queues are explicitly synthetic, with eight changed-coordinate requests.300 scalar copy cases and16 optimized refusals pass per run; five Debug fixtures include1000 cross-process cycles,27 ownership/metadata refusals and actual producer exit/cancel/fresh-session handling. Native clipped row copies preflight before mutation, snapshot only same-ID sources and bulk-copy opaque rows. Default/fallback continuation medians54.79/57.14ms, maxima102.49/88.50ms and0.19MiB post64 retained growth pass unchanged60/120/300ms/32MiB limits. Cold full bootstrap/World completion298.54/269.73ms passes500ms excluding separately reported saved digest assertion overhead; full verification times remain recorded. An impossible0.001ms64-queue run preserves correct pixels/provenance but fails performance with exit1. See [rolling evidence and boundaries](native-world-rolling.md). This does not connect the original live producer, establish original equivalence, physical presentation/input latency, camera/combat/full-game cadence or hardware readiness. Committed c9f99e8..ae448ba review has15 exact transitions/zero receipt gaps. Older touched source evidence remains historical; original inventory/unresolved flows are retained.

Final-source cold-admission revalidation: `NR.world-prepared-admission.rolling-source-20261010` retains prospective claims and exact frozen/compiler/source/input proofs for four World32 sessions,128 CPU/GPU completions,4896 saved checkpoint hashes,7000 seeded draws and three Debug fixtures. Cold116.67ms median/134.05ms maximum, warm37.80ms median/47.44ms p95/53.94ms maximum and4MiB retained growth pass the unchanged budgets. Earlier cold evidence is preserved with its original fingerprints; no original comparison or live stage is promoted.

### Auxiliary-bearing word clipping admission fix — 2026-10-10

The previous clipped core unnecessarily refused every nonzero auxiliary offset.
`RS.word-clip-aux-shadow` now bounds main colour reads at the earliest validated
auxiliary offset and leaves payloads/caller passes untouched. Actual route/entry
comparison passes 22,048 original cases, including 17,600 auxiliary-bearing
variants, and 110,240 classified fallback checks. Native policy
`NR.word-clipping-plane-bounds` passes sanitized 1,440 placements, 79 atomic
refusals and three empty no-ops.

Fresh Wine startup compares all 13,312 observed requests, including 705 clipped
and 12,607 interior, with no fallback/mismatch/error/stop/bypass. All use forward
backend0x596cb8. Eight distinct auxiliary-bearing input frames (two clipped,
six interior) independently match 1,420,800 canvas WORDs and all workspace words.
The sampler no longer fills its slots with a repeated menu banner; initial wrong
dimensional/overstrict clipped-diversity requirements remain retained failures.
[Scope, results and limitations](word-clip-aux-shadow.md) distinguish current
main-plane shadow support from indexed/payload effects, live scalar coverage,
semantic caller attribution, sustained sessions and clipped takeover. Historical
shared-source evidence keeps its original hashes and requires its own rerun.
Committed ranges652c01d..c9f99e8 andc9f99e8..d412789 have28+44 exact reviewed file
transitions, including append-only journals, with no unresolved gaps. Concurrent
font-rendering work is preserved separately from this reviewed projection.


### Guarded clipped direct-word takeover — 2026-10-10

`RS.word-clip-takeover` adds explicit `clip-takeover` mode4. It commits guarded
native main-plane pixels and recovered workspace to the original canvas, with
conservative caller x87 admission; empty/refused calls tail-forward original.
`NR.word-clipped-takeover-admission` keeps this safety policy separate.

Fresh actual-entry tests match22,048 cases:22,000 positive native body-zero
calls(17,600 auxiliary-bearing),48 empty original-once forwards and110,240
classified forced refusals.22,000 unmasked and2,240 occupied push-slot hidden
fixtures preserve original-once selected state/LastError. Four rounding modes
across24/53/64-bit precision and ASan/UBSan core boundaries pass. Mode3 regression
also passes22,048 original matches and110,240 classified refusals.

Four Wine World startup observations report13,312 native bypasses,705 clipped,
zero original forwards/comparisons/refusals/error/stop. Observed backend is only
forward0x596cb8 with53-bit x87 precision. Eight distinct auxiliary-bearing captures
(two clipped,six interior) independently match unchanged-original/native replay:
1,420,800 completecanvas WORDs andall16 workspace words. Equivalence covers the
retained cohort; counters alone do not compareall13,312 commits. Auxiliary caller
passes and other drawing remain original; no fullscene/session/performance claim.

[Scope and evidence](word-clip-takeover.md) retain the initial count-only failure,
source-bound fresh executions and42 file/94 behavior committed-history transitions
acrossd412789..0191a55 withzero gaps. Older wider/shared contracts remain historical
where source edits invalidate current evidence. Original font/text consumer live
integration, indexed/auxiliary effects, more active battle/camera coverage and
sustained delivery/recovery remain outstanding.

## Guarded rolling handoff, hardware and gameplay follow-up (2026-10-10)

The original V3 producer now has an adapter to ordered native rolling delivery, preserving its finite32-queue source journal, original destination/entry/caller/AX guards and actual post-writeback checkpoint/return acknowledgement. The new native closing records are anticipated only; source/extent/identity mismatches and missing original returns refuse. A separate GPU/producer entry observer exports owned entry state for independent original checking. Fresh source-bound live recovery passes32 actual guarded returns/45,448 descriptors/1,235 completed checkpoint assertions, zero surfaces/image fallback, warm34.98ms native/91.99ms delivery excluding measured diagnostics and cold320.56ms. Both queue2 peer-death tests terminate and clean up private Wine; the subsequent fresh session recovers. Independent original comparison is tracked separately; no original unbounded or normal-mode stage is promoted.

`NR.world-rolling-hardware.nvidia-20261010` validates512 CPU/GPU completions and ordered ACKs on NVIDIA TITAN RTX EGL with1224 saved checkpoint hashes,480 synthetic continuations/eight changed requests, bounded caches and zero surfaces. Continuation27.55ms median/35.49ms p95/49.82ms max, cold251.82ms and1.5MiB growth pass unchanged limits. Five Debug fixtures pass separately on software Xvfb. This establishes headless hardware completion, not live simulation/widgets/desktop/scanout.

`RS.world-rolling-handoff-equivalence` adds fresh scoped original comparison/headless integration:45,448 exact original raster/low16AX checks/21,815,040,000 canvas WORDs,1,235 checkpoints/557,276,912 WORDs and all32 live reply finals equal native replay, plus five native fixtures. Original destination pixels are never native inputs. Original executable/source/input/frozen dependency proofs are preserved; comparison is limited to captured sources/callers, with original unsupported branches/complete session/uncaptured gameplay still open. Committed d412789 review covers27 exact transitions/zero missing receipts; concurrent d412789..d6e32e9 review covers66 transitions/zero missing receipts and remains separate from this isolated branch.

`HOST.native-campaign.remaining-gameplay-20261010` is a separate fresh native menu/command session with successful summon, player lethal combat, Fireball damage, rejected casts, physical movement,25 portrait recenter cycles, native Mini/resume and normal Quit (exit0).8642 native frames/6478 paints, zero fallback/recovery; portrait interior57.171seconds at20.0101native/paint FPS, worst19.9275FPS, stable face error<=1. This is software Xvfb; original simulation/drawing remain active. Neither that session nor NVIDIA synthetic replay asserts guarded rolling combat or live hardware input cadence. Historical fingerprints remain unchanged.

The guarded rolling implementation commit0659aa5 has26 exact reviewed parent/current file transitions and zero receipt gaps, separately recorded from this new gameplay evidence. No native source changes accompany this increment.

## Rendering branch integration (2026-10-10)

The committed font/auxiliary-clipping branch and guarded rolling branch are reconciled in an isolated integration checkout. Existing evidence bodies and append-only receipts are retained. Shared CanvasSequence/runtime/launcher edits make earlier rolling evidence historical; fresh combined-source execution is required before claiming current validation. Ongoing uncommitted font layout/backend work in the main checkout is outside this merge.

Fresh combined-source evidence validates53,405 original raster/low16AX results and25,634,400,000 WORDs,1,235 checkpoints/557,276,912 WORDs and32 live-native reply finals, with five Debug fixtures. Original pixels remain outside native inputs. NVIDIA512 replay passes warm29.399median/43.525p95/64.519maxms, cold224.723ms andzero retained growth. Both actual peer-death tests refuse queue2 and stop private Wine. Passing complete32 before faults measures native53.660ms/delivery113.516ms/cold298.186ms; a later recovery passes pixels but fails unchanged delivery120ms at146.423ms. This performance failure remains explicit pending readiness. Disposable checkpoint observations now use checked close and atomic no-replace rename; protocol replies/reports retain durable publication. A40-image publication probe measured579ms QSaveFile versus42ms close/rename and40 fdatasync calls, without claiming whole-path pacing. The two committed branch ranges have124 exact receipt transitions andzero gaps. Continuous original delivery and normal full-path latency remain separate next milestones.

A fresh isolated post-fault recovery then passed32 original returns withnative46.349ms/delivery117.736ms/cold303.947ms on unchangedlimits. The146.423ms failure remains preserved; the narrow delivery margin is a normal-path performance concern, not erased by this passing result.

## Fixed-block continuous original producer (2026-10-10)

The explicit continuous adapter uses a35,869,312-byte two-slot mapped protocol,
source-owned localV3 blocks with64-bit session generation and real original
checkpoint/return acknowledgement before reuse. Original destination remains
NOACCESS until the validated reply; CPU/GPU history persists and Qt presents
the retained GPU canvas directly. Normal diagnostic images/journal/reply files
are absent. A source-bound64-queue probe passes99,809 rasters,64 native/original
completions and64 Qtpaints,zero fallback/remaining surfaces. The initial probe
correctly refused a concurrently reordered startup journal; continuous emission
now serializes source definitions/references and sequence/byte allocation.
The4096-queue/death/recovery milestone is still pending, as is normal whole-path
pacing: late probe queues take93..98ms through capture and native writeback,
which is too slow for the intended20Hz target. Independent original equivalence
beyond the separately compared32 prefix remains open. Source changes leave
prior finite integration proof historical;45 merge first-parent/current exact
receipt transitions havezero gaps.

### Glyph backend caller state and cold coverage — 2026-10-10

`RS.glyph-backend-state` compares an independent integer, argument and coverage
model with unchanged `0x581ec0`: 27,974 matches, all 1,205 glyphs in six installed
fonts, 20,736 synthetic configurations and eight separate two-active-slot
diagnostics. Callee registers, RET16, defined ADD ESP40 flags, DF, source,
padding, XMM and MXCSR checks pass. Original zero/one finite active x87 values,
control words, tags and TOP remain intact in 27,966 cases.
`NR.glyph-backend-state-admission` records 16 atomic refusals under ASan/UBSan
and a freestanding PE32 model with no undefined helpers. Malformed native
requests are not executed by original code.

Cold coverage preparation multiplies by float constant `0x3c820821`, precedes
geometric skip and leaves the sentinel zero. Tint arguments mutate on rejected
geometry, Y becomes row-end/final-run state, and AX-only early returns retain
upper EAX. Original x87 status changes in 23,324 executions. Upward cold rounding
exceeds unit coverage at index63 in 3,495 cases; all eight selected two-active-slot
draws fault and change active values. These observations constrain the next glyph
entry, epilogue and FP admission. No live bypass or complete FP/Win32 equivalence
is claimed.

[Scope, results and limitations](glyph-backend-state.md) preserve unsuccessful
versions and the final prospectively declared execution. Committed history
`0191a55..577d293` has 51 file and 103 behavior transitions with zero receipt gaps.
Existing glyph pixels, byte production and live shadow evidence retain their
separate scopes. Higher text layout/reset/lifecycle and wider replacement remain
open.


Continuous rendering performance checkpoint (2026-10-10): the fixed source
channel now has explicit V2 CRC32C/V5 owned-descriptor guards, direct owned native
queue consumption, optional CPU reference verification, bounded producer scratch
and native source leases, and a private GPU/Qt full-path profiler. Historical
finite32 and64-queue results remain historical. Preliminary128 normal queues
complete correctly but still fail the unchanged19 FPS budget; the earlier4096
attempt timed out at observed generation3710. No current4096 delivery or normal
performance milestone is promoted here. See native-world-continuous.md and the
preserved source-bound experiments. Public campaign, independent original raster
comparison, physical display/input and uncaptured branches remain separate.
### Native glyph entry and x87 kernel — 2026-10-10

`RS.glyph-backend` now implements the guarded RLE/x87 kernel and an actual
owned-context handled entry with RET16. Unchanged pinned original comparisons
match 28,142 executions and 22,656,678 canvas WORDs: 27,966 native handles,
eight two-active-stack original-once fallbacks and 168 targeted guard forwards.
Pixels, tables, volatile/callee registers, rewritten arguments, stack guards,
defined flags/DF, x87 CW/full status/TOP/tag/occupied values and XMM/MXCSR match.
The native helper reproduces cold coverage and arithmetic under all twelve
precision/rounding controls, including upward cold coverage aboveone.

`NR.glyph-backend-admission` records 24 atomic ASan/UBSan refusals, seven-slot
physical FP admission, strict warm coverage and explicit owned context. Native
PE32 linkage needs only the embedding original-trampoline symbol, with no CRT.
[Results and preserved experiment versions](glyph-backend.md) distinguish
implementation, headless original comparison and pending live replacement.
Committed range `577d293..eb3cd18` has 18 file and 28 behavior transitions with
zero unresolved receipts. No live hook is installed: process-memory/lifetime
admission, bounded live capture/replay, GPU glyph takeover and higher text
layout/reset/lifecycle remain open.


### 2026-10-10 other pixel producers

- `RS.font-line-layout`: unchanged NoCD rectangle text4a5ce0, height-limited
  text4a6190 and measurement4a6630 match16,480fixtures and135,004,160RGB565words,
  full cursor/contour state, line triples and consumed pointers. Source/object/
  padding guards and9atomic native refusals pass withASan/UBSan. Owned SFT
  composition stages whole text operations. Bounded malformed-source admission
  is native policy; allocation/lifetime, arbitrary ABI/FP and higher-text live
  bypass remain pending. See [text layout](font-line-layout.md).
- `NR.canvas-image-production`: memory-DIB cropping now matches frozen independent
  Wine DC output instead of refusing smaller canvases. Positioned BMP uses the
  same owned quantization/cropping helper.288fixtures/5,387,712words and288atomic
  malformed-format refusals pass. This is current native integration against
  historical driver pixels, not fresh wrapper/driver execution or live bypass.
  See [image production](native-canvas-image-production.md).
- `RS.canvas-fade-span`: original58ed80 ignores pitch and modifies exactly
  2*floor(width/2)*height contiguousWORDs; formatzero selects565, nonzero555.
  Native physical-plane reconstruction and logical canvas projection match
  972fixtures/1,738,592physical+visiblewords.10atomic refusals andASan/UBSan pass.
  Private Lock/Unlock adapters bound the comparison; real driver failures,
  wrapper ABI and native writeback/suppression remain pending. See [fade](canvas-fade-span.md).
- `NR.movie-frame-production`: owned opaqueRGBA and full sequence hashes,
  explicit limitedBT.601YUV420P conversion, full playback and cancellation pass
  against independent source decoding:3,868frames/888,459,264pixels, zero channel
  differences, all five installed movies plus changing synthetic alpha frames.
  Reference normalization policy is explicit; other colorspaces/ranges,
  original DirectShow/placement/control and audible devices are excluded.
  See [movie pixels](native-movie-frame-production.md).
- `NR.movie-enabled-startup`: fixed native broker admission for original
  `.\fmv/intro0.avi`/intro1 paths; a single leading current-directory component
  is accepted while traversal/interior dot components remain refused. Both
  enabled intros complete natively (1,777+1,004frames), followed by at least
  three original published frames. Native media completion/skip/path/ABI
  regressions also pass. This is bounded live observation; original post-movie
  drawing, active World transition and full-session ownership remain separate.
  See [enabled intros](native-enabled-movie-startup.md).

Prospective source/contract/scenario bindings accompany the execution reports.
Failed and superseded experiments are preserved separately. Current comparisons
supersede only their declared scopes; shared-source edits leave older unrelated
surface/world/media evidence historical, without implying current equivalence.
The committed-history review from577d293through1abdfe3 checks43file transitions
and56affected behavior transitions, with no missing exact receipts. Complete
live drawing replacement remains unavailable: unclassified writers, wider
player journeys, admitted suppression/state preservation, recovery and sustained
World delivery remain explicit boundaries in [the replacement plan](../../docs/live-drawing-replacement.md).

The final source-bound [live producer shadow](native-canvas-producers-final-shadow-20261010.json)
passes four startup World returns, 954 completed canvases/422875312RGB565pixels,
2002 positive glyph calls and allGPUmirrors, zero differences. It exercises
source BMP/JPEG/PCX, copies, panels, fades, HUD rasters, minimap and markers.
Original producer bodies remain active; higher text state is isolated-comparison
only, and memory-DIB/positioned-image branches retain their separate corpus scope.


### Opt-in live glyph process route — 2026-10-10

`RS.glyph-live-route` supplies bounded original-active shadow and guarded
native glyph body bypass through the existing producer tag10. The final automatic
Quick Battle startup records1,817 admitted calls and39 original forwards per
mode, zero adapter errors, and four World returns. Six complete captured inputs
per mode independently match unchanged-original/native pixels, coverage and
selected caller/FP state across1,684,800 RGB565 WORDs. Retained inputs are cold,
black interior glyphs under PC53/round-nearest with empty x87; do not infer other
live tints/clips/controls from isolated coverage.

`NR.glyph-live-admission` separately implements per-call private ownership,
every-region read/write permission checks, original frame/canvas/table/argument
nonaliasing, aligned trial storage, format/clip/source/coverage/background
rechecks, thread/busy guards and a65,536-call budget. Production memory/refusal
fixtures and both normal/native-menu launch staging paths pass. Policy has no
original malformed/failure-equivalence or replacement status.

The fresh kernel/shared-entry regression passes28,142 original/native executions,
27,966 native handles and24 atomic ASan/UBSan refusals; historical records retain
their original source hashes. [Scope, wires, reports and launch](glyph-live-backend.md).
Activate explicitly with `MNM_GLYPH_BACKEND=shadow|takeover` before the Qt launcher.
Other rendering and higher text layout remain original. Whole Qt gameplay,
font/DC lifecycle, broader glyph live inputs, GPU-owned text/window composition,
sustained recovery and performance remain pending. Shared scene/launcher edits
leave unrelated historical evidence stale; no unrelated stages are promoted.

The non-glyph writer increment adds an independent opt-in native fade DLL, with
original Lock/Unlock retained. [Isolated caller-state proof](fade-loop-state-comparison-20261010.json)
passes19,440executions/3,888native completions/15,552original-once fallbacks and
17,541,440physical WORD comparisons. [Synthetic Win32 admission](canvas-writer-admission-comparison-20261010.json)
passes15atomic phase/page/budget refusals, default-off/invalid-mode/signature
checks,128completions and LastError preservation. [Installed fade takeover](fade-writer-live-comparison-20261010.json)
passes all25captured loops/12,000,000WORDs against independent unchanged-original
replay, with zero forwards. Its writable original lease is a bounded selected
Win32 assumption; full driver lifetime, arbitrary FP/failure/reentry and native
surface/frame ownership remain separate. The fallback trampoline preserves the
whole loop because its outer back-edge enters the patched entry; failed attempts
are retained in the experiment history. This DLL does not share the glyph hook.

[Panel domain proof](panel-domain-comparison-20261010.json) passes7,490cases /
2,160,320physical WORDs, including degenerate/edge rectangles, asymmetric corner
passes, percentage/boolean branches, padding and exhaustive whitewash values.
Nineteen atomic native panel refusals preserve previous pixels. RGB addition,
blend/wave/projected-shadow rasters and legacy hidden minimap history now preflight
all touched dependencies before mutation. [Current saved producer regression](other-producer-checkpoints-20261010.json)
passes19atomic writer refusals/two positive dependency cases, the Debug native
history/minimap fixtures, all954saved startup/HUD checkpoints/422,875,312pixels,
allGPUmirrors and clean surface retirement. This is fresh native replay of pinned
historical inputs, not fresh live producer attribution or glyph takeover evidence.
[Image regression](canvas-image-producer-regression-20261010.json) refreshes288
placements/5,387,712WORDs/288malformed-input refusals; [canvas fade refresh](canvas-fade-span-regression-20261010.json)
refreshes972original cases/877,072physical plus861,520logical WORDs/ten refusals.
Historical hashes remain unchanged. Live image/panel suppression, wider HUD/
cursor/tooltip journeys, unknown writers and complete rendering remain open.

The committed-history review covers `a47ad55..1fc60f4`, following the retained
reviews through `59608a4` and `a47ad55`. [Exact parent/current hashes and receipt
chains](coverage/other-producer-history-a47ad55-1fc60f4-20261010.json) find no
unresolved transitions. This retrospective review asserts neither a past gate
pass nor new execution validation. [Experiment history](other-producer-experiment-history-20261010.json)
retains unsuccessful attempts and earlier source versions separately from the
seven final execution records above.


### Default Qt glyph takeover — 2026-10-10

`tools/run-qt-shell.sh` now defaults `MNM_GLYPH_BACKEND` to `takeover` when
absent. Explicit `off` disables staging/authorization; `shadow`, `takeover` and
an explicitly empty variable retain their selected behavior. Standalone
comparison/capture tools, direct Qt binary invocation and the original
`run-game.sh` launcher retain their existing selection. Both normal and native
live-menu staging inherit the public wrapper's default.

`NR.glyph-default-launch` records this intentional launch policy separately
from recovered glyph behavior. The source-bound
[glyph default launch report](glyph-default-launch-validation-20261010.json)
executes the public wrapper with substituted build/Qt commands, checks five
selection cases, and tests both real staging/authorization routes for default,
off and shadow. Off stages no glyph DLL/import or evidence directory; active
routes pin the DLL and staged executable hashes. Original manifests are verified
before/after. This is headless launch integration, not interactive Qt gameplay
or new pixel/ABI/performance evidence. Existing primitive fallback/admission
budget and partial replacement boundaries are unchanged.

The launch helper's new explicit-off branch changes older live/shared-entry
reports' source fingerprints. Those immutable reports and historical status
remain intact; no current-code original equivalence is inferred from this
configuration test. Long sessions, font/DC/device transitions, full interactive
Qt journeys, wider drawing replacement and performance remain pending.

Committed-history accounting reviews `a47ad55..1fc60f4`: 44 file transitions and
90 behavior transitions, with one retrospective movie-contract receipt and zero
unresolved receipt gaps. This makes no historical gate-pass or fresh-validation
assertion. The report is retained under `coverage/`.

After the concurrent rendering batch committed, the additional range
`1fc60f4..95c1f93` was reviewed: 49 file transitions and 63 behavior transitions,
with zero unresolved receipt gaps. All 170 launch-test sources retain the
recorded hashes; the existing headless evidence remains current for this launch
policy. The census and exact change receipts are refreshed against `95c1f93`.


### DRAW-01 scenario-tagged route accounting — 2026-10-10

The [route register](drawing-inventory.json) and [checker/importer](draw01-inventory.md)
implement the missing drawing-inventory accounting. They separate exact original
entries, aggregate producer operations and raw queue kinds, with stable IDs,
behavior/owner/classification links, scoped support decisions, required effects
and explicit gaps. The initial reviewed census has 100 routes, 24 static windows,
52 producer hooks, one separately anchored interior fade hook and eight retained
live observation cohorts. Fourteen required
scenario censuses, four incomplete dispatch records and three unclassified-kind
support decisions remain open; full DRAW-01 completion is not claimed.

`TOOL.drawing-inventory` owns the accounting implementation/tests. New recovered
inventory entries preserve selected graphics hosting, the unassigned WORD-copy
window, raw kinds16/17/20 and unattributed callers without asserting new whole
function boundaries or recovered original/native equivalence. Historical source
hashes and original binary discovery boundaries remain unchanged.

The closed-journal importer checks V1/V2/V3 headers, pinned journal hashes,
record/payload/queue bounds and a matching successful completion marker. Runtime
module-base normalization is explicit; exact static CALL bytes identify a target,
while missing/nearby/external PCs stay unattributed. Importing the retained
four-return producer journal accounts for 954 checkpoints and exposes 63
unattributed caller/operation groups. This is re-indexing retained observations,
not a new original game execution, module-base proof or writer-completeness claim.

Twenty-nine offline guard tests cover new entries/operations/kinds, hidden signed
kinds, false/stale closure, exact and relocated caller PCs, malformed journal
bounds/markers, duplicate census/selector declarations and historical status
preservation. CI checks the reviewed inventory and the guards. No original
artifacts are consumed and no binary/native rendering implementation is changed.

Committed-history review of `a47ad55..48ca294` checks 103 file transitions and
170 behavior transitions against exact receipts, with zero unresolved gaps.
The report is retained at `coverage/draw01-history-a47ad55-48ca294-20261010.json`.
Earlier tool runs keep their historical hashes; final 29-guard and retained-journal
re-indexing results use new `TOOL.drawing-inventory.*-v2-20261010` evidence IDs.
The [updated coverage summary](coverage/summary-draw01-20261010.md) preserves
original comparison/replacement statuses and the incomplete census boundaries.


### 2026-10-10: DRAW-02 portable independent comparison corpus

[The identity-bound comparison corpus](draw02-independent-comparison.md) retains
five existing original-driver surface operations and four complete 800x600 World
consumer outputs with their owned drawing inputs. Native replay consumes no
installed game or original executable. Exact source RLE bytes are wrapped in
native SPR containers; expected World pixels remain comparator-only. Surface
expected after-planes are zeroed before native invocation. Every destination word
including borders is compared; reports bind capture sequence, input hashes,
canvas extents, native binary identities and mismatch coordinates.

`NR.drawing-comparison-corpus` owns this native comparison/retention policy. It
preserves `RS.world-frame.original-20261007` and existing surface evidence at their
historical hashes; no new original execution or recovered/live status is asserted.
World capture sequences 1..4 represent spaced consumer samples after 120 skipped
calls, not contiguous producer history. World canvases exclude subsequent window
composition; surface destination-before remains an operation fixture input.

The selected component corpus implements a bounded DRAW-02 exit gate. All
fourteen required full presented-scene journeys remain explicit pending scope,
including camera actions, active battles, tooltips/picking and recovery. Strict
`--require-complete` refuses a broad milestone claim. DRAW-03..07 and full drawing
replacement remain separate. Current native replay and refusal validation are
registered under a new execution ID; older source fingerprints are unchanged.


Fresh DRAW-02 execution (`NR.drawing-comparison-corpus.native-20261010`) matches
all 4,320,000 destination words with zero mismatches on Mesa llvmpipe; twelve
admission/oracle-isolation/strict-completion regression checks pass. Original
manifest verification passes before and after retention/replay work. The
unanchored DRAW-01 WORD-copy window retains its documented static scope, but
understanding remains unknown until a valid original range/entry link exists;
this corrects an audit-invalid recovered-status claim without adding discovery,
comparison or replacement evidence.


The clean-checkout follow-up
(`NR.drawing-comparison-corpus.clean-checkout-20261010`) repeats all nine native
comparisons with the current test sources and zero mismatches. All twelve guard
checks also pass in an isolated repository subset containing no working tree,
installed game or original media. The first execution remains immutable; its
prior test hash is explicitly historical after this test-portability correction.


## DRAW-03 bounded startup scene composition (2026-10-10)

`NR.drawing-scene` adds a widget-free build-specific adapter over native producer,
resource and GPU World services. Storage starts undefined; matching GPU World
output becomes retained native input before ordered post-World UI/copies. An
explicit source-sequence/queue/canvas/extent boundary must seal before native
read/presentation. Unknown inside-World work, gaps, undefined history, missing
resources and malformed boundaries poison the instance; cleanup is checked.

The portable fixture retains the exact map2 startup prefix through20458,16actual
encoded source assets and six independent original full-canvas oracles. New native
execution matches2,880,000RGB565 pixels with zero differences, including the final
HUD/minimap/markers/cursor composition. Eight portable guards and native order,
refusal and cleanup tests pass. See [scope and reproduction](draw03-native-scene.md).
Retention checks full historical input/source/output hashes and preserves the
original comparison report; this is new native execution against frozen original
outputs, not a new original-game execution.

The output boundary precedes the physical display copy. Native defined-pixel
history, copies, writeback/readback and cleanup are in scope; original consumer
non-pixel state and arbitrary readbacks, display-copy/Flip/driver failures,
source-state generation, picking/hotspots, general windows/tooltips, camera/battle
journeys, alternate dispatch and unbounded delivery remain pending. Native policy
implementation/headless integration are separate from recovered original
comparison, observation and replacement; no live mode or takeover is enabled.
Broad DRAW-03 completion is not claimed. The source census and exact receipts
record this increment without refreshing old evidence fingerprints.

The DRAW-02-only commit snapshot repeats all nine native comparisons with
zero mismatches across 4,320,000 RGB565 pixels, registered separately as
`NR.drawing-comparison-corpus.commit-validation-20261010`. All twelve guards
pass, and CI now runs that portable guard suite. Review of the subsequent
`48ca294..98b4524` commit checks 20 file and 38 behavior transitions against
exact receipts, including append-only journal objects, with zero unresolved
gaps. The report is retained at
`coverage/draw02-history-48ca294-98b4524-20261010.json`. DRAW-03 implementation
and its in-progress validation are excluded from this snapshot.


The committed range48ca294..98b4524 was reviewed separately from this uncommitted
increment:20file versions and75behavior transitions have exact receipt chains,
with no unresolved intermediate versions. The append-only journal check and exact
hashes are retained in
`coverage/draw03-history-48ca294-98b4524-20261010.json`. This history accounting
asserts neither a past gate pass nor new execution validation.

## DRAW-03 persistent source-owned frame publication (2026-10-10)

`NR.drawing-journey` extends the native scene adapter with explicit ordered
publication while retaining native producer/World history. Queue-zero menus,
subsequent-frame invalidation, immutable old GPU leases, terminal sealing,
refusal poisoning and cleanup have native tests. A portable schema2 fixture
retains exact contiguous inputs through44927,16encoded assets and17 independent
full800x600 original planes: two queue-zero completions and fifteen post-World
HUD/minimap/marker/cursor canvases. Large source history is bounded, hash-bound
XZ fragments; oracles never enter the renderer.

Fresh native execution matches8,160,000 checkpoint pixels with zero differences
and independently matches all17 published frames. The earlier six-canvas fixture
is rerun after the service extension and matches2,880,000 pixels. The eight scene
and four journey portable guards plus native publication/refusal/cleanup tests
pass. New immutable results are `draw03-persistent-journey-comparison-20261010.json`
and `draw03-persistent-scene-comparison-20261010.json`; prior evidence remains
historical. See [journey scope](draw03-native-journeys.md).

Native implementation/headless integration advance within this finite startup
scope. Physical presentation/driver behavior, movie composition, World
destination/extent changes, active camera/combat, picking/consumer state,
save/load/map/results transitions, recovery and complete Quick Battle/campaign
journeys remain pending. Broad DRAW-03, continuous delivery and live replacement
are not claimed. Committed f60ec65..3b6785a has a separate exact receipt-chain
review with no accounting errors, retained in
`coverage/draw03-history-f60ec65-3b6785a-20261010.json`.


## DRAW-03 native World destination transitions (2026-10-10)

The source-owned adapter now separates global original queue identity from its
private World resource generation. Release/recreation, another destination ID
and changed extents reset owned World resources without discarding native canvas
history or cumulative queue/raster counts. Synthetic tests cover same-ID and new-ID
recreation, switching between defined destinations, ordinal repetition/gaps and
cleanup. Fresh startup comparisons match all17 full retained canvases (8,160,000 pixels)
and six regression canvases (2,880,000 pixels), with zero differences; all17 frame
publications also match. Native transition and portable guard tests pass. Reports
`draw03-transitions-journey-comparison-20261010.json` and
`draw03-transitions-scene-comparison-20261010.json` supply new evidence IDs; the previous results remain historical. Original destination/map-transition
comparisons, full journeys and rendering consumer state remain pending.

The exact committed range3b6785a..7127121 is reviewed separately in
`coverage/draw03-history-3b6785a-7127121-20261010.json`; this accounts for parent/current
receipt chains without claiming historical execution or a past gate pass.


## DRAW-03 independent raster return comparison (2026-10-10)

A separate native World raster result interface now exposes sampled low16 AX
with exact source/global queue/canvas/backend identities. Original raster return
observations are removed before native replay; the runner rejects nonzero
observed returns and emits a hash-bound result transcript. External comparisons
check state separately from full canvas pixels. Unknown contracts, provenance,
identity errors and mismatches remain explicit. Picking, node mutations, flags,
scratch globals and complete rendering consumer state remain pending.

The historical full c4ps704z run contains a selected native bypass at sequence45634,
queue16, whose EAX0xfffffff8 is not original return evidence. Both retained native
prefixes end before this operation. State comparison refuses histories reaching
a recorded bypass; this does not revise historical independent pixel results.
The exact committed range7127121..fb79078 is reviewed separately in
`coverage/draw03-history-7127121-fb79078-20261010.json`.

Fresh executions independently match25,883 World low16 results in the17-frame
prefix and1,964 in the six-canvas regression, with zero differences or unknown
returns. Full checkpoint pixels (8,160,000 and2,880,000) and all17 publications
also match. Native and eight scene/four journey/four result guards pass. New
immutable evidence is `NR.drawing-raster-results.native-20261010`, alongside
`NR.drawing-scene.raster-results-native-20261010` and
`NR.drawing-journey.raster-results-native-20261010`. Historical fingerprints
remain unchanged; full DRAW-03 remains pending.

### 2026-10-10: DRAW-02 full session acceptance denominator

The isolated DRAW-02 branch adds explicit gates for all fourteen drawing journeys
and twenty-one operation/failure contracts. Reference acquisition and native
parity are separate axes. Only contiguous, action-bound, independently captured
presented frames can close journey gates; operation/World/pre-display composition
passes remain bounded comparisons. Synthetic accounting guards exercise this
policy without claiming new original observations, rendering parity or takeover.
The initial suite pins the existing component and startup-scene catalogs. Complete
presented sessions and operation/failure acquisition remain pending. See
[full session comparison](draw02-full-session.md).

### 2026-10-10: DRAW-02 full retained sixteen-queue comparison sequence

The portable corpus now preserves all46,647 contiguous producer records,
16World queues,16encoded source assets and1,062independent original checkpoints,
covering474,236,912RGB565 pixels. Lossless bounded chunks and same-canvas oracle
deltas preserve exact historical hashes while keeping portable storage bounded.
Native replay receives only owned producer/source inputs; full output planes and
completion identities are compared externally. A first full native replay matches
all checkpoints with zero mismatches; final source-bound delta replay is recorded
separately. Historical one-raster bypass provenance remains explicit. This does
not establish presented frame/action continuity, new original execution, caller
or driver failure behavior or any complete session journey.


## DRAW-03 integration of the complete DRAW-02 startup corpus (2026-10-10)

Committed DRAW-02 acceptance and1062-plane retention increments are merged into
`draw03-journeys`, preserving both branches' findings, independent stage statuses,
immutable results and append-only receipts. Native transport strips only observed
raster EAX fields; stored producer bytes and original hashes stay unchanged.
Full retained history receives pixel comparison without a return-state comparison
across its recorded original-body suppression. The17-frame original-active
return-state prefix remains a separate validation domain. Full presented journeys
still require new original collector/publication/action contracts.

Exact committed parent/current receipt reviews are retained for3b6785a..faa862a,
faa862a..e4ff085 andfb79078..6208fcc. Imported historical validation remains
historical; the merge does not assert a past gate pass or new original execution.

Fresh integrated native execution matches all1,062 checkpoints and474,236,912
RGB565 pixels with zero differences and no remaining native surfaces. New
immutable evidence is `NR.drawing-sequence.draw03-native-20261010`; ten sequence
guards and eight acceptance guards pass. Original raster returns remain external
and the full sequence makes no state-parity claim across historical suppression.
The original-active17-frame result comparison remains separate.

### 2026-10-10: DRAW-02 unified suite and presented-session comparison ABI

A single native suite now runs the nine component outputs, six selected composed
startup outputs and all1,062retained sixteen-queue checkpoints, while separately
reporting original-reference and parity gates for14journeys/21operation contracts.
Presented-session admission requires independently registered original capture,
contiguous publication identities, ordered action/frame/owned-input bindings and
caller-visible effect results. RGB565/RGBA full-plane comparisons and expected
results remain outside backend inputs. Portable session retention refuses changed
provenance/overwrite; unsupported and native errors preserve independently
admitted references without claiming parity.26synthetic guards exercise these
contracts, including wrong returns despite equal pixels and dropped/reordered
frames. Existing collectors do not yet emit complete presented-session records;
all14full journey and21operation/failure acceptance gates remain pending. No new
original execution, full scene replacement or live bypass is claimed.


## DRAW-03 integration of the unified DRAW-02 suite (2026-10-10)

The newly committed391a903 suite is integrated into the isolated native branch.
Both CI guard sets, central behavior/evidence/scenario links and append-only
receipts are preserved. Current native executables exercise the three bounded
catalogs under one command; typed presented-session admission/refusal remains
synthetic until a collector emits actual publication/action/effect evidence.
Full presented journeys, native session decoding and complete consumer state
remain pending. Exact parent/current reviews covere4ff085..391a903 and the prior
native integration6208fcc..1472bc0 without asserting historical execution.

Fresh current-source unified execution matches all three bounded catalogs over
481,436,912 compared pixels, with zero differences. Both journey/session guard
suites pass. New immutable evidence is
`TOOL.drawing-journeys.draw03-native-20261010`; all14full journey and21operation
acceptance gates remain open. Current source/result separation and all historical
reference/pixel/state boundaries remain explicit.

The final merged-suite execution registers all three current policy contracts as
`TOOL.drawing-journeys.draw03-complete-native-20261010`:481,436,912pixels,
zero differences and26passing journey/session guards. The preceding sandbox
execution could not start its private X servers; its separate unchanged report
and `TOOL.drawing-journeys.draw03-sandbox-failure-20261010` preserve all three
admitted original references and claim no native parity or implementation
validation. The successful retry used private Xvfb/OpenGL processes outside
that socket restriction. All14presented journeys and21operation/failure gates
remain open. The original manifest verifies all2927immutable files unchanged.

## 2026-10-11 DRAW-02 original primary publication acquisition

Opt-in guarded opaque-copy COM return observation and independent primary
RGB565 readback now extend the producer capture through the final HUD/cursor
publication. The first one-return diagnostic compares89actual primary planes /
42,720,000pixels with zero differences from source-only native composition.
Twelve parser guards retain readback losses and refuse forged/partial records.
Original drawing remains active. Alternate primary routes, applied action/frame
identities, complete journeys, caller failure/recovery equivalence and live
suppression remain pending. See[scope and procedure](draw02-publication.md);
this does not change the fourteen-journey/twenty-one-operation denominator.

## 2026-10-11 DRAW-02 applied menu dispatch and camera reference

Publication V2 adds same-thread original menu adapter entry/return/result records
bound to actual COM attempt and next frame ordinals. Rejected/unfinished actions
remain distinct from success; physical camera injection is checked against
original owned minimap centers and independent World orientation snapshots.
The initial16-return capture records103primary planes, five successful menu
requests and all four World orientations. Native replay first refused its237MiB
V2 journal at a128MiB CLI read limit; the negative result is retained and the CLI
now follows the existing512MiB V2 envelope. Current-code native parity requires
a new run; complete14journey/21operation gates remain open.

The fresh prospectively declared sixteen-return camera run now passes independent
source-only native comparison of all103primary planes /49,440,000pixels, with
zero differences, no readback losses and complete native cleanup. Its266MiB V2
owned journal is admitted within the512MiB envelope. All four original World
orientations are observed separately from the minimap's0/2orientation set; actual
minimap camera centers change. Five original menu adapter requests have matching
entry/return/result phases bound to frame/call ordinals. Twenty-one parser guards,
the native drawing-scene ownership/refusal suite and the disabled original-bytecode
menu/battle fixture pass. Source-state observations improve camera/action
attribution; complete publication routes, full journeys and original failure/
recovery/caller semantics remain unadmitted.

## 2026-10-11 DRAW-02 portable primary references and suite integration

A new portable publication corpus losslessly retains the original-active camera
run's complete266MiB V2 producer journal in nine bounded chunks, seventeen encoded
source assets, exact COM/action phase traces and103independent primary planes.
The suite now exercises this source-only native replay alongside the prior three
component/composed/history catalogs. Its explicit`observed_primary_blt` boundary
contributes partial independent references while retaining all14full presented
journey and21operation/failure requirements. Full route/input/viewport/transition
and caller-result/failure/recovery coverage remains pending. Thirteen corpus
guards and the added suite boundary guard prevent oracle inputs, malformed traces,
wrong provenance and premature completion claims.

## DRAW-03 native session lifecycle — 2026-10-11

`NR.drawing-session` adds widget-free native session/segment/frame orchestration
over `DrawingScene`. Explicit transitions start undefined source histories;
complete frames carry generation identities and independent GPU leases. Running
faults discard owned resources, terminal completion and idempotent shutdown are
separate, and fresh recovery cannot reuse an old session ID. Native bounded
lifecycle fixtures and the scene regression have their own execution evidence;
they do not validate original gameplay actions, transition/failure behavior or
full Quick Battle/campaign journeys. DRAW-01/02 collectors and acceptance gates
are unchanged. The changed shared scene build file makes earlier build-bound
evidence historical; existing source hashes and receipts remain unchanged.

### DRAW-03 owned native session runner (2026-10-11)

`NR.drawing-session` now exposes a reproducible, widget-free owned-input runner
over the native session controller. `mnm-drawing-session-replay` admits the
intentional native v1 declaration, closed producer streams, encoded source asset
hashes and every independently read SPR catalogue source. It executes ordered
native stages, publishes complete RGB565 frames with immutable session identities,
refuses malformed or contaminated input, rechecks sources and shuts down storage.
This is a native lifecycle policy; it does not implement the distinct DRAW-02
presented-session ABI, original actions/caller-state, movie composition or live
replacement. The fourteen-journey and twenty-one-operation gates remain open.

Fresh `NR.drawing-session.runner-native-20261011` records 12 actual OpenGL lifecycle
cases, the existing scene regression, and 30 owned CLI cases: six complete native
planes/99 pixels across six segments, independently expected synthetic SPR/BMP
composition, repeated publication, interrupted World cancellation, 29 binding,
catalogue, schema, identity, boundary, containment and result-contamination
refusals. Cleanup and actual execution failure paths leave zero native surfaces.
Source/binary/log hashes and prospective scope/scenario claims are retained.
The earlier lifecycle evidence is preserved unchanged and is historical after
the runner/build/contract changes; neither result claims original equivalence.

Reviewed committed range `53bdc4a..9b15da3` is retained separately from the new
uncommitted source review, with no unresolved exact-receipt gaps. Shared scene
CMake edits leave previously bound drawing evidence historical; fresh native
policy execution does not refresh those original results. The source census and
new links are reviewed without changing the discovered binary inventory.

The main checkout gained a working-storage guard concurrently. Its default
200 GiB preflight refused existing usage; the user explicitly authorized a
temporary 1200 GiB ceiling for this bounded validation, a 0.1 GiB reservation
and the unchanged 100 GiB free-space floor. The guarded execution passed with
about 1125 GiB existing working usage and 203 GiB free. The task-local policy
was retained under working/tests/draw03; the repository default was not changed.

## GPU frame ownership milestone (2026-10-10)

`NR.world-gpu-resident-frame.history-32-20261010` proves native GPU canvas history over 32 retained World queues (66,529 raster requests, 15,360,000 RGB565 words), zero delivery/World readbacks, separate diagnostic comparisons and zero retained surfaces after teardown. The 512-frame synthetic and legacy controls also pass. This is source-only replay against an independently composed native CPU reference, not independent original pixel equivalence or live acknowledgement delivery. Actual original World writebacks and real DirectDraw remain active in the existing live mode.

Fresh shared image, fade and font regressions pass after canvas mirroring. Glyph live shadow/takeover and installed movie-frame decoding pass on the merged source. Enabled startup intros remain pending: the explicit enabled-preferences run-d6sbsph2 produced no media requests and timed out; its report is retained. Historical evidence has not been rewritten. Next milestones are versioned acknowledgement-only live delivery with zero GPU exports/original World writebacks, followed by replacement DirectDraw factory/interface/storage/lifecycle/presentation services with real DirectDraw disabled. The 19 FPS target and whole-session gameplay remain unproved.

## V4 resident delivery checkpoint (2026-10-10)

`NR.world-gpu-ack-delivery` has bounded live observation:4,096 original/native returns and native Qt presentations,6,770,930 raster requests, zero native readbacks/pixel replies/original World writebacks, zero retained surfaces and bounded caches/RSS. Independently owned CPU source patches now use sparse GPU coverage rather than thousands of row uploads. The current512 synthetic/legacy +32 retained-input native CPU/GPU history comparison passes separately; it does not assert independent original pixel equality. Both producer/consumer death at generation34 refuse terminally and a fresh128-queue recovery completes.

Performance remains failed:16.1786FPS,44ms median/58ms p95/664ms maximum original World duration in the long run. Neither19FPS nor60FPS is promoted. Original DirectDraw and outside-World original drawing remain active; the original CPU World canvas is stale after a resident ACK, so arbitrary original pixel reads/picking and whole-session gameplay are separate unproved boundaries. The launcher now admits only the exact selected V3 or V4 immutable envelope and mode. All failure/partial performance reports remain in the experiment-history catalog. The committed597005a first-parent range has174 exact file transitions with matching receipts; its journal is preserved without retrospective test assertions.

The first isolated native PE32 factory proof now passes128 lifetimes/2,176 selected caller words against real Wine DirectDraw with per-interface reference counts and canonical IUnknown identity. The native probe has ddraw disabled, no module loaded, no DirectDraw imports and zero retained factory owners. A null output pointer faults the original Wine factory and is excluded from the common valid-caller corpus; native safe refusal is a separate policy. Native preserves incoming LastError, while this Wine/Xvfb reference exposes1400; that difference is recorded explicitly, not treated as equivalence. Surface methods currently refuse terminally and the factory is not installed in the game. Full real-game DirectDraw retirement remains pending.

## Native DirectDraw retirement preparation —2026-10-10

The native PE32 graphics slice now admits Draw4 and owned offscreen RGB565
Surface1/2/4 services. The current isolated64-case comparison matches2,048
caller words and37,440 full pixel words against Wine11.16, with ddraw disabled
and absent in the native probe. Eight native refusal policies preserve117 words;
512 simultaneous tiny surfaces enforce the capacity and release all owners.
Observed eight-byte pitch and per-interface reference counts correct retained
initial mismatches. This does not execute original game instructions or retire
its backend. Primary/flip/clipper/DC/palette, native GPU history CPU dependencies,
actual original-game loaded-module refusal and public gameplay remain pending.

The separate offline import preparation passes four pinned PE32 staging
compositions, six original-byte mutation guards and four duplicate staging
refusals. Original sections and graphics IAT bytes/addresses remain unchanged;
only copied import metadata selects MnmDraw.dll and its anchor. No loaded-module
or live replacement status follows from this offline preparation. Original
artifact manifests pass before/after all artifact-consuming checks. The prior
native GPU4096-frame delivery remains16.18FPS and fails19FPS; no performance
status is promoted by isolated storage or import preparation.

## Native graphics startup service admission —2026-10-10

Current isolated clipper/DC service comparison passes64 cases,2,048 caller
fields and22,464 complete COLORREF/RGB565 words against Wine11.16, including
DC-during-Lock, repeated DC and Lock/refusal, GDI edits, clipper references and
zero-resource teardown. A separate native primary/backbuffer policy validates
351 words across flip and surviving child lifetime, plus duplicate primary
refusal. Current-source128-case factory and64-case storage regressions also
pass; previous reservation/link failures remain retained. The common PE32
builder checks complete compiler/builder closure and rejects DirectDraw imports.
All native probes disable ddraw and verify no module is loaded. This is an
isolated startup-services milestone, not real game/driver retirement; clipper
regions/occlusion and clipped Blt, palettes, unknown methods and actual backend-
disabled game execution remain pending. Logical modes/primary/flip remain native
policies, independently separated from original driver behavior.19FPS/60FPS
remain unproved; the earlier4096-frame delivery's16.18FPS still fails19FPS.

## Backend-disabled native graphics session —2026-10-10

The selected128-frame actual original-engine session completes on TITAN RTX
through MnmDraw owned PE32 RGB565 storage and native Qt GPU presentation, with
Wine ddraw disabled/module absent and no terminal unknown method. V4 exports
zero World pixels/readbacks/replies/writebacks. Original outside-World drawing
continues. Two guarded dynamic loader strings add24 data-byte exceptions to the
import-only staging; all other original bytes/instructions remain unchanged.
Fresh offline proof passes eight compositions/ten mutations/eight duplicates.
The expanded isolated service proof adds16 startup caller cases/256 words
matching Wine, including NULL normal cooperative level and Surface3/4 identity,
plus separate mode/memory/status/unknown-route native policies. Earlier isolated
reports retain their source hashes; fresh regressions are registered separately.

Performance remains failed:17.6600 original FPS,17.8759 Qt FPS,560 peak GPU cache
frames against512. The39ms median/53ms p95/72ms maximum World durations and
114,688-byte RSS growth do not excuse the failed FPS/cache gates. Further work
must reduce actual cost and enforce the cache limit, then repeat long-run and
peer-death/recovery testing. Original pixel equivalence, adapter-enumeration live
reachability, palette/clipped blits/unknown contracts, semantic quit, arbitrary
combat/casting and60FPS remain separate pending boundaries. The real game uses
saved device settings and does not enumerate adapters in this selected session.
No whole-engine replacement or performance promotion is asserted.

## Bounded cache and CPU validity performance increment —2026-10-11

The CPU World validity path crops non-shadow opaque invalidation once while
preserving transparent history;2,940 independent cases cover five modes, three
backends, signed extremes and clipped/inverted bounds. Eight selected native
tests pass with94 reviewed canonical compiler inputs. Metadata pressure now
evicts the least recently drawn sprite entry while retaining hot neighbours
on shared pages; pixel pressure retains bounded whole-page eviction. Scoped
World sessions enforce the existing512-frame atlas cap. Fresh512 synthetic and
512 legacy-control queues plus32 retained game queues match15,360,000 independent
native CPU words, with zero delivery readbacks and no remaining GPU resources.

The fresh actual128-frame TITAN RTX run completes with ddraw disabled/absent,
all128 Qt presentations and zero World readbacks/replies/writebacks. Peak cache
335 passes512; RSS growth1,474,560 bytes passes32MiB. Performance still fails:
16.9312 original FPS/17.1461 Qt FPS, World median43ms/p9551ms/max61ms. This
scene has2,108 final-frame rasters versus1,563 in the previous retained run, so
its20.49ms native consumption is not a controlled speedup comparison. Producer
capture15ms/9,516 region queries and14ms between-World work remain charged.
The failed target/build/dependency-label proof attempts are retained separately;
only the final source-bound pass is current evidence.19FPS,60FPS, backend-disabled
long-run/death/recovery, full gameplay and semantic quit remain pending.

A separate current-source CPU sample records527 samples/zero lost and3.01percent
in CanvasSequence::raster. It reaches19.5003 original/19.7034 Qt FPS in another
scene, but577ms cold delivery fails500ms. This is a retained profiling discovery,
not a full performance pass; source and scene variation prohibit a controlled
percentage-speedup claim.

The unprofiled4,096-frame actual DirectDraw-disabled session then completes
all4,096 original returns/native completions/Qt presentations, with zero World
readbacks/pixel replies/writebacks and zero native GPU teardown resources. Peak
cache321, RSS growth9,097,216 bytes, World median38ms/p9544ms/max66ms and
cold492.16ms pass unchanged bounds.18.6534 original/18.6580 Qt FPS still fail19;
this is a functional sustained retirement slice, not a performance pass or
original semantic COM quit proof.

## DirectDraw-disabled peer failure and recovery —2026-10-11

Both private consumer-death and original-producer-death controls pass at
outstanding generation34 with ddraw disabled. Each requires the expected
terminal refusal, no successful completion claim, stopped private Wine,
unchanged MnmDraw.dll and96-byte diagnostics showing native factory/surface
startup, real module absence and bounded resources. These are forced private
process controls, not original semantic COM Quit.

A fresh128-frame session after both controls completes all original returns
and Qt presentations with zero World readbacks/replies/writebacks and zero
GPU teardown resources. Every unchanged budget passes:19.5181 original/19.7385
Qt FPS, World median35ms/p9539ms/max44ms,cold447.63ms,cache306 and RSS growth
274,432 bytes. This establishes bounded fresh-session recovery/performance;
previous4,096-frame18.6534FPS failure remains retained. Arbitrary-scene sustained
19FPS,60FPS, original pixel equivalence, semantic Quit and public menu/gameplay
DirectDraw-disabled coverage remain pending.

The sustained current-source4,096-frame recovery then passes every unchanged
performance/resource gate:19.5419 original/19.5453 Qt FPS, warm World median36ms/
p9540ms/max61ms,cold439.45ms,cache366,RSS growth9,068,544 bytes. All4,096
original returns/native completions/Qt presentations complete with zero World
readbacks/replies/writebacks and zero GPU teardown resources. This proves the
selected sustained19FPS milestone with the real backend absent; the retained
other-scene18.6534FPS failure prevents an arbitrary-scene performance claim.
The public native menus/campaign route still uses its older graphics bridge;
its backend-disabled integration,60FPS and full gameplay/semantic Quit remain
separate outstanding retirement work.

## Public native menus with owned graphics —2026-10-11

The opt-in public Qt Launch/New Game/Region Enter/campaign/Escape/Mini Cancel/
resume journey passes with composed MnmDraw/MnmMenu/MnmRender, Wine ddraw
disabled/module absent and no unsupported native method log. Same-thread
original callbacks/World initialization/resume, native command paints and
screenshots remain required.227 Qt compiler inputs,8 native PE32 compiler/
builder sources and1,111 immutable executable/DLL/graphic inputs are bound
and stable; original manifests pass before/after.18 existing fail-closed report
checks pass. Builder failure and earlier source-only/provenance increments
remain separate historical records.

This route retains original CPU drawing and does not claim V4 World-body
suppression. Public stress throughput/casting/combat/semantic Quit, native
graphics defaults and the other ordinary OpenGL launch route remain pending.
The separate selected4,096-frame TITAN V4 run's19.54FPS/zero World pixel
roundtrip proof remains intact; public basic flow is not a throughput claim.


The2026-10-11 [public gameplay proof](native-draw-public-gameplay-pass-20261011.json) completes a difficulty3 campaign through native menus/command presentation with real ddraw disabled and absent. Successful Zombie casting, player melee/lethal enemy depletion, physical movement, blocked spell attempts, player Fireball, nine portrait recenter cycles, Mini/resume and normal original Quit pass. The19FPS aggregate stress gate passes: combat command/paint36.7755/25.6537FPS over115.539seconds; resumed59.8363/34.6316 over60.465seconds. Worst whole-flow window11.9266FPS remains explicit. Stable portrait interior17.99seconds gives20.0111FPS, worst19.2484 and maximum channel error1. Compiler/source/input fingerprints remain stable;18 validator checks and original manifests before/after pass. Forced termination does not supply this normal-Quit proof, and normal Quit does not assert semantic COM reference cleanup.

The new `test-native-campaign.py --hardware` creates and stops a private NVIDIA display and refuses software fallback. [Hardware failures](native-draw-public-hardware-negative-artifacts-20261011.json) remain failures: difficulty1 native and real-DD control both fail summoning with stopped World snapshots; their activation cause is unconfirmed. Difficulty3 advances World but reaches a50ms bounded copy-order timeout (observed64/65ms), invalidates history and refuses GAP/falls back. No native graphics unknown-method record appears; absence alone cannot establish successful gameplay. NVIDIA public gameplay/defaults and the other OpenGL launch route remain pending. This public route still executes original CPU drawing; the independently proven V4 World pixel-roundtrip removal remains a separate selected19.54FPS milestone.


2026-10-11 bounded copy contention yielding: `Sleep(1)` preserves50ms refusal/original-once policy and passes eight independent PE32/GPU fixtures (13 full-frame comparisons;4 successful/4 refused sessions). The fresh [private NVIDIA public campaign](native-draw-public-nvidia-gameplay-pass-20261011.json) passes full casting/combat/movement/spells/portrait/Mini/resume/normal Quit with real ddraw disabled/absent, stable sources/compiler/1111 graphics inputs, original manifests before/after and no fallback/recovery. Combat command/paint33.3999/25.1262FPS; resumed59.8223/31.5958. The aggregate19FPS gate passes while worst resumed18.2648FPS remains visible. NVIDIA/zink acceleration is checked by glxinfo; final Qt identity is cleared on normal Quit and should be persisted. Prior hardware failures stay historical. No controlled speedup, universal19/60FPS or native World body/V4 integration claim. Public native defaults and the other OpenGL launcher still require retirement and fresh proofs.


2026-10-11 public native menu defaults: native menus+command launches select owned graphics without MNM_NATIVE_DRAW opt-in; explicit0 remains a diagnostic legacy control. The [current default basic TITAN proof](native-draw-public-default-basic-pass-20261011.json) passes Main/Region/campaign/Escape/Mini Cancel/resume, native frames/paints, actual Qt zink/NVIDIA TITAN RTX identity, disabled/absent real ddraw, canonical compiler/input/source stability and original manifests before/after. The smoke automatically saves prospective claims before building/launching when omitted; renderer identity survives test teardown.18 validator checks pass. The [first extended default run](native-draw-public-default-gameplay-defeat-20261011.json) completes casting/combat/portrait but later has an inactive zero-health wizard and fresh Main, correctly failing the required journey; it is not a rendering pass or an engine workaround. Current default stress/normal Quit and the other ordinary OpenGL launcher remain outstanding. Earlier full NVIDIA gameplay remains historical after these source/contract changes; no old hashes are refreshed.

## Default standalone owned graphics startup — 2026-10-11

`NR.directdraw-native-opengl-launch` adds fresh bounded proof through the actual
standalone OpenGL staging/runner path: default MnmDraw, real ddraw disabled and
absent,20 native main-menu frames on actual Qt/NVIDIA TITAN RTX, no ordinary GPU
readback/upload/recovery, compiler/builder/generator/input fingerprints stable
and consumer resources zero after bounded abort. The native Qt menu/command
route already defaults to owned graphics. Legacy diagnostic launchers retain
explicit original comparison support. Gameplay/FPS/movies/normal Quit and
semantic original COM teardown are not asserted by standalone startup.
The full default campaign rerun failed presentation/recovery and remains a
negative result. Prior gameplay/performance evidence retains its original
hashes and separate scope. The f874d7b parent/current receipt audit covers11
committed transitions with zero missing receipts; no retrospective execution or
past gate claim follows. [Details](native-draw-device-surfaces.md).

## Bounded atomic-owner wait fixtures — 2026-10-11

`NR.bounded-copy-order` preserves the50ms policy while replacing contended
Sleep polling with atomic address wait/release wake. The Kernel32 wake stub
failure is preserved; KernelBase is used on modern Windows/Wine. Eight fresh
copy/lock/DC/flip fixtures pass independent complete pixels, return/LastError,
original call counts, four successful sessions/four expected refusals and native
cleanup. Live default NVIDIA gameplay is validated separately. Earlier shared
source results retain their hashes and pending freshness. The eb52c3a commit
receipt audit covers19 exact transitions, zero missing receipts, without a
retrospective gate or new execution claim. [Details](opengl-copy-order.md).

## Full default native NVIDIA campaign — 2026-10-11

`NR.directdraw-public-native-menu` has fresh full default (no opt-in) execution
on actual Qt/NVIDIA TITAN RTX: successful casting/combat/movement, spell refusal
and Fireball, nine portrait cycles, Mini/resume and normal Quit, real ddraw
absent and no unknown call/recovery/fallback. The19FPS gate passes with
25.2133FPS combat paint,31.1704 resumed paint and worst measured gameplay
window19.9456FPS. Compiler/source/installed inputs remain unchanged. Earlier
long-ownership failures are retained; stage diagnostics do not establish a
controlled fix or universal19/60FPS. Original CPU drawing in the public flow
and separate V4 native World delivery remain distinct. [Fresh evidence](native-draw-public-default-full-nvidia-pass-20261011.json).

The final strict public rerun additionally binds44 rendering compiler/builder
and28 generator/schema inputs prospectively. Full casting/combat/portrait/resume/
normal Quit passes with no fallback/recovery/real ddraw and active gameplay paint
windows >=19.8610FPS. The separate current4096-World result fails full-path FPS:
40ms median World work plus14ms between World frames,8941 region queries/frame.
All deliveries and paints complete, but performance remains a failure. These
milestones are recorded independently. The d29a37c receipt audit covers14 exact
transitions without gaps or retrospective execution claims.


## Fog readability window and current4096 performance (2026-10-11)

Continuous tag45 fog copying resets a checked readable-memory window for each
admission. It reuses region protection proof only within that synchronous loop,
with no original callback or retained cross-queue pointer proof. Fog bytes and
signed comparisons stay unchanged; legacy noncontinuous checks stay per-cell.
The independent memory fixture admits8192 bytes using2 region queries versus
8192 fresh checks, and refuses guard/uncommitted/null/overflow boundaries and
protection changes on the next admission. Assertions and ASan/UBSan pass.

The source-bound [4096 run](research/runtime/native-fog-window-4096-pass-20261011.json) passes all
unchanged budgets on actual TITAN RTX accelerated Zink:19.8825 original and
19.8868 painted FPS,cold451.04ms, all-warm4033 World samples median36/p9540/max67ms,
cache492/512,RSS growth9,388,032 bytes. All4096 returns and paints are checked;
coalescing, pixel replies, original World writebacks and GPU World readbacks are
zero. Real ddraw is disabled/absent. Last1023 region queries median7635; differing
scenes preclude a causal paired speedup claim. The earlier128 run failed its
unchanged500ms cold limit at535.73ms despite warm19.80/19.98FPS and complete delivery.

A subsequent full public campaign separately fails copy-order admission: its
surface_lock holder reached105ms and longer. This does not invalidate World
transport measurements or establish robust public gameplay. Public V4 fusion,
arbitrary-scene19/60FPS, original pixel equivalence and semantic COM Quit remain
separate unfinished milestones. Historical hashes and failures are preserved.

The native diagnostic-file increment isolates a reproduced rendering-lease stall
inside Wine CloseHandle (native status stage5,76..271ms) and moves handle closure
out of drawing callbacks. One startup-owned handle publishes unchanged96-byte v2
records under nonwaiting diagnostic writer admission; per-call module checks,
LastError and terminal error policies remain. Independent1024-overwrite assertions
and ASan/UBSan pass. Fresh default public TITAN casting/combat/spell/portrait/resume/
normalQuit passes19FPS with no real ddraw, fallback or recovery. Source-bound
historical World19.88FPS remains; current4096/fault revalidation and repeated
transition/copy/DC/flip checks are pending. This does not integrate V4 World into
the public command path or establish allscene19/60FPS/original equivalence/COM Quit.

Current status-file three-cycle TITAN repeat passes19FPS and normalQuit, with no
copy timeout/recovery/fallback. Eight current copy/Lock/DC/Flip fixtures preserve
13 full-frame comparisons and expected50ms refusals, zero ordinary readback/upload/
teardown surfaces. Actual selftest compiled closure is42 (older stage-fixture
prose44 was an overcount; immutable mapping retained); production bridge is44.
64 current native service comparisons preserve2048 caller/22464 pixel words and
351 policy words against Wine DirectDraw with native ddraw disabled/absent and
zero native resources. Current World4096/fault revalidation remains pending.

Current source-bound4096 TITAN World delivery after the diagnostic-file fix passes
all unchanged19FPS/resource gates at19.9697 original/19.9731 painted FPS,cold459.699ms,
4033 warm World samples median35/p9540/max60ms,RSS growth11,182,080 bytes. All4096
returns/completions/paints,zero coalescing/World readbacks/pixel replies/writeback/
native GPU teardown surfaces and real ddraw disabled/absent are checked. Both
current producer/consumer deaths refuse at generation34; a subsequent fresh128
session passes19.7653/19.9622FPS,cold472.004ms and all other budgets without slot
reuse. Actual standalone default OpenGL launcher also passes21 native menu frames
on actual TITAN Zink,zero ordinary readback/upload/recovery and zero bounded-abort
consumer resources. Default public native menus/command gameplay has separate
fresh full casting/combat/spell/portrait/resume/normalQuit and three-cycle proofs.
The public route still draws World on original CPU; public V4 fusion,full standalone
gameplay/movies/FPS/normalQuit,allscene19/60FPS,originalpixel equivalence and semantic
COM cleanup remain separate milestones. The observed original50ms frame cadence
is still a boundary to unique60FPS World updates. Historical failures/hashes remain.

### Native fade row damage and canonical dependency reporting (2026-10-11)

`NR.canvas-fade-row-damage` records one damage update per affected fade row,
with complete unknown-pixel prevalidation before any mutation. Independent
original58ed80 comparison passes972 cases/1,738,592 words;126 mirror projection
cases,11 atomic refusals and ASan/UBSan pass. Relative prepared-root reporting
now exposes the102 aggregate compiler/fixture paths previously prechecked;
earlier94-input prose is corrected, without refreshing historical hashes.
Normalized4096 cold502.500ms and separate perf cold505.320ms failures remain
preserved against the unchanged500ms limit. Source changes leave prior World,
public and standalone proofs historical; fresh performance/integration is
pending. This does not assert publicV4fusion,60FPS,movies,full DirectDraw API or
original-driver/semantic COM teardown equivalence.

Fresh102-input Release-O3 TITAN128 World evidence passes all unchanged gates:
cold450.270ms, original19.6259/Qt19.8767FPS, all-warm65 median39/p9549/max57ms,
zero World readbacks/replies/writebacks/coalescing and remaining surfaces. See
[native fade World128](native-fade-row-world128-pass-20261011.json). Longer4096,
peer-death/recovery and current-source public/standalone regression executions
remain the next separate milestones; no paired causal speedup is claimed.

### Native DirectDraw retirement: current long-run World milestone (2026-10-11)

Current102-input Release-O3 TITAN4096 passes every unchanged gate: all4096
original returns/native completions/Qt paints, original19.9362/Qt19.9390FPS,
cold405.796ms,all-warm4033 median36/p9542/max65ms,RSS growth6,565,888 bytes.
Zero World readbacks,pixel replies,writebacks/coalescing and teardown surfaces;
real ddraw disabled/absent. Relative prepared-root reporting exposes the complete
actual compiler/fixture mapping and all source/frozen/input pins remain stable.
[Proof](native-fade-row-world4096-pass-20261011.json). This establishes the
selected long-run no-pixel-roundtrip path and19FPS target, not universal scenes,
60FPS,publicV4fusion,movies,original pixels or semantic COM teardown. Peer-death
and fresh recovery,public gameplay and standalone startup regressions remain
separate current-code milestones. The26-file2d8a57f committed range has complete
parent/current receipts, with no retrospective execution assertion.

### Current native peer failures and fresh recovery (2026-10-11)

Both current102-input TITAN controls kill the owned producer or consumer at
outstanding generation34, require the expected terminal refusal and stopped
private Wine, and forbid a completed-session claim. Real ddraw stays disabled
and absent with bounded resources. A fresh128 session after both controls passes
all unchanged budgets: cold386.921ms,original19.6863/Qt19.8794FPS,warm65
median38/p9545/max58ms,RSS growth4,497,408 bytes;all128 returns/paints and zero
World readbacks/replies/writebacks/coalescing/remaining surfaces. See
[producer](native-fade-row-producer-death-pass-20261011.json),
[consumer](native-fade-row-consumer-death-pass-20261011.json),
[recovery](native-fade-row-recovery128-pass-20261011.json).
The6-filede707e7 committed range has exact parent/current receipts with no gaps.
This is bounded private fault/recovery evidence, not fairness,semantic COM Quit,
publicV4fusion,full API,movies,60FPS or allscene compatibility.

### Native status mapped view: eliminate drawing file IO (2026-10-11)

Fresh public full campaign fails resumed gameplay after combat/portrait exercise:
GAP/fallback,BltFast phase3 held82/146ms and native status stage4 oldWriteFile.
[Failure](native-draw-public-status-write-failure-20261011.json) is preserved.
`NR.native-draw-status-session-file` now uses a startup-owned96-byte file-backed
view; admitted callbacks only perform bounded memory snapshots plus unchanged
per-call module check. Explicit flush/unmap/close occurs only at teardown.
Independent1024 guarded snapshot/assertion/ASan/UBSan controls pass; actual
Wine/native64 service cases/22,464 pixels,16 startup cases/256 words and351 policy
words pass with final96-byte status readability and zero resources. No worker
or unload-policy change. Crash/concurrent external-read atomicity and OS paging
latency are excluded. Current public,World4096,peer/fresh recovery and standalone
proofs are pending after the DLL change; historical statuses/hashes remain.
The8-file283e9dc committed range has exact parent/current receipts and no gaps.

Mapped-status current-source full default TITAN public campaign passes:
casting/summon,combat damage,movement,Fireball damage and invalid-target/
insufficient-mana refusals,nine portraits(maximum channel error1),native Mini
Quit No/resume and normal original Quit; all requested19FPS gates pass. Native
PE499bd552 and actual8 native/44 renderer/28 protocol compiler-builder inputs,
Qt and installed inputs are stable,real ddraw disabled/absent,no unknown method,
recovery or fallback. [Proof](native-draw-mapped-status-full-nvidia-pass-20261011.json).
This fresh live execution supports the mapped contract's live_observation stage;
earlier pending review and WriteFile failure remain historical. It does not
assert publicV4fusion,60FPS,fullscene pixels,full API or semantic COM teardown.

### Native rendering batch complete: mapped-status final proofs (2026-10-11)

Current mapped-status DDS passes the complete resident World4096 TITAN run:
engine19.90895/Qt19.91260FPS,all4096 returns/completions/paints,cold499.079784ms
against the unchanged500ms gate (narrow margin),warm4033 median38/p9544/max66ms,
RSS growth9,478,144 bytes. Zero World GPU readbacks,pixel replies,writebacks,
coalescing and remaining surfaces;real ddraw disabled/absent. Both owned peer
kill controls at generation34 terminate as expected,stop private Wine and make
no completed-session claim. Fresh128 recovery after both controls passes all
unchanged gates:cold426.446818ms,engine19.75309/Qt19.96922FPS,warm65 median34/
p9540/max52ms,RSS growth364,544 bytes. All102 actual compiler/fixture inputs,
source/frozen/installed inputs and original manifests remain stable. See
[World4096](native-draw-mapped-status-world4096-pass-20261011.json),
[producer death](native-draw-mapped-status-producer-death-pass-20261011.json),
[consumer death](native-draw-mapped-status-consumer-death-pass-20261011.json),
[recovery128](native-draw-mapped-status-recovery128-pass-20261011.json).

Current default standalone OpenGL startup on actual Qt TITAN reaches20 native
menu frames/5311 commands with no ordinary GPU/RGBA readbacks,viewport uploads
or recovery and zero consumer resources at bounded abort.35 actual compiler
inputs plus the native DDS builder/protocol/installed closure remain stable;
real ddraw is disabled/absent. A later explicit snapshot is artifact capture,
not ordinary presentation. [Standalone proof](native-opengl-mapped-status-startup-pass-20261011.json).
Together with the preceding fresh public casting/combat/spells/portraits/resume/
normal Quit proof,this completes the current default-native retirement batch.
Raw final artifacts retain new exact hashes;historical evidence is preserved.
The14-file8084ee1 committed range has exact parent/current receipts with no gaps.

This is selected-session native policy and bounded execution evidence,not full
DirectDraw API/movie/original-driver pixel/semantic COM equivalence. Public V4
resident World fusion remains outstanding:the public frontend still invokes
original CPU World drawing.60FPS and wider scene coverage remain future work;
19FPS is proven only for the exercised scenarios,not universally.

### Native render integration into master (2026-10-11)

Combined the native GPU mirror/invalidation route with master's atomic CPU writer preflights and drawing-publication cleanup. The merge preserves both contracts. Master/native committed history is reviewed separately from current source validation; historical records and their hashes remain unchanged. The current integration retains public original CPU World drawing and the20FPS original pacer; public V4 GPU fusion and60FPS remain future milestones.

Current combined-source regression proofs: canonical fade972cases/1738592words with126mirror cases/11atomic refusals and ASan/UBSan; atlas8CTest fixtures; owned GPU32-queue history with512synthetic and512legacy controls; DRAW-03 canonical85 compiled-input session lifecycle/runner plus six expected complete planes99pixels; guarded32-queue live handoff and independent original raster pixel/AX comparison. NVIDIA rolling512-queue hardware proof passes unchanged budgets (warm median29.331464ms, cold287.001429ms). These are scoped independent milestones, not full-frame original driver equivalence.

The first combined public smoke exposed a bounded drawing lease timeout while ordinary diagnostic binary captures were enabled. Normal native sessions now omit those optional lock/blit/update binary exports under the existing explicit archive switch; explicit archive/finite observation remains available. The suite checks the normal session emits none. No lock timeout or performance budget was increased. Later attempts completed summon, melee/lethal combat, Fireball, invalid-target and insufficient-mana refusals and portrait stress without rendering fallback/recovery, but the wizard died in resumed camera stress before Quit. Those full journeys remain failures. The current exercise adds actual observed ordinary ground movement during resumed stress; no game-state, health, AI or balance writes.

The first merged4096 resident proof passed19.85835 engine/19.86225 QtFPS and all resource/roundtrip gates, with cold499.93009ms against500ms. It predates the ordinary archive policy change and is retained as historical. The current source resident proof and full public/standalone launch must supersede it before current integration promotion. The cold margin is narrow; neither60FPS nor broad hardware/scenario performance is established. Compiler-closure refusal attempts and missing-prepared-build setup failures remain preserved with their actual source hashes.

Current merged full public proof is now passed: `native-master-integration-current-public-pass-20261011.json` records actual TITAN summon/melee/lethal combat/movement/Fireball/refusals,9portrait cycles,60-second resumed camera/HUD/rotation/wheel stress with6observed living ordinary ground movements and normal original Quit. Native fallback/recovery are0, optional binary exports empty, private Wine shutdown explicitly verified, sources/compiled inputs/manifests stable. Current standalone20-frame startup, both peer deaths and fresh128 resident recovery also pass independently; recovery warm median41/p9547/max54ms under unchanged gates.

Two current4096 longevity runs remain failures despite sustained19FPS: concurrent warm maximum277ms and separate maximum1070ms exceed150ms. Separate run median35/p9543/cold416.377371ms, engine19.838614/Qt19.842055FPS and roundtrip/resources all pass. The final1024 detailed rows lost the earlier worst sample; this is a diagnostic visibility gap, not permission to waive the full-history maximum. Read-only completed-slot timing observation plus original wait tracing is being used to locate the stall; it does not alter leases, acknowledgements, game rules or budgets. Bounded128 retirement/recovery status and original raster comparisons are independent from4096 latency robustness.

The current untraced4096 TITAN rerun now passes all unchanged gates: engine19.963756/Qt19.969541FPS, warm4033 median35/p9541/max60ms, cold454.810426ms, RSS growth12664832bytes;all4096 returns/completions/paints,zero World readbacks/pixel replies/writebacks/coalescing/remaining surfaces. Read-only double-snapshot timing covers all4096 generations and confirms warm maximum60ms. The selected native residency/retirement milestone is passed for this run. Earlier277/1070ms failures and traced17.6FPS failure remain registered artifacts;root cause and universal concurrency/long-tail robustness are not established. No budget was relaxed. Concurrent master optional DRAW-02 journal admission is included;its default native public/standalone routes are revalidated independently.

Latest-master default native full public and standalone reruns are passed after the optional DRAW-02 admission header change. The full campaign again records casting/combat/Fireball/refusals,9portrait cycles,6observed resumed living ground movements,normal original Quit,zero fallback/recovery,empty ordinary binary archives and verified private Wine shutdown. Current statuses are supported by scoped fresh records;full driver/API/movie/semantic COM equivalence,publicV4 GPU fusion and60FPS remain pending. Prior failed long-tail tests are preserved;the clean4096 success does not establish universal concurrent-load latency.

### Final master-integration accounting and optional legacy reruns (2026-10-11)

The live rolling fade header was already an explicit validation dependency, compiled and SHA-256 pinned in the passing `run-ta3zgqhe` execution. A later duplicate implementation-role link changed only the contract identity. Removing that redundant link preserves the entire required-source set and restores the exact pre-execution contract `50836feb00d6716d16f814d6654efe06b31f969ad6e9a3cad6e650a927ae4294`; no historical result or source fingerprint was refreshed. The hardware target retains its independently reviewed implementation link.

Additional optional legacy software pixel-return reruns are preserved in `native-master-integration-rolling-file-sequence-failure-20261011.json`, `native-master-integration-rolling-delivery-latency-failure-20261011.json`, and `native-master-integration-rolling-independent-latency-failure-20261011.json`. One captured file stream skipped sequence507 (producer file-write race remains a hypothesis); two complete32-queue functional runs exceeded the unchanged120ms delivery median budget at123.243469ms and138.553323ms. No performance budget or historical evidence was altered. These failures leave legacy file-stream robustness and repeatable software-route latency pending; they do not establish universal native performance. Selected default TITAN V4/public gameplay evidence remains separately scoped.

## 2026-10-11 DRAW-02 common API session forwarding

The separate original-active session observer intercepts common surface methods0..35,
original executable input polling/scan-code conversion and applied menu phases.
Strict terminal/loss/gap admission preserves the partial observed-primary-API boundary.
A fresh synthetic PE32 fixture passes32callback cases across eight primary mutators,
including actual HRESULT/LastError/x87 preservation and eight full RGB565 planes.
Five parser guards pass. Actual game interaction, complete global presentation,
semantic input outcomes, real driver loss/Restore and native scene parity remain
separate; callback faults are never labeled real engine recovery.
See `draw02-interactive-session.md` and `NR.drawing-session-api`.

## 2026-10-11 DRAW-02 interactive accounting and failure results

The opt-in common API observer now serializes metadata/publication across original
threads without holding its journal mutex across driver readbacks. Eight synthetic
second-thread readback callbacks and all32 success/lost/busy/generic-error cases
pass with exact results, LastError/x87 and eight full planes. Surface3 GUID and
Surface7 factory ABI are corrected; tagged metadata/input-only sessions explicitly
exclude full readbacks. Seven strict admission guards and eight caller/fault guards
pass. Historical evidence is retained with its original fingerprints.

Extra primary driver readbacks perturbed live gameplay in diagnostics. The
interactive default uses metadata-only observation, which restored sustained
original gameplay in matched-schedule controls. The fresh bound original-active
run records2,444input calls, nine accepted original menu requests and19independent
physical viewport PNGs with journal-byte fences. It observes63creature sample ticks,
376movement records,112health-reduction samples and16original melee returns.
Original Results Continue/Quit and Quick Back return to fresh Main. Original
drawing/simulation remain active. Screenshots are explicitly unsynchronized with
an exact original frame; no complete presented pixel parity is asserted. Tooltip,
picking, complete selection/pause semantics, broader combat and global presentation
routes remain pending. Full-readback mode is diagnostic until interference closes.

The separate recovered failure comparison matches all4,480retained unchanged-original
scripted COM wrapper traces. Its1,536two-draw exhaustions preserve exact original
prefixes and cannot masquerade as successful return; invalid budgets are refused.
HRESULT/LastError comparison is independent of pixel equality and checks recorded
flags/entry identities; complete pointer-backed call-input identity remains open.
The native GL restoration policy additionally passes30known/unknown-content,
partial-write, explicit-reload and exhausted-recovery checks. Controlled faults
and native unknown-content policy do not validate true live driver loss/Restore,
restored palettes or whole-game recovery. The14journey/21operation gates remain open.

See [interactive acquisition](draw02-interactive-session.md),
[caller results](draw02-caller-results.md) and their immutable execution reports.
Committed-history review covers53bdc4a..49c337d,16exact file versions with no gaps.

### DRAW-02 integration review and strict record-kind refresh

DRAW-02 source/evidence commits49c337d/b570002 are reconciled with the independent
DRAW-03 session lifecycle/runner and storage commits9b15da3/2768aa3/e23e99c.
Combined accounting retains all existing scopes, statuses, evidence and receipts.
The committed-history review covers90file versions and appends22exact captured-output
receipts without asserting new execution or a past gate pass.

Final parser review rejects undefined V1 record kind6. Eight admission and eight
caller/fault guard methods pass. New prospectively declared executions retain
32forwarding cases/eight second-thread callbacks/eight full synthetic planes and
all4,480native failure schedules/1,536explicit budget stops. The fresh original-active
metadata run records2,756input calls,68gameplay sample ticks,531movement samples,
265health-reduction samples and3original melee returns, then verifies Results
Continue/Quit and fresh Main return. Earlier reports and their source hashes remain
historical; the new record-kind results support current code. Physical viewport
references remain unsynchronized and full presentation/native battle parity,
pause/tooltip/picking and real-driver recovery remain pending.

The final integration also preserves concurrent storage fix964393d. Its12guard
tests pass, including disappearing descendants and refusal of incomplete or
outside-root measurements. Exact committed-history review covers46file versions
in b570002..300e423 and e23e99c..964393d with no unresolved receipt gaps. The
combined source census retains1,729source/build files and1,525explicit behavior
links; this indexing reconciliation adds no engine validation or status promotion.

### Final concurrent master integration (2026-10-11)

Both histories and exact receipts are retained, including the concurrent working-retention race fix and DRAW-02 strict record-kind/input/API work. The new `native-master-final-draw02-original-pass-20261011.json` refreshes metadata-only original-active Quick Battle acquisition against the imported launcher helpers: strict no-loss/no-gap journal, sustained gameplay, original results Continue/Quit and return to fresh Main pass with original section/manifests/source guards. This is isolated Xvfb/software GL observation, separate from the TITAN performance proofs; native pixel parity, complete picking/tooltip/pause/combat outcomes and full readback interference remain pending. Pre-admission environment failures (missing display and copied general-prefix WOW64 initialization,diagnosed by bounded loader trace) are retained separately; neither is admitted as success.

### Public resident World composition,2026-10-11,first milestone

`NR.public-resident-world` seeds the ordered pointer-free V4 World reference
and same-size RGB565 GPU import. The exhaustive shared-context fixture passes
with zero ordinary import/World readbacks and ordered HUD/copy checks; existing
incremental command-consumer fixture passes. Report:
`public-resident-import-fixture-20261011.json`. Implementation remains partial;
public producer binding, CPU baseline separation, native menu transitions,
casting/combat and live performance are explicitly pending. No original
comparison or replacement status is promoted. Historical renderer evidence
is retained with its previous source hashes and may be stale for these edits.

### Public resident producer wiring and refusal discovery,2026-10-11

The opt-in public adapter imports acknowledged World leases before exact masked
CPU UI deltas; original simulation and CPU UI remain. Public owned-graphics
staging disables DirectDraw. Actual Titan execution reached71verified/imported/
released Worlds with no World pixel reply or World/native/RGBA delivery readback.
The campaign stress then stalls before Mini readiness and fails its deadline;
early35..41painted FPS is not sustained19FPS proof. Durable negative:
`public-resident-campaign-stall-20261011.json`. Casting/combat, menu resume/quit,
performance and arbitrary CPU UI destination-read equivalence remain pending.

Newly reached raw WORD, additive and owned colour leaves, rotated minimap prefixes
and concurrent secondary bitmap/copy work are explicitly scoped in
`public-resident-world.md`. Deferred captures forbid World source/destination and
retain64record/4MiB limits. The missing additive RET8 was reproduced and corrected;
its historical negative/positive source hashes are preserved in
`public-resident-leaf-stack-regression-20261011.json`. Fresh native-reference and
ABI fixtures are distinct from historical original kind8/minimap comparisons.
No original-comparison, integration or live-replacement status is promoted for
the incomplete public campaign. Updated sources may stale older evidence;
historical hashes are preserved rather than refreshed.

### Public resident surface and batching milestone,2026-10-11

Default offscreen caps0x800 now selects bounded owned RGB565 storage. Four exact
510x180 native cases and the unchanged64-case Wine surface comparison pass;
`native-default-offscreen-pass-20261011.json` retains current source/input claims.
The prior rejected descriptor/modal stall remains separately preserved.
Whole-World GL batching and bulk CPU knowledge discard pass the fresh owned
fixtures in `public-resident-batch-fixtures-20261011.json` without delivery
readbacks. Titan native work falls from~35ms to~24ms per World;19FPS is not met.
The public run reaches195verified/imported Worlds then refuses a background
primary-screen cursor copy. SourceID1 is distinct from guarded WorldID2; deferred
record/byte capacity is the remaining refusal reason. Exact historical bound
counters were absent; bounded512record traffic validation remains pending.
`public-resident-batch-background-copy-failure-20261011.json` is negative evidence.
Casting/combat, Mini/Cancel, quit and whole original pixel equivalence remain
pending. Independent statuses and historical source hashes are unchanged.
The previous committed range audit names a5ebae6..0063d12,56exact reviewed file
versions and zero missing receipts; no retrospective gate/execution assertion.

The follow-up512record/4MiB queue preserves every World-source/destination
refusal and passes nine malformed/bounds cases plus a512record primary cursor
copy burst. Fresh full owned fixtures are retained in
`public-resident-cursor-queue-fixtures-20261011.json`. The Titan follow-up passes
1100resident Worlds without a deferred refusal but stops at smoke Escape stage4
(Mini does not appear) and misses19FPS; negative report
`public-resident-cursor-queue-live-failure-20261011.json`. No status promotion.

### Resident World permission epoch milestone,2026-10-11

Native policy `NR.world-source-region-epoch` caches bounded readable-region
answers only for the guarded original World owner and resets both boundaries.
Other threads and nonresident routes remain fresh; source bytes/signatures still
copy/check each request. External permission changes within that guarded epoch
are excluded from this policy, separately from original equivalence. Fresh
10000-to2query,64region/permission-reset and full owned fixtures pass in
`public-resident-region-epoch-fixtures-20261011.json`. Actual Titan query counts
fall to~26/World and throughput rises to~18FPS, still below19. The default
tutorial Escape/Mini negative and prelaunch provenance refusal are retained
separately. Ordinary difficulty1 casting/combat/portrait/quit proof remains
pending; original comparison,live replacement and60FPS are not promoted.
The committed0063d12..b04ffef receipt audit reviews28exact file versions with
zero missing receipts; no retrospective gate or execution assertion.

The ordinary difficulty1 Titan run reaches225verified Worlds at19.7667aggregate
FPS before a projected-shadow bounds refusal. Actual memory queries average29.82
per World, with zero World/native/RGBA delivery readbacks. This is negative
evidence (`public-resident-region-epoch-shadow-failure-20261011.json`), not a
sustained19FPS or completed casting/combat/portrait/quit proof. The next bounded
fix investigates the right-edge shadow against actual opaque samples.

### Right-edge projected shadow admission,2026-10-11

Gen226's nominal shadow extends to805 while actual opaque coverage ends at786.
Native preflight checks projected coverage on bounding overflow, rejects real
covered overflow atomically, and crops unused GPU padding to owned storage.
Recovered shadow row selection/whole horizontal refusal remain separate from
this native storage policy. Fresh fixture and ordinary Titan gameplay results
are required before claiming this code validated; full original comparison and
sustained19/60FPS remain independent pending milestones.

Fresh complete `public-resident-shadow-fixtures-20261011.json` passes both
shadow backends, atomic genuine overflow refusal, cropped rows/whole horizontal
refusal, owned metadata without readback and existing1.44Mword/import/defer/ABI
fixtures. Sources/inputs are unchanged and all executed native fixture binaries
are hashed. This is native integration evidence, not original pixel equivalence
or a public gameplay/performance pass.

The campaign prelaunch compiler check formerly scanned all built test targets;
this correctly refused undeclared test inputs but incorrectly treated them as
shell dependencies. It now follows the shell's linked objects/local archives
and requires their exact compiler receipts.287actual repository source/header
dependencies are found on this build. Synthetic linked-target fixtures prove
unrelated/generated inputs are excluded and missing linked object/archive
receipts refuse. New durable result `public-resident-linked-provenance-fixtures-20261011.json`
is source bound; the earlier negative is preserved separately. New validation
metadata does not promote engine gameplay or original equivalence.

### Physical-stride distortion discovery,2026-10-11

`RS.displacement-linear-stride` records selected original596490/5968a4 forward
source loads crossing from right-edge x into the next physical row. The ordinary
Titan run passes the prior shadow edge but stops after268Worlds/19.7216aggregate
FPS at this missing branch, with zero delivery readbacks. Initial isolated
unmodified leaf execution proves10wrapped samples/960000canvas words; native
implementation and current comparison/live milestones remain independent.
Native storage uses bounded tight-stride row conversion; out-of-allocation and
padded/negative-offset cases remain excluded. Historical source hashes retained.

Fresh `public-displacement-linear-comparison-20261011.json` compares both
unmodified original leaves with the analytical tight-stride model, then native
CPU and both GPU backends.960000canvas words match, with551covered/10wrapped
samples, zero delivery readbacks, two atomic scene/one CPU allocation refusals,
19existing atomic refusals and the canvas regression. Sources/inputs remain
unchanged and both executables are hashed. Only the new scoped behavior's
comparison status is recorded; public integration/replacement and whole
original failure/state equivalence remain pending.

Full current owned fixtures pass in `public-resident-linear-full-fixtures-20261011.json`.
The ordinary Titan follow-up passes326Worlds then refuses genuine covered
projected-shadow overflow;17.2778aggregateFPS fails19, with zero World/native/
RGBA delivery readbacks. `public-resident-linear-pass-shadow-write-failure-20261011.json`
preserves this negative. Next work must investigate original shadow destination
stride semantics before extending native admission, then profile sustained
gameplay. No public integration/replacement promotion. The committed
b1f4ff1..884d263 audit reviews22exact file versions and finds no receipt gaps.

### Corrected projected shadow envelope,2026-10-11

Initial isolated original replay disproves the presumed shadow row-crossing
write:57e540 refuses the complete projected envelope before vertical clipping.
The native unprojected-width-only model was incomplete; prior transparent-
padding native-reference passes do not imply original equivalence. Both captured
edge shadows must be skipped, including opaque-safe padding. CPU/World/GPU
admission is corrected together. Historical evidence retained; fresh original/
native equality/refusal comparisons and public gameplay/performance remain
independent pending milestones.

Fresh `public-shadow-projected-envelope-comparison-20261011.json` proves both
captured shapes against unmodified57e540, native CPU and two GPU backends.
2.88Moriginal/5.76Mnative canvas words compare across whole refusal, exact-edge
admission and vertical crop preserving the full source envelope. Sources/inputs
unchanged; no delivery readback. Only the scoped shadow comparison is recorded.
Public gameplay/performance,RGB555,complete ABI/failure state and full live
replacement remain pending. Prior native-reference padding results are
historical and superseded for baseline admission.

Fresh post-correction distortion replay remains exact against both selected
original leaves (`public-displacement-linear-after-shadow-20261011.json`); the
complete owned fixture corpus passes (`public-resident-shadow-envelope-fixtures-20261011.json`).
The ordinary TITAN public run reaches448completed/imported/verified Worlds with
no renderer error and zero delivery readbacks, but17.7943aggregateFPS misses19.
Native dialogue clears while CPU diagnostic pixels retain Hermes text; the
bounded dual-image readiness check stops before casting/combat. Report:
`public-resident-shadow-envelope-dialogue-negative-20261011.json`. Whether the
CPU oracle is stale or UI lifetime is lost remains pending, as do full spells,
portrait/stress/menu/quit and sustained19FPS. No public replacement promotion.

The57e540 address-link scope was corrected from the initial linear-write
hypothesis to the recovered projected envelope rule. Both captured geometries
were rerun prospectively against that final contract. Fresh report:
`public-shadow-envelope-final-scope-comparison-20261011.json`; earlier execution
hashes and scope-bound limitations are preserved. Only the final scoped original
comparison is current; broad gameplay/UI/ABI/RGB555 equivalence remains pending.

Resident World dialogue readiness now retains both diagnostic images/OCR and
hash checks but uses the resident image for the World heading. Retired original
World drawing no longer clears CPU World pixels; that buffer cannot establish
current dialogue dismissal. The runner's explicit resident mode must match the
helper oracle tag; native visible Hermes still refuses. The bounded12second,
two quiet sample policy is retained. Completed gameplay separately requires
fresh identity-valid original summon/cast/combat observations, so quiet images
alone do not prove input admission. Two owned oracle cases pass:
`native-resident-dialogue-owned-oracles-20261011.json`. Equal-value CPU UI writes,
destination-dependent UI and full original pixel/internal dialogue-state
 equivalence remain pending. Fresh full Titan gameplay is being executed.

The first resident ordinary Titan run after dialogue correction successfully
creates a player Zombie, observes player Fireball93to73, five Zombie melee hits
and a party5to0 enemy finish.2816resident Worlds complete at19.5038aggregateFPS
with zero delivery readbacks/no renderer error. Full journey remains negative:
the eight-order retreat budget expires at(5,69) en route to(3,79). Every order
still advances the living125HPwizard, so evidence does not establish unreachable
terrain. Report/input:`public-resident-casting-retreat-negative-20261011.json`
and `public-resident-casting-retreat-input-20261011.json`. The bounded budget is
now12orders; the same within3tiles/aliveness requirement remains. Portrait,
Mini/quit, remaining refusal and complete journey proof are pending; no broad
replacement or original UI equivalence promotion.

Twelve-order follow-up remains negative: the living90HPwizard reaches(0,79),
exactly3tiles from the guessed(3,79) target, and strict arrival refuses. Camera
edge centering/projection mismatch is a hypothesis, not recovered behavior.
The waypoint now uses the originally occupied wizard tile; the same twelve
orders/within3tiles/aliveness requirement remains.3264Worlds complete with no
renderer error/readbacks, but17.9671aggregateFPS misses19. Report/input:
`public-resident-retreat-edge-negative-20261011.json` and
`public-resident-retreat-edge-input-20261011.json`. Profiling and full journey
proof remain pending; no target/performance gate is relaxed.

Returning to the originally occupied tile clears retreat and allows the complete
physical input helper,including insufficient mana and portrait stress,to finish.
The profiled journey also completes Mini Cancel/resume and Quit No/Yes/Main steps,
but remains negative: resident polling treats original producer cancellation at
normal shutdown as fatal.4498completed/imported/verified Worlds are balanced,
with zero delivery readbacks.18.8009aggregateFPS is below19 and is profiled cadence,
not an uninstrumented pass. Report:`public-resident-profile-quit-negative-20261011.json`.
99Hz user-space shell sampling over30seconds loses no samples and attributes32.51%
self CPU samples to full-canvas dual-background minimap composition; attribution
is diagnostic,not pure GPU time. Retained artifacts/source/binary hashes:
`public-resident-cpu-profile-20261011.json`. Normal acknowledged shutdown handling,
minimap candidate comparison and uninstrumented fulljourney/FPS proof are next.

Two native policies are seeded before execution. `NR.minimap-write-candidates`
retains two freshly distinct backgrounds and compares the union of fully admitted
owned service writes,including sentinel-equal words and duplicates. Arbitrary
callbacks retain the full-canvas scan; no destination oracle/CPU World read is
introduced. `NR.resident-normal-shutdown` accepts cancellation only after the
exact same-thread Main Quit callback ACK, no pending lease, balanced completed/
import/return counters and no prior error; imports after retirement refuse.
Unexpected cancellation remains terminal. The first owned attempt already
compares7.68Mwords across16padded/four-view terrain/overlay cases, with19396unique
service words and four atomic/undefined refusals; its shutdown fixture wrongly
tries to create a second native channel owner and is correctly refused. Retained
negative:`public-resident-candidates-quit-fixture-role-refusal-20261011.json`.
The fixture now simulates producer cancellation through a separate owned header
mapping. Fresh complete fixtures/original comparisons and Titan proof are pending.

Corrected complete owned execution passes with source/input stability:
`public-resident-minimap-shutdown-fixtures-20261011.json`.16four-view/padded terrain
and overlay cases compare7.68Mwords, with19396unique committed service words;
sentinel-equal values, duplicates, generic callback full scans, atomic rejection
and unknown CPU validity are preserved. Five Main Quit ACK variants, three clean
retirement cases and eight cancellation/import refusals pass; repeated retirement
is idempotent. Existing1.44MWorld-word/import565/ownership/shadow/upload/defer/query/
ABI/minimap wire and Preferences tests pass. The two new native policies reach
headless integration only. Original comparisons and fresh uninstrumented Titan
normal-Quit/19FPS proof remain pending; no original driver/fullUI equivalence.

Fresh isolated original comparisons after optional write receipts pass:
1360terrain cases/35,486,800words and4752overlay cases/53,312,088words,including
four views,padded storage,RGB565/RGB555overlay words,returned cell offsets and
creature/camera metadata. Sanitizers and source/original stability pass. Reports:
`public-resident-minimap-terrain-current-20261011.json` and
`public-resident-minimap-overlays-current-20261011.json`. These renew selected
pixel-library semantics; traced canvas composition remains a separately proved
native policy. Full machine state/driver/broad live equivalence stay pending.

Shared canvas changes are followed by fresh selected distortion and projected
shadow comparisons,including both GPU backends and complete owned canvas words:
`public-displacement-after-minimap-20261011.json` and
`public-shadow-after-minimap-20261011.json`. Both remain exact against the scoped
unmodified original leaves, with stable sources/inputs and zero delivery readbacks.
Historical reports/hashes remain intact. Fresh unprofiled full Titan proof is
running; no19FPS/completed normalQuit assertion is made from headless fixtures.

The first unprofiled minimap/shutdown journey completes all physical input and
native menu steps without renderer fallback:4640completed/imported/returned
Worlds,zero delivery readbacks,normal_quit_retired true,19.044765aggregate engine
FPS. Native work averages21.127ms and producer work9.069ms per World,versus
24.178/12.694ms in the earlier profiled negative run; different scene progression
and profiler overhead exclude a controlled causal speedup claim. The complete
test nevertheless refuses missing prospective render-build inputs before later
validation: the imported shadow build recipe and seven wire JSON schemas were
undeclared. Negative report `public-resident-render-provenance-negative-20261011.json`
retains that failure and immutable source hashes. The runner now declares the
render recipe/runtime/protocol superset before building or launching and retains
all exact compiler/protocol comparisons.19 fail-closed evidence tests pass.
A fresh full TITAN rerun is required; no completed live integration promotion.
Read-only inspection of that negative run's retained rates also finds a2.976s
resumed-play sample and a1.272s sample with0.786paintedFPS; these independently
fail the unchanged worst-window performance gate despite the whole-run19.045
engineFPS. The next run must pass both gates. Aggregate FPS alone is insufficient.

The next prospectively declared TITAN run stops on player death before required
Zombie melee,not a renderer stall. Negative `public-resident-combat-placement-negative-20261011.json`
is retained with unchanged source/private shutdown checks. Input traces show the
helper reused a pre-movement approach vector for its combat summon,then kept the
wizard exposed while waiting for autonomous combat. The helper now projects an
adjacent summon from current observed enemy/wizard coordinates and issues an
ordinary opposite ground order for wizard clearance. No simulation/health/AI or
performance/admission changes; native projection is a test strategy,not recovered
order/pathfinding equivalence.19fail-closed checks pass;fresh full proof pending.

The corrected combat journey now verifies casting,Fireball,Zombie melee/party
lethal,blocked casts,5portrait cycles with maximum1channel face error,Mini/resume
and normal Quit.4574balanced GPUdeliveries/19.144240aggregate engineFPS,no World
readbacks,realDirectDraw absent. Whole test remains negative because an intro
cycle0 sample records1frame/1.256s,below the unchanged9.5FPS worst-window floor.
`public-resident-complete-window-negative-20261011.json` retains all checks and
failure. The capture owner now records original-copy/native-readback/total capture
milliseconds to distinguish diagnostic serialization from gameplay latency. No
capture bypass,failed sample omission,budget relaxation or integration promotion.

The instrumented complete private TITAN campaign passes all unchanged gates:
4861completed/imported/verified returned GPUWorlds,19.389731aggregate
engineFPS,zero World/native/RGBA delivery readbacks,80B ACK/no World pixel reply,
realDirectDraw disabled/absent. Actual summon,Fireball,Zombie melee/party lethal,
both blocked-cast cases,physical movement,5portrait cycles,Mini Cancel/resume,
Quit No/Yes/Main,launcher/shell clean exit and private Wine shutdown pass. Stable
portrait maximum channel error is1; phase and worst-window throughput also pass.
`public-resident-campaign-complete-20261011.json` retains full result,actual input
and per-capture timings(max7.355ms owner time).
Five native policies reach scoped live observation; original comparison and live
replacement remain separate. Earlier1.256/2.976s failed rate windows remain
historical evidence of long-tail limitations;this one pass is not universal
cold/concurrent-load latency proof. Full CPU UI destination reads/lifetimes,
World/UI pixel and driver equivalence,movies and60FPS remain outstanding.
Master's separate drawing-publication header change requires another complete
proof on the combined tree before the authorized merge is finished.
### DRAW-01 complete-session census: expanded callers — 2026-10-10

The authorized end-to-end scope retains all fourteen required single-player
journeys and all drawing-side effects, alternate/default dispatch, caller
attribution, failure/retry/recovery and cleanup obligations. Incremental work is
performed on an isolated branch so concurrent drawing-comparison/scene changes
are preserved.

`TOOL.drawing-census` exports 909 exact rel32 CALL sites to all 61 registered
entry identities from the hash-verified original No-CD executable. The reviewed
supplemental source augments the historical selected-window census and preserves
empty discovered-owner lists. No nearby-PC or inferred function assignment is
used. Against the explicitly diagnostic preferred-base interpretation of the
historical four-return journal, 53 of the 63 unattributed groups have exact
static matches; ten indirect raster groups still need observed callee identities.
This does not establish a fresh runtime module base or refresh historical journal
evidence. Source changes leave older tool results historical until fresh guard
execution is registered. No original drawing body is suppressed.


### DRAW-01 original-active call journal increment — 2026-10-10

The isolated `draw01-full-census` branch adds a bounded, original-active PE32
entry/return metadata journal, strict decoder and private menu/campaign journey
driver. All original drawing work is forwarded. Sixty catalog splices are
accepted; `58edb0` is refused because original backedge `58edde -> 58edb1` enters
overwritten bytes. Failed attempts are retained. Actual callbacks/trampolines
pass synthetic register/flags/x87/SSE/error, callee-pop and same-stack tail
unwind checks; closed menu/shutdown metadata demonstrates a verified runtime
base and paired calls only. Scenario closure, caller ownership/indirect chains,
all writers, API/borrowed access, pixel/side-effect equivalence, recovery and
full dispatch remain independent pending obligations. Existing evidence hashes
and rendering status are preserved. See `draw01-inventory.md` and the new exact
scope-bound census evidence. History `98b4524..8764d90` was reviewed separately
from uncommitted journal changes, with exact receipt coverage retained.

### DRAW-01 original queue routing and omitted helpers — 2026-10-10

The route register now includes seven original consumer helper/cleanup targets,
107 routes and 68 entry identities. Expanded exact caller evidence retains 937
direct CALL sites. Observation admits 67 safe sites and still refuses the unsafe
fade interior. An isolated unchanged original consumer passes 2,432 synthetic
backend cases and retains all 105 raw dispatch branch slices. Selected wave
kinds 16/17/20, literal colours and metadata zero/default/AX/boolean policies
have high confidence within this routing scope; native pixels, concrete virtual
objects, auxiliary boundaries/prior-metadata changes, helper globals and enabled
cleanup remain pending. No comparison, integration or replacement status is
promoted. All dispatch and fourteen scenario closure gates remain incomplete.

Fresh six-size ABI execution and fresh helper-anchor consumer execution are
registered under new evidence IDs. Historical fingerprints remain unchanged.
Failed campaign attempts and compressed partial journal hashes are retained;
they do not establish closed census validation. Committed history
`8764d90..4d28021` was separately reviewed with 31 changed files, 94 affected
behaviors and no missing exact receipts. See the retained history report and
`draw01-inventory.md` for reproduction and scope boundaries.

### DRAW-01 closed bounded campaign metadata — 2026-10-10

Fresh original-active Adept campaign metadata retains 2,416,882 events and
1,208,441 paired calls with zero loss/open/IO, unchanged staged binaries and
declared sources, and normal original exit. Main/setup, campaign entry, World,
camera/cursor input, Mini/Preferences/World return, Quit confirmation, defeat
report, Main and shutdown were exercised. Original Hermes dialogue input holds
are handled through original input and retained screenshots. The host phase
reader now retries transient seqlock changes boundedly; persistent invalid/dead
host control remains explicit loss and is checked by fresh actual PE32 ABI
execution. Earlier failed/rejected journals and their fingerprints are retained.

This establishes bounded observation integrity only. All fourteen scenario
closures and four dispatch obligations remain pending; no rendering comparison,
integration or replacement promotion is asserted. History `4d28021..18d24f6`
was separately reviewed with 24 changed/link-reviewed files, 63 affected behaviors
and zero missing exact receipts. See the campaign observation and phase/ABI
evidence for exact source/contract bindings.

### DRAW-01 phase accounting and original movie-request path — 2026-10-10

Eight exact campaign host phases now contribute bounded entry activity to their
declared scenario inventories. Recorded metadata instrumentation is counted
separately: 67 accepted sites and one refused fade interior, alongside the 52
legacy producer hooks. Fresh 37-guard accounting checks pass without closing
any scenario or dispatch obligation. Historical observation hashes remain fixed.

An explicitly movie-enabled original run records both startup movie-wrapper
calls, followed by Main/setup and normal shutdown: 84,752 events / 42,376 paired
calls, zero loss/open/IO and stable binaries/sources. Wrapper returns1 after
313/189ms establish a fast return path only; complete original movie playback,
stream failure semantics, placement, controls and graphics side effects are not
validated. A successful metadata journal is not successful playback. All 18
DRAW-01 closure blockers remain. Committed history `18d24f6..f240927` was
reviewed separately with 12 changed/link-reviewed files, 13 affected behaviors
and zero missing receipts.


### DRAW-01 original save/load diagnostics — 2026-10-10

Disposable census staging now supplies the original Save output directory;
original campaign saves produce retained nonempty `.sav` files. The initial
Load fixture omitted list selection and faulted at `005b74dc` reading `10`.
This repeats with drawing hooks absent but the menu adapter active. A corrected
one-row selection fixture and loader admission/heartbeat timing are under live
validation. No successful save/load round trip, native persistence, graphics
failure equivalence or DRAW-01 scenario closure is claimed. Failed reports,
screenshots, original saves and hash-bound partial-journal archives remain in
`working/experiments/drawing-census/`; prior evidence hashes are unchanged.
Committed history `f240927..5abde11` was reviewed with zero missing exact receipts.


The [selected-row menu-only control](drawing-census-save-load-menu-only-success-20261010.json)
now finishes the bounded original Save/Load journey: a retained 353,641-byte save,
original World/Mini and Preferences return, original defeat/Main and exit0.
Sources and staged binaries remain stable. Drawing entry hooks are absent and
no call journal is claimed. The visible loading transition begins on list-row
selection, before the later explicit Load-button input; input ordering is retained.
The same corrected fixture with full drawing metadata remains under validation.
The two failed Load reports are retained under distinct immutable evidence IDs;
the successful control does not retroactively validate those attempts.


The [full original-active save/load journal](drawing-census-save-load-observation-20261010.json)
also completes with exit0: 3,607,228 events / 1,803,614 paired calls, zero
loss/open calls, verified actual base400000 and stable source/binary fingerprints.
The original-created 354,045-byte save is retained with SHA-256
`76bd3f35812ce21ad19dc18bf073772c54725dd65eb80e8c14afb55ffda040c4`.
This validates the bounded selected-row journey and journal; exhaustive caller,
borrowed graphics, state/pixel/lifetime/recovery and scenario closure stay pending.


### DRAW-01 original movie contract and helper census — 2026-10-10

The [unchanged original movie reference](drawing-movie-scoped-original-20261010.json)
passes1,760selected external HRESULT/control/Escape/Update/Blt/placement/cleanup
cases, including sentinel checks of wrapper state-write ordering. It corrects
EDX to DirectDraw device and the first stack word to destination surface.
[Contract findings and exclusions](draw01-movie-contract.md) retain exact
short-circuit, skip/result and media/video/sample/source release order. Native
Qt decoding/replacement and actual original codec/pixel/audio output stay separate.

The catalog now has109routes/70entry identities; its new exact caller export
has939direct CALL sites. The [expanded original movie-helper journal](drawing-census-movie-helpers-observation-20261010.json)
has69safe metadata hooks plus the original unsafe fade-site refusal,82,380events /
41,190paired calls, zero loss/open/IO, stable sources/binaries and normal exit.
Both intros enter original setup/sample: each helper returns0 and wrapper1.
These quick returns do not establish complete playback; exact sample Update/Blt
counts/results and original audiovisual output remain pending. The driver now
binds the actual menu source/header compiler inputs prospectively; older records
retain their original narrower source declarations.

The [fresh ABI fixture](drawing-census-movie-pop16-abi-20261010.json) adds the
movie wrapper's16-byte callee cleanup to0/4/8/12/20/24, with independent tail8
baseline and retained flags/registers/x87/SSE/LastError/loss/phase guards. It uses
synthetic bodies; actual69original splice manifests and live observations are
separate. Committed history5abde11..7bb1f46 has zero missing exact receipts.
All14scenario closures and4DRAW-01 dispatch obligations remain open.


### DRAW-01 original movie loop and error-site diagnostics — 2026-10-10

Fourteen explicit original movie SITE probes extend safe metadata coverage to83
sites (plus the retained unsafe fade refusal),123routes/84entry-or-site identities
and939exact direct CALL sites. SITE probes require explicit containing original
function anchors, preserve the return slot, and report before-instruction EAX
separately from function results and paired-call counts. The first retained
original run has82,328events/41,162paired calls/4SITE events, no loss/open/IO,
stable sources/binaries and original exit0. Each intro enters the loop once,
then cleanup with00040003; no successful Update/Blt-dispatch or selected error
site fires. This is immediate original end-of-stream in the observed Wine path,
not full playback. [Interpretation and API source](draw01-movie-contract.md).
The first inventory guard attempts exposed a missing future evidence record and
an incorrect build JSON pointer; those failed logs remain retained. Catalog
projection now addresses the actual nested decoded build, with a fresh accounted
run pending. Source-only metadata corrections preserve the first run's hashes.
History7bb1f46..42801df has zero missing exact parent/current receipts.


The [fresh accounted movie-site repeat](drawing-census-movie-sites-accounted-observation-20261010.json)
again observes two immediate `MS_S_ENDOFSTREAM` exits:82,316events,41,156paired
calls and4SITE events; zero loss/open/IO, stable declared sources/binaries and
exit0. Its catalog projection correctly addresses `/decoded/original_sha256`;
the earlier record and failed inventory guards remain unchanged. The inventory
reports123routes,83metadata sites/1refusal,939direct CALL sites and0unaccounted
activity; all18DRAW-01 closure blockers remain. [54fresh guards](drawing-census-movie-site-guards-20261010.json)
cover mode/anchor refusal, mixed counters and result separation, exact callers
and scenario/unknown-route accounting. Source-only decoder/catalog changes leave
older original reference, ABI and live evidence stale; historical stages/hashes
are retained without claiming current full original/native equivalence.

### DRAW-01 original Quick Battle and both result actions — 2026-10-10

The isolated original fixture now preserves original map2/default rules/empty
spell assignments and observes World/input/Escape/results/Continue/World/
Escape/results/Quit/Quick/Main/shutdown. Original context1 uses the documented
Escape-to-results path, so no Quick Mini/Preferences claim or experimental gate
activation is made. The initial incorrect Mini expectation is retained as a
failed attempt; its partial journal remains invalid for closure. The successful
first cohort has2,509,020events/1,254,510pairs at41entries, zero loss/open calls,
stable binaries/sources and exit0. Phase/input intent is separate from verified
camera/tooltip/effect behavior and pixel/graphics lifetime equivalence.
Committed history42801df..4878427 has69exact source receipts and111behavior
receipts without missing matches; this review is separate from uncommitted work
and asserts neither a past gate pass nor new execution.

The prospectively registered [repeat](drawing-census-quick-battle-accounted-observation-20261010.json) retains2,447,382events/1,223,691pairs, zero loss/open calls, stable sources/binaries and exit0. Both original Game Over headings were checked. Each phase projection stays a bounded cohort; all18closure blockers remain.

### DRAW-01 original campaign summon/effect observation — 2026-10-11

The [original-active cohort](drawing-census-campaign-effects-observation-20261011.json)
records a new living, player-owned Zombie after a physical original cast. Its
original callback has a raw mana debit of 4,352 in the sampled creation interval;
the inspected screenshot shows the summon effect and `1/15` HUD. The existing
read-only spell validator was applied after the run with that explicit scope.
There are 514 passive creature samples and 12 spell rows, each checked for
framing/sequences and retained at its recorded hash. Their formats lack terminal
and loss counters, so no all-tick/callback/lifetime claim is made.

The drawing journal retains 4,184,314 events / 2,092,157 paired calls, zero
loss/open calls, stable staged binaries/sources and normal original exit. It
nearly reaches the fixed 4,194,304-record cap; any overflow remains invalid.
Original campaign Mini/Preferences/World/Quit/defeat/Main/shutdown follows the
cast. Phase7/8 projections are declared before execution. Input labels remain
separate from semantic camera/tooltip and full effect-to-queue-to-pixel proof;
all14scenario/four dispatch and caller/graphics/lifetime closure remains pending.
Committed history4878427..92295c9 has15exact file and16behavior receipt matches,
without missing intermediate versions; no retrospective validation is asserted.

### DRAW-01 selected graphics API CALL boundaries — 2026-10-11

The [static export](drawing-api-sites-expanded-static-20261011.json) retains
314 selected original CALL instructions, 37 exact import identities and 15
whole-image graphics-import calls outside the selected wrappers. Three registered
entries have bounded normal-return instruction graphs with null discovered
anchors; their function identity and exception/indirect flows remain unassigned.
[Byte/ownership guards](drawing-api-sites-guards-20261011.json) pass 12 cases.
[Scope and remaining contracts](draw01-graphics-api-boundaries.md) separate this
static boundary evidence from concrete runtime targets, API results/cleanup,
Lock/DC borrowing, resource ownership, failures and complete writer coverage.
The central register seeds those touched contracts with independent statuses;
no scenario or dispatch closure is promoted. Committed history92295c9..e4be448
has exact intermediate file/behavior receipts without missing matches.

### DRAW-01 separate API journal wire and static splice planning — 2026-10-11

The [fresh planner](drawing-api-splices-accounted-static-20261011.json) accepts
250 exact CALL splices and retains64refusals:52incoming direct-flow conflicts
and12overlaps with existing census probes. The first output is retained with an
explicit source-mutation limitation and supplies no current validation.
[21synthetic guards](drawing-api-journal-guards-20261011.json) pass for the
separate API wire, concrete target/stack pairing, terminal checks and continuation
relocation. No original API or runtime observer has executed in this increment;
output/resource borrowing and complete drawing proof remain pending. All14scenario
andfour dispatch closures remain open. Historye4be448..f2d858b has exact12file/
14behavior receipts without missing intermediate versions.

### DRAW-01 separate API forwarding observer and synthetic ABI — 2026-10-11

The [separate DLL build](drawing-api-observer-build-20261011.json) compiles
250signature-pinned CALL probes, retaining64refusals. The [synthetic ABI](drawing-api-observer-synthetic-abi-20261011.json)
passes10bodies/four CALL addressing modes with0–24-byte cleanup. Its tested
registers/flags/x87/SSE/LastError, original argument placement, concrete target,
negative EAX and actual cleanup match baseline. Loss/frame/phase/cap/IO states
still forward the synthetic body through a fixed continuation. This fixture
executes no original graphics API or original splice manifest; actual original
installation/shutdown/journals, refused-site handling and output/resource/borrow
lifetimes remain pending. All14scenario/four dispatch closures remain open.

### DRAW-01 exact per-run API bindings — 2026-10-11

The [native writer/decoder round trip](drawing-api-observer-bindings-abi-20261011.json)
passes all10synthetic probes and records exact original/trampoline return PCs,
continuations and stub/module bounds. [12mutation guards](drawing-api-bindings-guards-20261011.json)
check physical identity and refuse overlap/alias/corruption. The named native
census leave export has a [fresh synthetic ABI](drawing-census-named-export-abi-20261011.json).
Physical module membership does not assign original/logical ownership, and
prepared allocations do not prove hook installation. Earlier source/claim
fingerprints remain preserved with current validation limits. Original API
journals,64refused sites, outputs/borrow lifetimes and all14scenario/four dispatch
closure remain pending. Committed history34ce28f..dc626c7 is separately reviewed
for exact intermediate receipts without asserting retrospective validation.

### DRAW-01 first original separate API journal and export fix — 2026-10-11

The [original cohort](drawing-api-menu-observation-20261011.json) retains
81,680core events/40,840pairs and7,876API events/3,938pairs at27API sites.
Both journals and the live allocation map close cleanly; no loss/open/IO or
unreadable API data, stable binaries/sources and normal original exit0.
Actual cleanup includes52bytes, exceeding the initial eight-word diagnostic
stack; formal full arguments and outputs are still pending. The final Main
screenshot is inspected; startup's phase2 snapshot is black, with no pixel
comparison claim.

Actual export inspection caught a missing assembly-symbol export despite its
annotated declaration. The earlier callback ABI report is retained unchanged
with an [explicit visibility limitation](drawing-census-export-declaration-limit-20261011.json).
Explicit linker export/table guards and a [fresh ABI](drawing-census-export-checked-abi-20261011.json)
now verify the name; current re-exported splices remain250accepted/64refused.
Historydc626c7..6e85d84 has exact intermediate receipts and separately names the
export semantic gap; no retrospective gate pass or original validation is
asserted. All14scenario/four dispatch, remaining callers/API contracts,
outputs/resource borrowing and failure/recovery closure remain open.

DRAW-01 incremental CALL observation (2026-10-11): the separate API observer
now admits 302/314 selected boundaries, including 52 single-byte INT3 sites whose
successors have incoming original branches; twelve core-hook overlaps remain
explicitly refused. Fresh 38 native guards and twenty-body PE32 ABI/binding
fixture passed, including untouched successor bypass and foreign exception
forwarding. The original menu cohort `run-opctpvl0` closed 41,380 core and 4,348
API pairs at 32 sites, with 382 calls at five breakpoint sites, zero loss/open/IO,
stable inputs/sources and normal original exit. Evidence is registered separately
as `DRAW.api-breakpoint.*-20261011`; historical fingerprints remain unchanged.
This does not close formal API output/lifetime or all-scenario/dispatch accounting.
The committed range `6e85d84..50674a8` has 75 exact file and 114 behavior receipts,
zero missing intermediate receipts; that review does not assert a past gate pass
or new runtime validation. The shared worktree remains untouched by this branch.

DRAW-01 selected API cohort (2026-10-11): the explicit API-only plan admits all
314 selected CALL boundaries; the separate core/API plan remains302with12overlap
refusals. Seven fresh native admission guards prevent API-only evidence from
claiming a core call journal. The enabled original movie cohort `run-yyj3me1m`
closed4,511API pairs at53sites (19movie-helper sites), zero loss/open/IO, stable
sources/binaries, actual null startup core module/leave and normal exit. Both
startup setup/sample HRESULT paths returned0 and their first Updates00040003;
this is EOF evidence, not full playback. Formal arguments/outputs, ownership and
borrow intervals, complete writer/scenario/dispatch accounting remain pending.
Registered `DRAW.api-cohort.*-20261011` and `DRAW.api-only.movies-20261011` retain
new results without refreshing historical hashes. The committed
`50674a8..defa681` range has20exact file and9behavior receipts with zero missing
intermediate versions; retrospective accounting asserts no past gate/validation.


DRAW-01 formal argument/output capture (2026-10-11): the independently versioned
V2 API observer retains up to16 formal/diagnostic stack words and four bounded
before/after raw buffers with explicit null/unreadable/unavailable states. Exact
import/movie-site layouts include the13-argument StretchDIBits and final movie
Release; cached receiver candidates assert only BEFORE byte equality. No object
read occurs after Release. Fresh55guards and26real PE32 ABI bodies pass, including
52-byte cleanup, failed raw outputs, both patch modes and a callee making its
candidate page inaccessible. Failed attempts and their caller-frame test
limitation remain retained. Original enabled-movie/API-only run `run-xix7fqds`
closes4621pairs at53sites, zero loss/open/IO, stable sources/staged metadata, core
absent and exit0; final original Main image was viewed. GUID/output-pointer slots
and a returned100x100rectangle are raw run-local observations. Semantic interface
ownership, pixel/palette/string buffers, complete writers/failures/recovery and
all14scenario/four dispatch closures remain pending. Older evidence fingerprints
are preserved. History `defa681..a69697f` has17exact file/11behavior receipt matches
with no missing intermediate versions, separate from this uncommitted increment.
See [V2 evidence and limits](draw01-graphics-api-boundaries.md).


DRAW-01 exact client layouts/borrows (2026-10-11): contract revision2 adds141
reviewed DirectDraw/Surface2/clipper CALL layouts while excluding20reload
callbacks and preserving legacy revision1 V2 decoding.68fresh guards and26real
PE32 cases pass. Original movie-enabled API-only run `run-12h9dfva` closes4559
pairs/53sites, stable sources/metadata, core absent and exit0.42interface-output
observations/40client Releases and one successful DC interval are retained;
1992CPU-overlap,8unobserved/alias Unlock-origin and9Release-with-open-borrow gaps,
plus5terminal open intervals remain explicit. The first false DDSD_LPSURFACE
output prerequisite and its original report hashes are preserved separately.
Successful Lock returns a pointer with flags100f. Full-code static review finds
70offset128candidates; their independent receiver/alias review and expanded
coverage are next. Complete writers, resource ownership/failures and14scenario/
four dispatch closure remain pending. History a69697f..2a1885c has28file/20behavior
exact matches with no missing versions. See [evidence/limits](draw01-graphics-api-boundaries.md).

The141site layout review retains22unanchored CALL locations explicitly; no
binary function or recovered-range expansion is asserted. The earlier passing
26body ABI declaration used an unsupported scenario level; its hash-bound
limitation and the separately declared/rerun final ABI remain distinct.


DRAW-01 whole-image candidate/client-alias increment (2026-10-11): opt-in66
untyped offset128 candidates expand the API-only plan to380 admitted sites; the
core/API plan remains368with12overlap refusals. Successful exact client creation/
Surface2 query observations, generation/vtable guards and matching pointer/cleanup
now support candidate Unlock assignment.91fresh guards and26PE32 ABI bodies pass.
Original movie-enabled `run-z6gijsyt` closes6439pairs/67sites, stable inputs/source/
metadata, core absent and exit0.14query aliases and1964closed client intervals
(one DC/1963CPU,1955candidate Unlocks) leave zero resource gaps/open intervals in
this cohort. Reviewed phase14image is the faded Quick Battle screen, not final
Main. All14scenario/four dispatch closure, complete writers/callers and external
lifetime/failure coverage remain pending. History2a1885c..910e3f7 has26file/13behavior
exact receipt matches and no missing intermediate versions; old evidence remains.
See [scope/evidence](draw01-graphics-api-boundaries.md).


DRAW-01 expanded original API gameplay cohorts (2026-10-11): unchanged380site
API-only observation completes three new prospectively declared runs, each zero
journal loss/open/IO/unreadable stack/target, stable sources/binaries/metadata,
core absent and normal exit0. Campaign/save/load `run-4qxfz709` records33476pairs/
74sites,128query aliases and8508closed client intervals; original353420byte save
SHA2ba05e42437c3a81117dbda7ff490d78f2ef3eb447a6f1b52c8370f0bd29adc5 is retained,
selected and loaded, with after-load World image reviewed. Quick Battle/results
`run-6vqq61md` has20006pairs/61sites,73aliases and5289closed intervals, reviewed
Game Over/Continue/second-results/Quit. Campaign effects `run-bs6fnbfw` has29851pairs/
75sites,115aliases and7313closed intervals, passive original living owned Zombie
slot4/type14/health90 and reviewed spell/creature image. All three have zero
resource gaps/terminal open intervals. The four expanded movie/gameplay cohorts
observe101unique CALL sites and36CPU closing sites;279admitted sites remain
unexecuted. These are separate cohort counts, not aggregate runtime frequencies.
All14scenario/four dispatch closure, complete writers/callers, failed/recovery/
external lifetime paths remain pending. History910e3f7..002633e has18file/12behavior
exact matches and zero missing receipts. See [evidence/limits](draw01-graphics-api-boundaries.md).


DRAW-01 API route reconciliation (2026-10-11): current offline inventory has503
routes, including380byte/return-PC/mode-bound CALL points separate from function
entries. Four independent retained original API-only cohorts project101unique
sites across14host phases, leaving279unexecuted sites and preserving unassigned
owners/unknown types.45native guards pass; require-accounted passes0unknowns and
require-complete fails with the expected18obligations (14scenario/four dispatch).
Scenario limitations now name actual original save/load/results/summon observations
without promoting census closure. Catalog source-only changes retain historical
original evidence hashes/staleness; the new proof validates offline accounting,
not current original renderer equivalence. History002633e..c43bfde has9file/
12behavior exact receipts, no missing versions. See [accounting scope](draw01-inventory.md).


DRAW-01 original queue client-state increment (2026-10-11): separate headless
native integration fixture executes byte-unchanged5002a0consumer58320unique cells,
empty/one/two queues, Video0/1, selected9/15/default kinds, signed spans/changed
prior metadata, clip480/600branches, rawhigh/negative returns and optional postlude.
Original row/container mutations, strict/clamped clips and exact original callers,
29160cleanup calls (includingemptyqueues) and counter resets pass.75312clip/
25500wave/19440copy calls are retained with exact raw-case/binary hashes. First
sandboxSIGSYS/unsupported scenario declaration and intermediate pass remain under
explicit attempt limits. Actual helper effects, pixels, other changed-prior kinds,
external API/lifetime and14scenario/four dispatch closure remain pending; recovered
implementation/comparison/integration/replacement statuses staynone. History
c43bfde..0b40de2 has11file/21behavior exact receipts with no missing versions.
See [client-state scope](draw01-inventory.md).


DRAW-01 all-kinds consumer client state (2026-10-11):984960unique unchanged-original
consumer cells pass across kinds-2..35, direct-marker/ordinary-offset, four raw
returns, Video0/1, empty/one/two rows, signed first spans/changed prior metadata,
480/600clip branches and optional cleanup.93exact CALL return PCs,1271936clip/
544000wave/492480cleanup calls are retained as offline counts. Raw559379392byte
output and16054745byte gzip decode to the sameSHA524f76b1099db750656eaa8494a2fca2d3fef3224bd602fafe999ad9125584de.
Original/source hashes are stable; the earlier58320cells remain historical without
refresh. All numeric/default table entries now link the selected client-state
contract, but actual helper effects, indexed/-127 changed-prior variants, pixels/
objects/lifetime and14scenario/four dispatch closure remain pending. Recovered
native implementation/comparison/integration/replacement remainnone. History
0b40de2..23bd1c0 has14file/85behavior exact receipts and no missing versions.
See [full scoped evidence](draw01-inventory.md).


DRAW-01 original render-postlude refinement (2026-10-11):12288owned unchanged-original
client cases cover4d8ab0enabled/readiness/null-Lock exits, signed text, real frame/
clip stores, aligned row metadata and ignored synthetic failure returns. The
route's old cleanup/lifecycle label is refined to composition; old evidence hashes
remain historical. [Evidence and retained failed expectation](draw01-inventory.md#original-optional-render-postlude-2026-10-11)
exclude actual canvas/font/COM bodies, pixels, lifetime and native equivalence.
Current45offline guards account503routes with zero unknowns;14scenario/four dispatch
closure remain pending.23bd1c0..1fe53d9exact committed-history review has10file/29behavior
receipts and no gaps. Independent recovered native statuses remain none.


DRAW-01 original queue object variants (2026-10-11):4924800unique unchanged-original
client cells pass direct/indexed markers, ordinary/-127offsets, changed prior
metadata, all kinds, low-AX1/high-EAX distinction and clip/postlude policies.
The raw/decompressed2972130848byte archive hashes match, native exit0/no timeout;
[Evidence and rejected pre-execution launch](draw01-inventory.md#original-indexed-and-offset-consumer-states-2026-10-11)
retain historical narrower results. Actual helpers/pixels/arbitrary objects/
lifetime and14scenario/four dispatch closure remain pending. Recovered native
implementation/comparison/integration/replacement stay none. Committed
1fe53d9..b85df54history has15file/86behavior exact receipts and no gaps.


DRAW-01 original Lock recovery client (2026-10-11):13824owned unchanged58b660
scripted-interface cases pass descriptor/pitch/pointer, lost/Restore/retry, key/
reload endpoints, teardown globals/DestroyWindow and conditional Unlock.1296cells
distinguish stored Restore from retryLock result;2304return a nonnull scripted
error descriptor, without asserting real valid storage or driver recovery.
[Evidence and retained cardinality error](draw01-inventory.md#original-lock-recovery-client-2026-10-11)
exclude populated/reentrant destructors, physical lifetime/pixels/reload/UI.
Fourteen exact caller-effect routes are classified;45current offline guards
account503routes, preserving14scenario/four dispatch closure. Recovered native
stages remain none. b85df54..d3cd04fhistory has11file/30behavior exact receipts,
no missing versions; historical live fingerprints are retained.


DRAW-01 reviewed API client route binding (2026-10-11):141existing exact DirectDraw/
Surface2/clipper declarations now require matching static build/source hash, CALL
bytes, primary PE32 formals, explicit receiver provenance and route classification.
239sites retain no client contract;20reload callback exclusions and empty
discovered owner sets remain explicit. [51current native guards](draw01-inventory.md#reviewed-exact-api-client-declarations-2026-10-11)
pass with503routes/zero unknowns and18completion obligations. This validates
offline interpretation, not new original execution, dynamic ownership/pixels/
lifetime or native graphics replacement; historical live sources remain stale.
Committedd3cd04f..f30b3b6history has17file/45behavior exact receipts and no gaps.

DRAW-01 now has exact per-run core allocation binding metadata alongside its
unchanged entry/return journal. Byte-checked relocation and continuation mapping
separates exact relocated E8 return PCs from E9 transfers, module membership and
unresolved logical callers. Thirteen offline guards and the actual PE32 binding
writer/decoder plus existing ABI checks pass. Prepared maps do not establish
installed hooks, complete callers/writers, resource lifetimes or scenario/dispatch
closure; the 14 scenario reviews and four dispatch gates remain pending.

DRAW-01 caller review now preserves physical PCs while admitting exact relocated
original CALLs and separately recording same-thread/same-return-slot inherited
continuations. Fourteen synthetic guards pass. Current native replay of the
unchanged 42,021-pair original menu journal resolves eight relocated Lock calls
and 17,182 original direct calls; 24,831 call-flow pairs remain explicit, including
24,825 raster indirect continuations and six destruction continuations. No new
original execution, semantic transfer edge, scenario or dispatch closure is
claimed. The prior original binding cohort and all historical hashes are retained.

DRAW-01 now retains a separate whole-image CPU call census: 127 exact CALLs
through four raster globals plus the CRT register callback CALL at59d98a, and
18 exact immediate pointer assignments (nine with discovered initializer484470,
nine with unknown owners). Twelve selection/refusal guards pass. Target sets
are static store facts; before-CALL target values, branch execution, alias writers,
raster semantics and integration of these additional128sites remain pending.
This does not close any of the14scenario/four dispatch gates or promote native
implementation/comparison/replacement of the recovered initializer.

DRAW-01's separate original-active128site CPU target observer now passes its
12admission guards and an original menu cohort:25071CPU pairs/42008core pairs,
zero loss/open/unreadable/IO/unassigned phase, matching core/API-wire allocation
identities, stable sources/metadata/binaries and normal exit0. Five menu sites
observe24825before-target values596cb8 through6903dc; the CRT site observes246
callbacks with177distinct targets.122CPU sites remain unexecuted. Graphics API
coverage, permanent/atomic bindings, caller crosswalk, aliases/lifetimes and all
14scenario/four dispatch closure gates remain independent and pending.

DRAW-01's current native CPU/core crosswalk now matches all24825menu raster
pairs using exact actual CALL return, thread, target, stack/cleanup, six diagnostic
words and BEFORE/AFTER registers/flags/LastError.16synthetic guards pass; current
replay preserves both independent journal orders/full group cardinalities and
all historical hashes without new original execution.246exit callbacks retain
unreviewed wrapper/non-core flow, including four observed core wrapper entries;
ordinary direct calls are outside this CPU-only crosswalk, and two outer cleanup
continuations still need transfer review. No typed CPU signature, atomic target,
writer/lifetime/pixel or14scenario/four dispatch closure is claimed. Committed
02addc6..e3ee037history has36file/89behavior exact receipts with zero missing.

DRAW-01 original canvas release/terminal wrapper client evidence now covers6217
distinct isolated cells:5184release/shared-cap/list/current/count/object cases,
1024callback flag cases (two invocations each) and nine optional CloseHandle
cases.864cells confirm one shared1000iteration cap still executes a second
surface Release;864confirm controlled unsigned count0decrement. Five existing
core/API routes link these bounded side effects. Scripted interfaces do not prove
physical reference/storage lifetime, invalid/null/reentrant paths, all list shapes,
callback registration or actual exit wrapper correlation. Four exact wrapper
addresses retain missing discovered function bodies. Immutable2927inputs and
original/source bytes verify before/after; native stages and14scenario/four
dispatch closure remain pending. e3ee037..c44885fhistory has26file/34behavior exact
receipts, no missing intermediate versions.

DRAW-01 current native terminal replay now matches all four captured CRT
wrapper-to-core tails against unchanged pinned original wrapper bytes and the
retained staged executable, complete exact thread/stack/target groups and
reviewed register/flags/RET0 effects.12state/group/byte/proof guards pass; initial
one-shot flag byte remains unobserved and AF undefined.24825direct raster pairs
plus four wrapper tails are matched;242other callbacks and two outer cleanup
root continuations retain independent pending flow/classification. No new
original execution, callback registration, physical lifetime/pixels, all writers
or14scenario/four dispatch closure is asserted. c44885f..034ce79history has15file/
34behavior exact receipts, no missing versions.

DRAW-01's opt-in88core/128CPU parent-frame cohort now passes11guards, fresh
128CALL planning/942direct-call export and actualPE32 compilation, while keeping
default83safe hooks/plans separate and refusing unsafe fade58edb0. Original menu
run closes41451core/24834CPU pairs with zero loss/open/unreadable/IO, matching
actual allocations, stable sources/staging and exit0.24592CPU/core matches include
24588rasters/four CRT parents; six inherited child frames identify four wrappers
and both outer cleanup parents with original roots4e9dee/557956. The third outer
CALL is unexecuted; inherited transfer edges, initial flags/registration,242other
callbacks, physical lifetime and all writer/API/side-effect/14scenario/four
dispatch closure remain pending. All raw artifacts/binaries survive stopped
disposable cleanup.034ce79..40a16e0history has28file/51behavior exact receipts and
zero missing intermediate versions.


DRAW-01 CPU/terminal route accounting (2026-10-11): reviewed inventory expands
from 503 to 636 routes with separate exact128CPU selectors and five explicitly
opt-in terminal lifecycle entries. Current59native guards and retained original
menu phase projections account for all reviewed inputs without promoting any
scenario/dispatch gate: 14+4 remain. Six CPU sites observed/122 unexecuted;
380graphics sites/141reviewed clients/101observed remain separate. Current
native planning preserves default83safe/one unsafe fade refusal and opt-in
88core/128CPU. Source/catalog changes intentionally leave earlier original
observer plans/journals historical; new native planning does not renew ABI,
original execution, full writer/caller/side-effect/lifetime or live equivalence.


Inherited terminal edge review (2026-10-11):14current native guards and a
source-bound replay of the retained88entry original menu journal verify all six
observed inherited transfers to58b3e0: four fixed/one-shot wrapper tails and two
outer4a37d0 tails. Original full-body bytes and per-run core bindings remain
pinned; the active same-thread/RET0 return slot, fixed receiver, bounded
MOV/TEST/OR register/flags/error effects and identical child/parent return state
are checked. Each outer tail requires exactly one preceding original CALL at
4a37ef, receiver642000, stack one word below the parent, and its actual returned
state reaching the641fc8 tail at4a37f9. Optional CloseHandle effects are not
assumed to preserve outer entry registers. Raw physical callers remain intact;
a tail transfer never becomes an original CALL edge.

Reproduce with `python3 tests/test-drawing-terminal-edges.py`, then the
`tools/review-drawing-terminal-edges.py` CLI with a new prospective declaration,
retained `run-_pwj3is8` and a new output path. Both original manifest checks pass
2927files; sources/artifacts stay stable. No original execution is repeated.
Zero remaining inherited edges in this journal is a bounded transfer result,
not complete semantic caller ownership. Diagnostic stack consistency does not
prove readability. Initial flag/handle state, callback registrations, the third
outer root,242other callback invocations, all pointer/writer/API effects and
physical lifetimes remain pending. All14scenario/four dispatch gates remain open.


Compact CPU journal increment (2026-10-11): explicit compact CPU/core staging
reuses the strict128byte V1 wire,4194304record cap, eight diagnostic stack words,
actual target/registers/flags/LastError/thread/SP/cleanup and per-run bindings.
Typed formals and pointer/output snapshots are absent. Four exact raster-global
namespaces and the59d98a CRT EAX CALL alone are admitted; default graphics API
V2 remains800bytes/1048576records. Readers require the explicit compact/no-formal/
no-buffer manifest and validate the actual wire independently. They never pad
invented V2 fields. `prepare-drawing-terminal-census.py --compact-cpu` requires
an explicit fresh approved plan SHA, beside all88actual core entries.

Current54native guards pass (10admission,19CPU-crosswalk,14wrapper-crosswalk,
11terminal staging). Both native ABI executions pass26synthetic bodies each,
including13JMP/13INT3,0..52byte cleanup, all13actual callee arguments, machine
state/LastError, negative EAX, foreign breakpoint, loss/cap/IO and binding/decoder
round trips. Default V2 additionally retains its formal/buffer/candidate checks;
compact V1 reports those captures absent. Actual current PE32 staging produces
88core/128CPU with V1/no-formal/no-buffer scope and original manifests2927.

The attempted original menu capture hit temporary-filesystem quota before
closed journals and a valid journey report. Its empty/partial outputs and quota
log are retained under `run-3zug99fn` and
`working/tests/draw01-compact-cpu-original-quota-attempt-20261011.json`; original
runtime validation remains explicitly pending. Source/native fixtures do not
renew original transparency, callback registration, physical lifetime or the
14scenario/four dispatch closures. Old observer source hashes are preserved.
Own stopped successful ABI prefixes were removed after retaining binaries,
reports and raw journals/maps; no other contributor outputs were cleaned.

### DRAW-01 integration with current master (2026-10-11)

The isolated integration combines DRAW-01 increments through `13ceb66` with
master `8ed5317`, retaining the other contributors' DRAW-02, DRAW-03, native
rendering and storage work. The five shared workflow/accounting/documentation
files were reconciled; all historical evidence records and exact receipts were
preserved. The committed-history report
`coverage/draw01-master-merge-history-20261011.json` reviews 89 commits and
2,151 watched file versions against their first parents with zero missing exact
receipts. This is retrospective accounting, not a past gate pass or new engine
comparison. The merged source census is newly pinned; historical fingerprints
remain unchanged.

Merged-tree Python checks and the coverage audit/gate are integration checks.
The compact original-game capture remains pending after the filesystem-quota
failure; the 54 compact guards and 52 native ABI cases remain scoped evidence
from increment 39. DRAW-01 still has all fourteen scenario and four dispatch
closure obligations open. This merge does not promote recovered comparison,
live equivalence or replacement status.


## 2026-10-11 DRAW-02 native physical presented-session backend

A strict owned-history backend now implements the independent session ABI through
source-only native composition and an application GPU viewport. It acquires complete
X11 screen pixels after buffer swap, with zero viewport QImage uploads and native
surfaces released. All65,536RGB565 colours and three menu/loading/results frames
match independent synthetic expectations. Seven executed controls include physical
screen containment, non-X11 refusal, unsupported caller effects, oracle-field
refusal and preservation of existing outputs; fourteen protocol guard methods pass.
All103retained startup frames match historical original primary readbacks through
actual native X11 presentation:49,440,000pixels,zero mismatches. Source claims are
prospective and stable; original manifests pass before/after. Earlier failed context
sharing, asset case-directory and undersized-Xvfb attempts remain in working.

The original references retain their observed_primary_blt boundary; this does not
assert new synchronized original physical acquisition, complete semantic input,
general battle/save/load ownership, native caller effects or real-driver recovery.
DRAW-02 fourteen-journey/twenty-one-operation gates remain open. The final prior
integration range300e423..9df6aa3 has nine exact file-version receipts and no gaps.
See [owned presentation protocol](../formats/drawing-presented-session.md) and
[immutable execution report](draw02-native-physical-session-20261011.json).

## 2026-10-11 DRAW-02 synchronized original/native physical witnesses

An opt-in post-copy fence now holds the original drawing thread at the two
already guarded opaque primary-copy sites while an independent X11 peer acquires
the entire client plane twice, checks stability and acknowledges exact frame and
attempt identities. Fresh original execution retains 103 physical witnesses,
source/queue/canvas/thread/menu-phase bindings and five successful menu dispatches
through 16 World returns. Other original threads remain active.

The first physical admission failed because bit-replicated RGB565 expansion differs
from the tested Wine display path. All 49,440,000 original physical pixels match
fixed nearest normalized expansion, including all 32/64/32 channel levels. The
explicit native presentation policy expands only native-derived words, supplies
no measured colour table and uses zero viewport QImage uploads. Both policies pass
independent exhaustive 65,536-colour presentation tests, eight physical/refusal
controls and fifteen admission guards.

Fresh original-screen versus separately owned native-screen comparison passes
all 103 frames / 49,440,000 pixels with zero mismatches; five physical admission
guard methods pass. Original manifests pass before and after both original
acquisition and comparison. Failed compile and bit-replication attempts remain
retained under working; no historical evidence hashes were changed.

This closes synchronized physical point acquisition and exact parity within the
guarded startup prefix. It does not close the fourteen-journey/twenty-one-operation
DRAW-02 gate: alternate/global publication, general battle/save/load producers,
semantic input/caller effects and true driver loss/Restore remain open. Exact
committed-history review covers 14 file versions in 9df6aa3..5bb6dce with no gaps.
See [physical fence protocol](../formats/drawing-physical-fence.md),
[original execution](draw02-fenced-original-20261011.json) and
[paired comparison](draw02-fenced-screen-comparison-20261011.json).

### Combined-source physical acquisition review (2026-10-11)

The DRAW-02 physical increments are reconciled with concurrent master8ed5317,
preserving both optional capture paths, original/native separation and all
append-only evidence. Fresh combined-code original acquisition again retains
103 stable full-window witnesses through 16 World returns. Independently composed
native X11 output matches all103frames/49,440,000pixels with zero differences.
Both native RGB565 display policies, all65,536colours, eight physical/refusal
controls and fifteen admission guards pass again. The original manifests pass
before/after original acquisition and paired comparison.

Exact history review covers 2,839 parent/current file versions across 30 commits
in5bb6dce..336edd1and9df6aa3..8ed5317. Seventy-five missing intermediate receipts
are appended retrospectively with affected behavior/gap identities and pending
validation; no past gate execution or new engine comparison is inferred.
A transient conflict-resolution truncation was detected before original execution,
restored from the untouched index and excluded from admitted evidence. Shared
source edits leave other historical subsystem results pending current freshness;
the new executions support only the declared physical point/presentation policies.
Full DRAW-02 journey, input/caller-result and real-driver gates remain open.
See [combined original](draw02-integrated-fenced-original-20261011.json),
[combined native](draw02-integrated-native-physical-20261011.json) and
[combined paired result](draw02-integrated-fenced-comparison-20261011.json).

### Concurrent DRAW-01 physical integration (2026-10-11)

The subsequent DRAW-01 census merge2cff6ae is retained alongside the physical
DRAW-02 increments. Its31additional behaviors change no fingerprinted physical
source or original-build binding; both physical policies retain current scoped
implementation/integration support without widening their claims. The combined
census records1870source/build files and1669explicit links.

Additional exact history review covers5434file versions across70commits in
8ed5317..2cff6aeand336edd1..d462b1c. Thirteen missing parent/current versions at
the final merge edges now have retrospective pending receipts, including their
affected behavior identities. Historical engine results remain historical; no
past gate pass, new original comparison or full DRAW-02 completion is inferred.

Integration candidate combines resident work with masterd955801's45incoming
DRAW-01/02/03 commits. Exact committed-history review records1262file versions
and zero missing receipts in `coverage/history-resident-incoming-master-20261011.json`.
Coverage/register/review histories are reconciled by stable IDs without dropping
incoming or resident findings;both ledger histories are retained. One executed
scene dependency changed: optional drawing-publication physical fence. Its
activation requires explicit DRAW02 publication/fence flags,excluded by the
resident route's clean environment. Both publication/fence headers are now
explicit resident validation dependencies before new declarations. Fresh owned
corpus and complete merged-tree TITAN gameplay are required;the earlier passing
result remains historical for this dependency change. No imported comparison
hashes are refreshed and no broad equivalence is asserted by reconciliation.

The complete owned corpus passes on the combined master candidate in
`public-resident-master-integration-fixtures-20261011.json`,with unchanged sources
and inputs. World replay/import565,no-readback/HUD/copy/cleanup and strict bounds,
minimap four views/candidate vs full scan,shadow/source-epoch/deferred/worker/leaf
ABI,balanced acknowledged Quit/refusals and Preferences all pass.19 Python
evidence checks pass too. This renews current native integration after the
publication-header dependency change;the complete TITAN rerun is in progress.

The first combined-master journey completes all functional/normal Quit gates,
4976balanced Worlds,19.338278aggregate engineFPS and zero delivery readbacks,but
fails one cycle0 window(6native/5painted frames in1.002s). Negative
`public-resident-master-window-negative-20261011.json` retains actual input and
source bindings. Capture owner maximum9.726ms does not account for that delay;
old unaligned counters cannot establish exact source/host causality. New read-only
telemetry aligns source publication and FPS windows on monotonic timestamps and
logs at most256host polls with>=50ms work or>=100ms dispatch gaps,separating World
and command work. It is enabled explicitly only by the resident smoke runner;
no game/renderer behavior,budget or failed-window filtering changes.19smoke and
3publication-sampler checks pass. Current merged live/performance proof is still
pending;source-only telemetry changes retain stale historical bindings honestly.

The first aligned run stops on the finite60s party-lethal deadline,with verified
Zombie melee,Wiz65HP and engaged Redcap13HP in final original snapshots;another
enemy had drawn the combat Zombie onto a different target. Negative
`public-resident-master-combat-deadline-negative-20261011.json` retains source,
actual input and bounded host timings. Startup resident polling took622ms(606.55ms World initialization)
outside active gameplay;logged command work stayed below16ms and no large
post-startup dispatch gap was logged. This is not proof of renderer latency or
complete FPS success. Completion observation is now bounded at120s;same engaged
enemy party lethal,Wizard aliveness,casting and every rendering FPS floor remain
mandatory. No AI/health/state/balance changes;fresh full integrated proof pending.

The120s completion run also expires: final snapshots show Wiz114HP targeting
enemy6,combat Zombie dead,engaged Redcap32HP and initial Zombie idle. Negative
`public-resident-master-combat-diversion-negative-20261011.json` retains input,
source and timing data. Waiting alone does not fix the clearance strategy. The
helper now retains original wizard engagement while the correctly placed adjacent
summon assists,and retreats only after the same-engaged-enemy party kill. All
survival/casting/portrait/normalQuit/renderer gates and120s finite bound remain;
no original gameplay state or balance change. Fresh integrated proof pending.

Combined master/resident final proof (2026-10-11):
`public-resident-master-final-campaign-20261011.json` retains the complete successful
private NVIDIA TITAN RTX campaign, actual input, capture timings and bounded
clock-aligned host poll rows. 4595 completed/imported/verified GPUWorlds,
19.160183 engineFPS; all unchanged phase, portrait and worst-window gates
pass (minimum sampled paint window 15.454545FPS, portrait worst window
19.841270FPS). Zero World/native/RGBA delivery readbacks,
80B ACK, no World pixel reply; real DirectDraw disabled and absent. Actual
summoning, Fireball damage, Zombie melee and same-enemy wizard party lethal,
blocked casts, physical movement, post-kill retreat, five portrait cycles,
Mini Cancel/resume, Quit No/Yes/Main and private original/shell exit all pass.
Stable portrait region maximum channel error1. Largest measured diagnostic
capture owner work 9.696ms; this is existing diagnostic capture, separate
from zero-readback resident World delivery. The helper now preserves original
wizard engagement until the verified kill, then retreats; the finite completion
window is120s. No actor health/positions/AI, balance or FPS checks are changed.
`public-resident-master-final-fixtures-20261011.json` refreshes the full owned
corpus on these exact final sources (7.68M candidate-minimap compared words,
exhaustive65536-word GPU565 import, strict malformed/ownership/shutdown refusals).
Original immutable-input manifests, compiled source closure and declared inputs
remain stable. Reviewed incoming master history8ed5317..d955801:45commits,
1262exact file versions, zero missing receipts; own551e6db..e1e9553:9versions,
zero missing. Coverage histories retain exact review reports independently of
new execution evidence. Native scoped live observation is renewed; full CPU UI
destination reads/lifetimes, independent World/UI/driver equivalence, movies,
all COM/loss-restore semantics and60FPS remain outstanding. Prior failed
latency windows and combat strategies remain preserved; one successful bounded
journey does not establish universal cold/concurrent-load latency.

Final combined-tree original comparisons were rerun after host/publication
source changes. `public-displacement-master-final-20261011.json`: two pinned
596490/5968a4 leaves,960000WORDs,551covered/10physical-stride wrapped samples
match native CPU/two GPU backends. `public-shadow-master-final-20261011.json`:
two captured geometries,2.88M original/5.76M native compared WORDs,whole
projected-envelope refusal,exact-edge equality and vertical-crop refusal match.
Original manifests and source/input hashes remain stable. Both retain scoped
original comparison; complete ABI/RGB555/live replacement is excluded.

The first merged-tree fenced comparison used256MiB diagnostic completion
storage. Physical acquisition passed88witnesses, but the producer journal
explicitly records832budget failures(code11); native composition correctly
refuses that incomplete input. Negative result and exact logs are retained in
`draw02-resident-master-diagnostic-budget-negative-20261011.json`. Original
frames are reference-only. This is not a passed pixel comparison or a resident
gameplay failure; repeat with the established1GiB diagnostic budget is separate.

Imported drawing milestones also pass on the final merged tree.
`draw02-resident-master-fenced-original-20261011.json` retains103fresh guarded
original post-copy X11 witnesses (49440000pixels) through sixteen World returns,
stable double captures/exactACKs, original work active, no refused producer
inputs and immutable manifests.
`draw02-resident-master-fenced-comparison-20261011.json` compares all103frames to
separately source-owned native physical presentation:49440000pixels,zero
mismatches,no original reference inputs to native rendering. This renews only
the finite fenced boundary; all original routes/threads/journeys remain excluded.
`draw02-resident-master-native-physical-20261011.json` passes both fixedRGB565
display policies over65536words,eight physical/refusal controls,15admission
guards and103retained historical frames/49.44Mpixels/zero differences.
These original-active diagnostic readbacks are separate from the public resident
zero-World-readback path; they do not reintroduce real DirectDraw there.

### Resident CPU/GPU/pacing profiling, 2026-10-11: instrumentation

`NR.resident-stage-profiling` seeds opt-in bounded renderer-context timestamps,
public CPU/original-tick rows, source-only ordered capture and a frozen NVIDIA EGL
resident replay benchmark. The new policy preserves original speed/pacing and
draw skipping. Fresh TITAN synthetic query overflow/identity/exception/destruction
and pixel stability controls plus six malformed capture controls pass in
`resident-profile-instrumentation-20261011.json`. Full public gameplay observation,
fresh CPU stacks after the minimap fix and representative completed replay
throughput remain pending. GPU intervals include CPU submission starvation;
original non-pacing residual is unclassified, not simulation CPU. No60FPS, original
comparison or replacement promotion. Historical linked renderer/public evidence
retains its original hashes and may now be stale. Merged-master first-parent
versions are separately receipt-audited with zero gaps in
`coverage/resident-profile-base-history-20261011.json`.

### Resident profiling, 2026-10-11: capture and pacing observation controls

The synchronous capture failure is preserved in
`resident-profile-synchronous-capture-negative-20261011.json`, including its
946.324ms write stall, fallback assertion and independently repaired private
cleanup. It is not a completed gameplay or performance result. The source-only
capture now uses a bounded256MiB compression/I/O worker; failures mark incomplete
capture while gameplay continues. V2 inflation/byte normalization occurs outside
benchmark timing. The public clock owner can forward its existing paired readings
to `ScenePacerObservationV1` without another clock/host call or IAT hook; scene
diagnostic timestamps match the configured real uptime clock. Original target,
instructions, speed and drawing policies remain unchanged.

Fresh exact Qt/Python archive controls,11 compressed malformed inputs,33 clock
byte mutations/LastError/paired callbacks and current exhaustive shared import/
normal-Quit regressions pass in `resident-profile-worker-fixtures-20261011.json`.
Full current public CPU/GPU/pacing and actual captured hardware throughput remain
pending. No original comparison/replacement or60FPS status promotion. Exact
`19882a4` source receipts are separately reviewed in
`coverage/resident-profile-instrumentation-history-20261011.json`; older evidence
retains historical source/contract fingerprints.


### Resident profiling, 2026-10-11: fresh CPU/GPU/pacing diagnostic

`resident-profile-cpu-pacing-20261011.json` completes physical casting/combat,
movement and normal Quit with5421 balanced resident Worlds and zero ordinary
readbacks/pixel replies. All5358 warmed rows observe both original pacing call
sites. Host acquire/decode/handoff/consume averages25.420ms (p9538.057), GPU
timeline9.707ms (p9520.070), pacer9.388ms and unclassified residual9.869ms.
Native minimap preparation is the largest resolved sampled symbol at15.88%.
The overall19FPS cadence gate failed; instrumentation/symbolization perturbs
cadence and no60FPS or uninstrumented integration status is promoted. A second
compressed capture fails on drawing-history GAP, retaining1857 completed Worlds
and independent cleanup/original verification. Full clean corpus/offline replay
remain pending. Symbolization is now deferred until owned campaign cleanup.
Committed19882a4..78af697 exact26-file history has zero receipt gaps; historical
source fingerprints are retained. See the profiling document for scope/limits.


### Resident profiling, 2026-10-11: completed combat-prefix hardware measurement

The complete capture journey remains failed: clean Apprentice attempts retain
wizard death and repeated native drawing-history GAP; one serialized GAP occurs
after2423 balanced Worlds, without concurrent coverage. That failure is linked
under both `NR.resident-stage-profiling` and `NR.public-resident-world`; its source
transition/cause is unclassified. Historical master successes are unchanged.
Explicit prefix admission passes eighteen new refusal controls and validates the
whole capture before excluding its pending2424th World. Default full-journey
admission remains strict. This scope supplies actual combat rendering inputs,
not a positive full native menu/spell/portrait/Quit claim.

Frozen TITAN NVIDIA EGL replays complete2423 verified Worlds at78.289/71.963FPS
with public-matching O2 flags and88.070/85.655FPS at O3. Timestamped runs complete
at77.705/85.889FPS. All six terminal word hashes match, with zero ordinary
readbacks and zero remaining queries/surfaces; sources, frozen builds, game
inputs and original manifests verify. Warm decode/handoff/consume averages
11.602ms O2 and10.073ms O3 versus26.384ms for the same public captured prefix.
All97 compiled C/C++/header hashes match. Public Zink/Qt shared-context work,
original simulation, capture/report I/O and compiler settings differ from
headless replay, so attribution is pending. Pipeline p9519.874/17.621ms exceeds
16.667ms: this is averaged offline60FPS headroom, not steady/public60FPS or
original equivalence. Comparison/replacement statuses remain independent;
no status promotion or production render/pacing/balance change is made.
See `native-resident-performance-profiling.md` and the matched-input JSON record.
Deferred CPU symbolization awaits a fresh sampled execution; growing live report
publication remains a separate diagnostic cadence limitation.
## Resident drawing-history admission investigation — 2026-10-11

The retained profiled GAP is traced to ordered-copy timeout, pixel epoch
invalidation and local session GAP1/command GAP2. Fresh owner-qualified Unlock
markers reproduce a cleanup/release-region hold at325ms, without identifying
its free/log/mapped-pump suboperation. A separate fresh run proves public
resident binding bypasses the callback/copy admission domain and times out
against cursor-copy tracker ownership at observed16ms. One earlier fresh
attempt reaches2,880 balanced Worlds without GAP but fails its retreat waypoint.
All outcomes remain negative journey evidence; no continuity repair, completed
Quit, uninstrumented FPS, replacement or original pixel equivalence promotion.
See [the investigation](resident-drawing-history-gap.md) for source-bound
failure chains, exact retained reports and pending admission/publication work.

## Resident drawing-history admission/publication repair (2026-10-11)

`NR.resident-bind-admission` and `NR.callback-publication-isolation` record native
scheduling policies independently from original-driver equivalence. A short
cursor copy/tracker owner made the pre-repair public World bind exhaust its
8ms tracker-only budget. Binding now joins callback/copy admission first.
Fresh PE32 baseline/fixed controls retain the refusal and prove exactly one
owned generation/nonce World, original Lock/Unlock results/LastError, poisoned
borrowed storage and acknowledged FIFO/END.

A fresh isolated TITAN campaign after that repair reaches1,309 balanced Worlds
then records Unlock phase47 at1,030ms: synchronous mapped-ring pumping still
holds the50ms drawing lease. Tracker release now delegates publication to an
already-created worker, retaining finite/no-worker fallback and explicit
lifecycle pumping. A selftest-only100ms callback publication delay produces
two pumps and211.3365ms Unlock before repair; after repair there are zero
callback pumps and9.2083ms observed Unlock. These are observed timings, not OS
scheduling bounds. Six PE32/GPU controls pass209 independent whole frames
(four valid sessions/two expected GAPs), and all eight reader-pressure cases
pass. The exact public rerun finishes7,091balanced Worlds, two menu returns and
normal Quit without GAP/timeout/fallback/readback. It remains an overall failed
benchmark: one3.243s paint sample and capture truncation at183/7,091Worlds.
Ordinary unprofiled performance is pending; complete instrumented capture,
whole-original equivalence and broad lifecycle matrix remain unproven.
See [drawing-history investigation](resident-drawing-history-gap.md) and its
separately retained failed/successful evidence.

Final header compatibility declares the publication helper `static inline`;
strict optimized/ASan/UBSan offline queue tests pass, and a fresh PE32 control
passes with a byte-identical rebuilt selftest DLL. Historical source hashes
remain unchanged. The separate ordinary TITAN journey then passes three120s
phases, two menu returns and normal Quit:7,284balanced Worlds,19.6335engine FPS,
32.98..33.55average paints/s, no timeout/GAP/fallback/readback and clean
retirement. This supports bounded ordinary public continuity/performance;
the captured benchmark stays failed/truncated, and original equivalence,
final-header casting/combat matrix and60FPS remain unproven. See
[ordinary TITAN result](resident-history-repaired-ordinary-titan-20261011.json).


### Original engine cadence observation controls,2026-10-11

`NR.engine-cadence-observation` adds an isolated opt-in No-CD DLL tracing primary
message-loop pacing, accepted-update/world-counter stores, eight global ANI
controllers, creature maintenance/budget scheduling and actual World-queue entry.
Original speed, interval, clock, draw policy and once-only calls/stores remain.
At most262144x96-byte events stay in memory; file writes occur only on process
detach after gameplay. Guarded staging verifies the source PE/hash and all76
site bytes, preserves the existing scene queue's original return PC, and refuses
overflow/foreign-thread/incomplete records. Seven PE32 thunk paths, ten stand-in
forwards, machine-state/LastError/marker stores and14 malformed decoder refusals
pass at final source hashes. Two prior address-reservation setup failures remain
separate; neither executed hooks or establishes an engine defect.

This is native diagnostic-policy validation, not original routine/pixel/driver
equivalence or a live scheduling contract. TITAN casting/combat/menu/Quit tracing,
all-animation/pause/network/alternate pacing branches, independent render-only
invocation and60FPS remain pending. See[experiment](engine-cadence.md).
The exact27ed7f8..e692a4f history review covers nine files with zero receipt gaps;
no retrospective gate or execution claim.


### Original engine pacing/update/World coupling,2026-10-11: TITAN pass

`NR.engine-cadence-observation.titan-20261011` records4545 balanced GPU Worlds
at19.29295 aggregate engine FPS, actual casting/Fireball/melee/lethal, both spell
refusals,movement,five portraits,Mini resume and normal Quit. NVIDIA TITAN RTX/Zink
is accelerated; source stability,owned Wine cleanup and immutable originals pass.
No delivery readback/pixel reply/pending World remains, and producer retirement
is clean. The final controlled/live observer DLL is identical.

All4355 complete advancing update windows have1 counter increment,8 global ANI
calls,1 budget-scheduler call and1 World queue at50 ms target/20 companion. The
189 non-advancing windows each produce1 World queue while omitting those selected
steps and creature maintenance. The accepted-update sequence still advances across all189 non-advancing windows.
Counter identity/order holds throughout. Mode
+fd5a is sampled as a low byte; full mode,other timers/updates,all animations and
original/driver/pixel equivalence remain excluded.154158 events flush only after
gameplay, without overflow or foreign thread. Final incomplete update is excluded.

The shared frame cap cannot be treated as rendering-only. Existing non-advancing
drawing is a candidate separation boundary, but still runs other world work,
including an unresolved conditional ten-call subsystem branch; do not toggle
the mode or lower pacing before repeated-draw state invariance is proved. Selected
scheduler/maintenance averages are small, not attribution of all simulation CPU.
60FPS and safe independent render production remain pending. `GP.tick` keeps
partial understanding/no native implementation/no original comparison; its prior
static findings and this bounded native diagnostic remain distinct.

`NR.public-resident-world.engine-cadence-campaign-20261011` independently retains
the full source/compiler-bound final-header gameplay regression, including portraits
and normal Quit. This instrumented pass does not replace ordinary uninstrumented
results or the retained capture overload/performance failure. See[experiment](engine-cadence.md).
Exact e692a4f..7c0e46a history review:19 files,zero missing receipts,with no
retrospective gate or validation assertion.


## Bounded repeated original scene production controls (2026-10-11)

`NR.scene-production-repetition` adds an opt-in, fail-stop diagnostic for original
004fd280 and candidate004fd315..004fd5ad. It leaves the50ms pacing/simulation
unchanged and restores the original normal return ABI. Exact protected snapshots
cover whole entity pools, ANI, RNG, scheduler, gameplay/map-context headers and
counters; module/renderer changes and ordered scene rows remain separate evidence.
All18 PE32 stand-in controls pass, including mutation/shape/unreadable refusals,
28 guard mutations, stack/flags/FP/LastError preservation. Three fixture negatives
are retained with their original hashes and corrections; no original engine defect
is inferred from them. Static helper roles remain partial; original live repetition,
full nested/map/visibility/timer/pixel safety and60FPS are pending.
The exact7c0e46a..6cdf7a4 history review covers8 files with zero unresolved receipts;
this is accounting, not a retrospective gate or execution claim.
See[experiment](scene-production-repetition.md).

## Original scene repetition failures and history-isolation candidate (2026-10-11)

Full004fd280 repeat2 changes five creature bytes; narrower004fd315..004fd5ad
repeat1 changes creature+774. Both stop on the first critical difference despite
exact ordered760/762 scene-row equality and unchanged selected counters, ANI,
RNG, scheduler, complete gameplay/map headers and other entity pools. Both actual
private TITAN casting/combat/refusal/movement/portrait/Mini/normal Quit campaigns
pass19FPS and integrity/cleanup; the full result is4459 Worlds at19.44656FPS.
Negative invariance reports and independently passing gameplay reports are distinct.

`RS.creature-draw-position-history` registers partial static0051fa60 recovery:
an overlapping24-byte copy request+754->+760, latest coordinate/value stores and
RET8, with one direct scene-builder call004fa4ef. Complete copy/pointer/consumer
semantics, isolated original comparison and native implementation remain pending.
Diagnostic mode3 is an explicit native extra-only history suppression policy;
normal drawing still forwards the original helper once. Whole protected snapshots
retain every byte. Wireversion2 reports primary/extra history counts, and independent
decoding checks complete fixed/pool regions and retained bytes against runtime
change counts. All30 final-source PE32 controls and17 decoder refusals pass; negative history/scene
count gates andversion1 compatibility pass. Actual TITAN mode3 state+scene equality
is pending. No default activation,
simulation pacing change, complete nested/map/visibility/timer/pixel safety or60FPS.
Exact6cdf7a4..0986a18 committed history review:23 files,zero missing receipts;
no retrospective gate or behavior validation claim.


## Scene queue cleanup correction and abort diagnostics (2026-10-12 UTC)

Initial frozen-history runs fail Fireball verification and then, on unchanged
retry, fault at address0 after ranged/combat input events. Native delivery stops;
normal Quit is not reached and detach-only state traces are empty. Source/original
and owned cleanup pass; both failures are retained and never counted as invariance.

The old slice demonstrably exits before original queue count/writepointer cleanup.
`RS.scene-queue-retirement` records partial004fd5ad..004fd5c1 postconditions; the
corrected native slice executes these original stores before its004fd5c1 exit.
Wireversion3 protects the complete16-byte header and refuses malformed post-primary
queues before any extra. No saved game state is restored. Final33 PE32 controls,
52 guard-byte refusals, queue ownership/postconditions and17 decoder failures pass,
includingversion1/2 compatibility. The diagnostic checkpoints once at first
failure/16cases, then stops probing; full journey timing/integration is separate.
Actual corrected TITAN state/scene/queue equality and successful combat/normal Quit
are pending. Original allocator/nested map/visibility/driver/pixel and independent
60FPS/default safety remain unproved. Exact0986a18..961ea3b committed-history
review:25 files,zero missing receipts; no retrospective gate or execution claim.


## Prepared-scene phase candidate and retained performance failure (2026-10-12 UTC)

Corrected queue cleanup balances4465 native Worlds and completes input/flow/normal
producer retirement, but the campaign fails19FPS at18.55584FPS. Formal gameplay
validation is not asserted after that performance failure. The checkpoint has9
exact protected/scene repeats; repeat10 retains protected bytes and queue ownership
but has1154 versus1150 rows, so further repeats stop. Keep both negative outcomes.
Concurrent accounting limits timing attribution; it does not explain the failure.

Mode4 moves repetition to004fd5c1 inside the original draw, before clearing
temporary creature preparation and camera/cache post-work. It uses the exact
original stack/FP/error context, existing guarded body/history policy and original
queue cleanup, then resumes original post-work/return once. No cleared state is
restored. `RS.creature-draw-preparation` records partial conditional0051e260
preparation and0052ab80 post-clear; missing-row causality is a hypothesis. Wire4
adds the phase withversions1..3 compatibility. Final44 PE32/18 decoder controls pass; quiet TITAN state+scene+19FPS+normal Quit
proof is pending. Complete original
helper/pointer/allocator/nested map/visibility/driver/pixel/default/60FPS remain
unproved. Exact961ea3b..bd177b6 committed-history review:19 files,zero gaps;
no retrospective gate or execution claim.


## Quiet prepared-scene repeat proof on TITAN (2026-10-12 UTC)

`NR.scene-production-repetition.prepared-titan-20261012` passes16 extra native
Worlds with exact ordered763..1349 scene rows and unchanged13 protected regions,
including complete queue headers/entity pools/gameplay/map-context headers,
counters/mode/pacing/ANI/RNG/scheduler.40normal history forwards/40extra skips,
zero extra forwards/critical/refusal/foreign events. Original queue cleanup and
once-only original post-work are retained; no cleared game state is restored.
The first retained pair independently confirms exact protected bytes; the other
15 comparisons are recorded by the guarded runtime. Module writes remain separate
unclassified evidence, and all nested/map/visibility/timer/cursor/driver/pixel
semantics remain incomplete. No accounting runs concurrently with gameplay.

The independent `NR.public-resident-world.scene-production-prepared-campaign-20261012`
passes4620 balanced Worlds at19.29201FPS, actual casting/Fireball/melee/lethal,
spell refusals,movement,five portraits,Mini resume/Quit-No/normal Quit and all
source/original/private cleanup gates on accelerated TITAN RTX/Zink. No delivery
readback/pixel reply/fallback/pending World; clean producer retirement and80-byte
ACK remain. Native policy integration advances only to `live_observation`;
original comparison/replacement and partial recovered helper stages stay separate.

Extra production/delivery averages28.67015ms (11.7612..52.9186,median27.037);
13/16 exceed the16.67ms60FPS budget. These paired-draw samples exclude byte-audit/
checkpoint cost and are instrumented, not sustainable independent timing. The
full native World path averages22.63860ms (producer9.40538,submit9.04008).
19FPS passes;60FPS and a safe default/steady independent schedule remain pending.
Simulation target50ms is unchanged. Earlier gameplay/crash/scene/performance
failures and source hashes are preserved, with causality still partial.
Exactbd177b6..38cba10 history review:17 files,zero receipt gaps; no retrospective
validation or gate assertion. See[experiment](scene-production-repetition.md).


### Prepared-scene cost attribution (2026-10-12, pending)

The scene-production probe now separates paired original queue-consumer wall
time (including native capture/delivery/wait) from extra scene build/surrounding
work, with guarded original post-call MOV replay and original consumer return
PCs unchanged. Diagnostic wireversion5 retains versions1..4. New synthetic
duration/count rejection and PE32 ABI/guard controls precede live TITAN timing.
The original50ms update target is unchanged; this milestone does not increase FPS.
Private fixture startup/display failures are retained under working/tests;
no failed initialization supplies a rendering/gameplay equivalence claim.


The timing milestone controls pass44PE32/20decoder cases. Source-bound TITAN
checkpoint16matches protected state and ordered scenes:30.55809ms total,
28.67579ms queue consumption/native delivery and1.88230ms build/surround.
The full campaign fails19FPS at18.68812FPS; no formal gameplay/performance
promotion follows. Immutable negative evidence is registered; native delivery
cost reduction and sustained render scheduling remain outstanding.


### Minimap scratch preparation (2026-10-12)

Known traced dual-background services recycle bounded owned planes/receipts,
restore sentinels after success/throw, validate before mutation and avoid sorting
idempotent duplicate assignments; generic callbacks retain full scans.
Independent16service/40recycle/24producer and refusal controls pass. Same2423
actual World prefix on O2 TITAN EGL preserves terminal hash and improves
completed throughput72–78to91–98FPS; producer CPU4.0to1.7ms. This is native
preparation/replay evidence only. Public gameplay, frame scheduling, simulation
cadence and steady60FPS remain pending; historical claims/hashes are preserved.


### Budgeted prepared-scene scheduling (2026-10-12, live pending)

Mode5 keeps original50ms pacing and at mosttwo extra prepared slices per primary
inside a60s predicted-cost deadline window after16warm observations. Every extra
retains full protected-state and ordered-scene checks, extra-only history skips
and first-failure stop without rollback. Bounded wireversion6 separately records
World cadence, original advancing intervals, prediction misses and exclusions.
60PE32/28decoder controls pass, including two extras, insufficient budget/target
refusal, real forecast overrun, first boundary exclusion and exact histogram.
Higher sustained live World FPS and unchanged advancing simulation rate remain
pending; no default, interpolation/uniform60FPS or original equivalence claim.


Report-worker TITAN trial (2026-10-12) remains NEGATIVE as a full milestone.
All238extras (16warm+222budgeted) preserve protected bytes/ordered scenes.
The60047ms window delivers23.54822Worlds/sec with19.85927Hz all-valid advancing
simulation, median50/p9554/max123ms, one excluded first interval and one deadline
miss. No release/report checkpoint stalls appear in the bounded phase log.
The25FPS threshold is unchanged and missed. Whole journey fails post-combat
retreat to observed[10,71] (last living wizard[14,61]); independent input trace
records summon,Fireball,melee and player-party lethal observations but formal
full gameplay/normal Quit checks remain unpassed. Aggregate19.682WorldFPS at
failure. Sources, original manifest and owned cleanup verify. This result proves
a bounded rendering gain, not smoother simulation, full success or60FPS.
See `scene-draw-budget-report-worker-live-negative-20261012.json`; prior failures
and source fingerprints are retained.


Final progress-worker trial (2026-10-12) passes the complete native campaign:
5042balanced Worlds,20.42298deliveredWorldFPS,19FPS and half-rate sampled-window
gates, summon/Fireball/melee/player-party lethal, spell refusals, independent
movement, portraits, Mini resume/Quit-No and normal original Quit. Sources,
original manifest and owned cleanup verify;80-byte ACK/zero pixel reply, delivery
readback/fallback/pending and current final resident retirement. No performance
sample is filtered and no gate is lowered. See
`scene-draw-budget-native-campaign-20261012.json`.

The outer scheduling target remains NEGATIVE:60.007s window24.53047WorldFPS
misses25, and20.42298aggregate misses20.5. All296extras (16warm+280budgeted)
preserve exact protected bytes/ordered rows; advancing simulation19.86622Hz,
median50/p9554/max142ms, one mixed first interval excluded and4deadline misses.
This proves a bounded drawing gain with unchanged simulation cadence, not60FPS
or interpolated motion. The route remains opt-in. Retain
`scene-draw-budget-progress-worker-live-negative-20261012.json`.

The prior corrected-projection trial completed every gameplay/Quit check but
failed short-window paint FPS; owner capture acquisition was at most14.20ms.
The smoke progress diagnostic still used synchronous per-second QSaveFile
publication; the final trial replaces it with one value-only worker, skips busy
intermediate diagnostics, propagates ready errors and retains the final flow
report/every FPS sample. Second-long recurring gaps no longer occur in this
bounded trial. Filesystem attribution is correlated, not a measured kernel-I/O
causality proof. The post-combat retreat uses ordinary isometric ground projection
instead of fitting screen coordinates from partial/obstructed path progress; the
original exact observed waypoint and aliveness checks remain unchanged.

Exploratory private NVIDIA xcb_egl admission attempts failed the existing Qt
resident import fixture with `Viewport unavailable`; no backend impossibility
or cause is inferred. Raw reports/logs remain in
`working/tests/render-progress-heartbeat/run-b29z02l8` and `run-xcv7asao`.
NVIDIA EGL offline replay performance is still separate from public Qt admission.
Committed578bd89..479ce20 history review retains30exact source transitions with
zero missing receipts in `working/tests/scene-draw-budget-worker-history-20261012.json`.

Final current-source60PE32/28decoder and bounded menu/report-worker controls
pass (`scene-draw-budget-final-controls-20261012.json`,
`resident-report-worker-final-controls-20261012.json`); three passive movement
validation tests pass. Implementation advances only to partial; GUI heartbeat
and report policies have scoped live observation, while the budget scheduler
retains no successful25FPS integration/default/original-equivalence claim.


Owned snapshot handoff milestone (2026-10-12): explicitV5/mode2/MNMSACK1
accepts owned validated scene source before native GPU execution. One capture
owner verifies unchanged prefix and exact actual return before slot release;
two outstanding snapshots include taken/native/GUI work until ordered import.
Mapped held-render/backpressure/overwrite/12failure/cancel/join controls,512legacy
V4leases and independent PE32 exact80-byte ACK/all80mutation controls pass with
source/original/private cleanup. Build setup/type failures remain under
working/tests/world-owned-snapshots. Native policy implementation is partial and
integration headless; live original NOACCESS/bind/pacing/full gameplay and the
independent native render/UI recipe are pending. No60FPS or original comparison.
See `world-owned-snapshot-controls-20261012.json`.


Prepared independent rendering milestone (2026-10-12): opt-in
`--resident-world --independent-world` uses the owned V5 capture worker and a
GUI-owned native rendering clock. Actual source return/import credits remain
ordered and bounded at two; the original source producer is never called for
extra native frames. The default V4 route remains separately admitted.

Native World drawing restores its exact GPU baseline and reuses resolved native
scene draws/resources. Nine storage controls,40destination-dependent repeats,
two source updates and four saved-source repeat comparisons pass in
`native-prepared-world-controls-20261012.json`. Source history advances only on
engine updates. Native HUD replay retains versioned GPU inputs at their actual
read boundaries: static uploads/copies are folded once, while World-dependent
destinations restore and execute their ordered native operations per frame.
`native-frame-recipe-controls-20261012.json` records30World/HUD repeats,
source mutation/deletion, partial-frame refusal,100folded uploads, versioned
masked writes,10swaps and indexed palette ownership. Ordinary readbacks and
teardown surfaces are zero. Diagnostic output reads are confined to the tests.
Resource budgets and conservative terminal refusal remain in force.

Live evidence remains negative. The first attempt exposed a null capture owner
at cold start, now fixed and regression-tested. The second attempt executed
7107native World frames (3661extra), reaching38.36569renderFPS with
18.64224sourceHz. It missed3327deadlines and later terminated on native command
history GAP reason2, with pending imports and no successful normal Quit. Its
bounded trace records a cross-thread copy-order timeout: waiting thread0x1dc,
owner0x194, owner/waiter site0x101, phase4 and native stage0; the wait/owner ages were
157/381ms despite the50ms policy. Phase4 follows original BltFast return and
precedes tracker commit. This locates an admission/commit stall; it does not
establish which scheduler/tracker/driver caused it or justify extending bounds.
The source-stable negatives remain immutable in
`independent-native-startup-negative-20261012.json` and
`independent-native-first-render-negative-20261012.json`.

Static HUD folding and separate extra-World/HUD submission timing were added
following this failure; full TITAN gameplay/pacing/Quit/performance must be
rerun. No60FPS, original equivalence, live replacement or interpolation claim
is made. The engine still produces scene state; native simulation/interpolation
remain separate work. Committed history e3b8400a..b3757828 was reviewed against
18exact parent/current file receipts with zero unresolved accounting gaps;
this is history accounting, not retrospective validation.
