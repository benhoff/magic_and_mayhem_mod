# Engine modernization coverage ledger

Ledger reviewed: 2026-10-05 (NS06/NS07/NS08/NS09 updates; broader review: 2026-10-04;
initial baseline: 2026-10-03).
This ledger tracks reconstructed functionality,
modern implementations, and replacement of original work during live execution.
Those are separate milestones: offline tests and capture hooks do not establish
completed engine replacements.

This review reconciles recorded audio, SPR/ANI scene, native menu and shared
protocol milestones with the current source tree. Audio reconstruction remains
offline; Qt output has separate native host-backend smoke evidence. Default
game launches retain Wine DirectSound.

Current assessment: substantial pathfinding reconstruction and native rendering
infrastructure exist. Selected input and media paths have optional adapters.
Native menus and a bounded ANI/SPR scene run independently as previews; selected
original menu transitions have live observation evidence with original work retained.
No complete gameplay subsystem is established as replaced by the reviewed
evidence. Whole-game functional and performance coverage are **unknown**.

Latest menu follow-up `run-qyfqz9tj`: the user identifies a 20+ second stale
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

Latest performance follow-up `run-8s6ddeg4`: the user still reports slow Qt
presentation. Retained state has 165 updates, but post-exit inspection provides
no live rate. Primary-scoped contention remains in the bounded log. Bounded
cross-thread guard waiting now delivers 20/20 synthetic overlap updates versus
0/20 with immediate skipping; this is continuity evidence, not a live FPS gain.
A read-only per-run publication-rate log now supplies the missing timing boundary.
Live presentation performance remains unverified. See
[bounded-wait evidence](render-startup-black-screen.md#2026-10-04-low-qt-update-cadence-bounded-tracker-waiting).

Latest follow-up `run-wf_pb32k`: the user reports intermittent menu and battle
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

| Dimension | Values | Meaning |
| --- | --- | --- |
| Implementation | None recorded; partial; scoped implementation | Code exists for the named scope; scoped does not mean the whole original contract |
| Integration | Offline; observation; optional adapter; live replacement | Observation forwards original work; an optional adapter can replace supported calls; live replacement requires actual game evidence |
| Validation | Static; synthetic; installed assets; live observation; live equivalence | Record all applicable types; observing a live call does not establish replacement equivalence |
| Confidence | High within scope; provisional; unknown | Confidence in stated evidence, not untested compatibility |

Synthetic x86 tests establish particular ABI/lifecycle contracts. Installed
asset comparisons establish byte/decode agreement, not playback or loader
equivalence. Displaying captured game frames in Qt does not replace original
drawing. Hooks forwarding original operations count as observation.

Most baseline rows use source inspection and linked documents' recorded results;
their checks were not rerun for the original ledger change. Asset rows were
updated after the six-chunk native asset workflow: installed raw-byte/PCM
checks and the pinned Windows file API audit were rerun with original-manifest
verification before/after. This does not promote those paths to live replacement.

The 2026-10-04 ledger refresh reviews source, linked research and the retained
scene report; it does not rerun game experiments or subsystem test suites.
Validation counts below are recorded results from those milestones, not new
results from this documentation update. Incomplete declarations and inspection
tools without a documented validated contract do not advance coverage.

## Subsystem coverage

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| PF01 Route request, search ordering, budgets and continuation | Scoped host models; offline replay and observation tools; original search retained | Static/synthetic; high within recovered rules. [Models](../../reconstruction/pathfinding/README.md), [sequence capture](pathfinding-search-sequence.md) | Compare real-map fresh/resumed searches in both known contexts: route bytes, budgets and flags |
| PF02 Neighbors, acceptance, movement predicates and costs | Scoped reconstructed rules with explicit world inputs; offline | Static/synthetic and isolated original i386 comparisons; high for tested helpers. [Evidence](../../reconstruction/pathfinding/README.md#evidence-and-confidence) | Match real-map candidates and costs; helper agreement alone does not establish complete search agreement |
| PF03 Frozen world ownership and per-context replay | Scoped snapshots containing one creature; offline | Synthetic capture/replay checks; high within scope. [World format](../formats/route-world-snapshot.md), [continuation](pathfinding-search-sequence.md) | Validate coherent live capture and refreshed continuation inputs; multi-creature scheduling remains separate |
| RE01 Persistent surfaces, rectangular updates, opaque/keyed/masked copies, palettes and compatible swaps | Scoped OpenGL implementation; offline replay/demo and sprite upload | Synthetic CPU/OpenGL comparisons; selected installed SPR draws/presentation match isolated original routines. [Renderer](../../renderer/README.md), [sprites](native-sprite-rendering.md) | Inventory unsupported operations and real-map usage; original live drawing remains active |
| RE02 Frame capture, owned pixel tracking and Qt presentation | Scoped bridges; observation and optional viewport | Synthetic x86/CPU/OpenGL/Qt comparisons; high within bounded contracts. [Presentation](opengl-presentation.md), [owned session](opengl-owned-session.md) | Validate complete maps/transitions without gaps; bounded captures do not prove continuous replacement rendering |
| RE03 Complete live rendering backend | Partial foundation; no complete replacement recorded | End-to-end equivalence unknown. [Renderer boundaries](../../renderer/README.md), [startup investigation](render-startup-black-screen.md) | Route live commands to native surfaces, preserve CPU locks, cover required operations and validate long sessions |
| IN01 Qt shell and foreign-window hosting | Scoped X11/XWayland host; optional shell | Synthetic embedding/detachment; high for fixture contract. [Hosting](qt-shell-hosting.md) | Actual game focus, dialogs, resize, detach and exit; native Wayland unsupported |
| IN02 Input forwarding and selected polling APIs | Scoped event forwarding and three USER32 hooks; optional adapter | Synthetic Qt/X11 and Qt-to-x86 checks; high within scope. [Input](qt-input-forwarding.md) | Live menus, movement, scrolling and focus; original window/message handling retained |
| ME01 Movies and supported WinMM file sounds | Scoped Qt playback/hooks; optional adapter with fallback | Synthetic ABI/lifecycle/decode and installed Intro0 decode probe; high within scope. [Media](qt-native-media.md) | Full live playback, audible output, skip/return and call frequency; unsupported calls remain legacy |
| AU01 DirectSound setup, static PCM upload and duplicate storage | Scoped reconstructed setup/native storage; offline | Static/synthetic and 356 installed WAV comparisons; high for bytes/ownership. [Evidence](directsound-buffer-setup.md), [storage](../../audio/README.md) | Live adapter remains; voice/mixer/output evidence is separate in AU02–AU05; game effects/voices remain Wine DirectSound |
| AU02 Recovered voice controls and retirement contracts | Scoped selected engine blocks and fake backend; offline | Static/synthetic: play/stop, looping, volume/pan, status/zero reset, deadlines and bounded frequency/cursor audit; high within selected contracts. [Evidence](directsound-voice-controls.md) | Selected retirement, scheduling, positional and camera contracts have separate AU08–AU11 evidence; whole-program call coverage and live timing remain unverified |
| AU03 Native secondary voice state | Scoped native implementation; offline only | Synthetic source-frame advancement, independent duplicates/controls, completion/loops and ownership; high within tested native policies. [Evidence and repeatable check](native-audio-voice-state.md) | Qt output/routing are separate in AU05/AU06; no validated live replacement; completion/cursor policies are not demonstrated Wine equivalence; fractional output-frame timing is covered separately in AU04 |
| AU04 Native stereo PCM mixing and resampling | Scoped native implementation; offline only | Synthetic PCM fixtures and 120 independent format/rate/loop scenarios, exact split-block continuity, gain/clipping and lifecycle checks; high within tested native policies. [Evidence and repeatable check](native-audio-mixer.md) | Qt output/routing are separate in AU05/AU06; no validated live replacement; linear interpolation is not band-limited downsampling or bit-exact DirectSound; real-time allocation/latency remain unverified |
| AU05 Native Qt PCM output | Scoped native device adapter; fixture and host-backend smoke validated | Synthetic format negotiation/partial-write/backpressure/lifecycle tests; default host stereo Int16 48 kHz tone delivered before and after restart. [Evidence](native-audio-output.md) | Optional voice routing has separate AU06 evidence; speaker audibility, live x86 replacement, long sessions, latency and device-change recovery remain unverified |
| AU06 Selected x86 DirectSound voice routing | Optional native adapter; fixtures and staging only | Native PCM/control/lifetime and mapped-channel tests, PE32 stdcall COM fixture, stale-host timeout and guarded disposable import checks. [Evidence](native-audio-voice-bridge.md) | No live game/aural equivalence; primary device metadata is scaffolding; pitch, nonzero seeks and cursor queries unsupported; live selection/fallback, transitions and timing remain unverified |
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
| AS04 ANI tables, forward playback and bounded sprite/terrain scenes | Native versions-3/4/5 ANI loader (older records normalized), selected No-CD forward/phase-switch and placement model, numeric group/facing and explicit two-layer and configured mode-one Qt preview with selected original depth/queue ordering and optional SPR bitmask visibility, plus selected ordinary terrain submission and a direct TTD/SPR Qt tile preview and owned MAP grid loading with bounded installed slices plus selected four-orientation camera/world terrain traversal and clipped presentation plus selected recovered camera map binding/viewport/position/scroll setters and owned ordinary-terrain initialization with recovered geometry/surface admission and explicit section-grid placement/rotation and owned authored-region recipe loading/mixed-height assembly and selected single-block candidate/location/seeded selection primitives plus multi-block descriptors/connectors/admission and bounded pruning plus owned placement/count rollback and a selected one-attempt backtracking driver plus two-pass Specific placement/rotation/location fallback plus ten-attempt reset/seed orchestration plus owned recipe/header catalog construction and original connector-threshold policy plus generation from complete available recipe catalogs and successful-assignment conversion to owned ordinary assembly plans plus complete installed generated MAP loading/assembly, geometry initialization and bounded generated Qt/OpenGL scene production plus owned mode-0 palette construction and controlled-light shaded scene drawing and installed global palette configuration and terrain palette-count preferences and owned per-cell fields with controlled source stamps and selected owned creature admission/eight-phase light refresh plus static-object region admission/additive stamps/two-phase publication and combined updater plus owned object-snapshot Qt terrain scenes plus installed effects light-table loading plus selected common effect-record placement; offline only | Reviewed 2026-10-05: 133 files/121,476 record byte matches; 347 forward traces/22,555 states and 2,140 phase-switch traces/70,620 states across 118 assets; 6,097,440 original placement helper matches; 198 layered frames/396 body states/792 layer states, plus 195 body-only frames/390 states; one configured attachment path with 520 original states and 462 complete native frames; 4,096 queue keys/1,285 sorts/164,480 full records; 72 synthetic and 24 installed overlapping queue frames; 8,192 visibility helper/1,024 reverse-pass matches and 50 installed visibility frames per normal/sanitized build; 15,204 terrain producer cases/15,317 records and 32 installed terrain frames/28 hidden decisions per normal/sanitized build; 683 MAP payloads/7,595,200 cells and 15,190,400 selected original cell checks; 56 installed MAP slice frames per normal/sanitized build; 18,428 safe world traversal cases/3,089,824 records across four views (four native-domain rejections), plus 96 installed terrain scene frames per normal/sanitized build (39,368 hidden and 22,762 partially clipped draws across both); 139,264 selected camera setter state comparisons and 16 installed camera traces/32 normal/sanitized images; 2,048 geometry fixtures/387,962 cells and 683 installed maps/7,595,200 cells with complete original geometry-pass byte matches, plus 96 initialized scene images; 8,192 synthetic section-copy fixtures/410,500 cells and 683 installed MAP selections/30,084,800 copied cells across four rotations with complete destination/source-side-effect checks, plus 96 assembled scene images; 40 installed recipe comparisons and 3,136 authored-region placement fixtures/46,656 blocks, two installed original descriptor/admission/placement fixtures/32 blocks, plus 32 authored-region scene images; 262,144 complete-state RNG draw matches, 207,840 single-block admission checks, 4,096 candidate/10,240 location tables, 78,584 original seeded choices and 1,232 installed descriptor matches; 1,736 expanded descriptor matches across all 683 MAPs, 39,344 multi-block-domain admission/carry checks, 2,048 connector lookups, 7,200 candidate/500 location tables, 7,200 seeded choices and 8,192 pruning fixtures; 3,136 placement/7,056 removal and 2,048 pruning-carry matches, plus 3,755 one-attempt solver state matches (1,940 backtracked, 231 multi-block backtracked) including 683 installed single-source catalogs; 10,786 Specific-driver state/warning matches (2,430 multi-block cases; 140 synthetic missing-connector refusals outside original execution), including 6,830 cases from 683 installed headers; 5,718 attempt-reset matches and 5,701 post-CFG generation-loop matches/19,883 attempts (84 retry successes, 1,552 exhausted cases, 241 seed wraps; 17 synthetic rollback-bound refusals), including 1,366 installed-header cases; 2,087 catalog/26,888 expanded-descriptor matches, 291 threshold-quirk cases and selected CFG fields for all 40 recipes (39 available installed catalogs; missing Medieval REGION0/Test inputs refused); 2,204 complete recipe-generation state matches with 1,194 owned plans/15,849 block assignments, 1,010 failed generations and 155 installed successes across 39 available recipes/four seeds (four unavailable Medieval REGION0 cases refused); seven complete installed layouts/130 block assignments and 1,223,200 initialized-cell matches with 112 normal/sanitized generated/authored scene frames across all views and visibility modes, plus ten failure-path checks; expanded coverage of all 39 available recipes/78 zero and wrapping-seed layouts, 1,702 block assignments, 17,987,200 initialized cells and 312 corner-camera frames (13 retry successes, no generation/camera refusals; two unavailable Medieval REGION0 seed cases refused without original execution); 1,028,096 baseline palette WORD matches across eight counts/two formats with separate 32/64-bit native checks, 400 normal/sanitized shaded tile/generated-world frames across all views and three realms, and six controlled-light refusals; 1,068 original global configuration read/admission/setter matches, 4,112,384 palette WORD comparisons across four profiles/eight counts/two formats with 32/64-bit native checks, 192 configured normal/sanitized generated scene frames and four config-request refusals; 2,112 original terrain preference admission/loader-dispatch matches and 240 preference-driven normal/sanitized generated scene frames; 564 original field/control/kernel/stamp fixtures with 32/64-bit native checks and 72 per-cell lit normal/sanitized scene frames matching original world queues and indexed draws; 96 original creature-refresh fixtures/2,304 ticks with 18,398,280 five-buffer byte comparisons and 11,520 state values matching 32/64-bit native and sanitized builds; 32,768 static admission checks, 4,096 additive stamps and 128 original static-refresh fixtures/3,072 ticks (1,536 combined creature/static ticks), 104,160,840 five-buffer bytes and 18,432 static state values matching normal/sanitized 32/64-bit models; 36 original object-snapshot ticks with complete five-buffer/eleven-state matches and 192 normal/sanitized object-driven terrain frames matching original four-view world queues and indexed draws, two default-final-tick frames and 22 pre-output refusals; fixtures and ASan/UBSan. [ANI](../formats/ani-native-loading.md), [controller](animation-forward-contract.md), [direction switches](animation-direction-selection.md), [placement/layers](animation-placement-attachments.md), [configured attachment](native-attachment-recipe.md), [queue](sprite-queue-order.md), [overlap scene](native-sprite-queue-scene.md), [visibility](sprite-visibility.md), [native visibility](native-sprite-visibility-scene.md), [terrain producer](terrain-submission.md), [TTD](../formats/ttd-native-loading.md), [terrain preview](native-terrain-preview.md), [MAP format](../formats/map-native-loading.md), [MAP cell checks](map-grid-loading.md), [MAP slice preview](native-map-terrain-preview.md), [world traversal](terrain-world-traversal.md), [rotated traversal](terrain-traversal-rotations.md), [camera setters](terrain-camera-setters.md), [map geometry](terrain-map-initialization.md), [section assembly](terrain-section-assembly.md), [authored regions](terrain-authored-regions.md), [seeded selection](terrain-region-selection.md), [region constraints](terrain-region-constraints.md), [region solver](terrain-region-solver.md), [Specific placement](terrain-region-specific.md), [generation attempts](terrain-region-generation.md), [region catalog](terrain-region-catalog.md), [generated plans](terrain-generated-plans.md), [generated scenes](native-generated-terrain-scenes.md), [expanded generated coverage](terrain-generated-coverage.md), [baseline palette shading](terrain-palette-shading.md), [configured global lighting](terrain-lighting-config.md), [terrain palette preferences](terrain-palette-preferences.md), [terrain light fields](terrain-light-fields.md), [creature lighting](terrain-creature-lighting.md), [static lighting](terrain-static-lighting.md), [object-driven lighting scenes](terrain-object-lighting-scenes.md); 132 whole-original table loads/35,244 DWORDs and profile-read contracts/11,748 type selections, 89 installed entries and six emitting types, native authored-position lighting composition and 69 normal plus 69 ASan/UBSan tests. [Effect tables](effect-lighting.md); 2,048 original common placements across 128 fixtures, 15,220,736 full cell checks and owned rollback tests; renewed 132 effect-table loads; 70 normal and 70 ASan/UBSan tests. [Effect record placement](effect-record-placement.md); whole effect trajectory step reconstructed and compared separately. [Effect trajectory](effect-trajectory.md); 6,391 whole-original same-cell movement calls/20,842 steps with produced recount coordinates and headless lighting composition. [Effect projection](effect-same-cell-projection.md); 3,232 original membership-on transition calls/10,288 steps, 8,911 owned traced cell changes/5,784 cache advances, complete guarded chain/cell/catalog reference checks and moving-light composition. [Effect cell transitions](effect-cell-transitions.md); TL09 adds 3,072 original column-query/unlink fixtures (6,144 calls) and 3,232 complete-parent calls over zero/nonzero terrain, guarded buffers, signed column layers and suppression conditions, plus strict/sanitized headless lighting and rollback. [Effect cell cleanup](effect-cell-cleanup.md); TL10 adds complete-parent blocked entry/return-1 validation, skipped destination insertion/subcell refresh, committed cleanup/cache/recount and active detached state, plus fresh ordinary/cleanup comparisons and strict/sanitized headless checks. [Blocked effect destinations](effect-blocked-destinations.md); TL11 adds whole-parent height exits with raw parameter/trajectory/counter updates before wrapping or fine/cell/membership publication, first/later lower/upper boundary cases and fresh ordinary/blocked/cleanup reruns with strict/sanitized headless checks. [Effect height exits](effect-height-exits.md); TL12 adds complete-parent disabled-membership motion preserving heads/links/flags and skipping column cleanup, bounded repeated/blocked continuations, current-cell/chain divergence and strict/sanitized admission checks with fresh prior comparisons. [Disabled membership](effect-membership-disabled.md); TL13 adds real 8x8x4 terrain-bit sampling gated by cached subcell coordinates, both membership settings, committed hit state/return 1 before subcell publication and fresh prior-contract reruns with strict/sanitized headless checks. [Effect terrain occupancy](effect-terrain-occupancy.md); TL14 adds complete 004e1200 creature footprint query with signed-coordinate bounds, MSB-first 16x16 rows and raw wrapped-height gating, original/native/sanitized exhaustive fine-coordinate and boundary comparisons, and strict rectangle-oracle units. [Creature occupancy lookup](creature-occupancy-lookup.md); TL15 adds complete 004e1260 fixed footprint initialization with raw height preservation and zero/nonzero templates, guarded original/native/sanitized overwrite and chained query proof. [Creature footprint initializer](creature-footprint-initializer.md) | Original legacy ANI playback, reverse mode, complete named action/attachment producer mapping, gameplay events, opaque metadata, full camera lifecycle, original CFG/file-error recovery and full map lifecycle and entity initialization, complete world admission/entities, remaining terrain producers and visibility activation and live clock/integration remain unverified; local slice layout, explicit world camera configuration and recovered-camera start selection, ordinary-terrain reference/object projection, clipping, authored-only selection when no generation seed is supplied and 49-candidate pruning bound plus typed Specific warnings and missing-connector/rollback bounds, child ticking/admission and restart are preview policies; scene pixels use an independent CPU oracle with original queue orders and selected visibility decisions for overlap fixtures, not original whole-scene rendering; baseline shaded frames additionally match original indexed draws fed recorded native queues with explicit count/levels/powers and chain ordinal zero; captured entity source inputs, type-specific effect setup, remaining effect movement/recount and region input production, special size-one lights, spectator/sentinel admission and remaining lighting lifecycle and non-power-of-two palette counts, effect-chain/ordinal selection and live replacement remain unverified |
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

Reviewed 2026-10-04. Native preview behavior and recovered engine transitions
have separate evidence. Original in-game menus and actions remain active.

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| UI01 Native navigation and result menus | Scoped Main, Quick Battle, campaign/battle Mini Menu, Victory/Defeat and Quick Battle result widgets; standalone previews with semantic actions | Synthetic asset/layout/input/model/navigation checks and installed-asset smoke/visual inspection; high within native preview policies. [Main](main-menu-qt-migration.md), [Quick](quick-battle-qt-menu.md), [Mini](mini-menu-qt.md), [Battle results](battle-results-qt.md), [Quick results](quick-battle-results-qt.md) | Engine data/actions, pause/resume, live screen ownership and transition integration remain unverified; fonts/colors and portrait placeholders lack original visual equivalence |
| UI02 Native Map Selection | Scoped caller-supplied stable-ID list, guarded confirmation and preview return navigation; offline only | Eight targeted Qt checks and installed-asset smoke recorded; selection/refresh/keyboard/transactional rejection fixtures; high within native policies. [Evidence](map-selection-qt.md) | Installed map enumeration, map loading, original selection/return semantics and live battle setup remain unconnected |
| UI03 Main / Quick Battle engine dispatch | Hash-pinned callback/controller recovery and bounded forwarding hook; live observation only | Fourteen original-bytecode PE32 fixture cases and live Main → Quick → Cancel → Main trace (13 records, one observed thread ID); high within selected ABI/transition scope. [Evidence](menu-engine-observation.md) | Other actions, keyboard/focus and longer sessions remain unverified; bounded semantic action integration is now UI17; original logic/drawing retained |
| UI04 Native Multiplayer Game Selection | Caller-supplied stable-ID session list and Join/Cancel preview routes; offline only | Synthetic selection/refresh/keyboard/asset/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](multiplayer-game-selection-qt.md) | Discovery, original session handles, transport/join behavior and engine action adapter remain unconnected; font/control styling approximate |
| UI05 Native Single Player Battle Setup | Configured sliders, supplied player/map model, semantic setup actions and Quick/Map caller navigation; offline only | Synthetic model/input/layout/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](single-player-battle-qt.md) | Original setup field/default/units mapping, wizard catalogs and battle creation remain unconnected; sprites/font/control styling approximate |
| UI06 Native Multiplayer Battle Setup | Separate host/guest layouts, supplied roster/settings, local chat/Ready and Create/Join/Map caller navigation; offline only | Synthetic model/ownership/input/chat/layout/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](multiplayer-lobby-qt.md) | Networking, authoritative membership/settings/readiness, chat delivery and engine action adapter remain unconnected; original sprite/control fidelity and live SFT text equivalence unverified (native fonts: UI14) |
| UI07 Native Region Entry | Supplied region ID/artwork, difficulty/availability and semantic Enter/auxiliary requests; standalone preview only | Synthetic model/input/layout/artwork/transaction/navigation checks and installed-art smoke/visual inspection; high within native preview policies. [Evidence](region-entry-qt.md) | Campaign flow/unlocking, engine region/difficulty mapping and auxiliary screens remain unconnected; icon/font/control styling approximate |
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
| UI18 Live native Single Player setup / Map / Start | Separate V2 data/action contract, original defaults/player generation/map order, guarded full settings transactions and original control callbacks; Start suspends native commands for original loading or waits for UI19 native spell selection; fresh ready Main/Quick return restores Qt | Eight targeted Qt CTests; pinned PE32 setup/list/rule setter/callback guards (including Lives propagation and removed sliders); bounded native setup Cancel, Map Cancel/OK, rule/player edits, then Start, with engine acknowledgements 1–12; inspected real battlefield and original spell-selection captures; genuine XTest original result Quit returns to original Quick Battle, confirmed by capture and retired-channel screen 22; additional two-battle direct-loading run restores Qt twice, refreshes second setup, and completes original Back/Quit at acknowledgement 16 with launcher status 0; inspected captures and before/after immutable checks; high within this scope. [Contracts/evidence](single-player-menu-bridge.md) | Native in-battle menus, longer play, hardware focus/input, OpenGL integration and suppression of original drawing remain unvalidated; spell selection is separately recorded in UI19; native edits are committed on Map/player/Start actions |
| UI19 Live native pre-battle Spellbox | V3 owner/inventory/recipe/name/talisman snapshots; local draft/reset; whole-loadout guards and original shelf/talisman/OK callbacks on the engine thread; original loading/play and fresh Qt root return; Quick Battle Single Player only | Fourteen selected Qt CTests pass, including V3 malformed pool/request and timer-draft checks; pinned PE32 callback/setter/OK fixture covers rejection, existing-assignment removal, cross-alignment placement, input guards and once-only dispatch; corrected live run selects two ingredients, proves all 63 engine control assignments agree, shows selected spells in inspected battle HUD, restores Qt Quick once and completes normal Back/Quit at acknowledgement 15; before/after 2927 immutable checks; high within bounded scope. [Contract/evidence](spell-selection-menu-bridge.md) | Campaign/multiplayer selection, all recipes/alignments live, actual casting, rebuilt spell-list byte comparison, timer expiry with local draft, native previews/in-battle menus, physical focus/input and drawing suppression remain unvalidated; unsubmitted drafts remain local |
| UI20 Mini Menu bridge preparation | Static Mini/callback/confirmation/parent contract; V4 guarded bridge and Qt/controller/session wiring; activation OFF | Pinned read-only export, isolated original-bytecode callback/guard and disabled-gate checks, focused V4 wire checks; high for those scopes. Proposed live Quick Battle Escape ingress instead reached Game Over and timed out, matching recovered context exclusion. [Evidence](mini-menu-engine-bridge.md) | Campaign Mini live ingress, Preferences/confirmation, actual pause/timers/audio and hardware input remain unvalidated. No forced context changes or default activation; normal V5 results flow is UI21 |
| UI21 Quick Battle results engine bridge | V5 engine-formatted display rows and original Continue/Quit dispatch; Qt results ownership and original gameplay/Quick return; context 1 only | Pinned read-only disassembly/vtable/Ghidra, original-bytecode receiver/ABI/effects/guard fixture, focused Qt wire/controller/widget regressions and three bounded live runs including results window-close with exact callback/ack trace and normal exit; high within selected scope. [Contract/evidence](quick-battle-results-engine-bridge.md) | Spectate/multiplayer/campaign, portrait sprites, original font/input equivalence, full victory paths, live nonzero scoring, complete pause semantics and longer play remain unverified; original drawing/tick/results updater retained |
| UI22 Preferences contract recovery | Pinned screen/settings identity, packed fields, original radio/slider units, availability, entry snapshots, OK/Cancel and profile writer; live Qt activation unchanged | Read-only disassembly/Ghidra and isolated original-bytecode enter/slider/button/writer matrix, including rollback, device/MCI branches and ignored write failure; targeted Qt preview regression; high for those scopes. [Contract/evidence](preferences-engine-contract.md) | Main-to-Preferences bridge is tracked separately in UI23; control-builder/display-rebuild availability is static only; actual audio, file durability, caller round trips, campaign/Mini variants and live pause remain unvalidated |
| UI23 Main Preferences engine bridge | V6 Main-only guarded settings/control snapshot, original preview/OK/Cancel delegation and Qt draft/window-close ownership; original services retained | Pinned original setter/callback and rejection fixture, nine targeted Qt tests, V5 fixture regression and automated bounded Preview/Cancel/OK/reopen/window-close run with exact ACK/dispatch trace, byte-identical Cancel file and edited-key readback; high within scope. [Evidence](preferences-engine-bridge.md), [behavior register](preferences-coverage-register.json) | Other callers/Mini activation, physical audio/CD, actual resolution rebuild, original write failures, restart durability and cross-launch persistence remain unvalidated; no engine replacement |
| UI24 Main Preferences cross-launch persistence | Native store for seven original-accepted Main settings; original writer/callbacks retained, atomic accepted readback and fresh disposable import | Ten targeted Qt checks, three synthetic Python checks and automated two-process Preview/Cancel/OK/reopen/restore/Cancel/Quit; all seven fresh engine/Qt values agree and Cancel preserves store bytes. Original manifests match 2,927 files before/after. [Evidence](preferences-persistence.md) | Other callers, original-menu fallback export, physical audio/CD, actual resolution rebuild and power-loss durability remain unvalidated; no engine replacement |
| UI25 Main Preferences display rebuild validation | Retained original leave/rebuild path, test-only forwarding observation and Qt resolution round trips | Six original leave branch/repeat cases, V6 observer regression, ten targeted Qt checks and automated four-change Main run: exact callback/ACKs, 800×600/640×480 client sizes, cleared flags, restored font modes and reopened settings. [Evidence](preferences-display-rebuild.md) | Gameplay resources validated only by isolated ordering; physical monitor/fullscreen/device failure, pixel/font equivalence and other callers remain unvalidated; no replacement |
| UI26 Campaign menu entry contract | Original Main New Game reset/request, Realm lifecycle prefix, admission and auxiliary/navigation branches; Qt bridge remains pending | 269 isolated original-bytecode cases (7 Main, 25 occupancy, 216 admission, 7 auxiliary, 14 navigation); read-only static export; all 2,927 originals preserved. [Contract/evidence](campaign-menu-engine-contract.md) | Full resource initialization, visibility/tutorial gates, campaign Mini mode 4 return, live ownership and Region Entry remain unvalidated; no native equivalence, live integration or replacement |
| UI27 Original campaign ingress observation | Opt-in guarded Realm custom-tick forwarding; fresh campaign observer only | Synthetic wrong-slot/bytes/receiver/repeat guards, 300-call deduplication and V6 original callback regression; automated Xvfb/Wine New Game records Realm states 0→1→2 and original Region Entry screen 18. All 2,927 originals preserved. [Evidence](campaign-menu-entry-observation.md) | Realm resources remain uninitialized; loaded-Realm ownership, Region Entry actions, Qt campaign dispatch and campaign Mini mode 4 return remain unvalidated; no native engine equivalence/replacement |
| UI28 Region Entry contract and return observation | Caller-dependent actions, deferred world request, opt-in Region Entry tick and Realm resume forwarding; original controls only | 175 isolated original-code cases, synthetic guard/forwarding checks, V6 regression and automated four-difficulty/Cancel run. Fresh Cancel resumes Realm then returns to Main; all 2,927 originals preserved. [Evidence](region-entry-engine-contract.md) | Qt semantic bridge, live Enter/world initialization, loaded-Realm return and auxiliary destination lifecycles remain pending. Historical UI26/UI27 source hashes retained, affected evidence now stale; no native equivalence/replacement |
| UI29 Region Entry engine bridge | V7 fresh New Game, Qt difficulty/Cancel dispatch, engine-confirmed Main return and window-close Cancel/Quit | Private original selector/Cancel guards, six targeted Qt checks, V6 Preferences/diagnostic regressions and automated live round trip; original drawing/tick retained. All 2,927 originals preserved. [Evidence](region-entry-engine-bridge.md) | Fresh Celtic region 1 only; Enter/world loading, other callers/regions, loaded Realm, auxiliary lifecycles and campaign Mini remain pending. Historical observer evidence retained, affected hashes now stale; no native engine equivalence/replacement |
| UI30 Fresh Region Entry Enter | V8 Qt difficulty/Enter and original fresh campaign loading handoff | Original callback/admission guards, World forwarding guards, six Qt checks, original-only and Qt Enter to three initialized gameplay ticks, V7 Cancel/Quit regression; all 2,927 originals preserved. [Evidence](region-entry-enter-engine-bridge.md) | Fresh Celtic region 1 only. Original loading/draw/simulation retained; campaign return, loaded Realm, other regions, auxiliary/campaign Mini and equivalence/replacement pending. Historical fingerprints preserved |

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

Reviewed 2026-10-04. `protocols/` owns the v1 wire schemas, generated C/C++ and
Python definitions, publication semantics and compatibility rules. Injected
render/input/media adapters, Qt clients and Python fixture/launcher tooling now
consume those definitions. Wire bytes and existing publication/fallback behavior
are preserved; this extraction does not advance live replacement coverage.

Validation: four independent protocol checks, four focused Qt tests, three
launcher preference tests and the existing synthetic frame, input and media
integration workflows pass. PE32 `.text` sections match the pre-migration build
for live and self-test bridge code. Original-manifest verification passes before
and after validation. Confidence is high within synthetic v1 scope; no new live
game validation. The input timer's literal 50 ms in actively edited `main.cpp`
remains a deferred one-line adoption. Audio v2 and capture/menu formats are
outside this change. See [migration evidence](../../protocols/VALIDATION.md) for
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
| GP08 | Terrain/sprite loading, animation and scene composition | Native SPR/ANI loading, selected forward/phase-switch and placement helpers, bounded group/facing and two-child/configured mode-one preview and selected original depth/sort queue and SPR bitmask visibility pass and selected ordinary terrain producer/direct TTD/SPR tile preview plus owned MAP loading and bounded installed MAP slice preview and selected four-orientation camera/world terrain traversal with clipped presentation and selected recovered camera setters plus ordinary-terrain initialization, geometry/surface admission and explicit section-grid placement/rotation and owned authored-region recipe loading/mixed-height assembly plus selected single-block candidate/location/seeded selection primitives plus multi-block descriptors/connectors/admission and bounded pruning plus owned placement/count rollback and a selected one-attempt backtracking driver plus two-pass Specific placement/rotation/location fallback plus ten-attempt reset/seed orchestration plus owned recipe/header catalog construction and original connector-threshold policy plus generation from complete available recipe catalogs and successful-assignment conversion to owned ordinary assembly plans plus complete installed generated MAP loading/assembly, geometry initialization and bounded generated Qt/OpenGL scene production plus owned mode-0 palettes and controlled-light shaded scene drawing and installed global palette controls and terrain palette-count preference dispatch and owned per-cell light fields/controlled source stamps and selected creature admission/eight-phase work/published/target refresh plus static region admission/additive staging/two-phase publication and combined updater plus owned object-snapshot lit Qt terrain scenes and installed effects light-table loading and selected common effect-record placement and whole effect trajectory stepping plus empty-world same-cell motion/recount and bounded membership-on cell transitions and headless lighting composition from authored state validated offline; captured entity light-source inputs, type-specific effect setup, remaining effect movement/recount and region input production, special size-one lights, spectator/sentinel admission and remaining lighting lifecycle, non-power-of-two palette counts and palette effect/ordinal selection, complete terrain, named actions, attachment lifecycle, full camera lifecycle, original CFG/file-error recovery and full map lifecycle and entity initialization, full world admission/entities, remaining terrain producers and visibility activation and live pipeline remain unimplemented. [Scene](native-animation-scene.md), [selection](animation-direction-selection.md), [placement/layers](animation-placement-attachments.md), [configured attachment](native-attachment-recipe.md), [queue/overlap](native-sprite-queue-scene.md), [visibility](native-sprite-visibility-scene.md), [terrain producer](terrain-submission.md), [terrain preview](native-terrain-preview.md), [MAP slice preview](native-map-terrain-preview.md), [world traversal](terrain-world-traversal.md), [rotated traversal](terrain-traversal-rotations.md), [camera setters](terrain-camera-setters.md), [map geometry](terrain-map-initialization.md), [section assembly](terrain-section-assembly.md), [authored regions](terrain-authored-regions.md), [seeded selection](terrain-region-selection.md), [region constraints](terrain-region-constraints.md), [region solver](terrain-region-solver.md), [Specific placement](terrain-region-specific.md), [generation attempts](terrain-region-generation.md), [region catalog](terrain-region-catalog.md), [generated plans](terrain-generated-plans.md), [generated scenes](native-generated-terrain-scenes.md), [expanded generated coverage](terrain-generated-coverage.md), [baseline palette shading](terrain-palette-shading.md), [configured global lighting](terrain-lighting-config.md), [terrain palette preferences](terrain-palette-preferences.md), [terrain light fields](terrain-light-fields.md), [creature lighting](terrain-creature-lighting.md), [static lighting](terrain-static-lighting.md), [object-driven lighting scenes](terrain-object-lighting-scenes.md), [effect tables](effect-lighting.md), [effect record placement](effect-record-placement.md), [effect trajectory](effect-trajectory.md), [effect projection](effect-same-cell-projection.md), [effect cell transitions](effect-cell-transitions.md), [effect cell cleanup](effect-cell-cleanup.md), [blocked effect destinations](effect-blocked-destinations.md), [effect height exits](effect-height-exits.md), [disabled membership](effect-membership-disabled.md), [effect terrain occupancy](effect-terrain-occupancy.md), [creature occupancy lookup](creature-occupancy-lookup.md), [creature footprint initializer](creature-footprint-initializer.md) |
| GP09 | In-game menus and interface logic | Native menu/result, map, session, battle setup, lobby, Region Entry, Character, Grimoire, Spellbox and Realm Viewer previews have UI01/UI02/UI04/UI05/UI06/UI07/UI08/UI11/UI12/UI13 evidence; selected original callbacks and transitions have UI03 observation evidence. Original in-game logic retained; bounded Main/Quick actions have UI17 live evidence; Single Player setup/map/settings/player actions and original Start handoff have UI18 live evidence. Pre-battle spell selection has a separate V3 adapter and UI19 evidence. Quick Battle results have bounded V5 Continue/Quit and display/ownership evidence in UI21; Mini activation remains disabled (UI20). Main Preferences has V6 bounded UI23 evidence; other menu integration remains outstanding. [Layout inventory](../formats/menu-migration-inventory.md), [Engine observation](menu-engine-observation.md) |
| GP10 | Entity lifetimes and ownership | Static No-CD creature allocation/reset/activation, cleanup versus release, slot reuse, selected reference repair and expiry recovered; secondary missile/effect admission/removal, third map-linked pool and teardown order mapped. NS01 adds a bounded native slot/generation and reference-ownership policy with native checkpoint restoration; no original lifecycle equivalence, live observation or native replacement. Complete death states, backing-array ownership, legacy save/load and reference audit remain open. [Evidence](entity-lifetimes.md), [native foundation](native-world-foundation.md) |

## Remaining work and next milestones

Reviewed 2026-10-04 against current source and linked evidence. This is a
planning summary of the boundaries above, not new experimental evidence or a
claim of replacement. Existing test/run counts were not rerun for this update.

| Area | Remaining work | Next bounded milestone and acceptance evidence |
| --- | --- | --- |
| Live frame capture/presentation (RE02) | Resolve readback-related surface-busy failures and live primary-frame admission; checkpoint/recording separation has a code fix awaiting fresh live validation | Record a menu → map → menu run with continuous game-owned frames reaching Qt, capture gaps/failures counted and original drawing retained; compare against readback-disabled operation |
| Native rendering and world scenes (RE01/RE03/AS04/GP08) | Complete section assembly/rotation, entity admission, remaining terrain producers, lighting/palettes, camera/map lifecycle and command routing with CPU-lock compatibility | Validate selected section-copy/rotation bytes against the pinned original, then compare a bounded assembled scene; keep offline scene agreement separate from live rendering replacement |
| Native menus (UI01–UI21/GP09) | Bind real engine data, semantic actions, acknowledgement/failure, screen ownership and pause/resume; recover remaining inventory/research/campaign contracts | Extend UI21 results to recovered portrait/control fidelity and broader outcomes; campaign Mini live ingress/pause/confirmation remains deferred in UI20; spell-selection adapter boundaries are tracked separately in UI19 and repeated direct-battle native restoration has UI18 evidence; validate hardware input and longer play before suppressing original menu drawing |
| Audio (AU01–AU28) | Live selection/routing, listener/map state, ownership/cadence and audible transitions; unsupported pitch/seek/cursor calls; physical device recovery and music coexistence | Exercise one supported live voice lifecycle with observed admission, PCM/output, completion and fallback outcomes; compare audible behavior and timing separately from exact offline PCM |
| Pathfinding (PF01–PF03) | Coherent live world capture, real-map search agreement, refreshed continuation inputs and multi-creature scheduling | Compare fresh and resumed requests in both known contexts: candidates/costs, route bytes, budgets and flags; retain original search until agreement is recorded |
| Input, hosting and media (IN01/IN02/ME01) | Enumerated focus/dialog/resize/detach/exit checks, game controls and full movie/sound playback/skip/return | Record a bounded actual-game interaction/playback protocol with explicit actions and outcomes; keep successful embedding evidence separate from frame-stream presentation |
| Native simulation (GP01/GP02/GP05/GP10/TH01) | Complete AI/combat/spell contracts, entity/reference ownership, world-update implementation and live thread/cadence evidence | Observe tick/thread/pause behavior and validate one selected entity or spell lifecycle independently before changing scheduling or replacing simulation work |
| Campaign, assets and persistence (AS02/AS05/AS06–AS20/CF01/GP06/GP07) | Live caller contracts, resource/reference reconstruction, triggers/progression, compatible save writing and simulation restoration | Obtain a real-save corpus and compare complete original read/write extents, then validate a bounded native round trip; preserve raw/unknown fields and separate structural decoding from restored gameplay |
| Application and protocol boundaries | Finish lifecycle extraction as integration requires it; extend versioned contracts to uncovered menu/audio paths | Validate each new action/channel independently with ownership, timeout/failure and fallback evidence; existing frame/input/media protocol extraction is already recorded above |
| Gameplay enhancements (GP01/GP03/GP04) | Commander orders, per-creature veterancy and more active mana tempo/economy remain unimplemented | Define and test intentional rules separately from recovered baseline behavior after the affected engine contracts are established |

Recommended immediate validation priority is RE02 continuous capture and
presentation. This is a proposed next milestone; the successful Wine-window
hosting run remains useful independently of that unresolved path.

**Unvalidated working-tree implementation:** `terrain_sections.hpp/.cpp` and
its CMake registration introduce selected ordinary-cell copying/rotation and an
explicit complete-grid assembly policy. No linked evidence report currently
establishes original agreement, installed-scene validation or live integration.
The registered `tests/terrain-sections-test.cpp` is present, but this review
did not execute it; this work does not advance AS04/GP08 validated coverage.
Random section choice,
object-linked rotation and entity admission remain separate boundaries.

## Coverage measurements

No aggregate percentage is assigned. Rows are a starter inventory, not equally
sized units of functionality. Source lines, tests, imports and row counts must
not become a whole-engine percentage.

For functional coverage, split each subsystem into stable behavior IDs first.
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
overlapping sounds; map transition; save/load; clean exit. This broader matrix remains proposed; the bounded live Main → Quick → Cancel →
Main observation in UI03 covers only that menu path. Record actual maps,
creatures and settings exercised.

## Binary-to-behavior accounting (2026-10-05)

The [traceability package](coverage/README.md) connects the hash-pinned No-CD
Ghidra discovery inventory to a cross-subsystem code/behavior index. The
reconciled register has 285 behavior/policy/gap entries, including all 104 spell
configuration IDs, and six bounded scenarios. It covers pathfinding/motion,
audio, assets, rendering/animation, Qt menus/services, runtime adapters,
protocols, native world/tooling and outstanding gameplay contracts. The detailed
Preferences register was consolidated without broadening its live-observation
scope. Selected original comparison reports and native scene reports retain
unchanged historical source fingerprints; code review itself is static evidence.

The [source census](coverage/source-index-20261005-final.json) indexes 844
implementation/header/assembly/build/schema/tool/test files, with 600 explicitly
linked and 244 still unlinked. Support files and in-progress work are retained;
a file link is not full functional coverage. The audit detects new, removed or
changed source files, mismatched explicit links, and focused-register changes
since consolidation. Census refreshes require review and never refresh original
validation hashes.

Run `python3 tools/audit-re-coverage.py` for current links, independent status
claims and evidence freshness. The indexing validation passed 39 synthetic guard
checks and the normal audit with no invalid claims. The snapshot retains 28
stale evidence records, 6,512 unclassified functions, all 190 unmapped imports,
1,543 unresolved indirect flows and three incomplete dispatch inventories.
`--require-fresh` rejects stale evidence; these historical warnings do not mean
the original comparisons failed when recorded. Source-census drift is reported
separately. No original comparison, live session or replacement was rerun here.

The binary inventory retains 6,675 discovered functions. Ghidra did not assign
the already-tested motion action `0x005104b0` to a function body; documented
recovered-range links preserve evidence without removing discovery gaps.
Executable gaps can also contain padding/data. Most original functionality still
requires mapping and bounded validation; no whole-engine completion percentage
is inferred from these entries.

The [generated summary](coverage/summary.md) is the reconciled snapshot; current
output comes from the audit. The [initial movement summary](coverage/summary-initial-movement.md)
and [initial validation](coverage/validation.json) remain historical.
[Indexing validation](coverage/code-index-validation-20261005-final.json) pins the
reviewed register, census and complete per-run audit report.

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
playback/return validation, live audio field/routing checks, semantic menu-action
integration, and native config/save file caller contracts. Selected animation
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
