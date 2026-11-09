# Owned copy routing safety — 2026-11-09

The **unsafe-overlap routing obligation is closed for the reviewed owned 2D copy contract**: every direct GPU COPY uses distinct canonical storage; shared storage takes the established self-copy route or refuses. A rectangle-intersection detector is unnecessary for this safety property. Storage identity checks remain necessary.

This is an architectural invariant with controlled execution evidence. It does not assert that the game never requests overlap, establish original traversal semantics, or promote either final rendering decision. No new acceptance row, original game, player capture or scene fixture is added. Existing accepted rows retain their original execution phases.

## Ownership and case partition

Within one renderer/session, each native surface has a private texture allocation. Canonical aliases retain one ID. Native provider interfaces likewise resolve to one owned `NsState`; its dual CPU views share that state's private allocation. Guarded Flip and native content swap permute backing rather than duplicating it. Unknown identities refuse admission. This proof assumes the existing owner/lifetime/copy guards and excludes arbitrary external memory/texture adoption, raw foreign GL mutation and unguarded lifetime-mode0 provider calls.

For any admitted source and destination there are two cases:

1. Different canonical storage: allocations are disjoint, so the direct copy cannot read its target.
2. Same canonical storage: use the existing frozen/ordered temporary route, or refuse before submission. This is conservative even when the rectangles themselves do not intersect.

The GPU emitter cross-checks `self` against identity equality, requires scratch distinct from both operands, freezes before the first self-copy piece, and emits only different source/target IDs. Identity validation uses zero/equality rather than arithmetic on IDs. The existing C control now exercises all 128 combinations of three IDs drawn from zero plus three distinct representatives and both self flags: **12 admitted routes, 116 refusals, no emitted in-place COPY**. These representatives cover every equality relation among the operands and zero. The existing maximum-256-piece and publication-failure controls remain in force.

## Reviewed entry families

The [source-bound report](copy-routing-safety-results-20261109/routing-03.json) pins the complete reviewed inventory, compiler project closure, tested binaries, inputs, logs and exact claim. Review covers:

| Entry family | Safety rule |
| --- | --- |
| Native allocation, clone, restore, context recreation and swap | Private allocation, identity refusal, or a backing permutation |
| `SurfaceBackend` aliases and recovered wrappers | Canonical ID retained; unknown/foreign IDs refuse; wrappers delegate |
| Primitive and masked copy, composition | Source/target and mask/target aliases refuse; dependent effects use separate scratch |
| Surface copy | Same-ID opaque/ordered-keyed temporary path within existing flags/budgets, otherwise refusal |
| Coordinate copy and prepared-frame callers | Guard every sampled input; retain the existing safe coordinate self-copy route |
| CPU uploads, bitmap reload, local-image and additive helpers | CPU input or separately allocated scratch, with no native target sampler |
| Presentation, GPU frame snapshot/import and mapped assembly | Separate frame/native allocations; owned reusable leases; native input/target guards |
| Command decoding and isolated dispatch | Validate owned identities and converge on the guarded renderer methods |
| GPU canvas mirror | Fresh canonical allocations; unknown IDs refuse; delegate to surface copy |
| Guarded native Blt/BltFast and CPU draw | Resolve owned interfaces; freeze shared CPU source; legacy direct GPU service refuses self |
| Native GPU/CPU plans | Derive self from owned state identity; CPU plan is preparation only |
| CPU bridge | Refuse ordinary self fast admission; use owned snapshots/checkpoints |
| GPU bridge and emitter | Cross-check canonical identities before publication; distinct scratch for shared source |

## Guard repaired and checked

`GlBlitter::copyCoordinates` already routed an opaque coordinate-source self-copy to `surfaceCopy`, and refused its keyed variant. Its other branch could still bind the destination as the pixel-input sampler. The helper is private but reachable through the production process dispatch. The new guard refuses that remaining pixel/target alias before drawing. The same helper now checks origins with subtraction-based bounds, preventing signed overflow before admission. `NativeFrameRecipe::ensure` already creates coordinate planes separately from original pixel handles, so supported prepared-frame calls preserve their routing.

The existing persistent-surface test exercises the real dispatch both locally and in the production worker: eight mapped/keyed alias refusals preserve both complete input planes and copy counters; eight positive cases retain safe self-copy ordering, keyed destination history and unmapped coordinates. Six maximum-int origin cases refuse before drawing and preserve both inputs. The existing 192 ordered surface cases, prepared-frame controls, 354 command-consumer frame comparisons and 408 retained clipping operations pass. The initial clipping invocation without its required stream refused setup; the successful run uses the existing hash-checked 408-case input. No original collector is renewed.

The first regression draft incorrectly called the private helper, and the next used an incorrect statistics type; both compile-refused source phases and diagnostics remain held. The passing regression uses the actual public dispatch and production worker transport. Temporary build intermediates and caches are in RAM; exact tested executables and source/configuration are SQLite-retained.

## Remaining boundary

Natural overlap is no longer a witness that must be hunted to establish **owned-copy memory safety**. Original caller output, keyed traversal, partial-error semantics, raw readers/writers, physical/external aliases and the wider `SC.natural-callers` decision remain independent obligations. The CPU facade's whole-call frozen policy and the backend's per-piece/ordered policy are not equated by this proof. In particular, one-pixel scratch reuse during batched keyed self-copy requires separate ordering validation: flush-before-deletion preserves separate storage, while queued sampling may still affect output ordering. No current combined-player or physical-driver execution is claimed.

The dispatch review also notes a separate resource boundary: malformed CREATE/ALLOCATE/CLONE identity checks follow temporary allocation/extraction. Invalid-creation cleanup needs its own lifetime review; this change does not claim that behavior complete. A future API admitting externally shared storage must establish canonical backing identity before it can consume this proof. Existing accepted journey matrices are not reopened merely to search for a natural self-copy.

The first controlled phase remains at `routing-01.json`; origin hardening is separately bound by `routing-02.json`. `routing-03.json` pins the admitted actual-master union and fresh affected controls; every old report/hash stays immutable. Reviewed incoming changes preserve private allocation/routing, and current merged controls verify the conjunction without another player/original run.
