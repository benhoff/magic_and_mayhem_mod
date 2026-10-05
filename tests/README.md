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

`python3 -B tests/test-re-coverage.py` checks discovery/evidence claim guards.
`python3 -B tests/test-coverage-gate.py` checks hash-bound change receipts,
shared-source impact, renames, pending validation, immutable original evidence,
discovery preservation and comparisons against real temporary Git commits.
It also exercises staged-tree isolation, real pre-commit rejection/acceptance,
and local hook installation, removal and preservation of existing hooks.
Both suites and `python3 tools/check-re-coverage.py` are included in tooling;
the dedicated coverage CI workflow runs them without installed game artifacts.
See [the change-accounting workflow](../research/runtime/coverage/README.md#required-change-accounting).

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

Application-owned GDI bitmap recovery uses actual Wine DIBSECTIONs, independent
engine copies, native command replay and Qt readback. It checks orientation,
failed-release retry, mismatched masks, unmatched DC ownership and a swapped
selected bitmap:

```bash
python3 tools/test-render-bootstrap.py --case dc --case dc-bottom-up --case dc-retry --case dc-format --case dc-unmatched --case dc-swapped --case dc-rgb24 --case dc-rgb32
```

The bounded live Forest of Pain check requires Pillow, Xvfb, Wine, X11/XTest and
a built Qt shell. It stages disposable game preferences and an isolated prefix,
compares title/difficulty pixels with Wine, observes idle publication and checks
Qt readback. It does not measure NVIDIA paint latency:

```bash
python3 tools/test-render-menu-delay.py
```

See [GDI context evidence](../research/runtime/opengl-game-owned-dc.md).

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

## FP Realm Viewer flag paths

Asset CTests include `fp-loader` and `fp-point-comparison`.
`python3 tests/test-fp-loader.py working/build/fp/mnm-fp-inspect --installation working/game-clean --report working/tests/fp-loader/installed-comparison.json`
compares every header field and point, rehashing installed inputs and verifying
original manifests before/after.
`python3 tools/export-fp-support.py --decompile` exports hash-pinned static
reader/constructor/point/flag-slot/caller evidence with read-only Ghidra processing.
See [FP validation scope](../research/formats/fp-native-loading.md).

## SFT font input

Asset CTest includes `sft-loader` (ownership, lookup, malformed/truncated input
and resource limits) and `sft-glyph-comparison` (independent profiles/frame/RLE
decoding, direct/indexed glyphs, aliases, empty input and atlas transparency).
Run the installed comparison and optional atlas export as documented in
[SFT loading](../research/formats/sft-native-loading.md); it guards original
inputs with manifest verification. Text layout and UI integration remain separate.

## NOD node input

Asset CTest includes `nod-loader` (ownership, signed/raw fields, all truncated
prefixes with repaired size words and storage limits) and `nod-record-comparison`
(independent packed decoding and complete byte reconstruction). The installed
comparison guards original inputs and compares all 683 files; see
[NOD loading](../research/formats/nod-native-loading.md). Graph reconstruction
and pathfinding integration remain separate.

## TXT and WBT input

Asset CTest includes `text-wbt-loader` and `text-wbt-comparison`: exact
byte/line preservation, typed scrolls and the installed WBT statement subset,
ownership, malformed input and decoded allocation limits. The independent
Python runner compares all 43 TXT/two WBT inputs and reconstructs original
bytes from reported line data, with manifest/input-hash guards.
See [reproduction and boundaries](../research/formats/txt-wbt-native-loading.md).

`dat-loader-test.cpp` exercises headerless Brain/Experien ownership, exact numeric
bits, every incomplete record prefix and decoded/count/input limits.
`test-dat-loader.py` independently parses both schemas, compares every native
field and reconstructs all bytes. Asset CTest runs seven synthetic fixtures;
`--installation working/game-clean --report working/dat-comparison.json` adds
both installed files with manifest checks before/after and unchanged input
hashes. See [DAT evidence](../research/formats/dat-native-loading.md).

`legacy-asset-loader-test.cpp` checks ANI v3/v4 normalization and SPR v2
ownership, empty/aliased frames, numeric metadata, masks and malformed extents.
`test-legacy-asset-loaders.py ANI_INSPECTOR SPR_INSPECTOR` compares independent
synthetic layouts; `--installation working/game-clean` compares every record
in 136 ANI files and every pixel/field in all seven v2 SPRs, with immutable
manifest guards. See [legacy evidence](../research/formats/legacy-ani-spr-loading.md).

Asset CTest includes `save-world-loader` and `save-world-comparison`: owned
world ranges, empty/populated optional branches, truncation/budgets and both
save envelopes. `tools/test-save-world-writers.py INSPECTOR` captures ten
selected original creature/missile/effect writer cases in an isolated 32-bit
host and feeds them into synthetic worlds. It hash-checks input, script-patches
only a disposable PE copy's fwrite entry and verifies the original manifest
before/after. No full original save or live restoration is validated.
See [world loading](../research/formats/save-world-native-loading.md).

## Native world and checkpoints

The standalone `game/` CMake project registers `native-world-lifecycle` and
`native-world-wire`. The former checks entity identities/reference repair,
transactional tick phases, command/storage limits, rollback, checkpoint
continuation and POSIX publication/refusal/errors. The independent Python
oracle checks exact wire bytes, malformed/recomputed-checksum inputs and
fresh-process split continuation. All fixtures are synthetic; no original
artifact, Qt or Wine is needed. See [build and checks](../game/README.md) and
[evidence/boundaries](../research/runtime/native-world-foundation.md).

`native-creature-movement` checks typed actions, phase order, command replacement,
target/entity release, blocked/limited routing, sixteen-point prefix replanning,
transaction rollback and map restoration with a deterministic navigation fixture.
`native-movement-process` runs the actual frozen reconstructed helper/search
adapter, independent complete v2 wire expectations and fresh-process position
traces. It also checks changed/missing maps, seam directions, zero-distance,
prefix replanning, corrupted route metadata and guarded file publication.
See [movement evidence](../research/runtime/native-creature-movement.md).

The `qt-texture-upload` CTest target in the Qt shell build verifies repeated
same-size frame uploads and dimension changes using framebuffer readback under
Xvfb/software OpenGL. Run it alongside `render-pixels` (including exhaustive
RGB565 conversion) and `frame-stream` with:

```bash
cmake --build working/build/qt-shell --target gl-viewport-upload-test render-pixels-test frame-stream-test
ctest --test-dir working/build/qt-shell -R '^(qt-texture-upload|render-pixels|frame-stream)$' --output-on-failure
```

Capture contention regressions can be run without Chaos:

```bash
python3 tools/test-render-bootstrap.py --case partial-lock-contention --case partial-blit-contention --case nested-lock-contention --case nested-source-contention --case pixel-miss-overflow
```

These exercise actual PE32 hook forwarding, continued partial screen publication
without source reseeding after unrelated misses, rejection of an uncertain
in-flight source, and conservative overflow recovery. Full frame comparisons
also verify that rectangle conversion preserves pixels outside the updated area.

## Native fine motion

`creature-motion-arithmetic` and `native-fine-motion-process` are registered by
`game/`. They cover bounded sample transitions, independent complete v3 bytes,
fresh-process intra-cell/fractional/cycle/prefix continuation and profile/map
refusal. The native movement unit test also covers fine-state rollback, failed
in-place restoration and cancellation. Optional
`python3 tests/test-original-creature-motion.py` compares the pinned original
motion action/consumption in an isolated i386 process with controlled callbacks
and verifies the immutable manifest before/after. See
[fine-motion evidence](../research/runtime/native-creature-fine-motion.md).

To compare immediate skipping with bounded cross-thread capture, run:

```bash
python3 tools/test-render-bootstrap.py --case thread-contention --tracker-wait-ms 0
python3 tools/test-render-bootstrap.py --case thread-contention --case thread-timeout
python3 tests/test-render-stream-rate.py
```

The thread fixtures use real Wine threads and QueryPerformanceCounter timing.
Report latency includes original synthetic drawing and capture/recording work;
it is not live Qt/game FPS. The rate sampler runs automatically for lock-capture
launches, or can inspect a running stream independently:

```bash
python3 tools/profile-render-stream.py PATH_TO_FRAME.bin --seconds 10 --output working/presentation-rate.jsonl
```


### Live Single Player setup

Build `mnm-qt-shell`, `menu-battle-bridge-test` and `single-player-battle-test`
from `apps/qt-shell`. `qt-menu-battle-bridge` checks V2 state/payload snapshots,
Windows-1252 names, malformed bounds and whole settings requests. The setup
widget check also preserves off-step original defaults without snapping.
`./tools/test-menu-observer.py --battle` executes selected original PE32 setup,
rule setter/callback and map list/button bytecode with engine display/loading
services stubbed, including whole-transaction rejection and removed players'
missing slider guards. Keep this distinct from live validation.

`./tools/test-live-menus.py --battle direct` exercises the real Qt buttons and
engine snapshots, chooses explicit disposable test settings to take direct
battle loading, captures the original viewport, and uses isolated XTest input to exit through
the original result Quit and require a Main/Quick tick. `--battle spells` preserves
original item/talisman defaults, exercises native reset and assignment, commits
through original callbacks and compares engine control assignments with Qt.
It requires a battle, Qt Quick return and normal Back/Quit. The spell run has a
180-second launch deadline; the direct run uses a 90-second game bound.
Both use isolated Xvfb and Wine prefixes and
original manifest checks before/after. Inspect captures before claiming actual
battle or spell-selection presentation. See
[contracts/evidence](../research/runtime/single-player-menu-bridge.md).

`./tools/test-live-menus.py --battle direct --battle-repeat` requires two Start
handoffs, two engine-confirmed Qt Quick Battle restorations, a fresh setup between
battles, and native Back/Quit followed by successful launcher exit. The game run
is bounded to 180 seconds; each battle exits through original result controls.
Explicit original-menu fallback remains permanent for that session. Existing evidence can be
rechecked without launching via `--validate-run working/tests/live-menus/RUN`.


### Native planar segment continuity

Build and run `ctest` from `game/` as documented in [game/README.md](../game/README.md).
`creature-segment-setup` checks bounded initialization; `native-segment-continuity-process`
checks independent v4 bytes, current/boundary saves, speed/turn/prefix continuation
in fresh processes, recomputed-checksum refusal and exact map rebinding.
`python3 tests/test-original-segment-setup.py` executes complete hash-pinned setup
with controlled eligibility/environment/animation dependencies;
`python3 tests/test-original-creature-motion.py` additionally checks separate
sample/animation cursors and zero-rate transitions. Both verify immutable input
before and after. These isolated checks do not establish live behavior or ANI
event production. See [NS04 evidence](../research/runtime/native-creature-segment-continuity.md).

### Native ANI-driven motion and checkpoints

Build `game/` and run its CTests. `ani-motion-composition` advances the recovered
ANI player with movement arithmetic; `native-ani-motion-process` checks independent
v5 bytes, event/reset/speed/turn/prefix continuation, owned ANI source deletion and
malformed/resource refusal. Its C++ companion validates failed in-place restoration
preserves both resources/state and subsequent tick behavior. `animation-forward-model`
checks owned controller restore and rejection before state mutation.

`python3 tests/test-original-ani-motion.py` compares unmodified original ANI
start/tick/restart together with the motion action, with only completion redirected.
`python3 tools/test-animation-contract.py` separately compares installed bytes and
selected controller traces. Both verify immutable input before/after. Profile
selection remains explicit numeric input; unsupported gameplay events are refused.
See [NS05 evidence](../research/runtime/native-ani-motion.md).

### Live pre-battle spell selection

Build `menu-spell-bridge-test` and run `ctest --test-dir working/build/qt-shell -R qt-menu-spell-bridge --output-on-failure`. Run `./tools/test-menu-observer.py --spells` for isolated original PE32 callback checks and `./tools/test-live-menus.py --battle spells` for the separate live path. See [contracts and evidence](../research/runtime/spell-selection-menu-bridge.md).

### Mini Menu bridge: pending validation

The dormant V4 Mini Menu implementation has **not been tested**. Its activation is OFF by default. The [offline contract](../research/runtime/mini-menu-engine-bridge.md) lists the later isolated callback/ABI, malformed wire/guard, V1–V3 regression and live pause/resume/Preferences/confirmation/return checks. Existing preview and spell bridge checks do not constitute Mini Menu engine evidence.

### Native terrain-aware movement

Build `game/` and run its CTests. `native-terrain-motion-process` checks independent
v6 wire bytes and fresh-process checkpoint/trace continuation across terrain
offsets, slopes, pure vertical up/down, category four, turns, seams, fractional
speed and prefix replanning, with and without ANI. Its C++ companion checks
planning/tick rollback, failed in-place restore and order/cleanup mode retention.
`test-original-segment-setup.py` now compares selected terrain/vertical/category-four
setup and ordinary coordinate snaps; `test-original-ani-motion.py` covers
planar/vertical motion with clamped height deltas and original ANI events.
Original runners hash-pin their disposable PE input and verify immutable files
before/after. See [NS06 evidence](../research/runtime/native-terrain-motion.md).

The `apps/world-scene` build adds `native-world-scene`: 72 masked/clipped mixed
CPU/OpenGL image comparisons, four diagnostic camera views and owned ANI display
boundary/refusal checks. `tools/test-world-scene.py SCENE SANDBOX` validates
installed SPR pixels across three synthetic terrain profiles/four views and
fresh-process visual continuation; it verifies the original manifest before
and after. Xvfb tests require a local display socket; run serially to avoid
concurrent display startup/cleanup collisions. See [NS07](../research/runtime/native-world-scene.md).

### Main Preferences engine bridge

Build `menu-preferences-bridge-test`, `menu-preferences-controller-test` and
`preferences-test` in `working/build/qt-shell`; run `ctest --test-dir
working/build/qt-shell -R 'qt-(menu-preferences|preferences)' --output-on-failure`.
`python3 tools/test-menu-observer.py --preferences` checks pinned original setters,
callbacks and V6 ownership/transaction guards in an isolated PE32 fixture.
`python3 tools/test-live-menus.py --preferences` automates Preview, Cancel rollback,
OK/file readback, reopen and Preferences window-close Cancel/Main Quit in Xvfb/Wine.
No manual interactions are required. See [scope/evidence](../research/runtime/preferences-engine-bridge.md).

### Main Preferences cross-launch persistence

`python3 tests/test-menu-preferences-store.py` checks synthetic importer/schema
and all-copy preflight. Build `engine-preferences-store-test`, then run
`ctest --test-dir working/build/qt-shell -R qt-engine-preferences-store --output-on-failure`
for original readback validation, atomic saving, locks and session conflicts.
`python3 tools/test-live-menus.py --preferences-restart` uses isolated config and
Wine prefixes to validate accepted settings across two fresh shell/game runs,
including byte-identical Cancel and normal original Quit. No manual interaction
is required. See [UI24 evidence](../research/runtime/preferences-persistence.md).

The world-scene build also adds `native-map-navigation`, covering ordinary MAP
projection, sealed crops, supported-cell admission, source ownership, frozen
layout and geometry/TTD/fingerprint mismatch refusal.
`tools/test-map-navigation.py BUILD_DIRECTORY` separately validates six installed
MAP crops against independent geometry bytes and original terrain predicates,
then checks route arrival, frame pixels and fresh-process continuation in four
views. It requires local Xvfb and verifies original manifests before/after.
See [NS08 scope/evidence](../research/runtime/native-map-navigation.md).

### Main Preferences resolution rebuilding

`python3 tools/test-preferences-contract.py` includes six original leave branch
and repeated-leave checks with private resource dependencies.
`python3 tools/test-live-menus.py --preferences-display` automates four original
resolution rebuilds, changed-draft Cancel and reopened controls in Xvfb/Wine.
It checks original client dimensions and font modes with a test-only forwarding
probe, then exits through original Quit. No manual interaction is required.
See [UI25 evidence](../research/runtime/preferences-display-rebuild.md).

### Campaign menu contract recovery

```sh
python3 tools/export-campaign-menu-support.py
python3 tools/test-campaign-menu-contract.py
```

The read-only export pins the No-CD build and saves original disassembly,
Ghidra functions, vtable prefix and realm configuration summaries. The targeted
32-bit oracle runs 269 original-bytecode fixtures with privately stubbed
loading/media dependencies: New Game reset order, region occupancy/admission,
auxiliary flags, Escape-to-Mini mode 4 and pending-return branches. Both commands
verify immutable originals before and after; the oracle uses warnings as errors
and a 20-second execution bound. No manual test or game launch is needed.
This does not test full Realm initialization, live navigation, campaign Mini
confirmation, Region Entry or a Qt bridge. See [UI26 contract and evidence](../research/runtime/campaign-menu-engine-contract.md).

### Original campaign ingress observation

```sh
python3 tools/test-campaign-observer.py
python3 tools/test-menu-observer.py --preferences
python3 tools/test-live-campaign-entry.py
```

The synthetic PE32 fixture checks opt-in forwarding guards, thread/error/return
preservation and bounded two-phase state logging. The live test stages a new
installation and private Xvfb/Wine session, clicks the original Main New Game
button from its CFG rectangle, and captures the original Realm-to-Region Entry
flow. It has bounded readiness/observation/game deadlines and cleans up only
its own prefix/process group. No manual interaction is needed. It verifies
original inputs before/after; it does not enter a region, alter campaign state
through a native dispatcher or validate campaign Mini mode 4. See
[UI27 evidence](../research/runtime/campaign-menu-entry-observation.md).

### Configured creature navigation

The world-scene build adds `configured-creature-profile` for selected CFG parsing,
recovered normalization, eight-facing ground ANI shape and derived sample/max
admission. `tools/test-creature-navigation.py BUILD_DIRECTORY` compares selected
original profile conversion, installed ANI sample/maximum construction and motion
states, then six configured installed-map crops, complete pixels in four views
and fresh-process continuation. It requires isolated PE32 execution and Xvfb;
immutable manifests are verified before/after. See [NS09](../research/runtime/native-creature-profile.md).

### Original Region Entry contract and fresh Cancel

Run `python3 tools/test-region-entry-contract.py` for 175 private original-code
cases (168 callbacks, four deferred-start flags, three pending Realm returns).
Dependencies are privately stubbed; no world is initialized.
`python3 tools/test-campaign-observer.py` checks forwarding guards, deduplication,
result/LastError preservation and Realm resume recording in a synthetic PE32 host.
`python3 tools/test-live-campaign-entry.py --region-return` uses isolated Xvfb/Wine
and automatic input to select all four original difficulty controls and Cancel a
fresh New Game back to Main. It verifies immutable originals before/after and
staged hashes, and terminates its disposable session within bounded time.
No manual interaction is required. Loaded-Realm return, live Enter and Qt campaign
dispatch remain pending. See [UI28 evidence](../research/runtime/region-entry-engine-contract.md).

## Stationary creature occupancy (NS10)

`native-stationary-occupancy-process` runs the owned occupancy/model-adapter cases
and three fresh-process checkpoint continuations. It covers occupied goals,
detour arrival, self exclusion, width/height boxes, generation reuse, cleanup and
release, next-edge and intra-cell obstruction, and transactional refusal. Use
`python3 tests/test-native-occupancy.py SANDBOX NATIVE_OCCUPANCY_TEST NEW_OUTPUT_DIR`
to preserve a report. `python3 tools/test-dynamic-occupancy.py` separately compares
the recovered cell and footprint predicates to the pinned original PE32.
See [NS10 scope](../research/runtime/native-stationary-occupancy.md).

### Live Qt Region Entry bridge

Run `python3 tools/test-region-entry-bridge.py` for original radio-selection/Cancel
behind V7 receiver, caller, generation/readiness, availability and one-shot guards.
Build the Qt shell and run targeted `qt-menu-region-bridge` and
`qt-menu-region-controller` CTests plus the existing Region Entry/V1/V6 checks.
`python3 tools/test-live-region-entry.py` automatically exercises Qt New Game,
all four original difficulty choices, Cancel, reopen and window-close Cancel/Quit
in isolated Xvfb/Wine. Use `--shell` and `--source-root` for a dedicated validation
build. Immutable-original checks run before/after; staged hashes are checked.
No manual interaction is required. Fresh Celtic region 1 only; live Enter and
loaded Realm remain pending. See [UI29 evidence](../research/runtime/region-entry-engine-bridge.md).

## Multiple moving creatures (NS11)

`native-multi-movement-process` runs owned synthetic MAP/ANI fixtures through
`native-multi-movement-test` and six exact fresh-process checkpoint continuations.
It checks shared planning debit/rotation, atomic conservative reservations,
conflict waits, generation/cancellation/cleanup, overlapping-state restore refusal,
32-driver capacity and tick rollback. Two staggered ANI actors match independent
solo simulations for 1,000 per-actor checkpoint comparisons.

Run `python3 tests/test-native-multi-movement.py BUILD/world/mnm-world-sandbox
BUILD/world/native-multi-movement-test OUTPUT` to retain immutable reports and
artifacts. [Scope](../research/runtime/native-multi-creature-movement.md) excludes
original scheduler equivalence, fine collision, mixed profiles and presentation.

## Multiple-creature presentation (NS12)

`python3 tools/test-multi-world-scene.py SCENE SANDBOX NEW_OUTPUT` compares
144 complete CPU/OpenGL frames and twelve fresh-process continuations across
terrace, slope and vertical fixtures in four views. It verifies each actor's fine
position/generation against the simulation trace, projection/depth, simultaneous
visibility and distinct opposite-facing displays, RGB565/PNG bytes and final
checkpoints. Installed SPR/TTD reads require original-manifest verification
before and after. `native-world-scene` additionally checks clipping/masks and
actor identity at equal depth, with no GPU surface leaks.

[Evidence boundaries](../research/runtime/native-multi-world-scene.md) distinguish
owned diagnostic presentation from original/live multi-creature equivalence.

### Fresh campaign Enter

Run `python3 tools/test-region-entry-bridge.py --enter` for original callback/admission guards, `python3 tools/test-campaign-observer.py` for World forwarding guards, and focused Qt Region Entry/V1/V6 CTests. `python3 tools/test-live-region-enter.py --shell <built-shell> --source-root <compiled-source-tree>` automatically exercises New Game, Adept and Enter through three original gameplay ticks in isolated Xvfb/Wine, ending at a bounded deadline. No manual testing required; campaign return remains pending. See [UI30 evidence](../research/runtime/region-entry-enter-engine-bridge.md).

## Scene selection and move controls (NS13)

`native-world-controls` tests the production Qt widget and Orders controller with
an owned synthetic multi-creature session: generation-safe selection, queued-only
commands, selected-actor movement, target preservation, invalid/stale/cleanup
refusal, blocked results, empty controls and 32 choices.

`python3 tools/test-scene-orders.py BUILD/world-controls-test NEW_OUTPUT` also
retains a screenshot and exact fresh-process pending-order continuation. Normal
and ASan/UBSan runs require a local Xvfb display, without original artifacts.
[Evidence boundaries](../research/runtime/native-scene-orders.md) distinguish
component/widget execution from original input or full-window picking equivalence.

### Campaign gameplay Mini Cancel

Run `python3 tools/test-campaign-mini-bridge.py`, targeted Qt campaign Mini/V4/widget/Region Entry/V1/V6 CTests, and both `tools/test-menu-observer.py --mini` / `--mini-disabled` fixtures. `python3 tools/test-live-campaign-entry.py --mini-cancel` observes original ingress/return; `python3 tools/test-live-campaign-mini.py --shell <built-shell> --source-root <compiled-tree>` validates native Cancel and Escape with original World resume twice, without manual input. Runs are bounded; timer pause and quit remain pending. See [UI31 evidence](../research/runtime/campaign-mini-cancel-engine-bridge.md).

## Native scene mouse picking (NS14)

`native-world-picking` checks 4,718,592 full-viewport masked picks against independent
forward ownership, terrain occlusion, transparency, clipping and actual Qt mouse
events over a synthetic CPU/OpenGL display. It checks declared layers in four
views, invalid masks/metadata, bounds, generation reuse, misses and Step-only
selected-actor orders. `python3 tools/test-scene-picking.py BUILD/world-picking-test
NEW_OUTPUT` retains reports and an exact fresh-process pending-order continuation.

The updated composition executables also run `tools/test-multi-world-scene.py`
with original-manifest verification around installed SPR/TTD reads.
[Evidence boundaries](../research/runtime/native-scene-picking.md) distinguish
native masked diagnostic picking from original rays or installed whole-window
interaction equivalence.

## Native stop/cancel validation (NS15)

`native-world-stop` exercises production Qt buttons and native controller/session
with owned synthetic data: selected-only stable queue cancellation, active fine
motion stop/reset, FIFO restart, stale/cleanup rejection and rollback/budgets.
`python3 tools/test-stop-orders.py BUILD/world-stop-test NEW_OUTPUT` retains
normal/sanitizer evidence and an exact fresh-process pending-v7-stop continuation.
Original input/stop and installed whole-window equivalence remain open.

## Native scene playback (NS16)

`native-world-playback` covers 10,000 irregular virtual-time oracle checks,
bounded catch-up, pause debt reset, real Qt controls/timers, deterministic native
tick batches, pending-order retention, exception/reentrancy handling and exact
checkpoint continuation. `python3 tools/test-scene-playback.py BUILD/world-playback-test
NEW_OUTPUT` retains normal/sanitizer reports and a fresh-process continuation.
Original cadence/pause and installed whole-window equivalence remain open.
