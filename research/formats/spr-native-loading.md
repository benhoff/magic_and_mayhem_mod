# Native version-4 SPR loading

Rendering asset chunk 2, reviewed 2026-10-04. Implements a bounded native
decoder and `AssetFile` loader for indexed and direct-colour SPRs. Validation
is offline and includes selected isolated original draw/conversion routines;
there is no native renderer connection or live loader replacement in this chunk.

## Interface and ownership

[Public API](../../assets/sprite_loader.hpp): `decodeSprite(bytes, limits)` and
`loadSprite(AssetFile&, limits)` return a `SpriteResult` containing either a
fully owned `Sprite` or `SpriteError`. No Qt types, original addresses or
borrowed input pointers cross this API. `loadSprite` reads from zero through
the existing bounded `readWhole` helper; callers retain ownership of the file.
Output survives input vector mutation and destruction of the file/store/tree.

The result contains:

- `SpriteStorage::indexed8`: owned byte indices, up to four owned RGB palettes,
  and each frame's valid embedded palette index.
- `SpriteStorage::rgb565`: owned `uint16_t` colour words and no palette/index;
  source words are explicitly decoded from little-endian bytes.
- Each frame's owned top-down `opaqueMask`, dimensions, signed origins, exact
  eight name bytes, encoded extent, two raw auxiliary offsets and owned opaque
  trailing-plane byte arrays.
- Source version/byte count and the opaque header word at +20 (`headerFlags`);
  the name does not imply its flag meanings have been recovered.

There is one pixel and mask byte per logical pixel, without row padding.
Mask 0 means transparent and the associated output pixel slot is zero. Mask 1
means opaque, including index/colour word zero. Consumers must use the mask;
zero is not a universal colour key. Empty installed records retain both zero
dimensions and metadata with empty typed pixel/mask vectors.

No shading tables, RGB555 conversion, blending, frame placement, animation
timing, SFT font interpretation or auxiliary-plane effects are performed.
Auxiliary bytes are retained without interpreting their effects. Each nonzero
offset owns bytes through the next strictly greater auxiliary offset, or frame
end. Aliases own independent buffers. These bytes count toward `decodedBytes`;
`scannedBytes` continues to bound repeated frame processing. Selected visibility
interpretation is separate in [SPR visibility planes](spr-visibility-planes.md).

## Format scope and validation

The implementation follows the selected original version-4 layout:

| Position | Interpretation within this scope |
| --- | --- |
| File +0 | `SPR\0` signature |
| +4 / +8 / +12 / +16 / +20 | Declared file bytes, version 4, frame count, palette count, opaque header word |
| +24 | 256 RGB triples per embedded palette, followed by frame-offset DWORD table |
| Frame data base | End of the palette/offset table; stored offsets are relative to this base |
| Frame +0 / +4 / +8 | Encoded bytes, width, height |
| +12 / +16 / +20 | Signed X/Y origins and eight raw name bytes |
| +28 | Embedded palette index, or -1 for direct-colour storage |
| +32 / +36 | Auxiliary offsets used to bound the main pixel plane |
| +40 | One delta/pixel offset pair per row, relative to the frame |

The offset table is authoritative. Reordering and identical aliases are
supported; each decoded entry owns independent buffers. Partial overlaps are
rejected. Frame headers, row tables, delta ranges and pixel ranges must stay
inside their containing data. Palettes are validated before allocation.

Each row alternates transparent/coloured runs beginning transparent. Run sums
must equal frame width. Indexed runs consume one byte per opaque pixel; direct
runs consume two. Control ranges precede the pixel plane, row offsets are
monotonic within their respective planes, and coloured bytes cannot cross a
row's pixel extent. Padding between pixel rows/planes is allowed: installed
files align the last row before auxiliary data. Trailing planes remain opaque owned data in this decoder.

Both dimensions must be positive or both zero; mixed zero/nonzero dimensions
are rejected as a native safety policy, whose original handling is unverified.
Version 2 returns `unsupportedVersion`, matching the supported scope of the
inspected original loaders; another legacy original path remains possible.
SFT/ANI and other signatures return `invalidFormat`.

Default limits:

| Budget | Limit |
| --- | ---: |
| Input bytes | 32 MiB |
| Frame entries | 4,096 |
| Width / height | 2,048 each |
| Aggregate decoded pixels | 16,777,216 |
| Pixel, mask and auxiliary storage | 48 MiB |
| Sum of processed frame extents | 32 MiB |

Limits are explicit caller policies, not recovered engine limits. Frame extent
charges include aliases to bound repeated compressed work. Size arithmetic is
checked against containing extents, cumulative budgets and host allocation
range before output allocation. Palettes, frame metadata, temporary extent
tables and the bounded input copy are additional memory; the decoded byte
budget counts pixel/mask and auxiliary arrays. Allocation/length failures return a limit
error. Actual memory exhaustion was not induced.

`SpriteError` records category, byte offset, optional frame index and detail.
Input failures retain the original structured `AssetFile` error, including its
operation and requested/resolved paths. Errors return no partially decoded
sprite. As with the asset interface, ordinary loading assumes stable files;
the experiment runner independently verifies hashes before/after.

These validations intentionally tighten malformed-input acceptance rather
than reproducing unsafe original reads. They are not claims of original failure
or diagnostic equivalence.

## Reproducers

Fixture-only build and tests (no game, Wine, display or MMSprite needed):

```bash
cmake -S assets -B working/build/assets
cmake --build working/build/assets --parallel 4
ctest --test-dir working/build/assets --output-on-failure
```

One frame through mixed-case Windows asset paths:

```bash
working/build/assets/mnm-sprite-inspect --root working/game-clean \
  --prefix C:/MagicMayhem --path 'c:\MagicMayhem\cReAtUrEs\rEdCaP.sPr' --frame 0
```

The CLI closes each file before inspecting owned output. It writes JSON to
stdout only: container summary, embedded palettes, selected metadata and
canonical little-endian pixel/mask hex and hashes. `--manifest` accepts an array
of `{ "path": "...", "frames": [0, 1] }` instead of `--path`; an empty frame
list still decodes/validates the whole file. `--frame` is repeatable for one path.
Manifest limits are 4 MiB and 4,096 entries; selected output is capped at 1,024
samples/16 MiB of pixel+mask bytes. Exit 0 means decoded, 1 means per-asset
errors, 2 means invalid invocation/manifest/sample selection. The CLI does not
run original-manifest verification; use the runner for installed experiments.

Installed corpus and isolated original-code comparison:

```bash
python3 tools/compare-mmsprite-binary.py \
  --native-inspector working/build/assets/mnm-sprite-inspect
```

This additionally requires the pinned external reader from the
[MMSprite evaluation](mmsprite-evaluation.md), both hash-pinned executable
builds, and ELF i386 execution/`g++ -m32`. Original routines run in isolated
private mappings with bounded process deadlines. The clean build contributes
static evidence; the No-CD build supplies executed original routines. Details
and limitations are in the [binary comparison](../runtime/sprite-binary-comparison.md).

## Recorded validation

[Retained native evidence](spr-native-loading.json) records loader/header/
inspector hashes, input hashes, selected native pixel/mask hashes and original
output hashes. Payload hex remains in ignored `working/`, rather than committing
decoded game assets. Original-manifest logs verify 2,927 originals before/after.
The recorded run is `working/tests/sprite-binary/run-fmp2ogqy/`; its
`original-before.log` and `original-after.log` retain those checks.

The native loader decoded all **174 installed version-4 SPR files**:
**59,407 frames**, **2,297 empty frames**, **129,297,163 decoded pixels**. The
seven installed version-2 SPRs were rejected explicitly. All embedded palette
bytes and sampled metadata were compared with independent raw file reads.
Alternating requests exercised mixed-case relative Windows paths and explicit
`C:/MagicMayhem` aliases.

All **91 selected native draw destinations** matched unmodified original
No-CD drawing routines, including empty records, all 27 palette-free files,
RedCap, effects3, and all four palettes in the selected terrain file. All
**77 native direct-colour samples** also matched the original RGB565-to-RGB555
conversion/draw results. The indexed oracle uses a supplied unshaded RGB565
palette; original palette construction, shading/lighting variants, clipping
scenarios and complete live loading were not tested.

The four assets CTests pass. The new fixture test covers owned indexed/word
buffers, opaque zero, signed origins/raw names, auxiliary metadata/padding,
empty records, reordered/aliased offsets, partial overlaps, malformed headers,
counts, dimensions, palette/row/run extents, cumulative limits, short reads,
seek/read errors and premature EOF. It also passed AddressSanitizer and
UndefinedBehaviorSanitizer; leak checking was disabled because LeakSanitizer
cannot operate under this environment's ptrace restriction. This is not a
leak-profile claim.

Confidence is high for supported parsing, ownership/limit policies, complete
installed decode success and the selected original draw/conversion agreement.
Whole-file decode success does not establish original-equivalent drawing of
every frame/effect. Rendering integration, original palette/shading construction,
SFT and ANI remain separate milestones. The next chunk should display one
verified indexed and/or direct-colour frame through the native renderer while
preserving mask and origin semantics.
