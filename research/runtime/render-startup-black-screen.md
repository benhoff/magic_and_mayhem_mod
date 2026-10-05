# Real-game black-screen startup investigation

## 2026-10-04 Forest of Pain difficulty picture delayed

User report: the picture takes 20+ seconds to update; Wine continues normally.
Latest experiment `run-qyfqz9tj`, launcher log `run-20261005T011144Z.HMNJC5`,
loads bridge `2c6755a3f866a3920e4f4754f915b7c97b1f4711fb315704cec4cdf39803c40a`.
Its per-second journal has no publications at seconds 14–18, 20–22, 24–26,
28 and 30–33, followed by mostly 18–20 publication calls/s from second 35.
Scene-to-journal timestamps were not observed, so those seconds cannot be labeled
as particular menus. Busy seqlock samples are rare. Bounded logs also contain
keyed offscreen `blit_unsupported` records; they do not identify the rejecting
branch and cannot by themselves establish scaling, clipping or effects.

An isolated original-game reproduction now confirms missing GDI-derived
checkpoints on the same menu path, followed by successful recovery with
application-owned bitmap capture. Read the [GDI evidence and boundary](opengl-game-owned-dc.md)
for before/after screenshots, hashes and fixtures. Unsupported blits now record
specific reasons plus source/destination rectangles and layouts, without extra
COM calls. This prevents future unsupported menu draws being misclassified as
general Qt slowness. Complete live rendering/performance remains unverified.


## 2026-10-04 low Qt update cadence: bounded tracker waiting

Latest user experiment `run-8s6ddeg4`, launcher log
`run-20261005T005041Z.Ob6Ur7`, loads bridge
`0ff21ae99a153c4d06f64b347eddde02d6c9022bb450e463b3ad1a6537872e54`.
The user still reports slow Qt presentation. Its retained stream contains 165
published updates, sequence 330, status 1 and 800×600 RGBA. Inspection samples
are static after the game process has exited; they do not measure live update
rate. The running shell's reported CPU usage was 5.1% (process lifetime average),
and an independent `glxinfo -B` probe on display :1 reports direct NVIDIA TITAN
RTX rendering. These observations do not measure the Qt paint latency or prove
that every slow interval has the same cause.

The bounded lifecycle log records contention and component resets on the primary
as well as offscreen sources, followed by rejected incomplete initialization.
It contains no `primary_region_presented` sample. Narrowing invalidation prevents
unrelated loss but still discards a primary checkpoint if the one-shot capture
guard fails on that primary. Confirmed implementation limitation: ordinary
cross-thread overlap was treated like an unobservable operation even though the
tracker never holds its guard over an original game COM call.

The guard now retries brief cross-thread overlap, using 64 `pause` probes and
`Sleep(0)` yields with an 8-ms GetTickCount budget. GetTickCount resolution limits
timeout precision; this is not an exact eight-millisecond latency guarantee.
The guard records its owner: same-thread entry fails immediately, and timeout
retains scoped pixel invalidation or conservative metadata invalidation as
appropriate. Original API calls still execute outside the tracker guard.
`MNM_RENDER_TRACKER_WAIT_MS` accepts ASCII decimal values 0–50; invalid values
retain the default. Zero reproduces the prior immediate-skip policy.
`tracker_wait_acquired` and `tracker_wait_timeout` provide bounded diagnostics.

A deterministic fixture uses two actual Wine threads: one holds the guard until
the drawing thread attempts entry, then releases it. Twenty partial updates
follow the two initial complete frames; a final full draw checks recovery.
High-resolution QueryPerformanceCounter samples include the synthetic original
draw and diagnostic capture work, not just the guard wait:

| Policy | Updates delivered during overlap | Median hooked draw | Retained frames |
| --- | --- | --- | --- |
| Immediate skip, 0 ms | 0 / 20 | 5.927 ms | 3 |
| Bounded wait, default 8 ms | 20 / 20 | 10.039 ms | 23 |

Baseline report: `working/tests/render-bootstrap/run-w9ht_vpa/report.json`.
Bounded-wait report: `working/tests/render-bootstrap/run-tbucetsl/report.json`.
This demonstrates capture continuity under the tested interleaving; the added
work captures previously missing frames and does **not** establish a live FPS
or latency gain. Recorded pixels agree with the independent synthetic engine,
CPU/OpenGL replay and Qt framebuffer readback. Four additional fixtures cover
same-thread fallback, uncertain-source rejection, metadata loss and 62 continuous
keyed/fill frames. A separate foreign thread held for 50 ms verifies safe timeout
and full-checkpoint recovery in
`working/tests/render-bootstrap/run-l13xuuxv/report.json` (median hooked draw
33.846 ms, maximum 35.503 ms in this run). Fourteen lifecycle/alias fixtures pass
in `working/tests/render-lock-lifecycle/run-xnftevyf/report.json`.

Capture launches now start a separate read-only header sampler. It writes
`presentation-rate.jsonl` in the experiment directory once per second and at
exit, recording published-update deltas, elapsed time, busy/invalid sample counts,
latest counter and bridge status. The sampler uses the version-one sequence
contract at roughly 60 Hz and stops when the runner/Wine process exits or its PID
is reused. It performs no COM calls, reads no pixel payload and modifies no shared
channel. Its update rate counts full and partial publications, **not** game ticks
or Qt paints; stationary screens can legitimately report zero updates. The log
path and tracker environment are recorded in the experiment manifest.
`tests/test-render-stream-rate.py` checks unstable/invalid headers, missed polls,
counter wrap, report boundaries and read-only CLI operation. A hash-checked fake
Wine runner integration also verifies automatic monitor startup and output.

Rebuilt production bridge SHA-256:
`2c6755a3f866a3920e4f4754f915b7c97b1f4711fb315704cec4cdf39803c40a`.
Live cadence and remaining capture-operation coverage still require an
instrumented game run; no continuous scene equivalence is claimed.

## 2026-10-04 intermittent Qt freezes after performance changes

User experiment `run-wf_pb32k`, launcher log `run-20261005T003044Z.OZGeZ3`,
loads bridge SHA-256 `ea681817d8425708fcff97491466aa723348ba96c1d54729945f7bb8786ef06b`.
The user reports intermittent freezes across menu transitions and after entering
battle, and explicitly confirms Wine continues animating and responding while
Qt freezes. The retained frame stream has sequence 122, frame count 61, status 1,
800×600 RGBA. Confidence: high for the observed divergence and retained state.

The lifecycle log records `pixel_tracker_contended` followed by multiple
`surface_pixels_epoch_reset` entries and `blit_incomplete_initialization` /
`fill_incomplete_initialization` rejections. Primary draws occur on threads
0x13c and 0x1b4. The log records at most four distinct samples per reason, so it
cannot establish the exact final-freeze interleaving or frequency. Confirmed
implementation mechanism: missed pixel tracking advanced the global pixel epoch,
discarding unrelated complete checkpoints. Partial updates could then fail to
reconstruct the screen until a full overwrite arrived. The proposed connection
to every reported freeze remains a hypothesis pending live follow-up.

Missed Lock, Unlock, Blt and pixel invalidation tracking now queues the affected
object token in a bounded 128-entry atomic queue. The next guarded operation
invalidates that observed component's checkpoints, pending locks and ordered
session evidence, preserving unrelated surfaces. Queued tokens are never
dereferenced. Duplicate misses remain separate to avoid a concurrent consumer
coalescing away a later mutation. Queue overflow retains the global pixel-epoch
fallback; uncertain identity/property tracking still invalidates metadata.
Unlock commits drain misses before matching their descriptor, and Blt commits
retain source/destination generation checks. No original API call is held under
the tracker guard, and no observer COM call or driver readback is added.

Every primary sprite previously converted the complete frame under the guard.
Primary copies/fills now convert only their changed rectangle if the stream
contains the exact preceding checkpoint, identified by surface token, generation
and frame count. Otherwise they perform full publication. The stream remains a
complete image with the existing version-one sequence contract. This reduces
avoidable guard occupancy without throttling updates or omitting a final draw.
Palette changes still republish the complete image.

Intermediate scoped-invalidation checks pass seven cases in
`working/tests/render-bootstrap/run-r4tynrbb/report.json`, including continued
partial drawing without reseeding the main source, an unrelated nested missed
Lock, and conservative full metadata resets on missed property/final Release
tracking. Final rectangle-publication checks in
`working/tests/render-bootstrap/run-hrrxcs8a/report.json` cover continued partial
updates, rejecting a pending draw after a missed mutation of its actual source,
queue overflow and full-checkpoint recovery, keyed sprites and partial fills
after the recording limit, 24/32-bit fill composition, and ordered fill GAPs.
Each recorded operation is compared against the independent synthetic engine,
CPU replay and OpenGL; the final Qt framebuffer is checked separately.
Three indexed follow-ups pass in
`working/tests/render-indexed-copies/run-rje264ag/report.json` (alias copy, nested
palette update, two-buffer rotation), and the mixed ordered session passes in
`working/tests/render-owned-session/run-0lebh7ul/report.json`. The rebuilt
production bridge SHA-256 is
`0ff21ae99a153c4d06f64b347eddde02d6c9022bb450e463b3ad1a6537872e54`.
The immutable manifest verifies all 2,927 original files before and after this
work. Synthetic results do not establish continuous live reliability or battle
pixel equivalence.

## 2026-10-04 battle visible: Qt performance follow-up

User experiment `run-yfi5mzh2`, launcher log `run-20261005T001107Z.eONbKf`,
uses bridge SHA-256 `63d7139ec82db4a5f8eaf7c4d8717b3b54e490cbee22d8df8e5fe82cf1d88bb7`.
The user confirms battle presentation works, with Wine smooth and Qt very slow.
Confidence: high for the reported experience; no live frame timing or pixel
comparison was collected. The lifecycle log also reports `surface_capacity`
and untracked fills after exhausting the former 32 metadata slots.

Capture now avoids full source/destination clones once bounded per-operation
recording finishes (ordered sessions retain snapshots). It updates only owned
checkpoints after successful original calls and unchanged epochs/generations.
Opaque copies use row copies, fills replicate one filled row, and keyed writes
retain exact native key comparison. The PE32 copy primitive uses `rep movsb`
instead of a byte loop without adding CRT or SIMD dependencies. RGB565 conversion
uses exact floor-scaled component tables; all 65,536 values match the independent
reference. Qt reuses texture allocation for unchanged dimensions and uploads
changed pixels with `glTexSubImage2D`.

Metadata capacity is 128 surfaces, with the same 64-MiB retained/pending pixel
bound and 32 pending Lock descriptors. The observed alias graph admits 256 edges;
a derived hash/union cache avoids repeatedly traversing it for each candidate
surface. Cache mutation, retirement, saturation and contention still discard
uncertain provenance. No new COM queries or driver readbacks are added.

Synthetic evidence: `working/tests/render-bootstrap/run-2wx55h5p/report.json`
passes five cases including 62 continuous live frames (opaque, keyed, partial
fill, failed final draw), 65 separately tracked assets and aliases, contention,
legacy aliasing, and ordered fill GAP handling. CPU expected bytes, PE32 output,
OpenGL replay and Qt readback agree where applicable. Fourteen lifecycle cases
pass in `working/tests/render-lock-lifecycle/run-qxjzr53r/report.json`, including
transitive aliases, final retirement and pointer reuse. Host pixel and frame-stream
tests pass. Five blit regression cases (conflicting aliases, reentry, failure,
recording budget and destination alias) pass in
`working/tests/render-lock-blits/run-j7rgisyi/report.json`.
`qt-texture-upload` checks twelve consecutive uploads and resizing
through framebuffer readback. These are synthetic correctness checks; live Qt
latency and full battle coverage still require a new launch.

## 2026-10-04 continued menu freeze: epoch reset and colour fills

User experiment `run-2i2a39c4`, launcher log
`run-20261004T234847Z.KhJ3NY`: Wine advances into battle, while Qt remains on
the menu. The retained header has five frames, sequence 10, status 1 and
800×600 RGBA layout. Bridge SHA-256 is
`45ce167cf133088c9abda9a745b218849b3b151d670acab9b07dc4095d9930d4`;
observer readback is disabled. No surface-failure file exists at inspection.
The lifecycle log now confirms exact descriptor source key `0x001f` is captured,
so the preceding source-key change is loaded but does not resolve the live freeze.
Confidence: high for recorded state and user-observed divergence.

Primary copies occur on threads `0x13c` and `0x1b4`. The log then contains
`unlock_epoch_invalidated` for `0x00c141f0`, an interface observed earlier in
the run, followed by missing primary progress. This demonstrates that the
known-component Unlock fix was insufficient: an earlier epoch reset can erase
the component before Unlock classification. The precise initiating interleaving
is not identified by the old bounded log. Pixels and metadata formerly shared
the same invalidation epoch, so pixel tracker contention could lose the primary
descriptor even after later successful full source locks.

Capture now has distinct pixel and metadata epochs. Missed Lock/Unlock/Blt
tracking and unknown successful Unlocks invalidate checkpoints and pending work,
while preserving validated layout, primary identity and properties. Rebuilding
pixels still requires a complete writable lock or an admitted full overwrite.
Missed metadata/property tracking, uncertain final Release, attachment changes,
alias saturation and unsupported successful Flips retain full metadata
invalidation. No original API call is serialized across the capture guard.
`pixel_tracker_contended`, `surface_pixels_epoch_reset` and
`surface_metadata_epoch_reset` distinguish these paths. This is a bounded
recovery implementation, not proof of the live interleaving's exact cause.

The log also records source-NULL Blt flags `0x01000400`
(`DDBLT_WAIT | DDBLT_COLORFILL`) on the offscreen and primary surfaces. These
previously invalidated destination pixels as unsupported. Later keyed copies
cannot initialize a missing checkpoint because transparent pixels retain the
old destination. Colour fills are now admitted with validated destination
layout, known absence of a clipper, a readable PE32 100-byte DDBLTFX and native
`dwFillColor` at byte `0x50` (local Wine `ddraw.h` layout evidence).
Flags beyond COLORFILL/WAIT, source rectangles, out-of-bounds rectangles and
colours outside the native bit width are rejected. Partial fills require a
complete destination; full fills can initialize one. Only successful original
calls commit, after epoch/generation checks.

Per-operation replay represents a fill as a constant native source plus an
opaque copy of the requested rectangle, explicitly derived from arguments.
CPU/OpenGL replay uses the existing command protocol. Ordered owned sessions
emit a GAP for a fill encountered during recording; no NULL source identity is
invented. This does not establish complete ordered-session fill coverage.
The remaining live battle comparison is separate from synthetic validation.

Executed evidence (2026-10-04): eleven fixtures pass at
`working/tests/render-bootstrap/run-6vc55izk/report.json`. RGB16/24/32 full
fills followed by keyed sprites match independent original-fixture pixels,
CPU/OpenGL replay and Qt readback. Failed fills do not commit; partial fills
without a base are rejected, while partial updates with a base retain borders.
Blt contention, Lock contention and unknown Unlock each recover to 22 exact
800×600 frames, recording only sixteen lock/blit files. Missed property changes
and contended final Release invalidate metadata and stop publication until it
is re-established, rather than inheriting stale state.

The fill/session boundary passes at
`working/tests/render-bootstrap/run-1ydrk4lp/report.json`, with an explicit
reason-6 GAP and no invented source identity. Fourteen lifecycle cases pass at
`working/tests/render-lock-lifecycle/run-hvber3zf/report.json`; four partial-lock
cases, including the 64 MiB budget, pass at
`working/tests/render-partial-locks/run-jju1drx1/report.json`; three Flip cases
pass at `working/tests/render-owned-flips/run-hh7dxch4/report.json`; three ordered
session cases pass at `working/tests/render-owned-session/run-0i6i586v/report.json`.
Production PE32 build and original-manifest verification before/after pass
(2,927 files). These 36 synthetic checks do not establish live scene equivalence.
Restart the shell and game normally with `./tools/run-qt-shell.sh --capture-locks`
to load the rebuilt bridge; compare battle, scrolling and return-to-menu frames
between Wine and Qt. Any remaining rejection/metadata reset is a separate
boundary, not grounds to publish guessed pixels.

## 2026-10-04 menu captured, battle presentation stalls

User experiment `run-o70h037m`, launcher log
`run-20261004T233725Z.ZccY8C`: the user confirms that the separate Wine window
shows the battle while Qt retains the menu. The retained frame header has
sequence 16, eight published frames, status 1 and 800×600 RGBA dimensions.
The manifest enables game-owned capture with observer readback disabled;
bridge SHA-256 is `f66cb2cab4b90667cdc28d5abbe227f4416d92b2b258317424349144c1118f10`.
Sixteen lock snapshots and sixteen propagated blit files exist; no
`surface-failures.log` exists at inspection. Confidence: high for this capture
state and user-observed divergence; complete battle pixel equivalence is absent.

The lifecycle log contains successful primary `blit_presented`, subsequent
`blit_incomplete_initialization` and `blit_unsupported` operations, and an
`unlock_unmatched` on the observed offscreen alias `0x00c141f0`. These pointers
identify this run only. Diagnostic deduplication prevents interpreting record
counts as call counts or establishing the exact first stopping operation.
The sixteen-file recording budget is no longer the live checkpoint budget.

Two code boundaries were identified and corrected:

- An unmatched successful Unlock previously advanced the entire capture epoch,
  deleting unrelated primary metadata as well as pixels. A known surface
  component now loses only its own checkpoint and pending locks. Unknown
  identities and contention retain conservative epoch invalidation. Failed
  Unlocks do not invalidate unrelated state. New diagnostics distinguish
  `unlock_target_invalidated` and `unlock_epoch_invalidated`.
- Successful application Lock/GetSurfaceDesc descriptors can expose a source
  blit key through `DDSD_CKSRCBLT` (`0x10000`), with low/high values at byte
  offsets `0x40`/`0x44` in both supported descriptor ABIs. The run contains
  Lock descriptor flags `0x1100f`, but the old diagnostic omitted those key
  values and capture used only observed SetColorKey calls. Exact descriptor
  keys are now retained; ranges remain unsupported. Missing-key rejection
  uses `blit_source_key_unobserved`; `source_key_descriptor`/`source_key_range`
  record flagged values. Unflagged fields are ignored. No additional COM call
  or original-game draw replacement is introduced.

Confidence is high for the code defects and descriptor layout (local Wine
`ddraw.h`); which boundary first stopped this run remains unconfirmed. Surface
table exhaustion now has the bounded `surface_capacity` diagnostic. Synthetic
validation and a fresh live battle comparison are separate milestones.

Executed fixture evidence: `working/tests/render-bootstrap/run-e19q0akg/report.json`
contains five passing cases. `continuous-unmatched` produces 22 exact 800×600
primary frames across a discarded unrelated lock, retaining only sixteen lock
and blit files. `descriptor-key` compares transparent copies with independent
engine pixels, CPU/OpenGL replay and Qt frame readback; descriptor ranges are
rejected without publishing a stale new frame. Existing continuous-release and
changed-description rejection cases pass. All fourteen Lock/Unlock lifecycle
cases pass at `working/tests/render-lock-lifecycle/run-gxq_btx5/report.json`.
Five primary blit regression cases (keyed, key-removed, alias, failed and chain)
pass at `working/tests/render-lock-blits/run-k5b14lif/report.json`.
Production PE32 build succeeds; original-manifest verification passes before
and after (2,927 files). These are synthetic checks, not live battle
validation. Close the game and shell normally, relaunch
`./tools/run-qt-shell.sh --capture-locks`, and compare the same battle in Wine
and Qt; the old process retains its loaded DLL.

## 2026-10-04 hands-on default-readback failure

User experiment `working/experiments/opengl-render/run-gpaw8536/`, launcher
log `working/logs/run-20261004T155857Z.x2pwnG/`: the user reports a surface-locked
dialog. The retained `surface-failures.log` contains
`application_bltfast 887601ae 0000013c 00c095cc 0000000c 00000010`.
Confirmed with high confidence within this run: the forwarded application
Surface2 BltFast returned `DDERR_SURFACEBUSY`, with DDBLTFAST_WAIT. The manifest
has `no_readback: false`, `capture_locks: false`, `skip_movies: false` and CD
music disabled. Its bridge DLL SHA-256 is
`a4a4c76fc86df21707041b02d84f40e82e333beb9b886c8f01786aa09a500849`;
the source executable is the ledger's pinned No-CD build.

This reproduces the busy-call symptom with default observer readback enabled.
Neither the locked source/destination nor the responsible interleaving is
identified. Mesa EGL, Wine pixel-format and Quartz errors are also present;
their causal relationship remains unknown. No game was relaunched during this
log inspection. Next bounded user comparison: `--capture-draws --no-readback`,
using the separate Wine window, preserving graphics and movie settings. If that
works, test `--capture-locks` separately for game-owned capture/presentation.

Follow-up user comparison `run-88j862gr`, launcher log
`working/logs/run-20261004T160112Z.OzSwgO/`: the user reports successful operation
with `--capture-draws --no-readback` and the expected absence of Qt frames.
This is live user evidence that the no-readback path works in this session,
implicating observer readback as a contributor without identifying the exact
interleaving. The same Mesa/pixel-format/Quartz diagnostics recur in the user's
successful log, so their presence alone does not establish the cause of the
surface-busy failure. Specific map/actions, duration and full movie behavior
were not supplied. Game-owned capture and Qt presentation need a separate run.

Game-owned follow-up `run-wuk2aaev`, launcher log
`working/logs/run-20261004T160308Z.EuvE2v/`: the user reports the game works but
Qt remains waiting for its first captured primary-surface frame. Manifest
inspection confirms `capture_locks: true`, `no_readback: true`, movies enabled,
the same bridge hash, and unchanged graphics environment. No surface-failure
file exists at inspection. Sixteen `lock-*.bin` snapshots contain 800x600 RGB565
pixels; the first is from one offscreen surface and the other fifteen from a
second. Lifecycle evidence includes `unlock_copied`, `unlock_succeeded`,
`blit_propagated`, `blit_initialized`, then `unlock_limit`. No presentation
success is recorded. Logged snapshot descriptors have offscreen/system-memory
caps `0x840`; the observed primary descriptor has caps `0x1000c200`.

Confirmed with high confidence: observed cross-interface Unlock matching now
works and real game-owned snapshots are produced; the 16-snapshot diagnostic
budget is exhausted. Source inspection shows this budget gates subsequent
pre-Unlock copying, rather than only file output. This can prevent later
checkpoint/presentation progress. The retained evidence does not establish why
the initial primary update was not captured, nor that removing the budget alone
would make live presentation complete. Diagnostic logs deduplicate and retain
at most four records per reason; their counts are not application call counts.
Continuous bounded-memory checkpoint maintenance separate from bounded disk
recording and evidence for primary update admission remain required. Original
gameplay success is user observation; Qt presentation remains unvalidated.

## 2026-10-04 continuous owned checkpoint fixes

Follow-up user launch `run-823dsolm`, launcher log
`working/logs/run-20261004T164626Z.nFL4H3`, reproduces sixteen RGB565
`lock-*.bin` snapshots, two blit replays and `unlock_limit`. The retained
lifecycle log shows an 800×600 primary descriptor (`caps 0x1000c200`),
offscreen writable locks, bootstrap/propagation to selected offscreen targets,
and no primary presentation record. The provided Mesa and Quartz warnings
alone do not establish the cause of missing Qt frames. Confidence: high for
recorded capture exhaustion; primary admission remains unresolved live.

Code inspection identified three bounded continuity defects:

- Lock copying was gated by accumulated disk bytes and snapshot count, so
  the owned checkpoint stopped refreshing after diagnostic recording ended.
- Blit/flip maintenance was likewise gated by diagnostic counts/bytes; indexed
  publication had an independent sixteen-frame cap.
- Uncontended final surface Release advanced the global epoch, discarding
  unrelated primary metadata/checkpoints along with the retired component.
  This is a demonstrated code boundary, not proof it caused this live run.

Lock, blit and flip file recording remains finite (sixteen files and existing
byte limits per recording family). Checkpoint storage and pending copies share
an independent 64 MiB bound and fixed identity tables. Successful supported
operations now maintain checkpoints and publish primary frames after recording
stops. Partial UPDATE replays are still emitted only for recorded lock IDs;
ordered session recording retains its own finite limits/GAP behavior.
Uncontended retirement drops only the released alias component and dependent
backbuffer links; uncertain contended retirement still invalidates the epoch.
No observer Lock/Unlock/QueryInterface calls were added.

`unlock_recording_limit` and `blit_recording_limit` distinguish bounded disk
recording from checkpoint rejection. `unlock_limit` now refers to live owned
memory capacity. `primary_blit_rejected` records the target/source tokens and
flags; its owner column is a state bitset: 1 source pixels known, 2 destination
pixels known, 4 destination layout known, 8 clipper state known, 16 clipper
attached. This diagnostic does not make clipped/unsupported operations safe to
admit. Existing metadata, mask, generation, thread and failure guards remain.

Repeatable checks include:

```bash
python3 tools/test-render-bootstrap.py --case continuous --case continuous-release
python3 tools/test-render-partial-locks.py --case budget --case memory-budget
python3 tools/test-render-owned-flips.py --case flip-budget
python3 tools/test-render-owned-palettes.py --case idx-budget
python3 tools/test-render-lock-lifecycle.py
python3 tools/test-render-owned-session.py
```

Executed synthetic evidence (2026-10-04): continuous/continuous-release
bootstrap cases each publish 22 exact 800×600 frames while recording only
sixteen lock and sixteen blit files. Partial-lock count-budget publishes twenty
exact frames; the byte-budget case records 64 MiB and still copies its later
partial update. RGB flip-budget publishes eighteen frames; indexed palette
updates publish eighteen frames. Failed calls, unsupported initial copies,
released aliases, missing palettes and finite session GAP/cleanup retain
separate regression coverage. Confidence is high within these fixture scopes;
no live presentation equivalence follows.

Retained reports: `working/tests/render-bootstrap/run-bmt0il35/report.json`
(five cases), `working/tests/render-partial-locks/run-wvanrq36/report.json`
(five), `working/tests/render-owned-flips/run-ha55zeo6/report.json` (five),
`working/tests/render-owned-palettes/run-h1_4l08a/report.json` (three),
`working/tests/render-lock-lifecycle/run-uyyyddc_/report.json` (fourteen),
`working/tests/render-owned-session/run-n1m2ywc0/report.json` (ten), and
`working/tests/render-lock-blits/run-jr7p7vm4/report.json` (one, eighteen primary
frames). The ordinary continuous case is also retained at
`working/tests/render-bootstrap/run-lojods5w/continuous/`. Reports belong to
their captured builds; inspect retained per-case DLLs for exact binary
provenance. Final flip recording-ID compatibility checks also pass:
`working/tests/render-owned-flips/run-41s55sdd/report.json` (two) and
`working/tests/render-indexed-copies/run-7d_ptgmx/report.json` (three, including
late-palette sparse operation IDs). Final live and self-test PE32 bridge builds pass. Original-manifest
verification passes before/after (2,927 files).

Fresh live validation remains required. Close the current game normally, then
launch the updated bridge:

```bash
./tools/run-qt-shell.sh --capture-locks
```

Use **Launch game**, enter a battle in the Wine window, move creatures/scroll
for several minutes, return to the menu and exit. Record map/actions/duration,
Qt frame continuity and any busy dialog. Inspect the new frame header and
lifecycle evidence for primary publication after the sixteenth snapshot; the
old running process retains its loaded bridge. Unsupported clipping/operations,
missing primary provenance, real-time costs and complete scene equivalence
remain separate boundaries. This change does not replace original drawing.

## Confirmed evidence (2026-10-03)

User run: `working/experiments/opengl-render/run-2lazh703/`.
Launcher log: `working/logs/run-20261003T154714Z.Tfdjsx/`.
The frame header has status 4 (old bridge loaded/waiting), sequence 0, frame
counter 0 and width/height 0. `draw-capture/events.bin` contains only its 16-byte
header. No blit checkpoint or history exists. Therefore this run has not reached
an observed surface operation or published any pixels; there is no evidence
that flip replay caused the black screen.

The log records repeated Mesa EGL driver-initialization warnings for NVIDIA PCI
ID `10de:1e02` and Wine pixel-format fallback messages. These are clues, not proof
of the cause. The staged executable's image base and guarded thunk match the
expected values (`0x00400000`, thunk `ff 25 14 50 5c 00` at RVA `0x19755a`). The
old loaded status does not prove that the runtime hook was installed or called.

On the same X11 display (`:1`), glxinfo reports NVIDIA TITAN RTX, NVIDIA 610.57.04,
and OpenGL 4.6. The Qt `--opengl-test` known-pixel fixture exits successfully on
that display. With software rendering selected, glxinfo reports Mesa llvmpipe
LLVM 22.1.8 / Mesa 26.2.1 and the same Qt pixel fixture passes. The required Mesa
and NVIDIA GLX/EGL libraries are present in both `/usr/lib` and `/usr/lib32`.
Thus Qt presentation and a software GL alternative work on this host; Wine's
actual game startup remains unverified.

## Implemented diagnostic/retry path

The bridge now distinguishes DLL load, failed hook installation, armed hook,
entry into DirectDrawCreate, success, HRESULT failure and failed interface
interception. It records the last HRESULT and intercepted create-call count.
Qt shows hard failures immediately and startup-stage diagnostics after ten
seconds without a frame. Older status-4 writers remain readable. The fixture
checks successful, failed and null-result creation while preserving arguments,
HRESULT and entry/final LastError. Existing surface/frame tests still apply.

Run a fresh shell after closing the failed game through its normal controls:

```bash
./tools/run-qt-shell.sh --software-rendering --capture-history
```

This selects Mesa before Qt creates a graphics context, using
`LIBGL_ALWAYS_SOFTWARE=1`, `__GLX_VENDOR_LIBRARY_NAME=mesa`, and the installed
Mesa EGL vendor JSON if present. The Wine child inherits this environment;
no prefix registry or system driver configuration is changed. Experiment metadata
records the selected graphics variables. The shell still waits for **Launch game**.
The option is a diagnostic fallback, not a confirmed fix for this game's startup.

Environment behavior is documented by [Mesa](https://docs.mesa3d.org/envvars.html)
and [GLVND's EGL vendor discovery](https://github.com/NVIDIA/libglvnd/blob/master/src/EGL/icd_enumeration.md).
For a hardware comparison, omit `--software-rendering` in a new shell.

If it still stalls, the new placeholder/status message separates a never-called
hook (status 5) from initialization inside Wine (6), a DirectDraw HRESULT (8),
hook incompatibility (9/10), or initialized DirectDraw with no observed primary
presentation (7). These cases require different follow-ups. No automatic
real-game relaunch was performed in this investigation.

Validation report: `working/tests/render/run-c9vopdrc/report.json` includes all
three startup fixtures and the existing Wine/native/Qt frame and draw checks.
All eight Qt shell CTests passed, including the explicit software-rendering
fixture and frame-reader diagnostic coverage. Production PE32 builds succeeded.

## Follow-up: armed hook with no calls

The user's software-rendering run `working/experiments/opengl-render/run-rh6zsxhs/`
has status 5, zero create calls, zero frames and an empty event inventory. The
user reports a black window throughout and subsequently closed it. A live stack
could not be obtained because Chaos.exe was no longer running. No new full-game
launch was performed automatically.

`./tools/test-render-import.py` now stages a **fixture**, not a playable game:
it verifies the pinned source SHA, adds the same bridge import, and replaces the
entry point with a small scripted sequence of API calls followed by ExitProcess.
The original game entry and game loop never run. The original source is checked
unchanged afterward. This tests the real PE base, thunk/IAT locations and loader
ordering that the older mock fixtures bypassed.

The original creation-only fixture passed at
`working/tests/render-import/run-_ckhm_uj/report.json`. Expanded fixtures passed
both imported EnumerateA and dynamically resolved EnumerateExA, followed by
DirectDrawCreate, under Xvfb/software rendering. The same sequence also passed on
the active NVIDIA display at `working/tests/render-import/run-68kjmrix/`. Thus
an import overwrite or universally broken adapter initialization has not been
reproduced. The actual game's earlier initialization still needs a live stack.

Static evidence: `0x0058f0c0` resolves DirectDrawEnumerateExA with GetProcAddress
(IAT `0x005c50d4`), calls it at `0x0058f102`, or falls back to the imported
DirectDrawEnumerateA thunk `0x00597560` at `0x0058f10c`. Actual drawing setup later
calls DirectDrawCreate at `0x0058a9c3` / `0x0058aaa4`. The game creates drawing
surfaces at startup `0x004e986a` before optional Intro0/Intro1 movie calls around
`0x004e992b` / `0x004e996c`; skipping movies is not justified as a fix for a
never-called drawing setup hook.

The bridge now observes both enumeration paths without changing their callbacks
or flags. Its GetProcAddress hook substitutes only the named EnumerateExA export
from ddraw.dll. Expected creation/enumeration thunks and the pinned image base are
checked before any import writes. Qt reports enumeration in progress, completion
or failure, and replaces a stale waiting placeholder when the launcher ends.
Microsoft documents the dynamic API lookup in
[DirectDrawEnumerateExA](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-directdrawenumerateexa).
Confidence: confirmed static calls and controlled API/loader fixtures; the
specific real-game stall remains unconfirmed.

Reopen the updated shell with the same software/capture command above. If the
black screen recurs, leave the Wine window running so a live stack can be read.
Changing more startup settings without that evidence would be guesswork.

Final callback/context fixture evidence:
`working/tests/render-import/run-6sehwgda/report.json` (software/Xvfb) checks that
both real enumeration callbacks ran before the intercepted creation. Existing
bridge regression evidence: `working/tests/render/run-0_vw3pl_/report.json`.
All eight Qt shell regression CTests pass with the extended diagnostic reader.

## Follow-up: frames captured, then a surface ownership error

The user's `run-g34xhs0b` manifest points to
`working/runtime/render/frame-6858c311-72fe-474c-b712-c606e34becc3.bin`.
Its header records one successful DirectDrawCreate, status 1, and 49 published
800x600 RGBA frames. This is a later failure than the zero-call run above.
The recorded graphics variables are empty: this run did not select the explicit
software-rendering fallback. Its capped event inventory has successful surface
creation, locks/unlocks and blits. History ends with GAP reason 6 (unsupported
operation), not reason 1 (concurrent history access).

The user recalls an error inside the Wine window about another thread owning
rendering. The pinned executable contains error descriptions saying access to a
surface or palette is refused when already locked by another thread. This matches
the recollection but does not establish the actual HRESULT or offending call.
The Wine log `working/logs/run-20261003T162725Z.YPJcO9` records Quartz media-type
failure `0x8007000e`, allocator decommit waiting and sample failure `0x80040211`.
Movie decoding is therefore a candidate; capture readback also briefly locks
surfaces, and its involvement has not been ruled out. No ownership checks have
been removed or application calls serialized as a speculative fix.

`--skip-movies` now sets `PlayFMV` and `PlayFMVOut` to FALSE only in the disposable
experiment's `[VIDEO]` preferences. It updates both plaintext and encrypted
preferences if present, validates both before writing either, verifies encoded
round trips, and records before/after hashes in the experiment manifest. It
changes no source installation or game balance settings. Start a fresh shell:

```bash
./tools/run-qt-shell.sh --software-rendering --skip-movies
```

Click **Launch game**. Leave draw/history capture off on this first retry to
remove its additional observer locks. The frame bridge still performs readback;
this is not a capture-free baseline. If this succeeds, repeat with
`--capture-history` to separate movie and observer interactions. If it still
hangs, keep the Wine window open for inspection. The bypass is a diagnostic
workaround, not a confirmed resolution of the underlying ownership failure.

Validation: `tests/render-movie-preferences-test.py` covers independent plaintext
and encrypted edits, preservation of unrelated settings/comments/line endings,
idempotence and rejection of ambiguous preferences without partial writes.
Stage-only experiment `run-ea231lr3` verifies the actual working preferences and
unchanged source hash without launching the game. All eight Qt shell CTests pass.
The user's `commands-0001.bin` replays successfully through Qt/OpenGL under
Xvfb/software rendering with framebuffer sample checks. This validates the
captured checkpoint, not uninterrupted gameplay or the incomplete history.

## Follow-up: movie bypass does not resolve startup

User runs `run-f9hulkut` and `run-g8y5d95e` both record the movie bypass,
software GL environment, no history capture, status 5, zero frames, zero
DirectDrawCreate calls and zero observed enumeration calls. Their initial logs
are empty beyond launcher messages. The user confirms a fresh plain
`./tools/run-game.sh` reaches the menu. Therefore the bypass is not a resolution;
the replacement launch path must be investigated before further movie tuning.
The plain comparison does not yet separate staging, import hooks, desktop
wrapping and the software-rendering environment.

A live WineDbg attachment to Wine PID 0x13c actually targeted the newer
`run-g8y5d95e`, not the already closed earlier run. Its initial snapshot stopped
on a newly created thread 0x188 at inaccessible EIP `0xffbb10ec`, and the launch
log subsequently recorded that page fault. No original game-thread stack was
collected. The timing and new thread make a debugger-induced attachment fault a
candidate, not proof of the original hang. Evidence is in the earlier run's
`live-debug/debugger.log`; the second attempted attachment records access denied
in the newer run's `live-debug/debugger.log`. Native GDB inspection was blocked
by Linux ptrace permissions. A temporary PE32 context-reader could not inspect
the target after it closed; it supplies no evidence about the startup cause.
No automatic full-game relaunch was performed.

`tools/debug-render-startup.py` prepares a copied PE32 i386 WineDbg and launches
an existing hash-verified staged executable from the outset. It checks both game
and bridge hashes and the mapped stream, restores the experiment's recorded
graphics environment, and leaves extra draw/history capture off. Its debugger
DLL override forces the copied x86 executable. It does not attach to a live game,
change system/prefix settings or patch another binary. It omits the explorer
desktop wrapper, an explicit diagnostic difference to keep in the comparison.

After closing other game/debugger windows, run:

```bash
./tools/debug-render-startup.py working/experiments/opengl-render/run-g8y5d95e
```

The tool prints debugger commands for startup `0x004e8d80` and drawing setup
`0x004e986a`. At stops, record `bt` and `info reg`; use `cont` to reach the next
stop. Avoid Ctrl+C on this WoW64 runtime: it also produced the inaccessible
`0xffbb10ec` debugger-thread fault described below. If already stopped there,
use `info thread`, select an original game thread with `thread 0xID`, and collect
`bt`, `info reg`, and `x /32x $esp`. Use `detach`, then `quit` to leave the game running. Terminal
output may be saved using `script`; preparation metadata is in `startup-debug/`.

Validation: `--prepare-only` verifies the actual staged game without launching
it. The copied PE32 debugger launched the pinned-image API-only probe, read
`$eip`, obtained a stack and continued to a normal exit under Xvfb in
`working/tests/render-import/run-6sehwgda/startup-debug-test/`. The probe does not
execute the game loop. This validates the debugger launch path, not the cause or
resolution of the user's hang. Confidence in the startup root cause remains low.


## Follow-up: startup breakpoints work; debugger interrupt faults

The user launched the copied x86 debugger and reached both startup breakpoints.
At `0x004e986a`, ESP is `0x0022f9f4` and EBP is `0x0000010a`; EBP is not a
usable frame-chain pointer, so the single-frame WineDbg backtrace does not prove
stack corruption. After continuing, the user reports a menu with incorrect
colors and a freeze. The mapped header at inspection records status 2 (primary
surface readback Lock failed), one successful creation and zero published frames.
There is no evidence that Qt pixel conversion produced those menu colors.

The user's subsequent Ctrl+C produced `0xffbb10ec` on a new stack at
`0x06eeff44`, with only ntdll/thread startup frames. This repeats the attachment
fault and is temporally associated with the debugger interrupt. It does not
identify the original game thread's stall. Confidence: high that interrupt and
attachment inspection are unreliable in this runtime; the underlying game freeze
and primary Lock HRESULT remain unresolved. The launcher now warns against
Ctrl+C and explains how to select an existing game thread if already stopped in
this fault. `info thread` lists threads; bare `thread` is not a listing command.

## Normal-launch new-game surface error

The user reached the menu and started a new game without WineDbg in
`run-p66k189j`, then reported the exact DirectDraw surface-already-locked-by-
another-thread error. This is distinct from debugger interrupt faults.
The mapped header records 103 published 800x600 frames, status 1, and one
successful creation. Metadata records history capture enabled, movies not skipped,
and no explicit software-rendering graphics variables. Quartz movie errors and
Wine/Mesa graphics warnings recur in `run-20261003T170031Z.YdADG5`.

The event file reached 2,048 entries: 836 successful application Lock calls,
835 successful Unlock calls, 362 successful BltFast calls, nine successful
CreateSurface calls and six successful Blt calls. Its final record is a
successful Lock. That difference is just the truncation boundary, not evidence
of a leaked lock. The inventory contains no failed HRESULT and cannot identify
the later error. Bridge observer locks bypass this application event inventory.

The bridge now logs negative application and observer HRESULTs separately in a
bounded, deduplicated `surface-failures.log`, including thread, surface,
interface and flags. Per-phase limits reserve space for later application
failures instead of allowing repeated primary readback failures to consume it.
This diagnoses the failing call; no ownership override, speculative lock retry,
or broad serialization has been introduced. The actual cause remains unresolved.

For a comparable normal launch, close the failed game and reopen:

```bash
./tools/run-qt-shell.sh --capture-history
```

Start a new game once. If the error recurs, inspect the new experiment's
`surface-failures.log`. Do not use debugger interruption for this reproduction.
The new synthetic primary-busy fixture verifies recorded HRESULT/thread/flags,
deduplication, no Unlock after a failed Lock, zero published frames, and preserved
application result/LastError. Existing Wine/native/Qt capture checks still pass.

## Failure evidence and observer-free readback comparison

`run-ugdn7zhm/surface-failures.log` records:

```text
application_bltfast 887601ae 00000140 0169ef24 0000000c 00000010
```

Confirmed: an original application BltFast returned `DDERR_SURFACEBUSY` on
thread 0x140, destination 0x0169ef24, Surface2 interface (kind 12), flags 0x10
(DDBLTFAST_WAIT). This call can fail because either source or destination is
locked; the current failure record does not contain its source/caller. No failed
observer Lock/Unlock was logged. That rules out an *observed* failed observer
Unlock, not a successful observer lock overlapping another application's call.
The event inventory has successful calls from engine wrappers at 0x58c05d,
0x58c488, 0x58c5be, 0x58c9af and 0x58cbb2; it does not establish which wrapper
made the later failed call. Do not interpret destination/thread pointers as
stable across launches. Root cause remains unconfirmed.

`--no-readback` now disables primary `capture()` and every observer `snapshot()`
Lock. It retains import/surface hooks and original application calls. This is a
comparison with the same staged executable and logging; no graphics driver or
movie settings change. Extra draw capture can record application events but
cannot produce pixel checkpoints/history under this mode. Qt clearly states
that its frames are disabled and the separate Wine window must be used.

```bash
./tools/run-qt-shell.sh --capture-draws --no-readback
```

Start a new game in the Wine window. If the error persists, observer readback
Lock calls are not required for that reproduction; if it disappears, that is
evidence implicating readback, but one timing-sensitive run is not definitive.
The synthetic mode fixture asserts zero primary Lock/Unlock calls, unchanged
forwarded Blt/Flip results and LastError, and zero published frames. Existing
capture regression remains separate; this is not a completed render replacement.

## First implementation of game-owned capture

The user's `run-iknkhwup` has `no_readback: true`, successful application events
and no surface failure file at inspection; the user reports successful new-game
play. This implicates observer readback as a contributor, but is not proof of a
specific interleaving or a failed observer Unlock.

An opt-in lifecycle implementation now tracks and copies full writable
application locks before their original Unlock, commits only successful Unlocks,
and forces additional readback off. `--capture-locks` enables it independently
of the old history. Nine PE32 fixtures pass, including poisoned pixel storage,
legacy/modern Unlock ABI, negative pitch, Unlock retry, rejected locks and
separation of offscreen/indexed snapshots from primary frame publication.
The mode is bounded diagnostic capture; full Blt/Flip-based presentation remains
unimplemented. Real-game validation is pending; do not describe this as an
end-to-end renderer fix. See `research/formats/render-game-lock-capture.md`.

## Live startup blocker: CD-audio error dialog

User lifecycle run `run-5bwpx2m5` has status 5, zero DirectDrawCreate and enumeration
calls, no lock snapshots and no failure log. The user confirms the separate Wine
window also has no menu. There is one live game process, Wine PID 0x138; no
surface lifecycle callback has been reached in this run.

A PE32 context helper used OpenProcess/ReadProcessMemory and balanced per-thread
SuspendThread/GetThreadContext/ResumeThread calls, without debugger attachment or
a remote interrupt thread. The main thread's frame chain ends at return address
`0x004e984c`. Static code immediately before that is MessageBoxA through IAT
`0x005c520c`, called at `0x004e9846`; this precedes drawing setup at `0x004e986a`.
Reading the retained MessageBox arguments yields owner 0, text `0x005fd2a0`,
caption `0x02a80310`, flags `0x00040030`. The exact strings are:

- Caption: `CDROM ERROR !`
- Text: `There is an undetectable problem in loading the specified device driver.`

Evidence: `run-5bwpx2m5/thread-contexts.txt` and
`thread-contexts-message.txt`; helper source/build is in
`working/tests/live-thread-snapshot/`. It did not dismiss the dialog or restart
the game. This confirms a modal startup dialog, rather than a lifecycle capture
stall. It does not prove every earlier zero-call launch had the same dialog.

The source prefs enable `[SOUND] CDMusicEnabled=TRUE`. Render staging now sets
that key to FALSE in the disposable plaintext and encrypted preference copies,
validates both before writing, verifies encoded round trips and records hashes
plus `cd_music_disabled: true` in the manifest. It uses the configuration path
rather than suppressing MessageBox errors or patching another instruction.
This avoids attempted CD music initialization in new render experiments. Existing
experiments retain their old preferences; restart into a new experiment to use it.

Validation: three staged-media preference tests pass, covering both file formats,
section scoping, preservation of other settings/comments and rejection without
partial writes. Stage-only experiment `run-cl4gjl1v` verifies CD music disabled,
lock capture enabled/readback disabled, and unchanged source preference hash.
No automatic real-game launch was performed; successful menu startup with this
change still requires the user's retry.

## Successful gameplay, but no lifecycle snapshots

The user confirms menu/new-game startup works in `run-e5utrmvj`. Its manifest
has CD music disabled, lock capture enabled and observer readback disabled.
At inspection it has one successful DirectDrawCreate, status 7, zero Qt frames,
no `lock-*.bin` snapshots and no surface-failure file. This establishes gameplay
progress and absence of recorded failures, not successful real-game capture.
The game was no longer running when a subsequent descriptor inspection was
attempted; no debugger attachment was performed.

The next step is to observe rejection reasons before implementing blit
propagation. Static code uses different interface fields in some surface wrapper
paths; cross-interface matching is a hypothesis, not a proven explanation for
the zero snapshots. The bridge now writes bounded successful/rejected lifecycle
metadata to `lock-capture/lifecycle.log`, including descriptor size/flags,
format/masks, owner/current thread and the supplied Unlock argument. A fresh
normal `--capture-locks` launch will distinguish interface/argument matching from
format, partial/read-only locks, or other rejected cases. No extra surface locks
or interactive debugger commands are required.


## Confirmed cross-interface Lock/Unlock mismatch

Evidence: user gameplay experiment `run-9vglav41/lock-capture/lifecycle.log`.
The bridge accepted Lock on Surface2 `0x016869bc`, but rejected Unlock on
`0x016869c0` as `unlock_unmatched`, with argument `0x058f0030` matching the
recorded pixel pointer. Other observed pairs include `0x0174fb54` /
`0x0174fb58` and `0x0172b99c` / `0x0172b9a0`. Confidence: confirmed differing
interface tokens and matching pixel arguments explain the tracker rejection.
Their four-byte difference is launch-specific evidence, not a general alias rule.

The accepted descriptor has size 108, dimensions 800x600, pitch 1600, 16-bit
RGB565 masks and caps `0x840` (offscreen/system memory). These are not directly
locked primary surfaces, so fixing snapshot capture alone does not establish Qt
presentation. Following successful blits to the primary remains a separate step.

The tracker now links supported surface interfaces only after successful
application QueryInterface calls, including transitive relationships. It checks
Unlock's ABI using the unlocking interface rather than the locking interface.
No extra COM query or surface Lock is issued. Failed queries and unobserved
aliases cannot establish provenance; final Release retires the component to
prevent pointer reuse from inheriting aliases. Relationships and snapshots remain
bounded. Fourteen synthetic PE32 lifecycle fixtures validate these cases and
original-call counts, results, arguments, LastError and pre-Unlock pixel copies.
A fresh real-game launch is still needed to confirm this fix on the user's
surface pairs; success means `unlock_copied`, `unlock_succeeded` and native
`lock-*.bin` files rather than only `unlock_unmatched`.
