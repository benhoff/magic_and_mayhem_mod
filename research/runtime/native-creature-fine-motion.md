# Bounded creature fine motion and checkpoint continuation (NS03)

## Scope and confidence

NS03 adds an opt-in forward-animation sample driver to the headless native
movement session. It replaces waypoint-per-tick pacing for that profile with
fine-position progress, an accumulator, sample cursor and completion threshold.
It remains one creature on one bounded frozen navigation resource. No injected
replacement, Qt gameplay, live agreement or original-save conversion is claimed.

High confidence: selected arithmetic transitions of No-CD `0x005104b0`, and
selected route-consumption/discrete-coordinate transitions of `0x00512460` with
`0x005070e0`, agree with isolated original x86 execution on synthetic objects.
The animation event stream and environmental/behavior callbacks are controlled
fixtures. Those comparisons establish the tested contracts, not original full
animation timing, motion initialization or creature behavior equivalence.

## Recovered evidence

Pinned executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`tools/export-creature-behavior.py` now exports four additional instruction ranges:

| Entry/range | Confirmed static behavior |
| --- | --- |
| `0x005104b0..0x00510e3a` | Motion action, accumulator admission, sample displacement, animation events, completion and behavior/environment dispatch |
| `0x00510e80..0x00511c0c` | Segment setup, scalar inputs, sample-bank selection, reverse-animation setup and transition into action 2 |
| `0x00512460..0x005127f9` | Route-point consumption, position update, environmental callbacks, cursor increment and continuation flags |
| `0x005070e0..0x00507189` | Discrete XYZ and fine XY/Z snap, followed by terrain/type-dependent height adjustment |

All offsets below are build-specific unaligned DWORDs in the creature record,
not native layouts or stable cross-launch pointers:

| Offset | Selected meaning |
| --- | --- |
| `+0x977`, `+0x97b`, `+0x97f + 28*i` | Current route index, count, seven-DWORD route point |
| `+0x608` | Eight-way motion direction |
| `+0xb3f`, `+0xb43` | Cumulative segment progress, current sample increment |
| `+0xb47/+0xb4b`, `+0xb4f/+0xb53` | Cumulative XY displacement and current signed XY increment |
| `+0xb57/+0xb5b/+0xb5f` | Height delta, clamped interpolation displacement, origin height |
| `+0xb63/+0xb67/+0xb6b` | Rate, duration/divisor, accumulated rate |
| `+0xb77/+0xb7b/+0xb7f` | Animation substep count and presentation residual XY |
| `+0xb83/+0xb87/+0xb8f/+0xb93` | Initial residual XY, current sample pointer, initial sample pointer |
| `+0x14/+0x18/+0x1c` | Fine XY/Z coordinates |

At `0x5104d1`, rate is added to the accumulator, followed by signed division by
the stored duration. A nonpositive quotient does not move. For each admitted
iteration, an animation sample increments progress; a vertical route point
instead doubles that sample and zeros XY increments. Directional planar
increments use `(0,-1),(1,-1),(1,0),(1,1),(0,1),(-1,1),(-1,0),(-1,-1)`.
Fine XY becomes grid XY times 32 plus cumulative signed displacement divided
by six, truncating toward zero. Fine Z is the origin height plus
`clamp(heightDelta * progress / 192, -16, 16)`. Presentation residuals accumulate
old minus new fine XY. Animation event 2 restores their initial values and the
initial sample pointer. Every executed substep subtracts the duration.

At `0x510907`, an unsigned progress comparison against 192 consumes the route
point. The completion branch returns through behavior/environment handling;
it does not automatically execute unused iterations on the next route point.
The type-12/category-2-or-3 branch forces an increment of 32 and one iteration,
after positive admission. The model compares that arithmetic too; the native
adapter excludes this special profile.

At `0x512527`, consumption clears height delta/interpolation and the selected
movement flag, passes the current route XYZ to `0x5070e0`, then invokes separate
environmental/animation helpers. If the creature remains active, `+0x977`
increments. Reaching `+0x97b` clears `+0xb8b`. A positive `+0x963` countdown is
also decremented. Native movement adopts the tested coordinate/cursor boundary;
it does not invent semantics for the remaining callbacks/countdowns.

Segment setup stores raw `0x505840` output as duration at `0x51153f`, applies
speed modifiers, then invokes `0x505920` with the existing rate as in/out state.
The conditional reset paths, current animation category, reverse animation and
live speed continuity still need separate recovery and comparison. NS03 uses
the existing recovered route scalar as its rate and raw scalar as duration;
this is a bounded integration policy rather than a reconstructed setup routine.

## Implementation and persistence

`reconstruction/motion/` holds the independent arithmetic model. The native
simulation receives a `Navigation` motion-driver interface; only
`apps/world-sandbox/frozen_navigation` composes the recovered model and profile.
Use `move-fine` to opt into it; existing `move` and v1/v2 saves retain their
policies. Forward category 0/4 profiles use twelve owned samples from the frozen
scalar-type bank; the cycle-end event stream is explicitly supplied by the native
adapter. Reverse/frozen animation and type 12 are refused. Numeric inputs are bounded;
invalid rates, divisors, samples or more than 4,096 admitted substeps fail.

The driver checks every planned/saved edge's profile. Rebinding also verifies
rate/duration, directional displacement, sample-cursor consistency, residuals
and map identity before committing. Fine movement participates in transactional
ticks: callback failure retains the prior state; cleanup, release, blocked edges
and replacement orders discard the intra-cell continuation. New segment and
replacement orders reset to the last consumed grid cell with zero accumulator,
progress, residuals and sample index. That reset is a native policy; preserving
original interruption/speed continuity awaits further evidence. Z origin is
`gridZ*16`; terrain-dependent height offsets remain excluded.

Native v3 extends each motion record with the driver selection and optional
56-byte continuation. See [wire format](../formats/native-world-snapshot.md).
No pointer, floating-point integrator or external animation cursor is persisted.
Unbound/changed map resources and invalid profiles fail before publication or
session replacement. Native v1/v2 encoding stays byte-for-byte compatible.

## Validation

Reproduce original comparisons (requires the pinned working PE and i386 toolchain):

```sh
python3 tests/test-original-creature-motion.py
```

The runner verifies the immutable original manifest before and in a `finally`
block after the experiment, verifies the whole PE hash and entry anchors,
compiles an i386 executable and maps a disposable PE copy into its private
process. It redirects only animation event/reset and completion for the action
comparison. The separate consumption comparison restores the original helper
bytes, executes actual `0x512460` and `0x5070e0`, and redirects three environment/
animation callbacks. No installed binary or original input is patched.

455,640 action invocations match accumulator, progress, cumulative XY, fine XYZ,
residual XY, sample cursor and completion. Cases cover all eight directions,
vertical/planar steps, forced-32 arithmetic, zero/multiple admitted substeps,
nonuniform samples, cycle rollover, positive/negative/clamped height deltas and
completion with unused admitted iterations. 136 consumption cases cover every
valid cursor for route lengths 1..16, exact fine/grid snap, selected resets,
last-point flags and countdown decrement. Terrain records and unrelated behavior
are synthetic/disabled. The manifest verifies all 2,927 original files.

24 normal and 24 ASan/UBSan CTests pass. Leak detection is disabled because of
the traced execution environment; no leak result is claimed. New process checks
use an independent complete v3 byte oracle, an independently authored checkpoint,
17 uninterrupted versus 2+15 restarted ticks, animation-cycle/fractional-rate
140 versus 26+114 ticks, 16-point prefix replanning at 72 versus 40+32 ticks,
signed seam/diagonal positions, malformed states with recomputed checksums and
changed/missing map refusal. Native unit checks include rollback, failed in-place
restoration, cancellation and driver-selection preservation. Existing v1/v2 and
pathfinding/tooling regression checks pass. Artifact/source fingerprints and
commands are in [validation record](native-creature-fine-motion.json).

## Remaining boundary

Complete motion setup and speed continuity, original ANI event production,
reverse/special movement, terrain-height offsets, environmental/behavior/combat
callbacks, dynamic multi-creature occupancy and live timing agreement remain
open. The next motion chunk should compare segment initialization and successive
route-point speed/animation transitions before widening this driver. AI, spells,
campaign triggers and compatible original-save writing remain separate gaps.
