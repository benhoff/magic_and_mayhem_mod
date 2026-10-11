# Owned canvas image production

`NR.canvas-image-production` owns the selected source-only RGB565 DIB/BMP
composition policy in `CanvasProducerReplay`. The memory DIB record previously
passed the full decoded image to `CanvasSequence::update`, refusing a smaller
destination. GDI clips such images to the destination. Both DIB-at-origin and
positioned BMP now share bounded RGB565 quantization and destination cropping.
Border pixels remain unchanged, and no destination oracle supplies native input.

Canonical uncompressed positive 40-byte DIBs with 1/4/8-bit full palettes or
24-bit BGR are supported. Other formats retain explicit refusals. Memory-DIB
output is compared with the frozen independent Wine Surface2 GetDC/GDI/ReleaseDC
capture in `tests/fixtures/surfaces/surface-dib-ddraw.json.gz`; this executes new
native code against recorded driver output, without claiming a new driver run.
Positioned file-image cropping is a separate native policy comparison. The
legacy loader's sequential reads and the BMP producer's file offsets are kept
distinct. Live wrapper ABI, failed GetDC, DC clipping/palettes and bypass remain
separate contracts.

The initial comparison passes 288 cases: 36 retained independent driver outputs
and 252 positioned-file placements, 5,387,712 RGB565 words, and 288 atomic
malformed-format refusals. Origins include negative placement, fully outside,
and signed integer extremes. This validates current native image production
against the frozen source/driver corpus, without rerunning the historical driver
or promoting the original wrapper contract. Final scenario-bound evidence is
retained in [the current comparison](native-canvas-image-comparison-20261010.json).
