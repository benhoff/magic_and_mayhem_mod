# Surface2 driver clipping and error outputs

2026-10-06. The [final capture](surface-driver-clipping-surface2-20261006.json)
records 408 independent calls to Wine 11.16's built-in DirectDraw driver through
`IDirectDrawSurface2`, the interface documented for the original rectangle
wrappers in [the drawing inventory](render-drawing-inventory.md). This is a
bounded **external driver baseline**, not recovered Windows 1998 driver behavior
or execution of the original game's failure/retry branches. Confidence is high
within the recorded environment, interface, format, geometry and flags.

## Independent capture

The PE32 probe creates distinct 8x6 RGB565 offscreen system-memory surfaces
(`DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY`, masks `f800/07e0/001f`) under
normal cooperative mode. It initializes the source with `0x1000 + 73*i` and the
destination with `0xa000 + i`, captures both surfaces before the real COM call,
and locks them again afterward. Expected pixels or HRESULTs never enter the
probe. The native renderer is not linked or executed. Each record retains API
inputs, post-call rectangle bytes, HRESULT, full before/after native words,
pitch/format/caps descriptions and the clip list read back from the driver.
Pointers in descriptions are normalized to zero; pitches remain recorded.

Calls exercise Blt/BltFast, WAIT on/off, bounds on every edge and one corner,
complete exclusion, negative/excess source coordinates, empty/reversed rectangles,
no clipper, one region, two disjoint regions, an empty region, and an attached
clipper with no list. Four calls hold a source/destination lock with WAIT off;
additional calls select a missing source key or an unsupported flag. There is no
stretch, overlap, surface loss or forced restoration in this experiment.

The retained driver DLL hash is
`24c8c8fbbe0bac100efa5a716ebc78506e0f42e80d2af019b641911c0847ab28`.
`WINEDLLOVERRIDES=ddraw=b` forces that built-in implementation in a copied prefix.
The report also pins the probe, input/output binaries and current collector/test
sources. The original manifest verifies all 2,927 immutable files before/after;
no game executable or original instruction is patched or executed.

## Confirmed results

| HRESULT | Count | Recorded meaning |
| --- | ---: | --- |
| `00000000` | 96 | Success, including 64 calls with no changed pixels |
| `88760096` | 210 | `DDERR_INVALIDRECT`; **two calls partially changed the destination** |
| `887600cd` | 32 | `DDERR_NOCLIPLIST` |
| `887601ae` | 4 | `DDERR_SURFACEBUSY` for the held-lock cases |
| `8876023e` | 64 | `DDERR_BLTFASTCANTCLIP` |
| `80070057` | 1 | `E_INVALIDARG` for the tested Blt source-key call without a key |
| `80004001` | 1 | `E_NOTIMPL` for the tested Blt unsupported flag |

Without a clipper, both APIs reject the tested out-of-bounds rectangles instead
of cropping them. With an explicit clip list, Blt intersects the destination with
each region and translates the corresponding source coordinates. An entirely
excluded destination returns success without writes. An empty list behaves as
full exclusion for the tested positive-size Blt inputs; a missing list returns
`DDERR_NOCLIPLIST`. Empty/reversed Blt rectangle admission precedes that missing
list error in these cases. This is not a claim about all error combinations.

Source bounds apply to the resulting pieces rather than invariably to the
original source rectangle. For example, source `(-2,0,6,6)` to destination
`(0,0,8,6)` succeeds with region `(2,1,6,5)`, which removes the invalid source
columns. Conversely, source `(2,0,10,6)` with regions `(0,0,3,2)` and `(5,3,8,6)`
writes six pixels in the first region, then fails with `DDERR_INVALIDRECT` on the
second. Cases 194/195 capture this partial destination mutation with WAIT off/on.
All source pixels and supplied rectangle bytes remain unchanged in all 408 calls.

BltFast does not consume the clip list. Its observed destination admission comes
before clipper rejection; an otherwise admitted destination with an attached
clipper reports `DDERR_BLTFASTCANTCLIP`, including tested invalid/empty source
rectangles. Source validation follows for the no-clipper cases. The two tested
BltFast missing-key/unsupported-flag calls succeed and copy opaque pixels; do not
turn this Wine-specific result into a portable API guarantee. WAIT does not change
any paired HRESULT or pixel output in the captured cases.

## Offline verification and reproduction

Four offline tests verify all 408 HRESULTs and full destinations against a
separate Python model, fixture hashes, deterministic initial pixels, source/input
preservation, returned region metadata and WAIT pairs. Mutation tests reject a
wrong full-surface copy where disjoint clipping is required and an incorrect
success result. The model handles partial writes on a later-region failure;
it is not used by the capture probe or native renderer.

```sh
python3 -B tests/test-surface-driver.py
```

A fresh driver capture requires Wine, Xvfb, Clang and LLVM, plus the working Wine
prefix. Select new report and corpus names; the tool refuses overwrites:

```sh
xvfb-run -a python3 tools/capture-surface-driver.py \
  --report working/tests/surface-driver-new.json \
  --fixture-dir working/tests/surface-driver-new-corpus \
  --stem surface-driver-new
```

The first sandboxed attempt failed at Wine socket creation; its before/after
manifest checks and logs are retained in `working/tests/surface-driver/run-1kyiiymt`.
Successful execution needed local sockets outside that sandbox. Two preceding
Surface1 captures are retained separately as
[simple boundary observations](surface-driver-clipping-wine-20261006.json) and
[expanded error observations](surface-driver-clipping-wine-v2-20261006.json),
with their original fixture bytes and historical source fingerprints. They are
not current-source validation records. The final Surface2 run independently
repeats the expanded cases; no expected output is synthesized from those pilots.

## Remaining boundary

The operation matrix now requires before/after state for failures instead of
assuming failures are atomic. Native clipping and clipper wire representation
remain unimplemented. These synthetic API calls do not show which regions or
invalid geometries the real game produces, original wrapper reaction to these
errors, retry termination, Restore/loss semantics, primary/video-memory surfaces,
HWND-derived clip lists, region order permutations, stretch, keys/fill interactions,
other formats or drivers. The game wrappers' successful-call argument evidence
remains a separate milestone. No original pixel-equivalence, native clipping
integration or live replacement status is promoted by this external-driver corpus.
