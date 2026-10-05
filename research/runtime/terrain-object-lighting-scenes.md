# Owned object lighting through native terrain presentation

## Contract and confidence

`TerrainLightingCycle` owns the previously recovered light field, creature
cycle and static-object cycle. A snapshot advances creature lighting first,
passing the current static active/requested controls, then static lighting,
passing the creature publication flag and the same supplied changed value.
This models selected no-CD **004f27a0**, **004f27d0** and **004f2ad0** behavior;
see [creature lighting](terrain-creature-lighting.md) and
[static lighting](terrain-static-lighting.md) for binary evidence and exclusions.

The composition uses an owned candidate copy and commits it only after both
subcycles accept the snapshot. A rejected static snapshot therefore cannot
leave a partially advanced creature field. This atomic refusal is a deliberate
native admission policy, not original invalid-input recovery. Copies of the
composition and decoded snapshots are independent. There are no host pointers,
Qt widgets or Wine dependencies in the recovered composition.

The Qt terrain preview loads [MNM_LIGHTING v1](../formats/terrain-lighting-fixture.md)
snapshots with `--light-fixture`, applies a chosen prefix and samples its final
published field into owned tile inputs. It passes those tiles through the
existing four-orientation world queue, palette selection and OpenGL renderer.
It does not force immediate publication: staged or disabled cycles retain
exactly the published bytes produced by the recovered update order. JSON
contains each applied tick's eleven cycle state fields, consumed changed flag,
all five buffer hashes, fixture hash and selected tick. Without fixture mode,
the existing controlled-stamp and unshaded preview paths remain available.

## Original comparison

The [report](terrain-object-lighting-scenes.json) pins no-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168` and hashes
source files, helper binaries, installed inputs and authored fixtures.
`tools/test-terrain-lighting-scenes.py` checks the immutable manifest before
and after, including failure. No original executable or installed file is
patched. The original routines execute in a private isolated PE32 mapping.

For each fixture tick, the helper sets bounded original collection records,
light-index table entries, relation bytes and region admission inputs, then
executes the whole unmodified combined updater. It compares all five guarded
buffers, eleven creature/static state values, source-record guards and the
outer changed-flag clear against the native 32-bit composition. The actual
original buffers and states become the reference for the 64-bit Qt preview.

The presentation comparison uses decoded installed MAP geometry and the
original published light field as input to the original four-view world
traversals and queue sort. Complete queues must match the preview, including
shade values and cell ownership. The resulting original queue then feeds the
original mode-zero palette/indexed drawing helper; every RGB565 pixel must
match native OpenGL output. Normal and ASan/UBSan previews must also match each
other. These are controlled object-driven terrain scene comparisons, not a
claim to reconstruct the original entity producer or complete game screen.

Reproduce with normal/sanitized terrain-preview binaries:

```sh
python3 tools/test-terrain-lighting-scenes.py \
  --preview working/tests/terrain-lighting-scenes/build/mnm-terrain-preview \
  --sanitized-preview working/tests/terrain-lighting-scenes/sanitized/mnm-terrain-preview
```

The run compares 36 original ticks, 2,976,000 five-buffer bytes and all eleven
cycle states. All 192 object-driven frames match across three realms, eight
selected ticks, four orientations and normal/sanitized builds. Two default-final
selection frames also match, and 22 invalid requests fail before creating output.
The full suite passes all 68 tests in normal and ASan/UBSan builds. Both manifest
checks verify all 2,927 original files; the documented example renders successfully.

The new evidence fingerprints the current object-fixture pipeline separately
from preserved frozen-source provenance. Prior preview reports retain their old
entry-point hashes and may be reported stale after the CLI addition; those
historical reports are not rewritten into current-code claims. The current
comparison validates the selected object-fixture path, not every older preview
mode or all registered engine behavior.

The [focused accounting package](terrain-object-lighting-coverage.json) updates
the lighting behavior/evidence IDs and preview scenario without rewriting older
results. It passes `--require-fresh` with no warnings. The full shared audit has
no errors; historical source staleness and unrelated census changes remain
visible. The [source-census snapshot](terrain-object-lighting-source-index.json)
refreshes only this chunk's code/build/test links and hashes, retaining the prior
review for unrelated files. Machine-readable audit outputs remain under
`working/tests/terrain-lighting-scenes/`.

## Remaining boundaries

Fixture records, relation bytes, region admission coordinates/extent and tick
cadence are authored native test inputs. Installed creature/static collection
production, type-table loading, gameplay or animation-derived light activation,
region/camera lifecycle, spectator/sentinel owners, special size-one lights,
allocation/reload failures, other smoothing behavior and live observation or
replacement remain separate milestones. Geometry initialization still excludes
runtime entities. No gameplay balance changes are introduced.

The next bounded step is recovering installed static-object light-table
construction and source-record production, then comparing those admitted owned
inputs against the existing authored-fixture interface. Original startup and
runtime producers must be validated before using their outputs in native scenes.
