# Static-object terrain lighting and two-phase publication

## Evidence and confidence

Confirmed within the bounded fixture for no-CD Chaos.exe SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The isolated PE32 fixture executes the unmodified routines at **0049cf60**,
**004ff700/004ff820**, **004f1c60**, **004f2ad0**, and the combined outer updater
**004f27a0**. No original executable or installed input is patched. The
[comparison report](terrain-static-lighting.json) records source/helper hashes,
byte/state counts, manifest checks and normal/sanitized validation.

Reproduce with:

```sh
python3 tools/test-terrain-static-lights.py
```

The runner pins the executable hash before mapping it, verifies the original
manifest before/after (including failures), and compares the same deterministic
fixture in original/native 32-bit, native 64-bit, and native 64-bit ASan/UBSan
builds. Every tick compares all five guarded light buffers and six static-cycle
state values. The combined cases execute the actual outer updater, including
its final changed-flag clear; the native fixture invokes creature then static
models in that order with shared controls. The scoped APIs have no Qt, Wine,
application-widget or native gameplay-policy dependencies.

The 128 fixtures compare 3,072 ticks (including 1,536 combined ticks),
32,768 independent admission checks, 4,096 additive stamps, 104,160,840
five-buffer bytes and 18,432 static state values. All three builds match.
The full suite passes all 67 tests in normal and ASan/UBSan builds; both
manifest checks verify all 2,927 original files. The previous creature-only
comparison also passes all 2,304 ticks against the current field code.

The [lighting accounting package](terrain-lighting-coverage.json) contributes
stable behavior/evidence IDs to `coverage/register.json`, preserving other
subsystem records. The lighting package passes an audit with `--require-fresh`;
the full shared audit retains unrelated historical movement-source staleness.
The shared [inventory and accounting workflow](coverage/README.md) supplies
the pinned binary inventory used by these lighting records.

## Static object admission

The static collection at **00689498** contains a record pointer, capacity at
**0068949c**, and count at **006898dc**. Records have stride **0x22e** and require
nonzero DWORD **+4**. DWORD **+28** is a type/table index. **0049cf60** returns
`DWORD(collection + 12 + type*12)`, used as the light index; zero skips the
object. Installed type-table creation and semantic names remain unrecovered.
The native API consumes the selected owned light index rather than a host
pointer or the build's global table.

The recount and visitation predicates are distinct. Recount passes record
**+1c2,+1c6** to **004ff700**; visitation passes position **+8,+c**. Stamping
also uses layer **+10**. The native object stores both coordinate pairs and
retains stale recount behavior when the two predicates disagree.

**004ff700** constructs a source square from position minus `index/2`, with
upper endpoints equal to lower endpoints plus **index**. The reference square
uses region object **00689930** center **+39,+3d** and extent **+59**, halved
with unsigned shift. Both rectangles pass through **004ff820** using global
map width/height **006c5494/006c5498**:

1. If an upper endpoint is strictly greater than its period, subtract the
   period from both endpoints along that axis.
2. Split negative lower endpoints into one, two, or four rectangles; split
   endpoints include the period itself, not period minus one.
3. Admission requires either source X endpoint to fall inclusively inside the
   reference X interval and either source Y endpoint inside its Y interval.

This is not a general symmetric rectangle-intersection test. A source interval
that contains a narrower reference interval can be refused. The original also
**does not reset its reference-fragment iterator for subsequent source
fragments** at **004ff7f9**. Only the first source fragment reaches endpoint
comparisons; the model preserves this quirk. Neither behavior has been
"corrected" into a new lighting policy.

## Additive stamp

**004f1c60** uses the same recovered distance kernels as creature stamping,
but modifies only prior/staging **map +7d0**. Each destination update is signed
`clamp(old + kernel + 127, -127, 0)`, with no ambient cap and no base/work write.
The reflected diagonal signs `(+x,+y)` and `(-x,-y)` always receive an addition.
The two cross signs are added only when both X and Y offsets are nonzero.
Negative Z is admitted only at nonzero Z offset. Thus the central XY axis
receives two additions; wrapped aliases also accumulate. Signed additions and
saturation are preserved, including overlap between objects.

Light indices **2..33** select extent `index/2 + 1`, giving two adjacent indices
per kernel extent **2..17**. The index controls admission-square width separately
from stamp extent, so paired indices share stamp bytes but need not share
admission. Index **0** means no light. Index **1** selects the special size-one
entry; its metadata/kernel-storage overlap remains outside this reconstruction.
Higher indices are refused by the owned API.

## Two-phase cycle

**004f2ad0** is enabled by map **+80c**. Its global state is:

| Global/field | Owned state |
| --- | --- |
| 006e8da0 | phase, 0 or 1 |
| 006e8da4 | remaining eligible count |
| 006e8da8 | quota |
| 006e8dac | next slot cursor |
| map +642a | active |
| map +6426 | requested |

The original light-field initializer **004ecdd0** sets active/requested to one;
the native static cycle starts with the same flags. When inactive, the routine
recounts eligible objects. If inactive, with no eligible objects, no request
and no changed flag, it returns without advancing phase.

At phase zero or any nonzero shared changed flag **005e41a8**, prior is refreshed
from published **+7cc** when **+642e** is nonzero, or target **+7d4** otherwise.
The routine sets active, recounts, sets requested to whether any sources were
counted, and resets the cursor. The normal quota is unsigned
`(remaining + 1) / (2 - phase)`. Exactly changed=1, with remaining nonzero,
forces phase one and quota=remaining. Other nonzero changed values restart
without that forced completion. Only admitted stamps consume quota, advancing
remaining and cursor as in the creature cycle; unsigned stale-count wrap is
retained.

Phase increments modulo two. On wrap, prior is copied to published and active
is cleared. Requested remains as last recounted; the creature updater uses
these flags to decide whether its next field goes to published or target.
Static publication leaves the creature updater's **+642e** flag unchanged.
**004f27a0** runs creature then static updates and clears changed afterward.
The combined fixtures verify this order and sharing. This routine performs
batched additive publication; it does not establish weighted interpolation.

## Boundaries and next step

Owned admission accepts in-bounds centers, extents no greater than the smaller
map dimension, light indices 2..33 plus zero, valid stamp layers, and capacities
up to 65,536. Scan count cannot exceed owned capacity: the original recount
walks the count without a capacity guard, though visitation does check capacity.
The native model validates active light records before mutating state/field.
These rejection rules are deliberate native bounds, separate from original
behavior on malformed pointers, unsupported indices or coordinates.

Original initialization, allocation failures/reloads, special index one,
installed static-type light-table construction, production of the two object
coordinate pairs and region center/extent, spectator/sentinel creature paths,
and other smoothing or weighted-interpolation functions remain unvalidated.
No entity capture, preview CLI integration or live replacement is claimed.

Next is an owned composition interface and controlled object-fixture input to
produce per-cell lit terrain frames through the existing Qt/OpenGL renderer.
That can validate combined source updates and scene presentation offline before
recovering installed object/table production or introducing live observation.
