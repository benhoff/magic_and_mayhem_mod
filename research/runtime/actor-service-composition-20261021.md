# Actor service composition and concrete providers

The address-based service composition now dispatches the112previous service
bodies,34concrete supplier bodies and three actor lifecycle entries through
one owned memory backend. An unknown address returns `nullopt` to the backend;
it does not acquire a recovered implementation merely because another behavior
links that address. Application integration and live replacement remain separate.

## Concrete recovery

| Group | Entries | Recovered behavior |
| --- | ---: | --- |
| Animation |6|Descriptor count/sequence lookup, state clearing, restart, initial instruction scanning and forward/reverse ticks. |
| Camera, flight and raster state |5|Camera rectangle dimensions, corpse flight predicate, fastcall raster binding/clipping and exact byte text-flag mutation. |
| Particle ownership |12|Position wrapping, colour quantization, random-motion construction, original XOR stream state, priority methods, deleting destructors and pool admission/eviction. |
| Scenario and owner selection |9|Owner-name lookup, event eligibility, named-region overlap with wrapped XY bounds, comparison helpers, repeat/finalization and action subtype dispatch. |
| Widget providers |2|Panel backing allocation/callback publication and copy submission; panel mode switching, invalidation and virtual calls. |

Pinned original build: NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Static instructions and decompilation are retained in
`working/tests/service-composition-20261021/`. The original manifest is checked
before and after original-consuming experiments; no original file is modified.

The panel redraw body4d47f0passes the parent object in ECXto its copy/deferred
submission helpers. The initial native body passed the child widget; the original
comparison exposed this mismatch and the implementation now preserves the parent.
The original particle pool's callback-visible reloads, signed priorities and
index-minus-one hint update remain intact. Its raw admission semantics are kept
separate from the existing native token policy that refuses duplicate ownership.
The final matrices guard the pinned250-entry cyclic RNG index table and retain
all250state words. Explosion sequences repeatedly wrap that real table; the
earlier eight-slot test input is preserved as an earlier bounded attempt.
The table at5eb940contains250DWORD successors `(index+1)%250`. The selected
particle receiver6574c8stores two DWORD indices followed by250DWORD state words
at+8. This is transition and caller composition proof; seed-route admission has
its existing separate evidence.

## Composed caller scope

The selected21caller roots cover marker singleton/vector construction; animation
setup followed by zero to four ticks; task/action dispatch; explosion through
200particle constructions and pool admissions; popup population, removal and
scroll raster state; scenario eligibility/action/finalization; peer repair;
occupancy attach/detach; task reset; marker, list, spell and audio retirement;
original experience progression; and preloaded actor activation, release and
cleanup. The activation cases subsequently execute release and cleanup on the
same actor and owned map/storage. Those64sequences are included in the21-root
matrix, not counted as additional cases.

The fixture uses independent native inputs, bounded allocator storage, explicit
heap primitives and named resource/platform/virtual endpoints. It compares full
selected owned storage/globals, defined results and ordered callback arguments
with callback-time state digests. Native completed-service sets are observability
of native dispatch only; they are not original function-entry tracing. Selected
original bodies and reused helper ranges stay byte-guarded during execution.

The current11matrices match10,688original/native cases:2,176concrete-provider
cases,1,344composed caller cases and7,168regression cases for the prior nine
service suites. All64preloaded activation-to-release/cleanup sequences return
normally, as do the other admitted composed roots except three deliberately
invalid audio cases whose original access-fault prefixes match. The new provider
matrix retains nine matching division faults. The native vector exception test
also passes171completed-prefix/failure combinations, without Windows SEH credit.

The initial expanded composition attempt retained invalid, uninitialized vector
lengths in two callers. Original CRT memmove performed partial writes before an
access fault, while the owned native memory service refused the invalid extent
before copying. This is retained negative evidence in `service-composition-5`;
no malformed-copy equivalence is asserted. The comparison domain now explicitly
supplies valid owned vectors. Existing raw access/division outcomes and the
original indeterminate item contribution remain separately bounded findings.
An expanded provider list also exposed overlap between the fixture's anonymous
stubs and fixed callback tokens. The code arena is now separate and the complete
trampoline layout is checked for bounds and overlap before any installation.
All eleven matrices were rerun after that fixture repair.

Machine-readable execution records are stored through SQLite under
`research/runtime/service-composition-results-20261022/`; the coverage register
binds each new result to its prospective scope and complete source closure.
Historical DSVC results and source hashes are preserved.

## Remaining boundaries

The broad Critical `RS.actor-downstream-services` contract remains partial.
The composition supplies the selected caller paths; it does not supply every
actor/session configuration or close the transitive graph.

- Source-controller registration still uses an explicit endpoint in this matrix.
  Its existing owned logical suppliers have separate evidence and are not
  credited with new caller-specific composition here.
- Font layout/glyph raster, bitmap/resource decoding and acceptance, attack/task
  providers, the large scenario evaluator5654e0and concrete scenario action
  subtypes remain downstream. Scenario eligibility and subtype dispatch are
  recovered bodies; supplying an action callback does not recover that action.
- Other particle classes, indirect targets and actual external buffer/resource
  providers retain their ownership/failure/lifetime requirements. The named
  priority/deleting methods cover the selected random-motion class only.
- Windows SEH delivery, throwing cleanup, malformed CRT partial-copy faults,
  arbitrary concurrent publication, complete producer-to-pixel retirement and
  supported-session/live replacement remain separate.
- The retained legacy driver failure on Wine and the lack of a legacy Windows
  host are unchanged; no physical-driver equivalence follows from these tests.

## Reproduce

Run each matrix into a new retained output directory through the storage guard:

```sh
python3 tools/working-retention.py run --reserve-gib .5 -- \
  python3 tools/test-actor-services.py --suite providers --output working/tests/providers-new
python3 tools/working-retention.py run --reserve-gib .5 -- \
  python3 tools/test-actor-services.py --suite composition --output working/tests/composition-new
```

The nine prior suites remain available. `tools/reconcile-service-composition.py`
checks routing against the actual model case entries and retains inspected call
frontiers, scoped leads, supplied endpoints and unresolved indirect sites. Its
report is a static review, not additional execution or transitive closure proof.
