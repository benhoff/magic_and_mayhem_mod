# Actor downstream service recovery

The72 direct actor service/helper entries have explicit bounded suppliers:
71PE32 value-address service models and the existing source-controller ownership
model. This is completion of the direct-entry inventory, not completion of the
transitive service graph or supported-session/live replacement.

The original [service matrix](downstream-service-results-20261021/service-matrix.json)
is preserved as the initial discovery state. The
[extended frontier review](downstream-service-results-20261021/service-matrix-extended.json)
accounts for every one of its83direct targets:35now have explicit raw service
bodies (34previously unlinked entries plus widget invalidation),43retain existing
scoped suppliers requiring caller-specific composition, and five are named CRT
boundaries. Seven constructor/canvas/compiler helper bodies also have dedicated
comparisons. Fourteen indirect call sites retain interface and ABI boundaries.
Entry coverage does not establish transitive session equivalence.

The2026-10-22 [service composition follow-up](actor-service-composition-20261021.md)
adds34concrete suppliers and a common149-entry router. Its21caller roots include
64preloaded activation-to-release/cleanup sequences;10,688current body/caller
and regression comparisons pass. The initial matrix above remains historical.
Full source registration, resource/font/scenario/attack providers, arbitrary
session ownership and live acceptance retain explicit boundaries.

## Implemented and compared

| Supplier | Direct entries | Original/native cases | Representation and boundary |
| --- | ---: | ---: | --- |
| `RS.actor-downstream-value-services` |42|2688|Complete owned bytes/globals, scalar and animation state, mutable lists, gauge pixels, vector construction and native exceptional prefix cleanup, audio query/retirement composed with actual scheduler lookup/retirement. External allocation, secondary and decoder providers stay explicit. |
| `RS.actor-downstream-controller-services` |10|640|Singleton publication, animation start/reverse orchestration, session cleanup, owner-dependent profile selection, task reset/action/cleanup and owner counts. Includes actual action dispatch/address suppliers; further controller/widget/constructor providers remain distinct. |
| `RS.actor-downstream-world-services` |9|576|Peer repair, map attach/detach, secondary release, facing, item/terrain/hazard dispatch, task targets and x87 valuation. Reuses existing raw occupancy/reference models and composes value/controller suppliers. |
| `RS.actor-downstream-ui-services` |10|640|Packed22-byte admission descriptor, health rectangles, secondary submission, popup/selection/message behavior, resource-bank ownership/failures and actor reporting. Stack records compare by content at named callbacks; platform/format/loader/widget bodies remain explicit. |
| `RS.actor-downstream-dependency-services` |18|1152|Animation scanning, owner selection, widget invalidation, deferred rectangles/localization, pixel fill, audio stack removal and heap retry/tagged free. Heap internals and widget virtual methods remain explicit. |
| `RS.actor-downstream-popup-services` |9|576|Health/icon redraw, popup reset/admission/overflow, focus, quota enable/disable, borders and scroll text. Composes dependency bodies; concrete raster/font providers stay explicit. |
| `RS.actor-downstream-collection-services` |4|256|Vector insertion/growth and aliasing, secondary admission/reclamation, original experience thresholds and scenario-event dispatch. Composes heap/world/value/controller suppliers. |
| `RS.actor-downstream-effect-services` |3|192|Item traversal/consumption, fastcall spell removal and explosion terrain/particle/list cleanup. Concrete effects/contributions/particle providers remain explicit; uninitialized original contribution branch is indeterminate. |
| `RS.actor-downstream-construction-services` |7|448|Widget and canvas initialization/retirement, reverse vector destruction and the cold finally body with an explicit parent frame;171native exception combinations verify completed-prefix cleanup. OS SEH delivery is separate. |
| `AI.source-controller-work` and `AI.owned-source-controller-chain` |1shared entry|8192each|Existing logical grouping/traversal/root/RNG/coordinate model and registration composition, each checked against original and sanitizer. Successful supplied allocation/vector domains and opaque byte25 representation limits remain unchanged. |

The nine service matrices compare7168original/native cases across112distinct
selected bodies. The native vector exception test adds171failure/completion
combinations. Earlier DSVC1results are preserved; current DSVC2reports carry the
final compiler/source closure and prospective scope identity. Every case compares complete selected
owned storage and globals, defined returns and ordered callback arguments;
callback-time storage also contributes a32-bit FNV state fingerprint. Named
original stack buffers compare their full declared contents; native scratch
storage is excluded from persistent ownership snapshots. Twenty-four cases compare
matching access/division fault outcomes and state at the fault (3value,10controller,
5world,1UI,5collection); these are original observations, not safer native policies or Windows
exception-unwind equivalence.

`tools/test-actor-services.py` derives root ABIs from the pinned original inventory
and binary, generates checked provider entry/IAT substitutions, and guards all
selected/reused original body bytes before installation and after execution.
Native models use independently seeded storage and provider inputs; original
outputs never feed native calculations. New evidence IDs begin `DSVC2.`; historical
reports and their hashes remain unchanged. Raw source variants, failed attempts,
decompilation and comparisons remain pinned under
`working/tests/downstream-services-20261021/`.

## Confirmed calling-convention repairs

At original51f7ad, the actor argument is pushed before the zero-argument463cb0
singleton lookup and remains for464030's `thiscall` removal (`RET4`). The visual
fixture previously split this as463cb0popping4 and464030popping0, masking the
mistake through collective stack balance. The native lifecycle and its provider
ABIs now represent the actual two calls.

Activation's5057b0selector is `fastcall`: ECXholds type and EDXholds owner. Both
native actor models now preserve the owner, and the independent/runtime fixtures
log it. The runtime backend has an explicit two-register call rather than a
thiscall approximation. All47520independent actor cases and192composed
original/native/runtime lifecycle cases pass after the repairs, including7138
body-byte refusal mutations,13protection refusals and13ownership refusals.

Instruction review also confirms signed/unsigned boundaries and actual receivers:
466870uses the supplied owner table and associated index at+0c;514e40uses the
chosen actor;465130uses actor+79c; gauge fill5338c0uses the embedded image at+1c.
Spell cleanup469000receives the spell slot in EDX; secondary cleanup49e170removes
only matching payload owners;52d0c0walks to the sentinel rather than retaining the
last payload. These details are implemented and compared in the named matrices.

Rectangle forwarding58c8a0and deferred58d5b0also receive the destination in ECX.
The health and panel services now retain their embedded destination objects rather
than losing the receiver. Scroll text4d9f30initializes only byte0 of a local DWORD;
4a5ce0consumes that argument as a byte (confirmed by instructions/decompilation),
so the comparison excludes the three uninitialized padding bytes only.

The item body468470contains a baseline indeterminate read: if neither healing
branch assigns its local contribution, it still uses that stack word. The native
model rejects this undefined input branch; it is an explicit boundary, not a
claimed deterministic recovery or gameplay balance change. The original health
and veterancy formulas are retained.

Constructor evidence includes464350,463270,4a4df0,4a4f70,58b3e0,59bf87and59c4ab.
The original cold cleanup body executes directly using a supplied parent EBP frame.
Its completed-prefix count and reverse destructor order are compared; a native
throwing-constructor test verifies cleanup and propagation without treating Linux
callbacks as Windows SEH dispatch. Destructors throwing during unwind and actual
OS exception delivery remain separate.

## Remaining work, kept separate

The broad `RS.actor-downstream-services` contract stays partial. The direct-entry
work does not promote these outstanding requirements:

- Compose the43existing scoped supplier leads and the newly recovered bodies
  through the complete actor/session lifecycle. Existing address links, bounded
  body proof and owned downstream providers do not supply that integration.
- Resolve the actual concrete raster/font/resource/scenario/particle/provider
  paths exposed by the expanded bodies, preserving new call frontiers and indirect
  targets. Five CRT services (sprintf/fopen/fclose/fprintf/memmove) are named runtime
  boundaries; the last uses native std::memmove. No generic callback is counted as
  recovered engine behavior.
- Validate Windows SEH delivery, throwing during cleanup and the explicitly
  indeterminate item contribution branch independently from normal cleanup bodies.
- Validate actual external buffer/resource publication, heap/provider failure,
  arbitrary concurrent ownership and supported-session/live replacement. The
  supplied legacy DirectDraw runtime still has the independently retained Wine
  failure; the user has no legacy Windows host. These facts do not block local
  engine recovery, and no physical-driver equivalence is claimed here.

## Reproduce

Run original-consuming commands through the storage guard, with a new retained
output directory for every attempt. The test tool fingerprints the complete
compiler closure and prospective behavior identity before execution, verifies
original input before and after, and rejects mixed-source results.

```sh
python3 tools/working-retention.py run --reserve-gib .3 -- \
  python3 tools/test-actor-services.py --suite values --output working/tests/service-values-new
```

Repeat with `controllers`, `world`, `ui`, `dependencies`, `popups`,
`collections`, `effects` and `construction`. The existing source suppliers use
`tools/test-creature-source-controller.py` and `tools/test-creature-source-owned.py`
with prospective claims from `tools/draft-coverage-claims.py`. Static inspection and
reconciliation use `tools/inspect-actor-downstream.py` and
`tools/reconcile-actor-downstream.py`; their results grant no execution milestone.

Current master work started at a2f87f21. The subsequent3abf6e2dcanvas initializer
commit was reviewed separately; raw encoded artifact transitions and decoded
execution identities remain distinct. The prior26274d12closeout lies in another
Git lineage, so its934-commit rev-list difference is not treated as incoming work
from this turn. See the retained incoming-history and resolution reports.
