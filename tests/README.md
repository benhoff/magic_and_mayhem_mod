# Tests

Store automated checks and repeatable manual test protocols here. Each gameplay
experiment should state its clean baseline, single variable under test, expected
observation, and rollback procedure.

`./tests/test-tooling.sh` includes baseline integrity checks and the readable
route-request, search, neighbor-generator, and creature-acceptance models.
Run `./tests/test-reconstruction.sh` alone for the C++17
models with address/undefined-behavior sanitizers, or
`python3 tests/test-decompilation-baseline.py` for checksum/PE-byte verification
and tamper rejection. These are host/static tests, not live-game equivalence.

`python3 tests/test-route-trace.py` builds the trace replay and checks synthetic
zero/nonzero route counts, normalized arguments, full snapshot comparison,
pointer/flag mismatch detection, malformed data and architecture fail-closed
behavior. It is included in the tooling suite. For opt-in live capture and its
current ARM64 Wine limitation, see the
[route capture workflow](../reconstruction/pathfinding/README.md#route-capture-and-replay).

`python3 tests/test-launcher.py` checks gamescope defaults, display sizes, option
forwarding, custom runner composition, and direct Wine fallback. It uses the
prepared working installation and a fake compositor; it does not launch a game.
It is included in the tooling suite.

`tests/route-search-test.cpp` covers the weighted/wrapped heuristic, neighbor
table order, priority ties and stale entries, score improvements and payload
copies, partial paths, exact budget boundaries and resumed searches, vertical
and seam-crossing output, 16-waypoint truncation, byte preservation and the
route-request bridge. Its movement graphs are synthetic. CMake builds a host
library and all fifteen C++ reconstruction tests and Python world tooling checks; see `reconstruction/pathfinding/README.md`.

`python3 tests/test-search-support.py` checks hash rejection before output,
the file-backed neighbor/direction/six-link tables, complete assembly exports and
artifact hashes. It is included in the tooling suite.

`tests/route-neighbors-test.cpp` checks both generator branches: ordered candidate
bytes, acceptance inputs, restrictions, costs, movement-state reuse, link masks,
strict limits, budget behavior, and search integration. Engine collision and
scalar helpers are synthetic callbacks; passing does not establish live equivalence.

`tests/route-creature-acceptance-test.cpp` checks all 128 destination/intermediate
cell combinations, short-circuit order, category resets, member construction,
state rereads, exemptions, wrapping/overflow, and generator integration. Lower
movement/cell tests are explicit synthetic providers; no live-equivalence claim.

`tests/route-movement-test.cpp` covers lower movement categories, early/late
rejection, boundary bands, clearance, intermediate groups and both adapters.
`python3 tests/test-native-movement.py` additionally compiles it for i386 and
compares the model to isolated original `0x4f3990` machine code using identical
lower-helper fixtures. It requires Linux x86 and a working 32-bit compiler;
it is optional and not part of the portable tooling suite. More than 25,000
cases compare result, category, and ordered queries. See
[native reference procedure](../research/runtime/pathfinding-movement-test.md#reproduce-validation)
for hash checks and isolation boundaries.

`tests/route-cell-test.cpp` checks the one-cell and wrapped 2x2 footprints,
all provider failure combinations, duplicated cells, parameter identity and
acceptance integration. `python3 tests/test-native-movement.py --component cell`
compares 35,201 cases with isolated original `0x4f47d0` machine code. That test uses a shared fixture provider for `0x4f3550`; the new occupancy test
below executes that routine directly. See
[cell evidence and isolation boundaries](../research/runtime/pathfinding-cell-test.md).

`tests/route-occupancy-test.cpp` validates the reconstructed `0x4f3550` column
scan and its connection to footprint/creature acceptance. Run
`python3 tests/test-native-movement.py --component occupancy` for 9,509 original
i386 occupancy comparisons and 576 comparisons through the original footprint
routine. Both target routines are unmodified in the private mapping. See
[occupancy evidence](../research/runtime/pathfinding-occupancy-test.md).

`tests/route-validity-test.cpp` validates the reconstructed `0x4f46b0` footprint
and movement adapter. `python3 tests/test-native-movement.py --component validity`
compares 35,201 footprint cases and 3,072 cases through original `0x4f3990` and
`0x4f46b0`, including categories and query order. Lower `0x4f3440` is a fixture;
see [validity evidence](../research/runtime/pathfinding-validity-test.md).

`tests/route-cell-validity-test.cpp` covers `0x4f3440` flag/classification rules,
height promotion and overhead scans. Run
`python3 tests/test-native-movement.py --component cell-validity` for 101,180
comparisons through original cell, footprint and movement routines. All three
remain unmodified in the private mapping; see
[cell validity evidence](../research/runtime/pathfinding-cell-validity.md).

`tests/route-record-query-test.cpp` covers `0x4f4330` single-query forwarding,
four-column support aggregation, fallbacks and movement integration. Run
`python3 tests/test-native-movement.py --component record-query` for 64,522
original i386 comparisons. The shared native bridge returns integer 0/1 in EAX
for engine checks; C++ bool's AL-only ABI is insufficient. See
[support query evidence](../research/runtime/pathfinding-record-query.md).

`tests/route-cell-support-test.cpp` covers current/below cell support and terrain
flag `+0xb0`, using the shared map view. Run
`python3 tests/test-native-movement.py --component cell-support` for 253,456
comparisons through five unchanged original x86 routines. Only the three
remaining movement checks use fixtures; see
[cell support evidence](../research/runtime/pathfinding-cell-support.md).

`tests/route-boundary-test.cpp` checks `0x4f41a0` modes, signed limits, lower query
order and parameter rereads. `python3 tests/test-native-movement.py --component boundary`
runs direct fixtures and a second process with six original routines intact:
128,232 comparisons total. Only the two remaining clearance helpers are fixtures
in the integrated process. See [boundary evidence](../research/runtime/pathfinding-boundary-test.md).

`tests/route-clearance-test.cpp` checks both remaining clearance predicates and
the complete movement-helper chain. Run
`python3 tests/test-native-movement.py --component clearance` for 120,842 cases
with all eight original routines intact and no instruction redirects. Two
additional one-second isolated probes verify original cyclic oversized scans;
the host model guards against repeating them. See
[clearance evidence](../research/runtime/pathfinding-clearance-tests.md).

`tests/route-scalar-test.cpp` compares both output DWORDs of `0x5205b0`,
including its untouched `0x505840` / `0x505920` chain, across 149,070 cases. Run
`python3 tests/test-native-movement.py --component scalar`; see
[scalar evidence](../research/runtime/pathfinding-creature-scalar.md).

`tests/route-world-test.cpp` checks owned frozen map inputs through complete
creature movement, neighbor costs and search. `tests/test-route-world.py` checks
capture round trips, module relocation, malformed inputs and reference-report
comparison using synthetic memory. CTest runs both after building `route-replay`.
This is integration evidence, not a real-world search equivalence claim.

World tests also cover consecutive budget-limited calls, independent contexts,
resets and unsafe continuation rejection. `tests/test-search-sequence.py` uses
scripted debugger events to test common entry/return capture and pairing without
a game. CTest runs this tooling suite too (17 tests total). See
[sequence evidence](../research/runtime/pathfinding-search-sequence.md).

`python3 tests/test-neighbor-shadow.py` checks synthetic PE import staging,
payload/budget mismatch detection and malformed expansion rejection. It also
runs in reconstruction CTest. `./tools/test-shadow-bridge.py` additionally
builds the PE32 DLL and runs the synthetic ABI/capture harness in a dedicated
Wine prefix, then compares its output against `neighbor-replay`. That test
requires Wine IPC and does not launch the game or validate engine behavior.

The Qt application has its own CMake/CTest project in `apps/qt-shell/`.
`ctest --test-dir working/build/qt-shell --output-on-failure` checks headless
startup and, when Xvfb is installed, discovery, embedding and detachment of an
external fixture window. These tests do not launch the game. See the
[Qt shell instructions](../apps/qt-shell/README.md).

The Qt project's OpenGL presentation tests cover raw palette/RGB conversion,
shared frame validation and GPU framebuffer readback. `./tools/test-render-bridge.py`
adds a synthetic PE32 Wine surface producer and verifies its actual mapped bytes
through Qt/OpenGL. It requires Wine and Xvfb IPC; it does not launch the game.

`python3 tests/test-render-capture.py` covers the bounded native-pixel blit replay,
source keys, malformed evidence and event summaries. The Wine renderer test
also checks actual x86 Blt/BltFast hooks with before/after snapshots, negative
pitch, row padding, busy-surface rejection/retry, preserved API results,
old-interface unlock arguments and one-capture limits. Evidence and live command:
[drawing inventory](../research/runtime/render-drawing-inventory.md).

The new `renderer/` has its own CMake/CTest project. Its tests compare 196 native
integer OpenGL draws with a CPU reference, verify capture parser agreement,
detect poisoned captured output, reject malformed files and preserve a caller's
Qt context. `./tools/test-render-bridge.py` also compares fresh x86 Wine captures
through the OpenGL blitter before checking Qt frame presentation. Tests use
isolated Xvfb and Mesa software rendering. See [renderer instructions](../renderer/README.md).

`render-surfaces-test` additionally verifies 192 ordered updates/copies on retained
GPU surfaces, palette changes without native rewrites, GPU RGB/indexed conversion,
stale/foreign handles, budget limits and destination preservation after invalid
commands. Qt CTest includes `qt-shell-surfaces`, which displays the new renderer's
palette-resolved image and checks framebuffer colors and orientation. Capture
integration checks native pixels and GPU presentation hashes separately.

`./tools/test-render-lock-blits.py` exercises game-owned RGB checkpoints and
Blt/BltFast propagation through actual x86 hooks without launching the game.
Owned pixels are tested after original Unlock poisons lock storage. Independent
fake-engine outputs, Python copies and OpenGL replay must agree. Cases cover
chains, aliases, key changes, failed calls, CPU reseeding, subrectangles, limits
and conservative invalidation; see
[scope and provenance](../research/runtime/opengl-game-owned-blits.md).

`./tools/test-render-lock-blits.py --primary` checks live primary routing from
synthetic x86 Lock/Unlock and Blt/BltFast calls. Per-operation counters and RGBA
bytes are compared with independent fake-engine pixels; the final frame passes
through the Qt/OpenGL viewport. `--unknown-primary-caps` verifies that the primary
bit without returned caps provenance cannot publish. The same test validates
arbitrary-color downscaled and portrait frame streams; see
[primary routing evidence](../research/runtime/opengl-game-owned-primary.md).

`./tools/test-render-bootstrap.py` checks never-Locked 800x600 RGB destinations
through synthetic x86 hooks, independent fake-engine pixels, Python copies,
OpenGL command replay and Qt framebuffer readback. It verifies application
descriptor provenance, full opaque initialization, incremental updates and
rejection of incomplete or contradictory metadata. No game launch is needed.
See [bootstrap evidence](../research/runtime/opengl-primary-bootstrap.md).

`./tools/test-render-owned-flips.py` checks application-observed two-buffer chains,
failed Flips, explicit/aliased targets, unknown back pixels, incremental updates,
chain mutation, operation limits and conservative rejection. Independent native
engine pixels must match ordered stream snapshots and Qt framebuffer readback.
See [owned Flip evidence](../research/runtime/opengl-owned-flips.md).

`./tools/test-render-owned-palettes.py` checks indexed primary capture and palette
updates using independent synthetic engine indices/colors, ordered RGBA stream
snapshots and Qt framebuffer readback. It verifies creation, aliases, partial
entry coverage, reassignment, failures, nested mutation, limits, final Release,
negative pitch and legacy Unlock, with exact application API call counts.
See [owned indexed evidence](../research/runtime/opengl-owned-indexed.md).

`./tools/test-render-indexed-copies.py` compares independent engine indices/colors
with Python copy/swap replay, native OpenGL command output and palette-resolved
RGBA hashes, ordered frame snapshots and Qt framebuffer readback. It covers
opaque initialization, index keys, aliases, legacy/negative pitch, failures,
nested updates, delayed palettes and distinct-palette buffer rotation. See
[indexed propagation scope](../research/runtime/opengl-owned-indexed-copies.md).

`./tools/test-render-partial-locks.py` checks rectangular writes through actual
PE32 hooks, independently owned native checkpoints, unchanged borders, ordered
RGBA frames and Qt OpenGL readback. It covers modern/legacy Unlocks, negative
pitch, retries, invalidation, rejection and limits, then uses a merged checkpoint
for primary blit initialization. No game is launched. See
[partial Lock scope](../research/runtime/opengl-partial-locks.md).

`./tools/test-render-owned-session.py` tests one ordered replay containing owned
full/partial uploads, copies, two-buffer swaps and indexed palette changes.
Synthetic x86 fixtures supply independent native pixels and colors for CPU,
OpenGL and Qt comparisons. Negative cases cover failed original calls, Restore,
held Locks, exclusive file creation failure and capture bounds. No game launch
or original artifact is needed. See [ordered session evidence](../research/runtime/opengl-owned-session.md).

`./tools/test-render-input.py` verifies Qt-written cursor/virtual-key state
through synthetic PE32 polling hooks, including press generations, LastError,
stale/invalid snapshots and guarded IAT/cooperative-window behavior. Qt CTests
also exercise actual targeted X11 keyboard/mouse events and DPI scaling.
See [input scope and evidence](../research/runtime/qt-input-forwarding.md).

The partial-Lock suite also compares emitted UPDATE sessions with independent
Python copies and native OpenGL output/RGBA hashes. Upload counts exclude CHECK
bytes. It verifies poisoned CHECK rejection, indexed palette changes while
locked, exclusive-create failure, and the replay-byte budget with 2048-row
patches. Recording failures must preserve native commits and application calls.

## Native media without the game

`python3 tools/test-native-media.py` creates its own AVI/WAV fixtures and
checks exact Qt-decoded RGBA/PCM, movie completion/skip, sound looping/stopping,
NOSTOP, fallback and the PE32 calling convention. It uses Xvfb and a dedicated
Wine test prefix. It does not validate audible output or run Chaos.exe.

When live testing becomes possible, open `./tools/run-qt-shell.sh --native-media`
and click Launch game. Check that the intro is visible and audible in Qt,
Escape reaches the menu, a new map starts, movie playback returns to game
frames, and held keys/buttons are released across movie transitions. Exercise
one-shot/looped WAV sounds and normal DirectSound effects together. Close the
game normally and check that relaunch creates a fresh media channel. Record
observed results separately from the offline checks.

## Native audio buffers without the game

The complete read-only asset milestone runs with
`python3 tools/validate-native-assets.py`; add `--fixtures-only` to build/test
without game or original artifacts. It records workflow logs/results and links
the raw-file, PCM, and Windows API audit evidence. The audit has independent
synthetic parser/reference tests in `tests/test-file-api-audit.py`. See
[workflow and scope](../assets/README.md) and
[remaining Windows file APIs](../research/runtime/windows-file-api-audit.md).

The audio build now includes the Qt asset backend and five CTests.
`audio-asset-input` checks the WAV adapter and upload CLI using independent
synthetic PCM expectations, mixed-case Windows requests, mapped drive prefixes,
legacy host paths, input/sample size limits, and rejected inputs/output paths.
The upload CLI closes its file before PCM upload, exercising ownership beyond
input lifetime. Installed checks below use the same interface, with Python's
WAV reader as the independent decoded-output reference.

Native asset path resolution has an independent fixture-only check:
`cmake -S assets -B working/build/assets`, then build that directory and run
`ctest --test-dir working/build/assets --output-on-failure`. It exercises
Windows path aliases, component case matching, rejection policies, symlink
containment, and filesystem errors without reading game/original artifacts.
The same CTest suite also runs `asset-file-io`, covering binary read/seek/size,
partial reads and EOF, file/store/buffer ownership, allocation limits, denied
opens, truncation, and controlled helper failure paths.
See [asset test scope](../assets/README.md).

CTest `asset-byte-comparison` validates the raw comparison CLI with temporary
fixtures and independent Python hashes. `python3 tools/test-asset-files.py`
then compares every installed loose file through sequential/seek reads,
records input hashes and inventories, and verifies original artifacts before
and after. It produces no decoded PCM or live-game evidence. See
[installed byte-comparison evidence](../research/formats/asset-file-comparison.md).

`python3 tools/test-audio-buffers.py` exports hash-guarded static audio evidence,
builds the native buffer model, runs lifecycle tests, and compares uploaded PCM
against Python's independent WAV reader for every installed sound. It verifies
original artifacts before and after. No game is launched and no audible output
is produced. See [DirectSound setup scope](../research/runtime/directsound-buffer-setup.md).

`python3 tools/test-audio-voices.py` exports the voice-control assembly and
indirect-call audit, then runs fake-backend contracts alongside audio/asset
regressions. It checks status failures, stop/reset ordering, volume caches and
duplicate notifications, pan, looping and scheduler arithmetic. No game, audio
device or live hook is used. See [voice scope](../research/runtime/directsound-voice-controls.md).

`python3 tools/test-audio-voice-state.py` verifies native playback state using
fixtures only. It checks independent duplicate cursors/controls, one-shots,
loops, reset/resume, commit/release behavior, rejected operations, 306 independent
frame-step scenarios and a reconstructed-controller fixture. It reads no game
or original assets and produces no audio. See [native state scope](../research/runtime/native-audio-voice-state.md).

## Offline stereo audio mixing

`python3 tools/test-audio-mixer.py` builds and runs all eight audio/asset tests
using fixtures only. It records source and executable hashes under
`working/tests/audio-mixer/`. The mixer checks 120 format/rate/loop scenarios
against an independent rational PCM reference, exact split-block continuity,
gains, clipping and buffer lifecycle. No game or audio device is required.
See [scope and evidence](../research/runtime/native-audio-mixer.md).

## Qt PCM output without the game

`python3 tools/test-audio-output.py` runs all nine audio/asset tests with fixtures
only. Add `--device-test` to probe the default output and deliver a quiet
two-second synthetic tone with stop/restart; each device command has a 15-second
timeout. Evidence under `working/tests/audio-output/` records hashes, logs and
backend delivery separately from speaker audibility and game replacement.
See [output scope](../research/runtime/native-audio-output.md).

## Native voice bridge and staging

`python3 tools/test-audio-bridge.py` tests command dispatch, owned PCM, mapped
channel lifecycle and x86 COM routing through a silent broker. Wine is required
for the ABI fixture; no game is launched. `python3 tools/test-audio-staging.py`
checks the optional DLL import in a disposable installation, with original
manifest verification before/after. These are separate from audible/live checks.
See [contracts and evidence](../research/runtime/native-audio-voice-bridge.md).

## Persistence input readers

Build `assets/` and run its CTests for `persistence-loaders` and
`persistence-codec-comparison`. The optional installed runner is
`python3 tests/test-persistence-loaders.py working/build/persistence/mnm-persistence-inspect --installation working/game-clean`.
It brackets installed artifact reads with immutable-manifest verification.
See [validation scope](../research/formats/persistence-native-loading.md).

## WZD wizard definitions

The assets CTests include `wizard-loader` and `wizard-text-comparison`.
For all installed definitions, run
`python3 tests/test-wizard-loader.py working/build/wizard/mnm-wizard-inspect --installation working/game-clean --report working/tests/wizard-loader/installed-comparison.json`.
This brackets artifact reads with immutable-manifest verification and compares
all properties/typed values against an independent Python parser.
See [schema and validation boundaries](../research/formats/wzd-native-loading.md).

## CUR cursor assets

The assets CTests include `cursor-loader` and `cursor-byte-comparison`.
For all installed CURs, run
`python3 tests/test-cursor-loader.py working/build/cursor/mnm-cursor-inspect --installation working/game-clean --report working/tests/cursor-loader/installed-comparison.json`.
The runner verifies originals before/after and compares all image planes,
palettes and hotspots against an independent Python decoder.
See [CUR validation scope](../research/formats/cur-native-loading.md).

`./tools/test-profile-api.py` compares generated ASCII profile calls in a PE32
Wine fixture with the native reader. It records exact bytes, integer rejections
and intentional policy differences, without launching the game. See
[profile API evidence](../research/formats/profile-api-comparison.md).

## PCX indexed images

The asset CTests include `pcx-loader` and `pcx-byte-comparison`.
Compare installed files with
`python3 tests/test-pcx-loader.py working/build/pcx/mnm-pcx-inspect --installation working/game-clean --report working/tests/pcx-loader/installed-comparison.json`.
The installed run verifies originals before/after and records source hashes.
See [PCX validation scope](../research/formats/pcx-native-loading.md).

## BMP RGB images

The asset CTests include `bmp-loader` and `bmp-byte-comparison`.
Compare installed files with
`python3 tests/test-bmp-loader.py working/build/bmp/mnm-bmp-inspect --installation working/game-clean --report working/tests/bmp-loader/installed-comparison.json`.
The installed run verifies originals before/after and records source hashes.
See [BMP validation scope](../research/formats/bmp-native-loading.md).

## JPEG RGB images

The asset CTests include `jpeg-loader` and `jpeg-pixel-comparison`.
They require Qt Gui/JPEG support and Python Pillow; no display server is needed.
Compare installed files with
`python3 tests/test-jpeg-loader.py working/build/jpeg/mnm-jpeg-inspect --installation working/game-clean --report working/tests/jpeg-loader/installed-comparison.json`.
The installed run verifies originals before/after and records source hashes.
See [JPEG validation scope](../research/formats/jpeg-native-loading.md).

`./tools/test-installed-audio-profile.py [Sounds.ini]` verifies originals before
and after comparing installed audio sections/values with PE32 Wine and decoding
both catalogs through reconstructed tables. It never launches the game or opens
an audio output device. See
[installed profile comparison](../research/formats/installed-audio-profile-comparison.md).

## MPS map placements

The asset CTests include `mps-loader` and `mps-record-comparison`.
Compare installed files with
`python3 tests/test-mps-loader.py working/build/mps/mnm-mps-inspect --installation working/game-clean --report working/tests/mps-loader/installed-comparison.json`.
The installed run verifies originals before/after and records source hashes.
`python3 tools/export-mps-support.py --decompile` exports hash-pinned static
reader/caller evidence through read-only Ghidra processing.
See [MPS schema and validation scope](../research/formats/mps-native-loading.md).

`./tools/test-installed-audio-manager.py [Sounds-directory]` exercises installed
manager startup, direct upload, every catalog admission and every randomized
group member, with independent WAV/PCM expectations and cleanup checks.
`--executable` accepts an instrumented fixture. It verifies originals and input
hashes before/after; no game or output device is opened. See
[installed native audio manager](../research/runtime/installed-native-audio-manager.md)
for successful coverage and the remaining quoted-path/missing-Stream gaps.

## EVT event areas

The asset CTests include `evt-loader` and `evt-record-comparison`.
Compare installed files with
`python3 tests/test-evt-loader.py working/build/evt/mnm-evt-inspect --installation working/game-clean --report working/tests/evt-loader/installed-comparison.json`.
The installed run verifies originals before/after and records source hashes.
`python3 tools/export-evt-support.py --decompile` exports hash-pinned reader,
writer and selected caller evidence through read-only Ghidra processing.
See [EVT schema and validation scope](../research/formats/evt-native-loading.md).

`./tools/test-installed-audio-manager.py --dequote-source-leaf` explicitly
selects native quoted-leaf compatibility; literal behavior remains the default.
The native manager regression tests also cover lookup precedence and resolver
rejections. `./tools/export-audio-filenames.py` exports hash-pinned original
filename/open instructions and records fresh or historical cabinet-list evidence.
See [audio source filenames](../research/runtime/audio-source-filenames.md).

## TAG sprite-name tables

Asset CTests include `tag-loader` and `tag-record-comparison`.
`python3 tests/test-tag-loader.py working/build/tag/mnm-tag-inspect --installation working/game-clean --report working/tests/tag-loader/installed-comparison.json`
compares all fields, validates companion SPR name/occurrence agreement and
rehashes installed inputs. Original manifests verify before/after.
`python3 tools/export-tag-support.py` records reproducible structural evidence
and the limited executable literal search; no original TAG reader is claimed.
See [TAG validation scope](../research/formats/tag-native-loading.md).
