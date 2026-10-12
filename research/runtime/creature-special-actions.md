# Original special action callbacks (NS32)

The retained NS27 export `static-v10/special_action_callbacks.asm` matches
SHA-256 `8395ac533eb7886ea57dd0e33e5286bcd287a8854dd0a4b1f7f772320c5cc550`.
Whole action21/23/24 entries521790,5217e0 and521dd0 are newly registered.
Native implementation/comparison and real World/session integration are pending.

Static findings for the pinned bodies:

-21 polls an optional overlay and main animation. Event1 clears reverse722,
  consumes the segment, copies pending to behavior, and invokes stun20/0/0.
  There is no intervening active gate after segment completion.
-23 increments signed direction, updates facing, rebinds120 and polls/rewinds.
  It first attempts ascent at current XY if the footprint ceiling and occupancy/
  tile admission allow it. A71c gate suppresses relocation in that path.
- Failed ascent tries five randomly selected XY candidates at the current layer.
  The first draw uses Y dimension and feeds the X argument, the second uses X
  dimension and feeds Y; coordinate constructors normalize later queries.
  Class0/24/25/26 also descend both columns and require regional connectivity.
  Random/diagnostic and query order are part of the recovered contract.
- Relocation releases current and reserved occupancy, snaps/reserves, then runs
  environment, animation and terrain callbacks in order. The random relocation
  path gates inactive actors before flight/reset/engagement cleanup.
-24 polls/rewinds, records the dead-body predicate, tries a lower layer with
  movement-kind2/waterline gates, then temporarily clears104 for adjusted landing
  geometry. Dead-body relocation and live occupancy/snap paths remain distinct.
  Exact restoration/terminal branches are under review.

Keep actual occupancy/geometry/region/ANI, corpse relocation, health, action,
effect and native application ownership separate from outer callback recovery.
This work adds no gameplay balance changes.

The first paired run rejected the adjusted-landing ABI:52206a/522072 overwrite
the existing22-byte descriptor's movement kind at+12 with3 and selector byte
at+16 with1. They are overrides, not appended fields. The native service now
receives those explicit values and the fixture checks the full descriptor.
The failed original run and immutable-manifest checks are retained in dev1.

Ascent's class0/24/25/26 branch skips regional connectivity when its source region
lookup returns−1; it still accepts the relocation candidate. A missing destination
region retries. This asymmetric branch follows the actual521b2a/521bd8 targets.
Descending adjusted landing may return inactive before restoring104; the native
port preserves that ordering. Actual lower service ownership remains pending.

Corrected development execution passes24576 original32/native64/sanitized64
cases across the three complete bodies. The corpus includes non-square12/16 by
14/16 maps, eight layers, all28 type IDs, class0/24/25/26 region admission and
ordinary classes, signed direction rollover, overlay/main polls, reserved
occupancy, death/response/damage and both engagement-release targets. Lower
services remain ABI-checked supplied functions. Five native refusals and explicit
branch-presence checks are being added before prospective final validation.

Fresh prospectively bound finalv1 execution in
`original-creature-special-flight-20261012.json` passes24576 paired cases and
five native refusals. Sources/compiler/declarations stayed stable; original
manifest checks before/after and original body guards pass. Thirteen native
CTest suites pass. Branch counters include390 finishing-after-removal,2361
direct ascents,648 five-attempt paths,92 inactive-before-flag-restoration,2732
dead descents and832 engagement releases.

Ascent/descent bodies remain outside Ghidra discovered function ranges; two
explicit pinned recovered-range exceptions preserve that inventory gap. Their
anchors use the retained development execution, with fresh bound final execution
supporting the current scoped implementation/comparison. Integration/replacement
remainnone. Actual lower World/region/ANI/stun/health ownership, installed
construction and native session/portable continuation composition remain pending.
