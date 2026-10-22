# Actor downstream service recovery

The72 direct actor service/helper entries have explicit bounded suppliers:
71PE32 value-address service models and the existing source-controller ownership
model. This is completion of the direct-entry inventory, not completion of the
transitive service graph or supported-session/live replacement.

The immutable [service matrix](downstream-service-results-20261021/service-matrix.json)
retains every entry, supplier scope, implementation and execution link. The
further frontier contains83direct targets:44have scoped native links requiring
semantic/composition review;39have no explicitly registered implementation link.
Fourteen indirect call sites retain their interface/ABI boundaries. None of these
counts establishes whole-function or whole-engine completion by association.

## Implemented and compared

| Supplier | Direct entries | Original/native cases | Representation and boundary |
| --- | ---: | ---: | --- |
| `RS.actor-downstream-value-services` |42|2688|Complete owned bytes/globals, scalar and animation state, mutable lists, gauge pixels, normal vector construction, audio query/retirement composed with actual scheduler lookup/retirement. External allocation, secondary and decoder providers stay explicit. |
| `RS.actor-downstream-controller-services` |10|640|Singleton publication, animation start/reverse orchestration, session cleanup, owner-dependent profile selection, task reset/action/cleanup and owner counts. Includes actual action dispatch/address suppliers; further controller/widget/constructor providers remain distinct. |
| `RS.actor-downstream-world-services` |9|576|Peer repair, map attach/detach, secondary release, facing, item/terrain/hazard dispatch, task targets and x87 valuation. Reuses existing raw occupancy/reference models and composes value/controller suppliers. |
| `RS.actor-downstream-ui-services` |10|640|Packed22-byte admission descriptor, health rectangles, secondary submission, popup/selection/message behavior, resource-bank ownership/failures and actor reporting. Stack records compare by content at named callbacks; platform/format/loader/widget bodies remain explicit. |
| `AI.source-controller-work` and `AI.owned-source-controller-chain` |1shared entry|8192each|Existing logical grouping/traversal/root/RNG/coordinate model and registration composition, each checked against original and sanitizer. Successful supplied allocation/vector domains and opaque byte25 representation limits remain unchanged. |

The four new matrices compare4544cases. Every case compares complete selected
owned storage and globals, defined returns and ordered callback arguments;
callback-time storage also contributes a32-bit FNV state fingerprint. Named
original stack buffers compare their full declared contents; native scratch
storage is excluded from persistent ownership snapshots. Nineteen cases compare
matching access/division fault outcomes and state at the fault (3value,10controller,
5world,1UI); these are original observations, not safer native policies or Windows
exception-unwind equivalence.

`tools/test-actor-services.py` derives root ABIs from the pinned original inventory
and binary, generates checked provider entry/IAT substitutions, and guards all
selected/reused original body bytes before installation and after execution.
Native models use independently seeded storage and provider inputs; original
outputs never feed native calculations. New evidence IDs begin `DSVC1.`; historical
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

## Remaining work, kept separate

The broad `RS.actor-downstream-services` contract stays partial. The direct-entry
work does not promote these outstanding requirements:

- Review and compose the83further targets against their exact native scopes;
  recover the39entries with no explicit implementation link where they are engine
  behavior rather than a justified CRT/platform boundary. Keep unresolved indirect
  widget/constructor targets and unassigned flows visible.
- Recover and compare actual exceptional construction/destruction and MSVC unwind
  ordering.59c441's normal construction is supplied; its exceptional destructor
  path,463cb0's464350constructor and463270destructor remain distinct.
- Compose the recovered service bodies with complete actor/session ownership and
  replace the remaining owned downstream providers in the full lifecycle fixture.
  Isolated body comparisons do not establish that integration.
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

Repeat with `controllers`, `world` and `ui`. The existing source suppliers use
`tools/test-creature-source-controller.py` and `tools/test-creature-source-owned.py`
with prospective claims from `tools/draft-coverage-claims.py`. Static inspection and
reconciliation use `tools/inspect-actor-downstream.py` and
`tools/reconcile-actor-downstream.py`; their results grant no execution milestone.

Current master work started at a2f87f21. The subsequent3abf6e2dcanvas initializer
commit was reviewed separately; raw encoded artifact transitions and decoded
execution identities remain distinct. The prior26274d12closeout lies in another
Git lineage, so its934-commit rev-list difference is not treated as incoming work
from this turn. See the retained incoming-history and resolution reports.
