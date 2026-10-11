# Native drawing-scene declarations (schemas1/2)

These JSON files are native toolkit formats, not recovered game file formats.
They use the existing [producer wire contract](canvas-producers-v1.md); no host
pointers or C++ layouts are shared.

`tests/fixtures/drawing-scene/scene.json` is an immutable retention catalogue.
Its build/evidence identity names the preserved independent original comparison.
`historical_stream` pins the full observed stream hash and the retained prefix
extent. `input` contains the exact prefix. `boundary` names the final source
sequence, completed World queue count, logical canvas generation and pixel extent.
`producer_counts` accounts for every retained operation. `assets` bind requested
source names to actual encoded assets. `checkpoints` bind original oracle ordinals
to source sequence, canvas and dimensions. `remaining` names excluded behavior.
`retention_sources` fingerprints the retention tools; `original_inputs_verified`
records successful checks of the historical stream, source assets and outputs.

Each binary artifact entry has a repository-relative `path`, `encoding: gzip`,
compressed `sha256`, and `raw_sha256`. Original source/output entries also retain
`historical_path`, whose raw hash must occur in the original report. Decode limits
are32MiB per artifact. Paths cannot escape the repository or enter original/.git.
Asset names may use captured Windows separators and case; native materialization
uses one case-folded directory tree and rejects duplicate case aliases.

The comparator creates a separate **renderer declaration** with only `schema`,
`boundary`, `assets` (requested source-name to encoded SHA-256), and
`retain_checkpoints` (native output ordinal selection). It gives the native process
only that declaration, producer bytes, an isolated source-asset root and a new
output directory. Oracle paths and pixels are excluded.

Native output contains selected tight little-endian RGB565 checkpoint planes,
`scene.565`, and `scene-report.json`. The report binds source sequence/canvas/
extent to checkpoint hashes, records complete World request count, GPU output
readiness and cleanup, and exposes refusal errors. A successful native output is
not itself proof of independent original equality. The external comparator checks
all selected full planes and verifies that `scene.565` is the final checkpoint.
Neither file format establishes physical presentation equivalence or live bypass.

Schema2 adds ordered `frames`, each with the same fields as `boundary`. The
last frame equals the terminal boundary; source sequences strictly increase and
completed World counts cannot decrease. Zero queues admits menu/loading frames.
Each retained frame must name its own independent full-canvas checkpoint. The
renderer receives frame identities only and writes `frame-NNNN.565` plus frame
hashes. Every publication is compared separately from diagnostic checkpoints.
Sealing an already published terminal boundary adds no frame.

Schema2 may represent `input` as `raw_sha256` plus an ordered `chunks` array.
Each fragment has packed/raw hashes and gzip or XZ encoding. XZ decoding requires
one complete stream with no trailing bytes, at most128MiB dictionary memory
and32MiB output; concatenation requires at most eight fragments,128MiB total and
the exact prefix SHA-256. This packaging preserves the original wire bytes and
does not reset producer, queue, checkpoint or canvas identities between chunks.


## Native raster-result separation

Portable artifacts preserve their immutable captured bytes. Before native replay,
the comparator zeroes record field21 (offset84) for kind9 rasters, including
owned-source references, without changing other wire bytes. It retains original
observations externally. Font and minimap source/state fields have separate
contracts and are not part of this raster return claim.

The native runner rejects nonzero raster field21. `raster-results.tsv` contains
one line per World raster: decimal source sequence, global queue, canvas,
backend, and low16 AX or `unknown`, separated by tabs. Its count, SHA-256 and
unknown count accompany the report. Count/order/identity errors refuse; unknown
or different returns fail the separate result comparison. Captured return state
at/after a recorded native bypass cannot establish original-state equivalence.
