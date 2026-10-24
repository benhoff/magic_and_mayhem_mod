# Three terrain rendering implementation closeouts — 2026-10-24

This batch completes `RE.terrain_camera`, `RE.terrain_sections` and
`RE.terrain_submission` within their existing selected offline scopes. The
high-priority plan already identified the section-copy implementation as supplied;
its missing milestone was current validation, not a replacement algorithm.
Original understanding, comparison, integration and replacement remain independent.

The pinned No-CD executable is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high within the executed boundaries. Original instructions execute
in private mappings with owned receiver, map, queue and resource inputs. The
immutable original manifest passes before and after each recorded comparison.

| Contract | Change and current evidence |
| --- | --- |
| Camera setters and scrolling | Expose `scrollTerrainCameraAxes` for original `4f7b50`, and reuse it from screen-direction scrolling. Direct movement ignores view and normalizes zero displacement, whereas the screen helper retains its zero-input early return. Both axes commit together; checked overflow remains native policy. The original binding, viewport, position, screen and direct-axis paths compare 270,336 represented states across 8,192 fixtures. |
| Ordinary section copying | Retain the existing frozen-source algorithm. New controls cover all 324 overlapping crop/destination/rotation combinations and 108 in-place late malformed-definition/object refusals, comparing every cell word. Original comparison checks 8,192 synthetic copies/410,500 cells and four rotations of three installed Forest maps/307,200 cells, including destination borders, extra layers, original source-definition mutation and native source immutability. |
| Ordinary tile production | Fix the native queue shade representation: after wrapped subtraction and the lower clamp, retain the signed low WORD stored by the original. A bias of -32768 with light zero produces shade -32768, rather than the previous native 32768. Strengthen comparison to require canonical signed native shades, not just equal low bits. There are 15,716 original cases/16,341 queue records, including 512 new bias, player, cut-level, role and orientation controls. Native tests also cover coordinate wrapping, frame fallback and unchanged refusal state. |

[Final execution](rendering-nineteenth-three-results-20261024/execution-4.json)
passes six native/ASan/UBSan controls and all three original comparisons, with
prospectively declared contracts, complete compiler header dependencies and stable
source/input hashes. The section installed cohort is three maps; the historical
683-map and whole-image results are not claimed as newly executed. Original
preview journals in the producer log are not a new native scene-pixel comparison.

The new producer test is also registered in CTest. The separate
[normal CMake and regression check](rendering-nineteenth-three-results-20261024/ctest-and-shade-regression.json)
runs the three native test targets and confirms that the old native producer
fails the new shade control. It supplies no original-world or GPU acceptance.

## Negative evidence and retained boundaries

[Execution 1](rendering-nineteenth-three-results-20261024/execution-1.json)
retains a failed proposed signed-height expansion at the native queue's existing
negative-height refusal. [Execution 2](rendering-nineteenth-three-results-20261024/execution-2.json)
retains the subsequent original-reference fault: raw level `07ffffff` indexes the
original light-offset table at `4f89ac` with `ECX=03ffffff`, outside its owned
fixture domain. This is a failed out-of-domain experiment, not evidence that a
natural game level is broken. No original table patch or invented allocation is
used to turn it into a pass. The height-domain expansion was removed; the existing
native level bounds are unchanged. [Execution 3](rendering-nineteenth-three-results-20261024/execution-3.json)
retains the successful preceding version before CMake was added to the prospective
producer declaration. Every report and historical source fingerprint is preserved.

Object-linked section rotation, original cell/container/allocation ownership,
camera resource acquisition/lifecycle/input timing and the opaque `+99` marker,
tile picking/water/overlay/creature producers, shade quantization/palette chains,
full original-world production, whole-scene pixels, DirectDraw retirement, FPS and
live replacement remain open. The native immutable source, overlap staging and
atomic malformed-input/overflow refusals are explicit policies, not original
failure or malformed-pointer equivalence. Shared broader execution bundles that
fingerprint these changed files remain historical pending their own renewal.

## Reproduction and storage

From the owned checkout, invoke the main checkout's storage guard with absolute
paths, because it launches its child from the main checkout:

```sh
python3 /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py run \
  --reserve-gib 0.12 -- python3 "$PWD/tools/test-rendering-nineteenth-three.py" \
  --output "$PWD/working/tests/terrain-three-next" \
  --report "$PWD/research/runtime/terrain-three-next/execution.json" \
  --asset-root /home/hoff/swdev/magic_and_mayhem_mod/working/game-clean
```

The runner creates no installation or Wine prefix. Existing installed files and
original media are read-only inputs. Raw compiler products, decoded interchange
and logs remain pinned in the owned checkout's ignored `working/tests`; durable
reports use SQLite references. The startup storage inventory reports zero cleanup
review candidates; no payload is removed and the 90 GiB reserve remains enforced.

[Incoming committed-history review](rendering-nineteenth-three-results-20261024/incoming-history.json)
checks exact parent/current file hashes and append-only receipts over
`0c0d4038..56214abe`. This review is separate from current execution and does not
assert a historical gate pass.

All eight unique source-file versions from the failed attempts are retained in
[the verified failed-source record](rendering-nineteenth-three-results-20261024/failed-source-variants.json);
each byte sequence matches its original prospective execution hash.

The required review database and backups exceed the main drive's remaining
headroom. [The task worktree relocation](rendering-nineteenth-three-results-20261024/worktree-relocation.json)
preserves all18,360 file identities on the existing work drive at
`/home/hoff/Videos/mnm-rendering-nineteenth-three-20261024/worktree`,with the
original checkout path retained as an alias. This is storage provenance only;
no payload is discarded and no engine comparison is inferred from moving it.
