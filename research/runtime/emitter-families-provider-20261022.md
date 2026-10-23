# Remaining emitter family ownership (2026-10-22)

The native composition now has 275 public routes, adding 48 for the remaining
emitter families. `RS.actor-emitter-family-composition` has scoped implementation,
recorded original comparison and headless integration. The Critical
`RS.actor-downstream-services` umbrella remains partial.

The original is the pinned `nocd-40209ca7` build. The composition uses actual
`5457e0` admission, `545910` reverse manager ticks and `5458b0` cleanup.

| Family | Constructor | Bytes | Update | Deleting destructor |
| --- | --- | ---: | --- | --- |
| Random additive | `546660` | 69 | `5468a0` | `546630` |
| Default owner | `547210` | 84 | `547380` | `547280` |
| Custom owner | `5472c0` | 84 | `547380` | `547340` |
| Flagged trail | `548ac0` | 65 | `548bc0` | `548b90` |
| Fading animated | `549d70` | 130 | `54a140` | `549e30` |
| Simple colour/gravity | `548560` | 50 | `54aa30` | `548600` |
| Terrain child | `548650` | 90 | `548860` | `5487f0` |
| Inherited owner-additive child | inline `545650`/`545b70` | 100 | `546350` | `546270` |

Only each actual allocation extent crosses the memory boundary; the private
160-byte model does not read past smaller original objects. Colour planes have
separate owned generations. Default-owner predicate `5472b0` reads owner+`798`;
custom predicate `547370` reads owner+`110`.

Fading construction allocates its base terrain plane before publishing outer
fields, R250 state and the final vtable. Destruction frees the plane while the
family vtable and pointer remain callback-visible, then clears the pointer and
resets the base vtable before object release. Terrain shade changes call the base
shape colour method: compare raw requested RGB before quantization, then refresh
the existing plane. Copying only the kernel's RGB words would miss this work.
PE32 cell values translate to two explicitly retained grid generations; host
pointers stay inside the storage boundary.

`families-matrix-6.json` records 768 independently authored cases, 32 real manager
ticks per case, 24,576 ticks and 5,376 full snapshots per build. Original32,
native64 and ASan+UBSan compare **3,326,037,036 identical bytes**, SHA-256
`b6ee1ea2097d591938addbca70074b1e3676de129accab38c0fe153de87f7653`.
All 91 source/compiler inputs remain stable. Original bodies and manifests,
borrowed owners/descriptors/terrain/resources and allocation canaries remain
intact. Actual manager/deleting cleanup leaves no residual owned generations,
without fallback frees or original output becoming native input. Confidence is
high within this declared domain.

The six cohorts have 128 variants each. Two real 1,792-object prefixes cover
rejection and positive eviction/transfer. The original capacity path requires a
candidate priority **strictly above 1,024**: simple and newly created terrain
children cannot evict even lower-priority residents. A phase-six additive emitter
supplies the positive case. Each build records 8,978 created objects, 187 object
allocation failures, 673 planes and three evicted resident generations.

Cases vary default/custom owner activity, predicates and binding; terrain shading,
occupancy and retained-grid rebinding; dimension inputs 0/2/3/17; RGB555/565;
preloaded primary/fallback animation; and caller draw-state inputs. World extent
is 128×96×64. Plane high/low globals are fixed at 1.3/0.7. Flag rearming uses actual
nonraster `54ac00`; additive raster draw's flag byte is supplied explicitly, so
this suite grants no raster-drawing credit. The 112 named service specifications
are retained boundaries, not missing-function counts. The earlier synthetic
nested updater is absent from this suite.

The attempt catalog preserves route-discovery/compiler failures, preliminary
plane-disabled cases, the fixture's cleared plane globals, the invalid flagged
eviction assumption, and a duplicate dependency in a prospective declaration.
Runs stopped after a demonstrated discrepancy or before extending the scenario
retain their exact inputs and partial outputs. Matrix5 passed execution under the
duplicated declaration; Matrix6 repeats the same 91 source files under corrected
metadata. Only Matrix6 supplies current full execution credit. Historical hashes
remain unchanged.

Static inspection retains 60 complete central-inventory instruction ranges and
read-only Ghidra decompilation where available; 24 older-project decompilation
absences are explicit. The runner protects selected original body bytes. Exact
history review `a5ab8270..6d0dd7bc` finds 97 matching file transitions and no missing
or rewritten receipts. This accounting does not assert past gate passes or new
engine execution.

Incoming `6d0dd7bc` independently completes ten bounded implementations: eight
Critical, one High and one Supporting. Dashboard partial counts are now Supporting
4, Low 1, Medium 3, High 34 and Critical 8. The eight Critical partials retain two
recovery, two validation and four acceptance needs. Seven assessments route to
review because incoming source changes invalidate their existing fingerprints;
those fingerprints and wider requirements are preserved. The actor umbrella's
assessment is renewed for this family batch. The first UI regression exposes its
old denominator expectations; a second run exposes three further obsolete
tranche/completion expectations. Both failures remain in SQLite. The corrected
checks use the exact committed changes; the earlier UI-attempt catalog retains
its historical test-source fingerprints.

Remaining boundaries include block-zero descriptor indeterminacy, plane allocation
failure, arbitrary callback mutation/SEH, malformed inputs, nondefault plane FP
globals, actual resource loading and backend acceptance, original-world/planner/
task cadence, other special/scenario/indirect providers, and installed/live/
physical replacement. This proof does not complete original campaign ownership or
all Critical work.
