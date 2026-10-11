# DRAW-02 independent drawing corpus

The [portable corpus](../../tests/fixtures/drawing/corpus.json) binds five
retained original-driver surface operations and four complete World consumer
canvases to exact drawing-input identities. The native checker compares every
RGB565 destination word, including borders, and records mismatch counts,
coordinates, bounding boxes, binary hashes and rendering metadata.

This implements the bounded DRAW-02 comparison contract. Broader DRAW-02 session
coverage remains pending: none of the fourteen required journeys has a complete
presented-scene comparison. World consumer output excludes later HUD/window
composition. Surface captures have no established gameplay scenario. The checker
retains these limitations and `--require-complete` fails until wider coverage is
implemented and independently validated. It does not promote drawing takeover.

## Reproduce

```sh
python3 -B tests/test-drawing-corpus.py
cmake -S compat/legacy -B working/build/draw02 -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/draw02 --target mnm-render-replay mnm-world-frame-preview --parallel 4
python3 tools/check-drawing-corpus.py --headless \
  --surface-executable working/build/draw02/scenes/resources/sprites/renderer/mnm-render-replay \
  --world-executable working/build/draw02/mnm-world-frame-preview \
  --report working/tests/draw02-new.json
```

Replay uses no installed game, original executable, Wine or original media. Qt,
OpenGL and an X display are required; `--headless` starts private Xvfb displays
with software Mesa. An unavailable backend is an error, with no CPU substitution.
Exit 0 means all admitted comparisons match; 1 means a mismatch or unmet requested
completion gate; 2 means invalid/inconclusive inputs or execution failure. Reports
must be new, remain within the repository and cannot target `original/` or `.git/`.

## Identities and oracle separation

Every case pins native input bytes and canvas dimensions. World identities also
pin the original capture sequence and draw count. These are capture identities,
not inferred simulation ticks, camera actions or pointers. The four sequences
1, 2, 3 and 4 were collected with 120 skipped queues and interval ten during
selected map2 automatic battle setup. Capture sequence differs from the original
consumer call ordinal; these samples are not contiguous history.
Surface identities pin the raw capture, sanitized input and exact return PC.

The existing surface captures contain source, destination-before and
destination-after. The checker zeroes the entire after-plane before starting
native replay; the comparator alone retains the original output. Surface
destination-before is allowed for operation comparison and is explicitly not
source-only whole-scene reconstruction. The five cases reuse the unchanged
[surface corpus](native-surface-fixtures.md), including its historical collector
provenance limitation.

World inputs contain ordered source RLE frames, colours, clips and displacement
tables. A deterministic packer wraps these exact source frame bytes in SPR v4
containers. Indexed containers have an unused zero palette because actual
RGB565 colours come from the owned World request tables. Destination pixels are
never used to derive these resources. The native process receives only these
containers, bindings, the owned input file and a zero initial native background.
Original output files are separate compressed artifacts and are not passed to
the renderer. Both isolated original execution and live consumer output hashes
must agree with the frozen oracle and its registered historical report.

Every input and oracle is bounded, losslessly compressed and pinned twice:
compressed hash and decompressed hash. Input paths and original-output hashes
must appear in the immutable original comparison report. Frame/output metadata
must agree independently. Duplicate identities, escaping paths, retained frame
pointers, unsupported modes, malformed extents and changed provenance refuse
admission. Inputs, provenance and native binaries are checked again after replay.

## Retention and remaining work

`tools/retain-drawing-corpus.py --output tests/fixtures/<new-directory>` reproduces
retention from the historical working artifacts named in
`RS.world-frame.original-20261007`. It validates those files against the unchanged
report before writing a new corpus. It consumes working captures only, and
refuses overwrite. It does not rerun the original or refresh old evidence hashes.
Ordinary replay needs only repository fixtures; missing historical working files
affect re-retention, not replay.

Original acquisition and native replay remain separate actions. Any new capture
must verify the original manifest before and after, pin the collector/build and
input identities, preserve an independently captured complete output, and declare
its actual scenario and comparison boundary. Camera motion, active battles,
effects, HUD/tooltips, picking, save/load, transitions, errors and recovery need
additional full presented-scene inputs and oracles. Existing glyph, text, image,
fade, movie and producer-history reports remain separate bounded evidence; this
corpus does not reinterpret them as completed journeys or new original execution.

## Full-session suite extension

The [full-session suite](draw02-full-session.md) now runs this bounded corpus
alongside the six DRAW-03 composed startup checkpoints and all1,062checkpoints
of a16-queue retained startup history. It records independent-reference and native
parity gates for all14journeys and21operation/failure contracts. Its presented
session adapter validates exact original action/frame identities, full RGB565 or
RGBA planes and caller-visible results, with portable retention and explicit
unsupported/error outcomes. These guards do not create original presented
captures. None of the14presented journeys is yet closed; the original nine-case
corpus and all historical evidence hashes remain unchanged.
