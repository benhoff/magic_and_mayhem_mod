# Broader generated terrain coverage

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone broadens the preceding [generated scene pipeline](native-generated-terrain-scenes.md)
without changing production behavior. It tests all 40 shipped recipes at seeds
zero and `0xffffffff`, two corner cameras, varied span/cut/pan values and selected
views/visibility modes. Exact observed counts and per-case boundaries are in
[the companion report](terrain-generated-coverage.json).

## Reproduction and provenance

```sh
xvfb-run -a python3 tools/test-terrain-generated-coverage.py \
  --source-root SOURCE_SNAPSHOT \
  --preview working/build/terrain-generated-scenes/mnm-terrain-preview \
  --sanitized working/build/terrain-generated-scenes-san/mnm-terrain-preview \
  --preview-report research/runtime/native-generated-terrain-scenes.json
```

The wrapper pins the executable and verifies immutable originals before and
finally after execution. It extends the preceding validated frozen source baseline with a recorded
wrapper overlay. Unrelated newer animation changes are outside this terrain
binary baseline. Existing normal and ASan/UBSan preview binaries are reused only
when their hashes match the preceding validated report and every recorded
C++/header/CMake source from their frozen baseline matches the new source tree.
This includes application, native asset, reconstruction and renderer inputs.
Production services, widgets and original executable instructions are unchanged.
The source/input/helper hashes and the prior report hash remain explicit.

## Corpus and comparison

The independently parsed three realm CFGs define 80 recipe/seed selections.
Seed `0xffffffff` checks accepted unsigned-32-bit CLI input and recovered seed
wraparound; zero remains an explicit supplied seed. Complete available source
MAP payloads are decoded, preserving Specific-then-Random source-entry order.
The existing original generation adapter compares final state and emits concrete
original grid assignments. Failed generation is checked against both previews:
matching exhaustion must report its attempt count/next seed and create no output.

Successful assignments feed the existing independent Python section-copy,
rotation, mixed-height projection and geometry transcription. The unchanged
original geometry pass compares every initialized cell, and native previews
must match the full-cell hash as well as source provenance, projection counts,
dimensions and seed/attempt metadata. This includes cells outside each viewport.

Each successful layout requests two camera configurations:

| Camera | World position | Span | Cut | Pan | Visibility |
| --- | --- | --- | --- | --- | --- |
| Northwest | `(0,0)` | up to 12 | all layers | `(256,64)` | off |
| Southeast | `(width-1,height-1)` | up to 16 | half the layers, minimum one | `(-24,192)` | on |

View is `(region ID + realm index + seed index + 2*camera index) mod 4`.
The two seeds and two cameras request all four views per recipe without claiming
the full camera/view/visibility cross product. The prior smaller corpus separately
checks all views with visibility off/on at the centered camera.

Both native previews execute before the corresponding original traversal. If
both refuse an existing rotated-grid boundary, their exact errors and lack of
output are checked and the original traversal is **not executed**. Accepted
camera cases compare original traversal, producer, sorting and visibility records,
all owner/tile flags, complete queues, independently decoded/composed RGB565 and
RGBA pixels and zero retained OpenGL surfaces. Owners and tiles must have equal
lengths. View-specific layer cuts and clipping policies remain recovered/native
boundaries, rather than new camera behavior.

Native generation bounds, unavailable inputs and matching original exhaustion
are separate report categories. Recognized generation bounds are checked before
the helper maps or executes the original, and must match both previews. Other
errors or mismatches fail validation. Missing Medieval REGION0/Test files remain
explicitly unavailable; no assets are borrowed and original file-error recovery
is not run. Refusal counts are not counted as original equivalence matches.

## Observed results

All **39 available recipes / 78 layouts** matched at both seeds, comprising
**1,702 concrete block assignments** and **17,987,200 initialized cells**.
All **312 frames** (156 per build) matched, including 2,586 hidden draw records;
none were blank. Every available recipe has accepted cases in all four views.
Thirteen layouts needed successful retries, and all 39 maximum-seed layouts
exercised wrapping seed arithmetic with matching final state/seed metadata.

| Realm | Available recipes | Layouts | Initialized cells |
| --- | ---: | ---: | ---: |
| Celtic | 10 | 20 | 3,223,200 |
| Greek | 13 | 26 | 5,784,000 |
| Medieval | 16 | 32 | 8,980,000 |

There were **zero generation failures and zero camera-boundary refusals** for
these selections. Two unavailable recipe/seed cases are Medieval REGION0 at
both seeds; all four corresponding native invocations refused the missing Test
inputs without output. Those cases never entered the original generation or
traversal routines. Original missing-file continuation is not claimed equivalent.
The original geometry helper also passed its existing 2,048 synthetic cases /
387,962 cells. All 2,927 immutable originals verified before and after.

Confidence is high for the recorded accepted cases within these bounded domains.
Pixels remain unshaded embedded-palette composition rather than the original
whole-scene renderer. Full map lifecycle, entities, lighting, water, other scene
producers and live integration remain separate. Broader camera coverage may
expose original boundary behavior; preserve evidence before changing that policy.
