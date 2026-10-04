# Native JPEG loading

Reviewed 2026-10-04. Scope: a reusable owned RGB asset reader using Qt's JPEG
codec, plus an offline inspector. This does not implement JPEG decompression
from scratch or replace the existing menu/renderer image paths.

## Installed evidence

All 752 `.jpg` files under `working/game-clean` have JPEG start/end markers
and total 56,398,896 source bytes. Pillow reports RGB images, 741 baseline
and 11 progressive. ICC profiles occur in 386 baseline and six progressive
files; all observed EXIF orientations are absent/default (one). The largest
installed dimensions are 800x600. Total decoded pixels: 126,954,548.

All installed RGB bytes and dimensions match Pillow 12.3.0 through complete
pixel hashes. Confidence is high for this corpus and the tested native policies.
Qt and Pillow are separately invoked wrappers and can share underlying libjpeg
algorithms; this is not evidence of two independently reconstructed JPEG codecs.
Original game decoding, rounding and caller behavior remain unverified.

## API and implementation boundaries

[assets/jpeg.hpp](../../assets/jpeg.hpp) exports `JpegImage`, `JpegLimits`,
`JpegResult`, `decodeJpeg(bytes)` and `loadJpeg(file)`. Link `mnm-jpeg-loader`.
Public types use standard-library owned storage and contain no Qt objects.
The implementation privately links Qt Gui for QImage/QImageReader, without
widgets, QPixmap, original addresses or runtime hooks. Read-only input uses
the existing AssetStore/AssetFile backend. The inspector and native tests use
QCoreApplication; no GUI application or display server is required.

A result owns tightly packed, top-down RGB bytes, width/height and source-byte
count. It survives the input buffer, file handle and codec image. Grayscale
is expanded to RGB. No alpha or transparency policy is inferred.

[QImageReader documentation](https://doc.qt.io/qt-6/qimagereader.html)
describes explicit format selection, size queries, transformation control,
codec errors and its process-wide allocation limit. The implementation fixes
the codec to JPEG, disables autodetection and automatic metadata transforms,
and checks dimensions before reading pixels. It leaves the process allocation
limit intact. The [QImage RGB888 format](https://doc.qt.io/qt-6/qimage.html#Format-enum)
is used for conversion; each visible row is copied without codec row padding.

Native policies, rather than recovered original error behavior:

- Require `FF D8` at the start and `FF D9` at the end. Truncated framing and
  trailing bytes after EOI are rejected. All installed files meet this rule.
  This is a framing check, not full entropy-stream verification: codec recovery
  of internally damaged images remains Qt/libjpeg behavior.
- Report codecUnavailable if Qt's JPEG handler is absent. Availability relies
  on the runtime codec installation. Other image formats are not autodetected.
- Decode full-resolution pixels in encoded orientation. Do not apply EXIF
  rotation/mirroring, ICC conversion, resizing or application color management.
  ICC metadata is not exported or applied. This matches the existing menu
  load policy and the comparison reference; no original color policy is claimed.
- Supported/evidenced cases: installed RGB baseline/progressive and synthetic
  grayscale baseline/progressive. Other variants follow Qt's available codec,
  without a compatibility claim for CMYK, unusual precision or corrupted data.
- Defaults: 64 MiB input, 8192x8192 dimensions, 16,777,216 pixels, 48 MiB owned
  RGB and 64 MiB per codec image. Before decoding, budget four bytes per pixel
  conservatively; after decoding/conversion, check actual codec image extents.
  Compute products in 64 bits and check output container capacity.
- Budgets bound input, exported RGB and each codec pixel image. They do not
  bound total process memory or libjpeg coefficient/scratch allocations.
  Source, decoded image, converted image and RGB output may coexist. Qt's
  allocation guard is an additional constraint and is not mutated per call.
- Errors retain a code/detail and, for AssetFile failures, the backend Error.
  Codec failures return malformedData with the codec detail, including when
  Qt itself rejects an otherwise excessive image before reporting dimensions.
  Signed-read-limit overflow is invalidArgument; C++ allocation/length failures
  become limit errors. No partial exported asset is returned.

## Reproduction and validation

Build dependencies: Qt 6.8 or newer Core/Gui and an available JPEG handler.
Comparison tests additionally require Python Pillow; CMake checks the import
when BUILD_TESTING is enabled. The tested Qt runtime is 6.11.2.

```bash
cmake -S assets -B working/build/jpeg -DBUILD_TESTING=ON
cmake --build working/build/jpeg --parallel 4
ctest --test-dir working/build/jpeg --output-on-failure
python3 tests/test-jpeg-loader.py working/build/jpeg/mnm-jpeg-inspect \
    --installation working/game-clean \
    --report working/tests/jpeg-loader/installed-comparison.json
```

`mnm-jpeg-inspect ROOT PATH.jpg` closes its AssetFile before emitting JSON
with dimensions, source-byte count and RGB SHA-256. It exits with status 2 on
failure. Case folding and containment use the existing AssetStore resolver.

Native fixtures verify owned RGB output, padding removal, every truncation of
a valid file, framing errors, malformed marker-only input, dimension/output
limits and a 60,000x60,000 SOF mutation rejected before pixel allocation.
Eight independent Pillow-generated fixtures cover RGB/grayscale,
baseline/progressive and EXIF orientations one/six; references deliberately
omit transpose and ICC conversion. Invalid/missing files and parent traversal
must fail. The optional installed comparison verifies the immutable original
manifest before/after, including on failure, and records per-file source hashes.
Installed extracted inputs and immutable source preservation are separate checks.

All 17 asset CTests pass. Both JPEG tests pass under AddressSanitizer and
UndefinedBehaviorSanitizer with leak detection disabled because tracing prevents
LeakSanitizer here. Qt/libjpeg dependencies themselves are prebuilt and not
sanitizer-instrumented by this build. The comparison report lives at
`working/tests/jpeg-loader/installed-comparison.json`. All 2,927 immutable
originals verify. The ordered records, encoded as JSON with sorted keys and
compact separators, have SHA-256
`6844e4823929a1c220edaf0a8bb0293a48ffa1a7e03484cb7df337f7ecd2f5d8`. The tested native inspector SHA-256 is
`23410aea22225542c35b0b145e8a5980c98cb174b016985f5d5c551dbe25bca1` (GCC 16.2.1, C++17, Qt 6.11.2).

Remaining milestones: original decoder/caller equivalence, placement and color
policy, renderer upload, migration of menu callers and live visual comparison.
See the [coverage ledger](../runtime/coverage-ledger.md).
