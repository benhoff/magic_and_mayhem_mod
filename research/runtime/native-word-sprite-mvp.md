# Scoped direct-word sprite rendering MVP

The MVP replaces the two selected word backends in the pinned No-CD sprite renderer, retaining
the original simulation, presentation, clipping paths and auxiliary passes.
This is a native CPU raster route, not a complete native World frame or GPU
scene takeover. Live shadow and takeover runs reached World; all eight retained
takeover canvases and workspace states match independent original execution.

## Recovered boundary

Build SHA-256 is `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The cdecl backend entries `0x596cb8` and `0x597086` both begin with the six
complete instruction bytes `55 8b ec 56 57 53`. Preparation checks the executable
hash and both signatures. Installation checks signatures again, builds separate
trampolines, and redirects only those entry bytes. Refused requests execute the
unchanged bodies through their matching trampolines. Caller-side dispatch and
auxiliary passes remain active. Slot `0x6903dc` selects these backends in the
inspected configuration setter; other backend implementations remain unchanged.
This does not replace indexed dispatch or complete scene/terrain/effect drawing.

Both selected backends subtract signed frame origins from caller argument slots,
write native WORD colours from alternating skip/opaque byte runs, preserve
integer registers other than EAX, and return zero in the admitted branch. Their
unclipped branch uses contiguous control/pixel streams starting at the first row
offset pair; subsequent offset pairs must describe that same stream to be admitted
natively. The 16-word scratch range at `0x5f1e50..0x5f1e90` has selected writes;
the adapter mirrors them, retaining untouched words. Backend `0x597086` also
writes its last alignment-adjusted opaque run length at `0x5f1e7c`; `0x596cb8`
leaves that word untouched. The bounded branch has original execution comparisons; other branches stay outside scope.
These global addresses are bound to the pinned preferred-base build, not stable
heap addresses. Each destination and frame is revalidated on each call.

## Native admission and modes

`renderer/sprites/word_raster.c` is platform-independent C with no Qt or legacy
addresses. It validates the entire bounded direct-word frame before writing any
destination byte, retains opaque zero and borrowed destination padding, and
refuses malformed streams, aliased frame/canvas storage and clipped/empty/indexed frames. Admission intentionally
excludes exact right/bottom edges to retain the original backend's branch selection.
This safety policy does not reproduce original malformed-input handling.

The standalone `renderer/sprites/word-raster` CMake project builds the portable
library and its host test without Qt, Wine or original media.

`runtime/scene/word_route.c` owns build-specific globals, scratch-state translation,
Win32 access checks, the entry hooks and diagnostics. `shadow` runs native
pixels in independent scratch storage, forwards the original draw, and compares
all destination bytes and workspace words. Any mismatch stops further admission.
`takeover` writes the admitted draw directly and returns without calling the
original backend. Refused routes tail-call the current original slot before
native writes. The assembly boundary saves integer registers, flags, x87/SSE
values and LastError; handled draws retain original argument-slot mutation and
the defined zero-return flags. Exact x87 instruction/data pointer history is
outside this contract. The hook is process-lifetime and supports the engine's
single drawing thread; concurrent original raster calls remain unvalidated.

The first eight admitted takeover calls retain complete before/after canvas,
encoded input and workspace records for independent original execution. Native
rendering consumes the encoded frame and before pixels only. Expected original
outputs are never supplied to it. Statistics distinguish seen, admitted, bypassed,
originally executed, compared, mismatched, refused, sampled and failed captures.
Fallback and successful partial replacement are separate recorded outcomes.

## Reproduce

```sh
python3 tools/test-word-sprites.py
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/capture-scene-game.py --word-sprites shadow
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/capture-scene-game.py --word-sprites takeover
python3 tools/test-word-sprites.py --live-directory PATH_TO_EXPERIMENT/word-sprites
```

Experiments stage only disposable working installations. Hash and entry-byte
checks precede import staging and hook installation. Original manifest verification
surrounds installed-artifact tests, including failure. An i386-capable execution
host, Clang/LLVM PE tools, Wine and Xvfb are required.

The ordinary Qt shell may select this partial route explicitly using
`MNM_WORD_SPRITES=takeover ./tools/run-qt-shell.sh --frame-readback --skip-movies`.
The render experiment manifest records its mode, DLL hash and diagnostic directory.
`shadow` selects comparison instead of bypass. The remainder of the displayed
frame is still original drawing. Continuous command presentation has its existing
independent capture/recovery limitations; the frame-readback command avoids claiming
that this raster milestone validates them.

## Remaining boundaries

Clipping and empty frames deliberately fall back. Other call sites, shading,
indexed sprites, complete resources/camera/scene queues, effects, terrain,
HUD/text/cursor, movies, surface loss/recovery, sustained gameplay and physical
driver validation remain separate. Diagnostics copy canvases and write files;
this experiment supplies correctness evidence, not a performance improvement.
The authoritative [drawing replacement plan](../../docs/live-drawing-replacement.md)
continues to own complete-session requirements.

The first dispatch-site-only experiment in `working/experiments/scene-observer/run-drl_ai6_`
installed successfully but saw zero direct-word calls; all 246 unique frames in
its first World queue were indexed. It establishes no bypass. The subsequent
backend-entry shadow run `run-_o268pke` recorded 12,608 admitted original/native
comparisons, zero pixel/workspace mismatches and 940 forwarded refusals. The
backend-entry takeover run `run-i_suvlzy` recorded at least 12,608 bypassed calls,
705 forwarded refusals and eight complete before/after samples, with zero capture
errors. These counters are finite observations through battle startup, not
indefinite gameplay or complete World-frame equivalence.

Fresh retained comparison evidence is
[native-word-sprite-takeover-20261007.json](native-word-sprite-takeover-20261007.json):
256 synthetic original matches / 119,288 canvas pixels, 647 intentional native
refusals, one 64-bit portable CTest, and eight live takeover samples / 1,070,400
complete canvas pixels with exact workspace matches. The separate
[shadow record](native-word-sprite-shadow-20261007.json) retains the live pairs;
[launcher staging](native-word-sprite-launch-staging-20261007.json) records the
actual opt-in Qt staging entry point, without claiming GUI Launch-button execution.
These records advance `RS.word-raster`, `RS.word-route` and `NR.word-admission`
only within their registered scopes. `RS.word-route` is the first recorded scoped
live raster replacement; complete scene/World takeover remains unavailable.

The isolated runner's first takeover comparison stopped during report construction
on a relative-path error after its first pixel/workspace match. That failed report
remains at `working/tests/word-sprites/run-i7ekt_ub/report.json`; the corrected fresh
run `run-7_mgbb3s` completes all eight comparisons. No historical hashes were changed.
Assembly register/flag/floating exception-state stress, full Qt session/shutdown,
loss/recovery, long play and physical-driver tests remain pending beyond the finite
successful startup/caller-operation evidence.
