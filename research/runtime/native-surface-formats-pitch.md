# Native surface formats and row pitch

## Required format boundary

The retained game drawing captures establish RGB565, including five original
800×600 opaque BltFast outputs at return PC `0x58c5be`. This chunk adds scoped
native format compatibility; it does not infer that every native format occurs
in gameplay. Build `nocd-40209ca7` has source SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

Static review of the previously exported render-support instructions confirms:

- `0x58a9b0` forwards caller depth to fullscreen display setup. At `0x58aa63`
  (`ba 08 00 00 00`) it compares that depth with 8 and records an indexed/nonindexed
  branch through `0x6f8174`. This establishes a setup branch, not gameplay format
  execution coverage.
- `0x58ad90` passes a supplied descriptor to DirectDraw2 CreateSurface and queries
  Surface2. Its later accounting uses `height * width << 1` at
  `0x58b241..0x58b24b` and initializes a WORD key at object `+0x2c`. These selected
  wrapper assumptions must stay separate from a general native 32-bit key API.
- `0x58b660` calls Surface2 Lock; `0x58b6b0` reads returned pitch from `0x6f68f0`
  and stores it at object `+0x18`. A driver stride must not be replaced by width
  times pixel size in reconstructed CPU access.

The retained export manifest pins fullscreen, creation and Lock instruction
artifacts, respectively, to `9c0acd1b1825162026af8aa522c240759306a1d581f3da61df16738679dc3348`,
`437d018eda9df2c2e4e3132de79c99c26b3bcd19c0852b2553180ec840bd1b51` and
`df1b13e535b2ac9509e62bf328e7ae63c5b0d167fa806106efd595f0a82373e0`.
See [drawing inventory](render-drawing-inventory.md). This is static review,
not new original execution or a complete caller/descriptor census. Confidence
is high for the named instructions and retained RGB565 captures; required
format reachability beyond those observations remains a final acceptance task.

## Independent driver observations

[Final capture](surface-formats-final-capture-20261006.json) runs a standalone
DirectDraw2/Surface2 probe with built-in Wine, never original game instructions.
Immutable-input manifests pass before/after; the capture reservation serializes
Wine sessions. The report pins the probe, collector, input/output binaries,
fixture and actual driver DLL. Earlier [initial](surface-formats-capture-20261006.json)
and [DirectDraw2](surface-formats-dd2-capture-20261006.json) captures remain
historical with their original hashes. The latter's collector failed only its
relative-path report write; the report was reconstructed from unchanged sources
and retained completed artifacts before correcting that collector. Neither old
report is current-code validation.

The 3,500-case matrix uses 7×5 surfaces, five formats, five requested memory
layouts, seven fill/copy/self-copy families, five clip states and four key states.
All direct `DDSD_LPSURFACE` creation requests fail with `E_INVALIDARG`. A separately
recorded fallback allocates a normal surface and calls Surface3 SetSurfaceDesc
with explicit caller memory, pitch, width and height. Its results are retained
independently of creation results; this API is not asserted to occur in the game.

| Native format | RGB masks | Allocated pitch | Admitted caller binding |
| --- | --- | ---: | --- |
| Indexed8 | none | 8 | padded +12 |
| RGB555 | 7c00 / 03e0 / 001f | 16 | padded +20 |
| RGB565 | f800 / 07e0 / 001f | 16 | padded +20 |
| RGB24 | ff0000 / 00ff00 / 0000ff | 24 | none of tested layouts |
| RGB32 | ff0000 / 00ff00 / 0000ff | 32 | padded +32 and tight +28 |

Negative pitches, tight indexed/16/24 pitches, and the sampled RGB24 padded
binding are rejected. There are **1,400 admitted draws and 2,100 rejected setups**.
No outputs are manufactured for rejected setups. The driver caller buffer
contains explicit guard/padding bytes; successful Lock pointers match the input
storage. All native words, stable descriptor fields, key outputs, HRESULTs and
admitted caller storage are retained. High confidence applies within this exact
Wine driver/matrix scope; Windows/display hardware and arbitrary mask/layout
support are not established.

The input-derived [offline oracle](surface-formats-offline-20261006.json) checks
526,400 words and includes 13 completeness/mutation tests. Its expected output
is computed independently from inputs; captured after pixels are never fed into
the native renderer. Error outputs are checked along with successful pixels.

Measured branches:

- Fill stores the indexed depth or union of active RGB masks. `0xd3e2f197` becomes
  `0x97`, `0x7197`, `0xf197`, `0xe2f197`, `0xe2f197` in the five formats.
- Keyed copies compare `(pixel & activeMask) == installedKey`. An installed RGB555
  high bit or RGB32 high byte is retained in key metadata and prevents a match.
  A low installed key matches pixels with differing unused bits. Opaque and
  successful keyed writes retain the whole native word, including unused bits.
- Duplicate palette RGB entries at indexed values `0x81` and `0x82` do not make
  both indices transparent. The operation compares native indices.
- Key mutation appears in Lock descriptor flags (`0x10000`) and both key words.
  Missing-key GetColorKey fails and preserves the output range sentinel.
- Valid idle missing-key Blt rejects with `E_INVALIDARG`, including empty and
  missing clip lists. BltFast rejects any attached clipper first; with no clipper
  its missing key follows the existing measured opaque path.
- Keyed self-overlap reads preceding writes in row-major order. Opaque self-copy
  freezes each clip piece. Clipping and complete unchanged error outputs match.

## Owned byte-row implementation

`PixelRows` owns bytes plus dimensions, signed pitch and the logical top-row
offset. Little-endian decoding/packing supports the five canonical formats.
Only active pixel bytes are touched: row padding and leading/trailing guards
survive. Complete layout and word validation precede writes. Dimensions are
bounded to 2048, storage to 64 MiB and absolute pitch to 32768. Widened arithmetic
rejects INT_MIN pitches, SIZE_MAX offsets, truncated rows and escaping negative
row addresses. Arbitrary masks, cross-format conversion and stretching refuse.

`SurfaceBackend` now imports/updates/reads/writes leased byte rows, accepts all
five formats for same-format fill/copy/keys and uses full native-width key state.
Imported bytes are copied into owned GPU storage; there is no borrowed caller
pointer. Descriptor row pitch follows imported storage metadata through two-buffer
flips, while key/mask/object metadata stays associated with the surface. Default
owned pitch rounds to eight bytes, a native allocation policy matching the
sampled allocated 7-wide and prior 8-wide descriptors. Lock still returns an
opaque token and a logical CPU snapshot, not a real pitched driver pointer.

Signed and tight byte imports are intentional native capability even when that
layout is rejected by the sampled driver. Four additional layouts replay every
allocated draw against the same independent logical outputs. The
[native comparison](native-surface-formats-20261006.json) checks **1,400 driver
cases + 2,800 signed-row variants**, 667,800 native words/descriptor/presentation
values and 2,150,400 packed bytes. Presentation conversion is an explicit native
policy; this capture does not compare new GDI conversion behavior. Sixty native
refusals check malformed layouts/masks, leased updates, unknown restored content,
key overflow and incompatible same-depth formats. CPU writes, alias retirement,
reload and flips with different signed pitches also have native guard checks.

[Draw regression](native-surface-formats-backend-regression-20261006.json) freshly
compares 5,114 existing results and 538,272 pixel values, with 258,048 palette
entries/8,664 stable descriptors. [Ownership/recovery regression](native-surface-formats-ownership-regression-20261006.json)
compares 11 driver flips, retained loss/palette states, 4,480 original retry traces
and 1,536 bounded stops. The original retry trace evidence is separate from native
physical loss policy. [Final regression record](native-surface-formats-final-regressions-20261006.json)
records all 16 renderer CTests passing on committed `fc65636` plus the format
changes, across the suite and serial rechecks after parallel Xvfb cleanup errors,
plus 13 oracle tests and coverage audit/gate suites. The earlier regression
record remains historical. The intervening commit declared a compat census root
before committing its scanner update; the narrow scanner/audit/legacy-root test
updates are included to make the staged gate reproducible. Shared uncommitted application/resource work
is excluded from the isolated tested tree.

```sh
cmake -S renderer -B working/build/renderer -DBUILD_TESTING=ON
cmake --build working/build/renderer --parallel 4
python3 tests/test-surface-formats.py
xvfb-run -a python3 tools/check-native-surface-formats.py --build working/build/renderer --report working/new-format-replay.json
```

Replay needs Qt/OpenGL/Xvfb but no Wine, game installation or original media.
Choose a fresh report path. Remaining acceptance work includes the required
caller/descriptor census and resolving or explicitly excluding remaining palette
flags, uncaptured leases/recovery/asset branches and conditional complex effects.
RGB555 DC conversion, physical original multi-format execution, original negative
pitch, arbitrary masks, cross-format conversion, live/wire replacement and Windows
behavior are not claimed. Chunk 3 closes the bounded native formats/pitch work;
it does not close the complete surface milestone.
