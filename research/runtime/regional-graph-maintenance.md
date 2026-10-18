# Regional graph maintenance (NS74)

This contract is for No-CD build 40209ca7. It owns connection maintenance on
existing navigation nodes. Static graph construction, World startup finalization,
World danger production and live replacement remain independent milestones.
Findings below are based on pinned disassembly, read-only Ghidra exports and
bounded original-backed development runs. The prospective source-bound execution
report determines current comparison/integration status.

## Runtime boundary

The earlier 53f710 lead is a **startup** caller from470950. It appends map-supplied
points, calls 53c070 for each, reindexes/sorts existing connections through 53ee10,
adjusts nodes near authored region boundaries through 53fbc0, invokes the selective
53bb40(1,1) pass, computes map-point lookups and drains 5404a0. That entire startup
finalization is not implemented by this milestone. The 36 authored boundary
records and their movement/relocation semantics remain partial discovery.

The actual runtime step is 5404a0. World 46afc0 calls it at 46b7bb after the ordinary
creature/owner passes when regional navigation is enabled; another guarded mode
calls it ten times at 46b7d4. The helper itself returns immediately while global
6e1f48 is nonzero. Native normal frames take one maintenance slice after their
existing creature/environment pass. This is headless composition, not recovery
of the rest of World scheduling, alternate modes or pause behavior.

Terrain-edit callers 4eb040/4eb490/4eb8a0 compute4f49e0 before/after signatures,
classify them with 540430 and enqueue540140 unless the classifier returns-1.
Other producers include authored effects and 4f35d0's structural flag removal.
Native cell editing is an explicit input policy; it does not claim those full
spell, destruction, object, effect or map-editor operations.

## Recovered algorithms

- **4f49e0** returns three integer booleans: legal height 1/width 1 ground at the
  cell, *failure* of a height 1/width 1/free/kind 3 probe, and ground legality one
  layer above when present. All probes have special-byte 1. The third value is
  a legality bit, not a measured terrain height.
- **540430** chooses general mode 0, opening mode 1, closing mode 2 or-1 for no
  repair. This is an exact six-boolean decision tree; the tests enumerate all 64
  combinations. Native names describe maintenance policy, not complete spell
  semantics.
- **540140** uses the recovered lower-bound helper on wrapped `X-4` and `X+5`,
  including its inclusive endpoint/wrapped scan behavior. It admits nodes with
  `2*max(abs(dx),abs(dy))+min(abs(dx),abs(dy)) < 10`. Priority is the original
  wrapped three-axis4f4b30 metric. The 1345-entry ring stores node/mode/priority.
  Duplicate queued nodes merge unlike modes to 0 and retain the smaller priority;
  a decrease bubbles through the ring with the original swaps. Full rings drop
  new entries. An active node may be queued again because only queued entries
  participate in duplicate detection. Popped slots retain their old mode and
  priority while their node becomes-1.
- **5404a0** pops at most one source per call, scans destination IDs in order,
  and starts each call with 10 work units. Wrapped XY distance must be below 9.
  A fresh pair whose endpoints disagree on height 1/width 1 **ground** legality
  is rejected beyond distance5. Resuming a pending pair bypasses that fresh
  admission check. Maintenance changes connections without sorting or replacing
  node IDs.
- **53c070** is the synchronous local reconnection helper. It uses the same
  ground-transition admission and calls 53c320 with a fresh descriptor for each
  admitted pair. It adds/updates successful bilateral connections; unlike queued
  repair, its failed attempts do not remove an existing connection. Static
  53bb40 uses kind 3 transition admission, so those helpers are not interchangeable.
- **540820** visits width 2 then width 1 for ground, free, kind 3 and kind 1 groups.
  Kind3/kind 1 stages require a nonzero water boundary. Existing ground limits
  seed alternatives; the maximum-height ground fast path fills all categories.
  A fresh general/closing pair clears its working descriptor; opening mode starts
  with the previous descriptor. Closing mode restricts probes to previously
  admitted heights. Stage-entry copies repeat on a resumed call, including the
  kind 1 lower-bound initialization. No descriptor is published while its pair
  remains unfinished.
- **5416d0** keeps a height cursor, probe limit and generic search frontier.
  Previously admitted probes receive 125 units; new ground probes41, alternatives 31.
  The caller's slice budget also limits work. Greedy success does not debit the
  caller's budget. Greedy failure plus generic expansion does, including the
  recovered discarded final expansion. If local work expires while the probe
  still has allowance, the pair resumes next frame. Crucially, a zero caller
  budget selects the special125-unit mode **once at helper entry**, retaining
  that choice across later height probes even if the caller's budget changes.
  The tested global 6f1938 adjustment is zero. The unused helper category 3
  underwater-height branch is not claimed by the reachable540820 stage model.
- On completion, **540820 publication** updates both directions, or removes both
  when no cost remains. Existing matching slots precede empty slots. When a new
  ground-capable edge meets full storage, each endpoint may evict its highest-cost
  non-ground connection and clear that connection's reciprocal slot. Publication
  can still fail after one endpoint has evicted a slot if its peer has no eligible
  slot; native code preserves that order rather than making this inner operation
  artificially atomic. Traffic and danger rows remain separate.

`reconstruction/creature-ai/regional_maintenance.*` stores explicit queue, stage,
height, working/previous descriptor and probe state. The native geometry binding
supplies a resumable generic descriptor search with canonical cell IDs. No host
pointer or legacy container representation enters a checkpoint.

## Retained paths and searches

Connection repair does not sweep actors to clear their regional paths and does
not reset the separate regional graph search. Cached node IDs remain meaningful.
A later route/motion operation encounters the changed geometry through the
existing recovered checks. An order that already failed need not restart merely
because a passage reopens. This milestone introduces no automatic retry rule.

The helper comparison suspends an original/native regional search with budget 1,
retains an actor path, edits terrain and runs maintenance through completion,
then resumes the same search with budget 53. Actor paths and original actor bytes
must stay unchanged by maintenance. Coupled journeys close/reopen the only wall
crossing while movement is active. A declared ordinary move command at tick 1536
then verifies use of the repaired graph. Goal-frame counts in results are frames
at the destination, not distinct arrival events or claims of automatic retry.

## Native editing and persistence policies

`editNativeTerrain` accepts up to 4096 ordered cell edits on a configured native
regional graph. It validates canonical coordinates, terrain kind indices,
16-bit mask/value containment and preservation of actor reservation bits 0/1.
It applies before/after signature classification and enqueueing sequentially on a
private continuation, then validates and publishes the batch. A failed later edit
rolls back the whole batch. `encounter::editTerrain` preserves the same transaction
at the session boundary. Kind/terrain flags are explicit caller inputs; auxiliary
objects, occupants and spell/destruction consequences are not inferred.

CMD1/v8 adds optional maintenance state and the retained 28 normalized height/
width/movement-kind definitions. Canonical tokens, ring/cursor bounds, descriptor
references, probe frontier/record bounds and source definitions are checked.
Versions 1..7 remain readable; an explicit older write refuses to discard enabled
maintenance. A v7 graph resumes without enabled maintenance; this milestone does
not silently rebuild or migrate its original startup configuration.

```sh
native-encounter run-regional INSTALL X Y WIDTH HEIGHT SEED TICKS NEW_SNAPSHOT
native-encounter edit-terrain SNAPSHOT X Y Z KIND MASK VALUE NEW_SNAPSHOT
native-encounter resume SNAPSHOT TICKS NEW_SNAPSHOT
```

CLI editing/resumption uses retained inputs and continuation bytes in a fresh
process. It requires no installed asset access, Wine or Qt after initial loading.
The existing1MiB CMD1 byte bound and 4096-region admission remain native policies.

## Validation boundaries and reproduction

Use `tools/test-regional-maintenance.py` under the working-storage guard, with
prospective claims from `tools/draft-coverage-claims.py`. Four helper fixtures
cover installed 32x32x23 Celtic Plains terrain, boundary 0/6, a wall opening,
512 signatures each, all 64 classifiers, local reconnection, repeated edits during
pending work, blocked slices, retained actor paths, interleaved regional searches,
checkpoint restoration and malformed/refused inputs. A separate declared 1500-node
queue-only fixture exercises full1345-entry storage, drops, mode merging, priority
decrease and ring wrap; it does not claim native 1500-node checkpoint admission.

Four paired journeys use independently constructed original/native graphs, two
seeds and 2048 ticks, comparing existing actor/ANI/motion/route/shared-budget/R250
projections, every terrain/occupancy cell, every connection slot, the complete
queue and maintenance cursors each tick. Native 64-bit and ASan/UBSan/LSan64 traces and
final snapshots must match. In-process checkpoints and fresh-process chains span
edits, pending maintenance and the later move command. Static graph construction,
regional navigation, static generated journeys, command-session and installed
encounter regressions run separately. CLI edited-checkpoint continuation and
refusal are also exercised.

Raw discovery exports remain under working/tests/regional-repair-ns74-export-v1
through-v6. An early v2 export used a relative Ghidra output path and failed;
subsequent exports use absolute paths. Development comparisons and failures are
retained separately; they are not prospectively bound execution evidence.
All original inputs remain read-only and manifest-verified before/after use.
No new binary function boundary has been inferred by this milestone.

The three 5416d0 category tables at 5421e4/5421f8/54220c are registered separately
for cache, allowance and success handling. Values 0/1/2/4 map to this scoped
maintenance contract; category 3 and out-of-domain selectors remain explicit gaps.
The startup orientation table at 540130 is linked only to partial finalization
research. These mappings retain all existing unknown binary ranges/flows.

## Recorded execution

The [immutable NS74 report](regional-maintenance-bound-ns74-20261015.json) passes
198,447,618 helper checks across four fixtures and 7,495 maintenance slices.
Original 32-bit/native 32-bit journeys pass 16,384 ticks over four gate scenarios
and two seeds. All eight reach their declared destinations; 32 edit events include
10 edits overlapping retained actor routes, and 836 frames change connections.
These are bounded headless results, not full World/campaign or live replacement.

Native 64-bit and ASan/UBSan/LSan64 traces and final snapshots match through
2,112 journey restores (2048 in-process and 64 fresh-process steps).
The helper fixtures additionally round-trip their continuations, retain actor
paths and resume an independent regional search after terrain repair. The full
1345-entry queue tests exercise overflow, mixed modes, priority decreases and wrap.
Eleven static graph fixtures, 258440 regional helper checks, five 1024-tick static
generated journeys, both five-test command suites and both installed encounter
regressions pass. CLI edited checkpoints resume identically; refused edits publish
no checkpoint. All 254 prospectively declared sources and
246 compiler/configure inputs, installed artifacts and 2927 immutable
originals remain unchanged. Original executable text remains guarded.

The first source-bound run was deliberately interrupted to correct fixture setup:
calling full regional initialization before repositioning actors left duplicate
reservations. The final fixture builds the graph before reserving its selected
actor positions. The interrupted report remains failed and retains its exact
superseded fixture under retained-sources; no historical hash is refreshed. Only
v2 supplies the published execution proof. A daemon restart later orphaned the
validation shell without stopping its children. The process/source/claim state was
checked before continuing, and free-space monitoring was restored for the existing
process group. Recovery/storage logs are retained in the main checkout's
working/retention directory; completed comparisons were not restarted.
