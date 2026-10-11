# Native drawing-scene declarations (schema1)

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
