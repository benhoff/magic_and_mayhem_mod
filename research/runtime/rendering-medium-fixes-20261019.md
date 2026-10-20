# Medium native rendering review and fixes

The dashboard's 54 Medium entries are planning priorities, not 54 independently
confirmed defects. Sixteen have partial implementation milestones and 38 have
scoped implementations. Recovery, independent comparison, current integration,
live replacement and performance remain separate dimensions.

## Confirmed native finite-edge defect

`GlBlitter::snapshot` retains scratch texture allocation dimensions across draws.
The displacement shader previously used that allocation height plus the snapshot
origin as its sample bound. After a larger snapshot, taps past the logical bottom
of a smaller canvas could sample stale scratch texels instead of retaining the
destination. This defect is in native finite-edge policy; it does not establish
the cause of the historical intermittent public pixel mismatch.

`tests/masked-displacement-test.cpp` reproduces the defect deterministically: an
8 by 8 additive snapshot fills scratch with `0xbeef`, followed by a displacement
on the final row of a 4 by 2 canvas. One tap is valid and another exceeds the
logical eight words. The pre-fix shader fails the independent destination check
in `working/tests/rendering-medium-20261019/before-regression.log`.

The shader now receives the destination's logical height. Scratch capacity and
old contents cannot admit out-of-canvas samples. The regression also checks all
17 supported nonnegative offsets after scratch growth. Default strict admission,
covered sample proof, absolute row phase, valid linear row wrapping and explicit
finite-edge retention remain separate policies. Original backing padding is
never supplied as native canvas pixels.

## Repeatable controls

Run the self-contained native controls with Qt, desktop OpenGL, Xvfb and Clang
available; the output directory must be new:

```bash
python3 tools/working-retention.py run --reserve-gib 2 -- \
  python3 -B tools/test-rendering-medium.py \
  --output working/tests/rendering-medium-next
```

The original comparisons additionally take `--pe`, `--displacement-input` and
`--bottom-edge-input`; snapshot queue controls take `--snapshot-source` and
`--snapshot-request`. The runner checks their pinned hashes. Restore archived
working inputs with the retention tool before using them.

`tools/test-rendering-medium.py` freezes the source tree before building. It
checks the compiler dependency closure, records executable hashes, and reports
workspace drift separately from stability of the sources actually executed.
Optimized and ASan/UBSan status helper controls accompany native renderer,
palette/colour caches, prepared World/UI replay, resident history, ACK, mapped
channel, upload, clipping and refusal controls. Optional selected original
comparisons require the unchanged pinned No-CD image and immutable input hashes,
with original manifest verification before and after consumption.

Standalone palette-row, frame-recipe and World delivery/finite-edge controls are
now registered with CTest. These controls previously required manual invocation
despite being built with `BUILD_TESTING`.

An optional prepared build is reused only after checking its compiled inputs,
CMake inputs and executable hashes. Every control and original comparison runs
again, including earlier failures. The final report retains the originating
build path and report hash; reuse grants no execution credit. CPU-only controls
run directly, avoiding unnecessary Xvfb startup/cleanup failures.

The renewed linear fixture also exposed a stale test expectation: anchorY584
has no covered tap outside the logical canvas, although its nominal rectangle
envelope crosses the allocation. The test now requires that draw to succeed and
compares it with owned CPU words. AnchorY600 contains a genuinely out-of-range
covered tap; both atlas variants must refuse it without changing the preceding
completed image. This restores the distinction established by covered-sample
admission rather than reintroducing the old conservative refusal.

## Recorded bounded results

`rendering-medium-native-controls-20261019.json` retains the successful sixth
run without altering earlier reports. All 28 controls and all 11 discovered
CTests pass. It includes 18 high-water edge checks, valid near-edge admission,
genuine overflow preserving completed pixels, two native atlas variants matching
the isolated original linear output, and 12 mapped snapshot failure cases with
ordered import/backpressure/cancellation/owned-source controls.

`rendering-medium-final-controls-20261019.json` repeats every control and
comparison after correcting the registered displacement scope's obsolete
nominal-envelope wording. All checks pass again under the new prospective scope
identity. The earlier successful scope-bound result remains immutable.

The selected original comparisons match 960,000 linear words, 960,064 separately
backed bottom-edge words, and 530,432 grayscale words across eight cases including
all 65,536 RGB565 values. Manifest verification brackets original consumption.
The padded original case is evidence about that authored backing only; it cannot
establish live allocation size or equivalence for native retained edge pixels.

`rendering-medium-gpu-copy-plan-20261019.json` records all 23 copy-plan cases,
guard/publication failures, immutable self/keyed aliases and FIFO resource cleanup.
Its actual DLL and fixture include closures required additional explicit links
for retirement, CPU guards/conversion, producer refusal and reader/palette helpers.
Earlier setup attempts retain the Wine initialization timeout, missing declaration
refusals, and incomplete-prefix failure. The successful attempt used a separate
copy of a complete private prefix.

Earlier native suite attempts retain provenance normalization/setup failures,
the incorrectly paired bottom-edge capture, the stale near-edge test expectation,
and an Xvfb cleanup failure on a CPU-only test. They are not successful aggregates.
The final run rechecks unchanged compiled inputs and executes every test again.

## Acceptance boundaries

A source-stable frozen hardware attempt reached native gameplay and failed its
first positive summon. The retained `combat-06-invalid-target-native.png` shows a
Hermes introduction still covering the scene after portrait recentering. The
subsequent right-click produced no summon. Inference, with high confidence:
the intervening selection click was consumed by the modal. An experimental input helper repeated its bounded image-based
dialogue-readiness check after recentering and after refused casts, before sending
the next action. It does not retry an already consumed positive cast, change
engine state, or relax summon/mana/combat assertions. Live validation of that
harness correction is recorded separately from the renderer controls.

The first experiment acknowledged two queued pages but still failed with a later
Place of Power Hermes message. That negative is preserved separately. The helper
then required a continuous quiet window after the last modal, and observed spell
selection before submitting the one positive cast. A selection consumed by a
modal can be reselected after dismissal; an executed cast is never replayed.
This is a bounded test-admission policy, not a recovered atomic engine UI gate.

The continuous-quiet run still failed the single positive summon with a visible
targeting cursor, and a separate run retaining original UI pixel writers failed
the same assertion. Both source-stable negatives remain immutable. The Hermes
guard is insufficient to repair this integration failure; native input/target
admission needs further recovery. This result does not isolate a defect in the
new native pixel-producer route or justify an original-equivalence, normal-Quit,
19 FPS or 60 FPS acceptance claim.

The next bounded investigation is original input/target admission: correlate
the existing Region mouse callback observer with polling-adapter installation,
button generations and original spell-dispatch entries. The retained attempts
publish the expected Qt mouse generations but show no player Zombie cast return.
That does not identify which downstream gate refused or missed the action.
The original-UI control still uses the native facade and resident World; it is
not an unmodified-driver control. Further recovery must distinguish these paths
before changing engine behavior or declaring the old pixel instability repaired.

These unsuccessful input-helper experiments are not part of the rendering
implementation commit. Their exact executed source variants were retained and
checked against the failed reports' hashes under
`working/tests/rendering-medium-20261019/harness-variants/`. Concurrent helper
changes in the main working tree remain intact. The selected rendering commit
keeps its reviewed baseline helper.

All 54 entries are enumerated with their stages and remaining gaps in each new
run report. Native unit controls cannot close original allocation/caller recovery,
unobserved clipped/self/keyed live GPU copy routes, CPU-reader and failure domain
coverage, complete public journeys, or sustained hardware performance.

The historical 60 FPS misses, queue/ownership failures and intermittent pixel
mismatch remain open until their corresponding current experiments pass or
identify a reproducible cause. Passing a bounded regression is insufficient to
declare those earlier failures repaired. No draw-order, simulation pacing,
resource budget, deadline or acceptance threshold is relaxed by this change.

Incoming commits after `2c90c3f2` are reviewed in the separate hash-bound history
reports for `4ca24a4a`, `34730ab8` and `0a7410fd`. Their 16, 59 and 12 file transitions have exact
receipt chains, with no unmatched transitions or historical evidence rewrites.
This accounting is separate from the uncommitted Medium changes and asserts no
past gate pass or new engine validation.

Incoming commit `f4e47d73` changes session/checkpoint dependencies after the live
negative runs. Those negative records retain their original hashes and are now
historical. The passing Medium renderer controls still match their declared
source closure; no fresh live result is claimed. Newly required static-review
links from that commit are explicitly pinned as pending source review.
