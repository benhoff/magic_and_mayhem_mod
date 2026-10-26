# Native alternate World integration and original ownership search

This work extends the [native owner](alternate-canvas-native-owner-20261025.md)
into the resident World rendering service. It also extends the original writer
search. Native policy and original recovery remain independent.

## Native resident route

`MNM_NATIVE_ALTERNATE_CANVAS=1` selects an owned GPU World target in
`CanvasWorld::beginResident`. The ordinary native GPU history must be fully
defined. A separate alternate generation is zero-initialized using the existing
`AlternateCanvas` owner, and its exclusive lease spans entry, ordered World
subqueues, copy-back and successful return. Subsequent pixels stay on the GPU;
the CPU plane establishes initial clear/publication/lease identity and is not a
CPU mirror of subsequent drawing. The render owner outlives its GPU storage.

The current live layout admits zero padding at 640x480 and 800x600. It copies
native history at each entry and completed native World pixels back to ordinary
storage at each checkpoint, before subsequent UI producers. This history seed
and zero-padding layout are explicit native policy. The original borrowed
dispatcher and its World-before-Lock/failure behavior are unchanged. The live
stream contains completed admitted World packets; it does not emulate an
original alternate Lock failure or write original slots `6e1f68/6e1f6c`.

Canvas identity, physical storage replacement and video extent changes replace
the scene and its alternate owner together. The old scene is destroyed before
its borrowed alternate surface. Exported presentation frames refer to ordinary
storage, so later alternate retirement cannot invalidate an output lease.
Malformed/incomplete frames release their lease and GPU plane on service
destruction without publishing partial World pixels. Normal worker shutdown
returns GUI leases, destroys command/World services on their render thread, then
destroys the renderer. The asynchronous GPU owner uses the same World service;
its counters cross threads in completed job metadata rather than through a
shared mutable owner.

Prepared replay restores its native baseline, draws into the alternate target
and copies back. Retained-image replay restores the completed ordinary image.
Neither advances source generations. Application reports include the actual
alternate publication/frame/copy-back/replay counters. The campaign test's
`--alternate-canvas` option requires those counters to agree with completed
Worlds, in addition to its existing normal-Quit/source/import/readback checks.
The option requires resident rendering and remains opt-in.

## Native controls

[controls07](alternate-canvas-live-results-20261025/controls07.json) passes
18 complete World frames, 48 prepared replays and 29,721,600 independent pixel
checks. The fixture covers both video modes, changing logical/physical canvas
identities, destination-dependent half blending, split World checkpoints,
ordinary output leases surviving alternate replacement, invalid/undefined
history, busy/out-of-frame calls, abandoned malformed frames and zero remaining
surfaces. No production frame readback is added. Diagnostic comparisons read
native pixels only after each checked operation.

The same execution passes existing resident-transition (21 queues), prepared
replay and retained-image controls. These are native GPU controls on Mesa,
separate from original complete-frame equivalence and performance acceptance.
The existing independent CPU owner/borrowed-dispatch evidence remains unchanged.

`tools/test-alternate-canvas-resident.py` records prospective behavior/scenario
and source fingerprints before execution and writes durable results directly
to SQLite. Its bounded working JSON declaration and child campaign report are
explicit interchange inputs. The private X server uses authenticated TCP and
Mesa; an isolated temporary filesystem contains its sockets/lock files.
Run controls or add `--live --source-game <owned-baseline> --prefix-template
<verified-idle-prefix>` with fresh `--output` and `--report` paths. Use the main
checkout's storage guard with absolute command/output paths. The default live scenario uses the existing Apprentice difficulty and two
20-second physical gameplay phases. The live launcher also requires `working/manifests/game-nocd.tsv` matching its baseline's required
files and the unadapted hash-pinned NoCD executable.

## Original behavior recovery

The original executable remains NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
[flow03](alternate-canvas-live-results-20261025/flow03.json) verifies the retained
recursive frontier hash and every decoded instruction against that image,
adds the selected complete initializer/copy/dispatch windows, and processes
542,955 decoded instructions. The original manifest passes before and after.
The exact historical [frontier input](alternate-canvas-live-results-20261025/frontier-input.json)
is retained in SQLite with its original hash, rather than regenerated as a
different historical discovery result.

The new constant-only direct-CFG fixed point carries facts across branches,
intersects facts at joins, loses uncertain loop values and discards register
facts at calls. Eleven synthetic controls exercise branch-carried aliases,
conflicting joins, overlapping writes, subregister/call invalidation, loops,
external entry points, escaped addresses, wide x87 stores, implicit LOOP
count invalidation and implicit bulk stores. The search
finds 6,008 branch entries with constants. This is a candidate search, not a
formal whole-program alias or reachability proof.

The [candidate review](alternate-canvas-live-results-20261025/flow-review02.json)
checks all three scalar overlaps: `4e4163` writes x padding at `6e1f70`,
`4e416d` writes y padding at `6e1f74`, and `4e4c4e` writes the independent
configuration flag at `6e1f78`. None publishes either pointer. No slot-address
escape is found within this constant domain. There are 2,972 implicit bulk
writer sites: 27 have bounded constant destination/count ranges, none overlapping
the pointer pair even when both direction-flag possibilities are included.
The other 2,945 bulk sites, 53,977 computed stores and all 2,948 prior unresolved
indirect edges remain unresolved. Missing constants are not evidence of absence.

Confidence is high for pinned bytes and the reported bounded candidates. The
original allocator, natural nonzero publication, initial backing pixels,
base/source relationship, complete writers and retirement remain unrecovered.
The prior complete-session zero-write observations remain historical; this static
search adds no campaign/load/save/multiplayer execution or global dormancy proof.
The native route does not supply those missing original facts.

## Retained failed attempts and boundaries

controls01 refused the host temporary-filesystem X11 lock quota. controls02's
Xvfb crashed while selecting the host NVIDIA EGL provider; the runner now pins
Mesa. controls03 passed before malformed-frame teardown was added. controls04
refused missing test targets after the game runner's production-only CMake
configuration; controls05 explicitly configures its required test targets. Final controls06
also pins all fixture/regression compiler inputs prospectively and checks the
392-file actual dependency closure; prior results retain their original hashes.
The initial CMake compiler check also refused the host `/tmp` quota; build
scratch and build output use this task's RAM staging.

flow01 rejected a one-byte-too-large focused disassembly boundary before any
candidate claim; flow02 uses the exact recovered exclusive boundary. live01
refused the copied installation's previously adapted executable. The owned copy
was restored by a script checking both exact before/after hashes, with prior
bytes and [staging receipt](alternate-canvas-live-results-20261025/staging01.json)
retained. The shared installation and immutable originals were unchanged.
live02 refused the missing local installation manifest. live03 rendered and
quit normally but did not meet the existing 64-source-World minimum; its failed
acceptance and native counters are retained. No failed attempt is promoted.

Padded live World coordinates/stride semantics, asynchronous live execution,
device recovery with this option, full original frame comparison, wider game
journeys and performance acceptance remain separate pending boundaries.

Live04 passed 316 ordered source/import/return generations and 667 extra native
replays, with zero World readbacks and normal Quit, before the final runner and
fixture provenance declarations. Its hashes and prior scenario scope remain
historical. Live05/06 retain an Initiate tutorial refusal: an actual
[Summon Zombie card](alternate-canvas-live-results-20261025/tutorial-refusal01.json)
requires prescribed gameplay input, while the unchanged physical Mini helper
only dismisses a witnessed Hermes heading. No unknown-modal dismissal, tutorial
patch or relaxed rendering acceptance is introduced. The final live scenario
uses the existing Apprentice configuration; Initiate tutorial integration stays
open.

## Final live result

[live07](alternate-canvas-live-results-20261025/live07.json) passes the final
prospective Apprentice scenario: 255 source Worlds, 255 command imports, 255
verified original returns, 556 independent prepared replays and 811 alternate
copy-backs. World, native and RGBA readback counters remain zero. The original
process exits normally, source/import/return credits balance, the native session
retires normally, the private Wine clients stop, source fingerprints remain
unchanged and original manifests pass before/after. All actual compiler inputs
match the prospective declaration. This is functional live observation on Mesa,
not an original alternate-owner or performance-equivalence result.

## Local evidence retention

The [bounded raw bundle](alternate-canvas-live-results-20261025/raw-validation01.json)
retains 943 individually verified executable,compiler-dependency,log,wire-status
and diagnostic-image members. The compressed177,225,812-byte payload is in
SQLite. Installation/prefix trees and six oversized channels remain in this
owned worktree under its `.retain` pin; none were removed. RAM build outputs
are temporary; the tested executables and dependency receipts are durable in
the bundle. All new generated research artifacts are SQLite references.

[Source retention](alternate-canvas-live-results-20261025/source-versions02.json)
keeps exact current/intermediate source bytes matched to their original hashes.
It explicitly retains one older limitation: the20261020 frontier generator
`inspect-all-original-recovery.py` at fingerprint `e32ed636...` was not located
in current source or Git history. Its retained raw frontier and cited inputs
remain hash-verified; the fresh search independently rechecks every instruction
against the pinned executable. The old generator is not claimed reproducible
and no historical source fingerprint is refreshed. All current native
execution/compiler inputs are retained and matched.

The [final audit04](alternate-canvas-live-results-20261025/audit04.json) has no
errors and confirms current native implementation/live-integration support.
Existing unrelated historical staleness remains. Audit02/03 retain rejected
static retention metadata (missing/empty source provenance);the final retention
review pins the actual SQLite writer/reader and adds no engine validation. The
legacy audit CLI requires physical output,so its exact outputs are retained as
bounded working interchange and imported through the SQLite writer.
