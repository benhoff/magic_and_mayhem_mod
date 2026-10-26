# Terrain light relation, refresh and resource suppliers

Build: no-CD Chaos.exe SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This closes bounded supplier contracts behind the previously supplied lighting
inputs. It does not assert full installed-world production or live replacement.

## Relations

High confidence in byte production and consumption; semantic labels follow the
existing ownership/visibility research and the script consequences below.
Owner records begin at `00643a80`, stride `0x154`. `00465440` initializes all
eight `+18` bytes to one and all eight `+20` bytes to zero. The consumer's
zero-initialized fixture default remains available; original construction is
explicitly represented by `initialTerrainLightRelations()`.

`+18` is the alliance/nonhostility relation: `00465500` writes the supplied raw
byte to both `[owner][other]` and `[other][owner]`, unless owner equals other.
The all-owner setter `00465570` follows the same pair rule. `+20` is shared sight:
`004655e0` performs the same symmetric write in that table. Neither setter
changes the global lighting refresh flag. Noncanonical bytes remain bytes,
not normalized booleans. Original sentinel/invalid-owner accesses are excluded;
the native helper refuses owners outside 0..7 before mutation.

The `0056a8d0` script dispatcher supplies the setters: `0056ab0d/0056abe0`
change relation18; `0056af83/0056b051` enable/disable share20. Enabling sharing
when relation18 is nonzero walks living active creatures of both owners and
calls `00508ca0` to reveal them to the counterpart. The existing
[visibility recovery](owner-visibility.md) independently establishes relation18
plus share20 as the shared-sighting predicate. Creature terrain light at
`004f27d0` uses the same pair. There is no transitive-sharing inference.
Stored tables can still be asymmetric; the light consumer reads owner to viewer.
Script name lookup, notification/reveal consequences and installed initial
alliance configuration are not executed by this supplier fixture.

## Refresh inputs

Static inspection of all direct `005e41a8` references identifies these writes:

| Producer | Effect |
| --- | --- |
| `004f1890`, store `004f1bc9` | Kernel rebuild enables the creature/static updaters and sets changed=1. |
| `004f2dd0` | Explicit request sets changed=1. Direct callers include `0048f916` and `0051ea96`. |
| Successful map-load continuation, `005023a8` | Sets changed=1 before rebuilding kernels. |
| Outer updater `004f27a0`, store `004f27b9` | Clears a nonzero flag after creature then static updates. |

The explicit request routine is executed in this fixture. Kernel rebuild and
map-load caller paths are statically recovered here; earlier kernel and combined
cycle comparisons remain separate evidence with their original hashes.
Creature/object movement is sampled by existing batched cycle enumeration;
it must not be replaced with an invented immediate refresh on every mutation.
A relation setter alone likewise does not imply a refresh event. Indirect writes,
full scheduling and producer-triggered scene publication remain unvalidated.

## Resource lifetime

`TerrainLightResourceContracts<Memory>` follows guest addresses in owned storage,
matching other build-specific memory models. It is separate from the native
`TerrainLightField` value/RAII policy and does not expose host pointers to widgets.

`004ecdd0` uses `map+7cc` as the sole allocation sentinel. If zero, it stores
`((layers+1)/2) * map[+10]` at `+7e0`, then requests five equal allocations in
`+7cc,+7d0,+7d4,+7d8,+7dc` order. If nonzero, all existing handles and the old
byte count are reused; this is not a dimension-aware resize. It fills the five
buffers from the raw byte at `005e18f0`, calls the kernel service `004f1890`,
then sets requested/active (`+6426/+642a`) to one and published (`+642e`) to zero.
The fill byte is distinct from ambient/ramp configuration.

`004ecbe0` clears the first `0x4c` object bytes, fills `+1b0..+38f` with `ff`,
and independently frees/nulls auxiliary allocation `+4c` when present. A nonzero
published handle gates all five light frees, in order
`+7cc,+7d4,+7dc,+7d0,+7d8`; all five handles and `+7e0` are then zeroed.
If published is already null, sibling handles/count are left untouched, even
for an inconsistent object. A second clear makes no additional frees. Reload
requires dimensions to be supplied again after clear, which zeroed them.

## Execution and remaining boundaries

Reproduce under the storage guard:

```
python3 tools/test-terrain-light-suppliers.py --main-root MAIN_CHECKOUT \
  --output /tmp/UNIQUE_LIGHT_RUN \
  --report research/runtime/terrain-light-suppliers-execution-20261025.json
```

The runner pins the executable hash and brackets original artifact consumption
with manifest verification. Whole owner initialization/pair setters, resource
initialization/clear and explicit refresh request bodies execute in a private
PE32 image. Checked allocator/free and kernel-service redirects are the only
fixture replacements; executable sections are RX during execution. The kernel
endpoint records the request without rebuilding: this fixture claims caller
order, resource state and byte fills, not execution of that callee. Original
heap internals, allocation failure/partial allocation, exceptions, invalid
original coordinates, script consequences, installed entity/static-type discovery,
whole-world scheduling and live replacement remain separate.

Original32, native64 and ASan/UBSan runs compare complete object/buffer bytes and
ordered service calls for 32 resource journeys, including changed dimensions,
reuse, teardown, repeated teardown, reinitialization and the null-sentinel branch.
640 raw-byte pair updates check all owner pairs and both tables; 40,960 directed
admission checks consume those produced tables. Invalid native owners are refused.
The existing creature-cycle reference also executes against the current header
and source in all three variants. The immutable SQLite report records counts,
source fingerprints, prospective claims and manifest checks; historical reports
are neither overwritten nor rehashed.
