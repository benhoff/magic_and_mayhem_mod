# Pinned MMSprite evaluation

Reviewed 2026-10-04. This is rendering-asset chunk 1: evaluate an external
reader against installed assets. No native SPR decoder, renderer connection,
original pixel comparison, or live loader replacement is established here.

Follow-up: [binary comparison](../runtime/sprite-binary-comparison.md) confirms
that the installed palette-free version-4 SPRs store direct 16-bit colour,
and records isolated original drawing/conversion evidence. It supersedes the
external-palette hypothesis below; this document retains the original reader
evaluation as a distinct milestone.

## Source and reproducer

Upstream: <https://github.com/sapphire-bt/MMSprite>, revision
`7d9bd05a2fb203d81bca11b808dc08b31fbd8c41` ("Update README images").
[Source lock](mmsprite-source-lock.json) records SHA-256 for every tracked
upstream file. The evaluator checks the revision, tracked file contents against
Git, and the committed hashes before execution and again afterward. Upstream
stays in ignored `working/`; its source is neither patched nor vendored.

```bash
git clone https://github.com/sapphire-bt/MMSprite.git working/MMSprite
git -C working/MMSprite checkout --detach 7d9bd05a2fb203d81bca11b808dc08b31fbd8c41
python3 tools/evaluate-mmsprite.py
```

An existing checkout can be reused at the pinned revision. The default asset
root is `working/game-clean`; `--source` and `--root` accept alternative paths.
No pypng installation is needed: this calls the Python reader directly and
hashes its RGBA output rather than invoking its PNG exporter. The reader runs
in a subprocess with a 120-second deadline; decoded samples have an evaluator
budget of 4,194,304 pixels. These limits are evaluator policies, not evidence
of upstream malformed-input safety.

Every SPR/SFT is parsed for metadata. Samples are the first, middle, and last
frames, plus the first empty frame and first frame whose raw palette index is
outside the reader's palette list. Sampling covers seven named representatives
and the first file in every extension/version/palette-count bucket. A single
frame may satisfy multiple criteria. The report lists missing named samples
when another installation is used; default clean-installation coverage below
has none missing.

The evaluator records installed file SHA-256 before parsing and rechecks all
files and the selected file set afterward. Original-manifest verification runs
before and after, including on reader failure. Evidence logs and full per-frame
metadata stay under `working/tests/mmsprite/`; the compact report is retained
as [evaluation evidence](mmsprite-evaluation.json). Inputs remain unchanged.
The recorded run is `working/tests/mmsprite/run-ocpfbf3s/`; both original
verification logs report all 2,927 files verified. The compact report retains
all 187 asset hashes and the sampled metadata/output hashes; it omits the
unsampled frame records to avoid committing the full generated metadata dump.

## Installed results

| Container | Reader version field | Embedded palettes | Files |
| --- | ---: | ---: | ---: |
| SPR | 4 | 1 | 130 |
| SPR | 4 | 0 | 27 |
| SPR | 4 | 4 | 17 |
| SPR | 2 | 1 | 7 |
| SFT | 3 | 1 | 6 |

All 187 files parsed, containing 60,774 frame records and 108,759,407 input
bytes. Every reported frame extent stayed inside its containing file. This
does not validate every row offset, frame-offset table, or compressed run.
Metadata includes 2,297 zero-width/height frames and 1,366 raw palette indices
outside the reader's palette list. Outlier values include -1 and large positive
integers; their original-engine meaning is unconfirmed.

Ten files supplied 30 sampled frames. Twenty-eight decoded with exactly
`height` rows of `width * 4` RGBA bytes. Two raised the reader's
`InvalidDimensionsException`: Celtic Forest terrain frame 141 and effects3
frame 984, both zero by zero. Empty frames are legitimate installed records;
their original runtime treatment still needs recovery. This result does not
count them as malformed input or successful image decodes.

Named representatives cover RedCap creature animation frames, Celtic Forest
terrain (four palettes), Buttons UI sprites and timer (no embedded palettes),
LOGO (version 2), effects3, and body text 640 font data. Additional bucket
representatives cover ScanSmallEdge, 16Mb_Anims/Ariadne, and Tooltip Text.
Output hashes are reference-reader outputs, **not original-game pixel hashes**.

Confidence: high for these installed counts, metadata parsing, unchanged-input
checks, and sampled output shapes/hashes. Provisional for interpreting the
reader's fields as original-engine contracts. No claim of whole-corpus pixel
correctness or game-equivalent rendering is made.

## What the reader contributes

Static source inspection of `py/mm_files.py`, `py/base_classes.py`,
`js/mm-reader.js`, and `c/fmt_spr.c` supplies a useful SPR layout/decoder model:

- The SPR reader uses a 24-byte header, RGB palettes of 256 entries, a skipped
  four-byte-per-frame table, and sequential frame records. For version 2 it
  subtracts four bytes from the calculated first-frame position. The C code
  instead consults the offset table; the evaluator has not independently
  established all table semantics.
- Frame metadata contains size, width/height, signed centre coordinates, an
  eight-byte name, palette index, and paired delta/pixel offsets per row.
  Versions above 2 have another eight bytes the Python reader skips.
- Row deltas alternate transparent and coloured runs, starting transparent.
  Coloured pixels are palette indices. The Python reader emits zero RGBA for
  transparent runs and palette RGB with alpha 255 for coloured runs.
- The SFT reader extends the SPR machinery with a 40-byte header and extra
  table-size values. Its six installed files parse; only two fonts are sampled.

These are confirmed descriptions of the pinned implementation, supported by
installed parsing/samples. Exact original offset-table, trailing-table, palette
selection, and displacement behavior remain separate reverse-engineering work.

## Gaps relevant to a native loader

1. Missing palettes become synthetic grayscale in MMSprite. The subsequent
   binary comparison establishes a direct RGB565 word path for the installed
   version-4 palette-free files. A native result must represent those words
   explicitly; grayscale is a diagnostic policy, not faithful colour.
2. Preserve a transparency mask separately from palette indices. Transparent
   runs and opaque index-zero pixels must remain distinguishable; the C exporter
   chooses an unused index for transparency whereas Python exports RGBA.
3. Preserve raw dimensions, frame origins, names, and empty frame records.
   Python rejects empty frames; JavaScript coerces zero dimensions to one.
   Neither policy establishes the original engine's behavior.
4. Palette fallback differs between implementations. Python's test is
   `index > len(palettes)` rather than `>=`; equality would raise IndexError.
   No equality case was found in this installation. Negative/large indices
   otherwise fall back to palette zero. Recover original semantics before
   promoting that fallback to a native compatibility rule.
5. Python validates signature and total file size but does not comprehensively
   bound frame counts, dimensions, row extents, or run lengths. Implement
   explicit checked offsets, allocation budgets, palette bounds, and run-width
   checks in the native decoder; do not expose this reader to untrusted assets.
6. Eight extra frame bytes and possible trailing data are unused by Python.
   Their effect on drawing is unknown. Original comparisons must cover variants
   where these fields matter, including palette/effect handling.
7. ANI is format research here, not a runnable Python animation decoder. The C
   declarations and `formats.md` disagree on header details, and some animation
   group labels are guesses. ANI timing/events/direction mapping remain outside
   this SPR evaluation.
8. The checkout has no tracked LICENSE file or licensing declaration found by
   inspection. Source attribution leads through the README to Nikita Sadkov's
   OpenXcom forum work; code reuse terms remain unestablished. This chunk keeps
   upstream external and records its provenance.

## Next bounded chunk

Define and implement an owned SPR result through `AssetFile`: indexed pixels,
transparent-run mask, embedded palettes, frame metadata, explicit empty
frames and a direct-RGB565 variant identified by the follow-up. Start with
version-4 RedCap and its embedded palette,
then check its decoded indices/mask/metadata against the pinned reference.
Keep reference agreement separate from original-game equivalence. Add an
original decoder or draw observation before claiming game fidelity, and connect
one verified frame to native OpenGL only in the subsequent integration chunk.

Direct-colour UI, version-2 compatibility, fonts, terrain, and ANI can then extend the
verified scope. The current Qt path/file interface already supplies the input
boundary; this research does not require a Wine dependency in the native decoder.
