# Native surface operation matrix and original fixtures

Milestone 3.1, 2026-10-06. The [operation matrix](native-surface-operation-matrix.json)
separates required work, conditional discovery backlog, implemented native
operations and available independent original output. It is not a percentage of
DirectDraw or a complete renderer equivalence claim. Scope is the No-CD build
SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Required scope

| Matrix ID suffix | Required contract | Independent original fixture coverage |
| --- | --- | --- |
| `format.rgb565` | Native RGB565 words and color interpretation | Five selected copies; native words compared, displayed RGB conversion independently checked offline |
| `format.other` | Required indexed/masked formats, pitch and layout | Pending; current synthetic indexed8/RGB555/24/32 coverage remains separate |
| `copy.opaque` | Whole/rectangle Blt and BltFast, borders and errors | Five RGB565 BltFast rectangles at one return PC |
| `key.exact_source` | Exact native source key and state changes | Pending |
| `palette` | Entries, shared ownership, binding and lifetime | Pending; v2 native resource policy exists |
| `clip.rectangles` | Boundary/empty handling and coordinate adjustment | Pending; already bounded rectangles are not clipping evidence |
| `clip.regions` | Required attached clipper regions and changes | Pending; reachable region shapes remain unresolved |
| `fill` | Argument color, wrapper truncation, rectangle and retries | Pending; native constant UPDATE route exists |
| `copy.self_overlap` | Same-surface copies, aliases and overlap directions | Pending |
| `restore` | Loss, Restore, retry, content validity and state reload | Pending; transport recovery/checkpoint policies are separate |
| `lifetime_updates` | Complete initial state, CPU writes and retirement | Pending original-driver sequence; native foundation exists |
| `flip` | Selected front/back presentation and identity | Pending original paired outputs; native two-buffer SWAP exists |

Ranged/destination keys, conversion/stretch, extra effects and complex flip
chains stay conditional until caller evidence or an explicit scope decision
requires them. Their unsupported branches are retained, not declared unused.
Baseline behaviors refer to existing coverage IDs; selected original RGB565
copy equivalence has its own `RE.surface-copy.rgb565-bltfast` scope.

## Portable retained evidence

[The corpus](../../tests/fixtures/surfaces/corpus.json) preserves five historical
`draw-capture/blit-0001.bin` samples and their launch manifests, independently of
the disposable working directories. Captures are losslessly gzip-compressed;
the catalog pins compressed, original raw, manifest and expected-output hashes.
Manifests are preserved byte-for-byte. Their absolute historical paths are
provenance only and are never opened by replay. The repository contains no
executable or patched game binary in this corpus.

All five captures are successful opaque RGB565 BltFast calls with WAIT at
return PC `0x0058c5be`, in the selected `0x0058c4a0` rectangle wrapper. Each
source is 40x40 and the complete destination is 800x600. Three samples copy
40x40, one copies 40x3 at the lower-left edge and one copies 40x1 at the bottom
edge. Changed destination pixels are respectively 190, 190, 190, 6 and 0.
The unchanged one-row sample is retained as such; it cannot prove active writes.
Caller addresses and interface tokens are build/process evidence, never replay
pointers. Scenario/gameplay phase is not inferred from the pixels or caller.

The reviewed producer `runtime/render/draw_capture.h` snapshots source and
destination-before using saved original methods, releases observer locks before
the forwarded draw, then records destination-after with a saved-method original
Lock. It checks descriptor/palette stability and successful output readback.
These samples are driver backing-surface output, rather than the separate
`lock-capture/blit-*.bin` hook-owned reconstructed output. They do not establish
physical-driver compatibility, capture noninterference, complete frames or
current hook-source validation. Historical manifests pin the staged executable
and DLL identities; collector source fingerprints were not retained in those
manifests, which remains a provenance limitation.

The fresh offline check supplies source and destination-before to the existing
CPU/OpenGL replay. Captured-after is used only for comparison. Native checks
cover every destination pixel, including borders. GPU-presented RGBA is compared
with independent CPU mask conversion; no independently captured original
display/framebuffer colors are claimed.

## Reproduce without game, Wine or original media

CPU verification:

```sh
python3 tools/check-surface-fixtures.py --report working/tests/surface-fixtures-cpu.json
python3 -B tests/test-surface-fixtures.py
```

Native OpenGL verification:

```sh
cmake -S renderer -B working/build/renderer
cmake --build working/build/renderer --target mnm-render-replay --parallel 4
python3 tools/check-surface-fixtures.py --backend opengl --headless \
  --gl-executable working/build/renderer/mnm-render-replay \
  --report working/tests/surface-fixtures-opengl.json
```

Report paths must be new. CPU mode never substitutes for a requested OpenGL
backend. Exit 0 means the admitted fixtures match, 1 means output mismatch and
2 means malformed/inconclusive evidence or unavailable rendering. Reports
explicitly retain missing required matrix IDs and `milestone_complete: false`.
Passing five copies cannot complete a required operation row or the milestone.

## Adding evidence

Use the existing [draw capture procedure](render-drawing-inventory.md) for more
eligible same-format opaque/exact-key copies. Hash-check original inputs and
verify the immutable manifest before and after original-artifact experiments.
Record game build, staged executable, collector identity, graphics environment,
scenario and original API result; preserve the completed capture and manifest.
Extend this strict corpus admission and meaningful refusal tests before adding
another operation. It currently admits only these RGB565 opaque BltFast cases.

Current MNMBLT01 cannot express fills, clippers, self-copy, failed operations or
Restore sequences. Those require an independently captured state/result format
before they can supply original fixtures. Do not serialize expected output as
UPDATE, infer alias identity from reused pointers, fill unknown initial pixels
with zeros, or treat undefined restored contents as deterministic bytes. Keep
synthetic models, original API output and native recovery policy separate.

Record new immutable comparison results and their source hashes; retain old
evidence when shared code changes. Later fixtures must preserve both successful
and failed operation state, and specify which restored pixels are valid. This
chunk establishes the matrix and a bounded original corpus; new collection for
the remaining rows is explicit pending work.

## Recorded result

[Fresh CPU/OpenGL execution](native-surface-fixtures-opengl-20261006.json) passes
all five complete destination comparisons and independent presentation color
checks with Mesa llvmpipe. Each fixture checks 480,000 native destination pixels;
2,400,000 pixels are compared in total. Five corpus/refusal tests pass, including
poisoned expected output, changed hashes/provenance, escaping paths and an
overstated restoration claim. The renderer replay target builds successfully.
OpenGL execution required permission to create the local Xvfb display socket.
No game or Wine process was launched. Original manifest verification passes
before and after this work. Missing required matrix rows remain explicit and
the milestone is incomplete.

The committed predecessor range `ade3e60..3dd6e27` was reviewed separately against
exact parent/current file and affected behavior receipts; no unresolved gaps
were found in that committed range. The retained
[history report](coverage/committed-history-surface-fixtures-20261006.json) does
not assert historical gate passage or new checkpoint validation. Concurrent
uncommitted recovery work is outside that review.
