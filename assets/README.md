# Native asset access

Chunks 2 and 3 implement path resolution and owned read-only file handles for
installed loose assets using Qt Core and C++17. The public APIs in
`path_resolver.hpp` and `asset_file.hpp` use standard C++ types, allowing native
loaders to consume them without Qt types entering reconstructed algorithms.

The complete six-chunk read-only asset milestone is implemented. Run the
combined workflow from the repository root:

```bash
python3 tools/validate-native-assets.py
python3 tools/validate-native-assets.py --fixtures-only
```

The default runs raw installed-file comparison, decoded-WAV comparison, and
the Windows file API audit, retaining a summary and step logs under
`working/tests/native-assets/run-*/`. Each artifact-consuming runner verifies
originals before/after, including failure paths. It assumes existing clean and
No-CD working installations and the recorded original manifest; it does not
install, launch, or modify the game. The audit additionally requires objdump.
Fixture-only mode needs no game/original files and builds the combined audio
suite plus synthetic audit tests. A failing step stops the workflow and records
its exit status/log; a complete report requires every step to pass.

```bash
cmake -S assets -B working/build/assets
cmake --build working/build/assets --parallel 4
ctest --test-dir working/build/assets --output-on-failure
```

Requires Qt 6.8+ Core, CMake, and a C++17 compiler; tests/validation also require
Python 3. No Wine, display server,
application event loop, game installation, or original artifacts are needed for
the fixture tests. Test fixtures use QTemporaryDir and are removed on exit.

Call `PathResolver::create(installationRoot, windowsPrefixes)` and inspect its
`Result<PathResolver>` variant. Prefixes such as `C:\\MagicMayhem` explicitly
map that installation to the configured host root. `resolve(request)` returns
`Result<ResolvedAsset>` containing both the matched host spelling and canonical
target, or a structured `Error`. Paths do not grant access outside that root.
No file handles, buffers, or original-game pointer values are exposed.

Matching folds ASCII case component by component. Collisions fail even for an
exact spelling. Requests must use ASCII names, though host installation roots
can contain Unicode. Traversal, reserved names, unsupported absolute paths,
and escaping symlinks fail explicitly. Prefix configuration accepts trailing
and repeated separators but rejects dot/parent components and overlapping
prefixes. See the [complete contract](../research/formats/asset-file-interface.md).

Qt handles host path/name representation through QFileInfo. Checked C++
filesystem iteration, canonicalization, and metadata queries retain operating
system errors; directory enumeration must not silently turn permission or I/O
failures into `notFound`. No directory listings are cached.

Tests cover relative and aliased Windows paths, nested mixed-case names,
Unicode roots, missing paths, file/directory types, name rejection, collisions,
contained/escaping/broken/looped symlinks, current-directory independence,
fresh listings, diagnostics, and denied directory enumeration. Collision tests
skip on case-insensitive fixture filesystems; permission tests skip when the
process can enumerate a mode-000 directory. Symlink fixtures require a host
that permits creating links. Linux fixture validation is confirmed; other host
platforms and original-game path compatibility remain unvalidated. The root
must remain trusted and stable during resolution.

## Read-only handles

Create an `AssetStore` with the same root/prefix configuration, then call
`open(request)`. It returns `Result<std::unique_ptr<AssetFile>>`; inspect the
variant for an `Error` before taking ownership. Handles are noncopyable, have
independent positions, close on destruction, and can outlive the store.

`AssetFile` exposes `size()`, `position()`, absolute `seek(offset)`, and
`read(destination, capacity)`. Sizes/counts/offsets are signed 64-bit integers.
Size and position return `Result<std::int64_t>`; seek returns `Status` (a
`Result<std::monostate>`). Read returns `ReadResult` with a transferred count
and optional error. The caller owns the destination; the backend retains no
pointers. Negative counts/offsets, null nonempty destinations, and seeks past
EOF fail before changing position. Seeking to EOF and null zero-length reads
are allowed. Thread confinement applies to every handle.

`readExact` loops over successful short reads, preserving partial counts and
I/O errors. EOF before the requested count returns `unexpectedEof`. Failed
reads leave any transferred bytes in the caller's destination. `readWhole`
requires an explicit size limit, allocates an owned byte vector, seeks to zero,
and reads the entire reported size. It returns complete output or an error;
successful output remains valid after the file closes. Rejecting a limit occurs
before allocation/seek. The generic helpers work with any `AssetFile` backend.

The QFile backend uses binary ReadOnly/Unbuffered mode and caps each read at
64 KiB, so callers must handle short reads or use `readExact`. It performs no
text conversion, exposes no write operation, and caches no file size. On Linux,
Qt may report a denied open as generic OpenError; immediately captured errno
provides the permission/missing/resource category without parsing error text.
Unknown backend failures remain `ioError`. Non-Linux error mapping remains
unvalidated. Files must remain stable during normal use; this is not a snapshot.

`asset-file-io` verifies binary bytes (including NUL, CRLF, and high bytes),
positions, independent handles, empty files, EOF/partial exact reads, seek
bounds, size limits, multi-read files, source preservation, store/handle/buffer
lifetimes, permission-denied opens, and truncation without stale read-ahead.
A controlled `AssetFile` fixture checks repeated short reads, partial I/O
failure counts, premature EOF after a size query, and size/seek failures.
Allocation-limit rejection is checked, but actual memory exhaustion is not
forced. Real device failures and descriptor exhaustion are not induced.

## Raw installed-file comparison

Chunk 4 provides `mnm-asset-compare` and an installed-validation runner:

```bash
python3 tools/test-asset-files.py
```

The runner verifies the original manifest before/after (also on failures),
builds the assets targets, runs the five CTests, inventories all regular loose
files in `working/game-nocd`, and compares them against independent binary
reads of the same files. Alternating requests use mixed-case relative Windows
paths or mixed-case `C:\\MagicMayhem` paths through an explicit alias. It hashes
both inventories before/after the full run and fails on mutation or differences.
This checks interface fidelity; it does not certify clean-media provenance or
decode proprietary formats. No game is launched or hooked.

Use `--root DIRECTORY --reference-root REFERENCE_DIRECTORY` to compare a
different installation/reference pair. Their union of relative filenames is
compared, so either side's missing files cause failure. The runner rejects
symlinks and non-ASCII relative names rather than silently omitting them.
Fixture tests exercise the resolver's supported symlink behavior separately.
Treat installations as stable during comparison.

Evidence is generated under `working/tests/asset-files/run-*/`:

- `manifest.json`: requests paired with absolute host reference paths.
- `comparison.json`: per-asset sizes, first differing offset, SHA-256 hashes,
  seek counts, completion flags, input stability, and errors; aggregate counts.
- `report.json`: full-run input inventories, binary hash, validation scope,
  file/byte counts, and overall input stability.
- `original-before.log` / `original-after.log`: immutable-artifact checks.
- `comparison.stderr`: CLI failure diagnostics.

The comparison CLI also accepts a standalone JSON manifest, for example:

```json
[{"path":"sOuNdS\\Spell click.wav","reference":"/absolute/reference/Sounds/Spell click.wav"}]
```

```bash
working/build/assets/mnm-asset-compare --root working/game-nocd \
  --manifest working/tests/assets-manifest.json \
  --report working/tests/assets-comparison.json
```

`--prefix PREFIX` is repeatable. Omit `--report` for JSON on stdout. Reports
cannot overwrite the installation tree, manifest, or listed references. The
CLI does not independently invoke repository original-manifest verification;
use the runner for experiments consuming original artifacts.

References are read using binary `std::ifstream`, independently of AssetStore.
Sequential reads compare every byte in 32 KiB blocks and hash the interface's
output. A second pass seeks through every block in reverse order and compares
all bytes again, including short final blocks. It explicitly seeks to EOF and
checks the next read. Empty files also pass through the EOF check. Memory is
bounded by fixed-size byte buffers plus manifest/report metadata.

Exit codes: 0 means every pair is equal and stable; 1 means stable differences;
2 means errors, changed inputs, invalid configuration, or report-writing failure.
Completed rows report `equal`, `different`, or `changed`; failed rows report
`error` with structured category/operation/path/detail. First differing offsets
are zero-based and include the first missing byte for length mismatches; equal
rows use JSON null. Input stability is independent of whether interface bytes
match the source. Snapshot checks cannot detect a transient change fully
reverted between reads, and do not provide a transactional filesystem snapshot.

The Python comparison CTest verifies hashes against Python hashlib, equality,
binary and empty files, first differences across block boundaries, unequal
lengths, earliest differences with reverse reads, missing files/references,
ambiguous paths, invalid manifests, exit codes, and protected output paths.
Concurrent mutation is detected by implementation guards but is not forced in
fixture tests. See [recorded installed validation](../research/formats/asset-file-comparison.md).

Chunk 5 connects this interface to `audio::loadWave(AssetFile&)` and the offline
reconstructed static-upload pipeline. The audio build also runs these asset
tests; installed WAV checks retain independent Python PCM comparisons. See
[audio usage](../audio/README.md). Chunk 6 completes the combined workflow and
documentation review. The [Windows file API audit](../research/runtime/windows-file-api-audit.md)
records remaining writes, save lifecycle, profiles, enumeration/metadata,
path/drive policy, resources, and delegated loaders. This milestone does not
replace the game's full filesystem subsystem or remove Wine from live play.

## Native SPR loading

`mnm-sprite-loader` provides `loadSprite(AssetFile&, SpriteLimits)` and
`decodeSprite(bytes, SpriteLimits)` in `sprite_loader.hpp`. The public result
uses standard C++ types and owns its palettes, pixels, masks and frame metadata.
Only version-4 SPR is supported: indexed byte pixels with embedded palettes,
or little-endian RGB565 words for palette-free sprites. Empty records remain
empty. Transparent runs have a separate mask, so opaque colour zero is preserved.

The loader validates file/frame/row extents, palette indices, run widths and
explicit aggregate input/decoded/work budgets. It uses the existing Qt-backed
AssetFile implementation, but decoding has no Qt or original-binary dependency.
Errors include a frame/byte location; input failures preserve asset diagnostics.

```bash
working/build/assets/mnm-sprite-inspect --root working/game-clean \
  --path 'Creatures\RedCap.spr' --frame 0
python3 tools/compare-mmsprite-binary.py \
  --native-inspector working/build/assets/mnm-sprite-inspect
```

The inspector closes input before emitting selected owned output as JSON; the
comparison runner validates all installed SPRs and selected original i386 draw/
conversion results with original-manifest checks before/after. The runner needs
the pinned external reader, known executable builds and a 32-bit compiler/runtime.
The new `sprite-loader` CTest uses generated fixtures only. See the
[contract, limits and evidence](../research/formats/spr-native-loading.md).

This rendering chunk decodes assets offline. Renderer integration, original
lighting/effects/palette construction, fonts, ANI and live replacement remain
separate work.

The following chunk now supplies a separate [native sprite renderer adapter
and preview](../research/runtime/native-sprite-rendering.md). It owns RGB565/mask
uploads and applies signed origins; the assets library remains independent of
the renderer and application widgets.


## Native ANI tables

`mnm-animation-loader` reads owned version-5 ANI header/offset/record data through
AssetFile; metadata and signed arguments are preserved without interpreting
unknown fields. Three installed older ANI files remain explicitly unsupported.
The native service has no dependency on the recovered player or application
widgets. See [layout, limits and installed byte evidence](../research/formats/ani-native-loading.md).

The separate [No-CD forward player](../research/runtime/animation-forward-contract.md)
recovers selected sprite/delay/repeat/jump/event/stop behavior through bounded
original-code comparisons. It has no asserted wall-clock rate or live replacement.

## Persistence and progression readers

`mnm-persistence-loader` exposes read-only owned loaders in `persistence.hpp`
for plaintext/packed CFG, RealmView configuration, region names, packed `.sav`,
decoded `.vas` and extracted realm state. The container codec supports raw,
RLE and LZSS data with checksums and limits. Save loaders parse the campaign
blocks and retain the unresolved world tail as opaque bytes.

See the [API policies, inspector and validation](../research/formats/persistence-native-loading.md).
These are offline readers; real save compatibility, writers, world restoration
and application integration remain unvalidated. Animation and audio loaders
remain separate services.

## WZD wizard definitions

`mnm-wizard-loader` exposes `decodeWizard`/`loadWizard` in `wizard.hpp` for
owned typed stats, AI weights, sparse starting spells/objects/magic items,
item flags and action records. Missing optional fields stay absent; exact source
bytes and unknown properties are retained. `mnm-wizard-inspect ROOT PATH.wzd`
prints all decoded values and the source hash as JSON.

See [WZD schema, policies and validation](../research/formats/wzd-native-loading.md).
All 101 installed files have offline reference comparisons. Original defaults,
resource binding and live gameplay application remain separate milestones.

## CUR cursor assets

`mnm-cursor-loader` exposes `decodeCursor`/`loadCursor` in `cursor.hpp` for
owned indexed 1/8-bpp cursor images, palettes, AND/XOR planes and per-image
hotspots. It retains every directory variant and does not flatten XOR operations
into alpha transparency. `mnm-cursor-inspect ROOT PATH.cur` prints image metadata
and palette/plane hashes as JSON.

See [CUR format, policies and validation](../research/formats/cur-native-loading.md).
All 20 installed files/22 images match the independent offline decoder.
Presentation, game-driven selection and live cursor integration remain separate.

## PCX indexed images

`mnm-pcx-loader` provides `decodePcx`/`loadPcx` in `pcx.hpp`: owned top-down
8-bit indices, a 256-entry RGB palette, dimensions and header metadata.
Version 5 RLE single-plane files are supported with bounded decoding.
`mnm-pcx-inspect ROOT PATH.pcx` reports metadata and pixel/palette hashes.
See [format, validation and remaining integration](../research/formats/pcx-native-loading.md).
