# Engine modernization coverage ledger

Baseline reviewed: 2026-10-03. This ledger tracks reconstructed functionality,
modern implementations, and replacement of original work during live execution.
Those are separate milestones: offline tests and capture hooks do not establish
completed engine replacements.

Current assessment: substantial pathfinding reconstruction and native rendering
infrastructure exist. Selected input and media paths have optional adapters.
No complete gameplay subsystem is established as replaced by the reviewed
evidence. Whole-game functional and performance coverage are **unknown**.

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

This baseline uses source inspection and linked documents' recorded results;
their checks were not rerun for this documentation change. Existing uncommitted
asset comparison work is provisional. No original artifacts were consumed.

## Subsystem coverage

| ID and behavior | Implementation and integration | Evidence and confidence | Remaining boundary and next validation |
| --- | --- | --- | --- |
| PF01 Route request, search ordering, budgets and continuation | Scoped host models; offline replay and observation tools; original search retained | Static/synthetic; high within recovered rules. [Models](../../reconstruction/pathfinding/README.md), [sequence capture](pathfinding-search-sequence.md) | Compare real-map fresh/resumed searches in both known contexts: route bytes, budgets and flags |
| PF02 Neighbors, acceptance, movement predicates and costs | Scoped reconstructed rules with explicit world inputs; offline | Static/synthetic and isolated original i386 comparisons; high for tested helpers. [Evidence](../../reconstruction/pathfinding/README.md#evidence-and-confidence) | Match real-map candidates and costs; helper agreement alone does not establish complete search agreement |
| PF03 Frozen world ownership and per-context replay | Scoped snapshots containing one creature; offline | Synthetic capture/replay checks; high within scope. [World format](../formats/route-world-snapshot.md), [continuation](pathfinding-search-sequence.md) | Validate coherent live capture and refreshed continuation inputs; multi-creature scheduling remains separate |
| RE01 Persistent surfaces, rectangular updates, opaque/keyed copies, palettes and compatible swaps | Scoped OpenGL implementation; offline replay/demo | Synthetic CPU/OpenGL comparisons; high for supported operations. [Renderer](../../renderer/README.md) | Inventory unsupported operations and real-map usage; original drawing remains active |
| RE02 Frame capture, owned pixel tracking and Qt presentation | Scoped bridges; observation and optional viewport | Synthetic x86/CPU/OpenGL/Qt comparisons; high within bounded contracts. [Presentation](opengl-presentation.md), [owned session](opengl-owned-session.md) | Validate complete maps/transitions without gaps; bounded captures do not prove continuous replacement rendering |
| RE03 Complete live rendering backend | Partial foundation; no complete replacement recorded | End-to-end equivalence unknown. [Renderer boundaries](../../renderer/README.md), [startup investigation](render-startup-black-screen.md) | Route live commands to native surfaces, preserve CPU locks, cover required operations and validate long sessions |
| IN01 Qt shell and foreign-window hosting | Scoped X11/XWayland host; optional shell | Synthetic embedding/detachment; high for fixture contract. [Hosting](qt-shell-hosting.md) | Actual game focus, dialogs, resize, detach and exit; native Wayland unsupported |
| IN02 Input forwarding and selected polling APIs | Scoped event forwarding and three USER32 hooks; optional adapter | Synthetic Qt/X11 and Qt-to-x86 checks; high within scope. [Input](qt-input-forwarding.md) | Live menus, movement, scrolling and focus; original window/message handling retained |
| ME01 Movies and supported WinMM file sounds | Scoped Qt playback/hooks; optional adapter with fallback | Synthetic ABI/lifecycle/decode and installed Intro0 decode probe; high within scope. [Media](qt-native-media.md) | Full live playback, audible output, skip/return and call frequency; unsupported calls remain legacy |
| AU01 DirectSound setup, static PCM upload and duplicate storage | Scoped reconstructed setup/native storage; offline | Static/synthetic and 356 installed WAV comparisons; high for bytes/ownership. [Evidence](directsound-buffer-setup.md), [storage](../../audio/README.md) | Playback controls, voice scheduler, mixer, output and live adapter; game effects/voices remain Wine DirectSound |
| AS01 Loose asset resolution and read-only handles | Scoped native backend; offline; installed comparison work provisional | Synthetic path/file/lifetime/error tests; high for native policies. [Assets](../../assets/README.md), [contract](../formats/asset-file-interface.md) | Installed raw-byte comparison and WAV integration; original loader compatibility unverified |
| CF01 Encrypted configuration and lifecycle | Scoped decode/encode and experiment tools; preparation/inspection | Static and documented live precedence; high within findings. [Container](../formats/encrypted-cfg.md), [precedence](../formats/cfg-precedence.md), [writer](../formats/cfg-writer.md) | Config-driven mods are separate from engine replacement; native config manager not recorded |
| TH01 Threading and modern scheduling | Static investigation; worker architecture proposed only | High for inspected timer/import facts; simulation ownership unknown. [Threading](threading.md) | Trace gameplay ownership and profile before changing concurrency or result timing |

## Gameplay areas without recorded replacements

These entries are baseline gaps, not proof that no research exists. Recovered
movement predicates do not cover the whole creature update or AI system.

| ID | Area | Boundary |
| --- | --- | --- |
| GP01 | Commander orders and general creature AI | Route callers observed; complete orders/state machines/update loop not replaced |
| GP02 | Combat, damage, targeting and spells | No native replacement recorded |
| GP03 | Per-creature veterancy | No implemented gameplay feature recorded |
| GP04 | Mana generation, spending and economy | No native replacement or economy change recorded |
| GP05 | Simulation clock and update ordering | Ownership, dependencies and timing compatibility not established |
| GP06 | Campaign, scenario scripting and triggers | No native replacement recorded |
| GP07 | Saves and persistent state | No native replacement recorded |
| GP08 | Terrain/sprite loading, animation and scene composition | Surface operations partly reconstructed; complete pipeline not replaced |
| GP09 | In-game menus and interface logic | Qt host controls exist; original in-game logic retained |

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

Suggested scenarios: startup/movie/menu; idle map; individual movement;
simultaneous orders; blocked destination; combat/spells; palette/effect updates;
overlapping sounds; map transition; save/load; clean exit. These are proposed,
not completed checks. Record actual maps, creatures and settings exercised.

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

Smallest useful next evidence: live search sequence agreement, real-game draw
operation inventory, media playback/return validation and installed raw asset
comparisons. Add timing percentages after counters and boundaries exist.
