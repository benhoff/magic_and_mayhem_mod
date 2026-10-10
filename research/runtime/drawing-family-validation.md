# Drawing-family validation

The bounded runner `tools/test-drawing-families.py` executes existing independent
comparisons and native guard tests for all nine inventoried drawing families.
It builds current renderer/sprite sources, records each command and log, brackets
original-artifact consumption with manifest verification, and refuses a successful
result if its source fingerprints change during execution. Reports and temporary
builds are retained under `working/tests/drawing-families/run-*`.

Prepare a prospective declaration before running:

```sh
python3 tools/draft-coverage-claims.py \
  --behavior TOOL.drawing-family-regressions \
  --output working/tests/drawing-family-claims-next.json
python3 tools/test-drawing-families.py \
  --claims working/tests/drawing-family-claims-next.json
```

Use `--skip-live` to omit the fresh direct-word startup takeover. This leaves
live testing explicitly unexecuted. Passing a group means its selected tests
passed; it does not mark the complete family implemented or replaced.

| Family | Executed subset | Remaining contract |
| --- | --- | --- |
| Copies / fills | Independent retained original rectangles, keyed/overlap outputs and HRESULTs; native GPU ownership/formats/clipping | Full caller admission and live original copy/fill suppression |
| Locked writes | Owned access/lease guards and PE32 drawing/recovery collisions; direct-word raster samples | JPEG destination writer and unidentified CPU producers |
| GDI / text | Retained DC raster bytes, reloads, palettes and ownership | Active GDI operations, original layout and painting suppression |
| Terrain | Private original ordinary submission and native queue state | Live traversal, water/overlay/picking/visibility |
| Sprites | Installed original/native GPU samples; optional fresh direct-word startup bypass | Clipped/alternate dispatch, sustained play and complete scenes |
| Effects | Whole original forward animation bind/start/tick and sanitized native state | Effect-to-queue-to-pixel attribution and producer replacement |
| UI | Native menu font/sprite/image fixtures; shared direct-word samples | Original HUD, minimap/tooltips/cursor composition and full suppression |
| Movies | Exact synthetic video/audio decoding and PE32 request/fallback bridge | Installed enabled playback, errors and coherent World return |

The effect and movie groups deliberately exercise existing foundations. They
do not close the missing visual-producer contracts. The coverage crosswalk's
family counts stay unchanged unless new original-route proof warrants a reviewed
mapping. Original shadow presentation, synthetic transport and native Qt layout
are independent milestones.

## Execution on 2026-10-10

The complete run is retained at
`working/tests/drawing-families/run-ijed84_t/report.json`. Its source fingerprints
were stable and both immutable manifests passed. It correctly failed on one
stale sprite-test expectation; the
[negative result](drawing-family-regressions-negative-20261010.json) remains
unchanged. An earlier sandbox attempt could not start Xvfb and was interrupted;
its logs remain under `working/tests/drawing-families/run-wvf93xvr`.

| Group | Result and bounded evidence |
| --- | --- |
| Surface operations and access/DC transport | All 16 renderer CTests passed, including independent original/driver fixtures |
| Drawing/recovery ownership | All 10 PE32 collision cases passed; 40 complete frame comparisons |
| Ordinary terrain submission | 15,204 original/native cases and 15,317 queue records matched |
| Effect forward ANI state | 224 fixtures, 448 bindings, 28,672 ticks and 29,120 states matched; normal/sanitized units passed |
| Installed sprite GPU drawing | Corrected rerun matches all 91 original RGB565 outputs and all 91 presentation hashes; 14 indexed, 77 direct, 2 empty, zero remaining surfaces |
| Native menu controls | Font, sprite and image fixtures passed |
| Native media | Exact synthetic movie RGBA/PCM, WAV PCM and eight PE32 bridge requests passed |
| Direct-word startup takeover | 15,104 bypassed draws, 705 forwarded refusals; eight retained complete canvas/workspace samples match independent original execution |

The sprite test still expected floor-scaled RGB565 expansion. Independent
[Surface2/DC measurements](surface-dib-ddraw.md) already establish bit replication
at every channel level. All 91 rendered samples matched that independent expansion;
the test expectation was corrected without changing rendering code. The
[fresh prospective-bound result](native-sprite-bitrep-regression-20261010.json)
also matches every original native-word draw. Existing sprite guards and the
exhaustive 65,536-color normal/high-DPI presentation tests pass separately.

The broad failed suite and corrected single-group execution retain distinct
source versions. Other passing groups were not rerun after a test-only change.
These results are selected-subset checks across all nine families, not a single
fresh all-family engine-equivalence result. No family is declared complete and
no additional producer is enabled for live bypass. Mesa llvmpipe was the tested
OpenGL backend; physical-driver and sustained-session behavior remain open.
