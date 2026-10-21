# Native world cell draw production

The complete No-CD `0x4f8960` producer now has a native implementation in
[world_cell_producer.hpp](../../reconstruction/rendering/world_cell_producer.hpp).
It produces the original packed World rows directly; it does not call the
original cell producer or row builder. The runtime entry is opt-in and uses the
existing live world/resource owners through explicit services.

Original input is No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The recorded function body is `[0x4f8960,0x4fbcb1)`. The adapter additionally
checks adjacent switch data through the next recorded entry at `0x4fbe00`, for
13,472 guarded bytes total. This read guard does not expand the discovery
inventory's function boundary or classify every intervening byte as code.

The original implementation and comparison remain separate from the existing
selected `RE.terrain_submission` helper and the independently implemented native
scene/source snapshot application. No earlier evidence fingerprint is refreshed.

## Supplied behavior

| Behavior ID | Native implementation |
| --- | --- |
| `RS.world-cell-terrain-production` | Ordinary concealment/cut/player admission, inclusive frame-index fallback, absent-layer flag writes, tile hit publications and depth selection, water gate/counter/rows. |
| `RS.world-cell-effect-production` | Head and linked effect admission/removal, distinct first-item spectator override, local four-view projection, counters for the original type groups, type8 boundary depth and shade policy, frame/kind submission. |
| `RS.world-cell-artifact-production` | Special map-object lookup and counter sequencing, artifact resource/frame binding, head distance picking versus subsequent sprite picking, original highlight and candidate stores. |
| `RS.world-cell-creature-production` | Occupancy/visibility writes, primary and additional-actor iteration, movement/history/wrap depth, wide/type17 placement, body palette chains and shadow, three auxiliary layers, picking/history/RNG jitter, attachment/status/owner markers and selection rectangles/corners. |
| `RS.world-cell-queue-publication` | Complete `0x4ffd30` packed row stores, submission count, capacity refusal/last-index return, signed shade quantization, four-view depth and high-height behavior, kind9 payload and untouched physical row tail. |
| `NP.native-world-cell-producer-adapter` | Guarded entry installation, checked packed memory access, native publication, explicit failure policies, import staging and exact capture compiler-input binding. |

The rows retain original borrowed object/resource words. This producer does not
invent host pointers, retain ownership across launches, allocate queue storage,
sort rows, consume rows, or assume that referenced resource lifetimes have changed.
The owner and pool backing are read from the current engine managers.

Important recovered details, with high confidence within the composed domain:

* View1 uses map **height** for its x-edge priority adjustment as well as its
  y-edge adjustment. The rectangular 8 by 10 fixture distinguishes this from a
  symmetric native geometry policy. Additional actors accumulate the correction.
* History depth chooses the greater x in views0/3 and the lesser x in views1/2;
  y chooses the greater value in views0/1 and the lesser in views2/3, after the
  original nearest-wrap history adjustment. Height chooses the greater value.
* Special map-object counter `scene+0x205` advances before its visual query.
  Provider checkpoints compare this ordering, not just the final count.
* `0x4eadf0` is fastcall: ECX is the actor, EDX is the x-coordinate pointer,
  and the stack carries y/z pointers. The fixture records incoming coordinate
  values as well as resulting rows. Wide actors and opposing views expose
  pointer swaps that x+y depth would hide.
* For queue heights at least500, all four original branches use x+y plus scaled
  height. Lower heights use the supplied lookup table and orientation signs.
* Tile picking updates `scene+0x1ed`; artifact picking compares against that
  field but writes `scene+0x1f1`. Actor picking publishes its body-row index in
  `0x657d40`, row depth in `scene+0x1e9` and pointer in `0x656600`.
* The `0x51fa60` history shift snapshots the overlapping six words before writing
  them, then resolves the possibly aliased descriptor after storing x/y.

The retained original decompilation has stack/receiver recovery warnings.
Disassembly and composed execution supplied the ABI and store-order cross-checks.
If ordinary terrain admission is skipped, original artifact distance-picking
stack anchors can be uninitialized; native picking uses the scene anchors.
That undefined original-context branch is outside the original comparison claim.

## Runtime and external boundaries

[adapter.cpp](../../runtime/world-cell-producer/adapter.cpp) matches every guarded
byte before instruction mutation, checks the image address and memory ownership,
prepares the entry page protection, installs one jump, restores protection and flushes the instruction cache. Protection refusal occurs before any patch. Source reads/writes are
checked and LastError is preserved. Invalid views and pool references, inaccessible
spans and executed zero shade division terminate explicitly without producer
fallback. The adapter's policies do not assert equivalence to arbitrary original
faults, corrupt pointers, invalid movement directions or exception unwinding.

Animation/visual-result queries (`487c40`, `507190`, `507330`, `51faa0`, `507250`,
`5072c0`, `520540`, `505aa0`), effect admission/removal predicates, gameplay
visibility/actor iteration, picking, RNG/math, relocation and rectangle updates
remain named backend services. Runtime calls their existing engine entry points;
the owned comparison supplies instrumented providers. Native code owns the
producer's decisions, resource frame lookup, palette/clip binding, all queue
publication and its own state/history stores. Porting those external service
bodies, resource ownership and the outer full-world traversal is separate work.

Use:

```sh
python3 tools/capture-scene-game.py --native-world-cell-producer
```

The existing `--native-creature-drawing` and `--native-actor-lifecycle` options can
be supplied as well. Defaults remain opt-in. Staging imports
`MnmWorldCellProducer.dll` through `.mnwcell`; existing original section bytes
must remain identical. Capture verifies that every actual DLL compiler input is
in its prospective declaration, and checks the staged DLL hash. Original files
are never patched in place. Regenerate guards with
`tools/update-world-cell-producer-guards.py` under the storage guard.

## Evidence and reproduction

[The final report](native-world-cell-producer-provenance-results-20261020.json)
is registered as `WCELL1.producer-provenance-original-20261020`.
The earlier successful [ABI report](native-world-cell-producer-results-20261020.json)
remains immutable; it predates adding the verifier script and input-manifest files
to the explicit provenance closure.

The prospective-claim-bound runner is `tools/test-world-cell-producer.py`.
It builds the real runtime dispatcher and assembly entry, composes 3,040 cases
through original/native/installed-runtime paths, tests all 13,472 guarded byte
mutations plus one protection/four ownership refusals, builds the PE32 DLL,
checks capture declarations against actual compiler inputs, and verifies
unchanged-section import staging. The original producer and row-builder bodies
are poisoned during native execution, including the runtime body after its entry
jump. Original output is never input to the native implementation.

Each mode produces the same 162,103,152 ordered bytes with SHA-256
`b89d281371a3f11c77659e2a11011bee7ebd7703d4ab87eb0b576b55e4d03dbf`.
Cases cover all34 effect type inputs, four views, eight movement directions,
rectangular wrap boundaries, wide actors, history wrap/high depth, picking and
selection, negative/positive shade quanta, pressure and zero capacity, fog/cut
admission, absent layers, linked actor/effect/artifact work and callback mutation.
These are bounded owned-provider comparisons, not an exhaustive input proof or
actual-game/physical-Windows/live replacement milestone.

Run from the main checkout, using a new output path:

```sh
python3 tools/working-retention.py run --reserve-gib 0.5 -- \
  python3 working/worktrees/creature-drawing-commit-20261020/tools/test-world-cell-producer.py \
  --output /home/hoff/swdev/magic_and_mayhem_mod/working/worktrees/creature-drawing-commit-20261020/working/tests/world-cell-new-run
```

Compressed complete traces, build logs and stage artifacts remain in
`working/tests/world-cell-composition-provenance-20261020/` in the implementing
checkout. Earlier failed/order/history/ABI attempts and their first differing
case bytes remain alongside it. They exposed and corrected counter ordering,
rectangular wrap extent, history x-extrema and fastcall pointer binding. Durable
reports/guards/sources are committed research; executable outputs and compressed
execution traces remain retained working evidence. No storage cleanup was used.
Committed incoming history accounting is retained separately in
`coverage/world-cell-incoming-history-20261020.json`.
