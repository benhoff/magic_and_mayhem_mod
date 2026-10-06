# Effect kind 34 and creator candidates

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within selected effect types 3/13/22/24/36, authored metadata
threshold zero, owned worlds and the prior bounded trajectory/counter contract.
No installed capture or live replacement.

Kind is parameter 8 at effect+4c, distinct from effect type at +28. Both original
candidate gather loops admit the creator ordinal only when kind equals decimal
34 (hex 22). The initial loop compares its saved kind at `00488501`; the
cell-transition refresh loop reads +4c directly at `00488912`. The exception
changes candidate admission only: null/invalid ordinals, status 27, signed fine
coordinate differences, footprint rejection and first-hit ordering still apply.
Kind 68 bypasses scanning and preserves +198. Kind 0 excludes the creator.

Native `transitionEffectCreatureWorld` now admits kinds 0/34/68 and admits the
creator in both snapshots for kind 34. It uses kind-0 bookkeeping within the
existing movement helper for kind 34, then restores raw kind/creator/count fields
before publishing or scanning. The helper's public empty-world admission remains
separate; creator ownership and trajectory initialization are not reconstructed
by this change. Unsupported kinds are rejected atomically as intentional native
admission policy, not a claim about the original's failure behavior.

The metadata DWORD used by the original termination threshold is authored zero
at `006b126c + kind*721`. This comparison does not prove nonzero metadata policies
or other kinds/types. Kind34 is not the kind52 cell-flag special case and does
not enter type35 terrain mutation in the selected contract.

`test-effect-kind34.py` executes the complete unmodified original parent and real
helpers, with guarded effect/cell buffers and immutable per-call creature/terrain
inputs. It compares original32/native64/ASan-UBSan streams for three suites:
fresh TL16 ordinary fixtures, fresh TL17 boundary/mutation fixtures, and a new
matrix across kinds 0/34/68. The new matrix covers all 27 ordered positions, six
geometries, creator/status/invalid/competing configurations, five selected types,
both membership settings, mixed exits and mutations between calls. Dedicated
refresh fixtures place the creator two cells from the initial cell, outside the
initial snapshot, and move into range in one step: only kind34 with live status
can hit the creator after refreshing. Independent native tests also check raw
field preservation, empty-world admission and atomic unsupported-kind rejection.
Strict normal/sanitized tests and CMake/CTest run alongside manifest checks before
and after. There are no instruction patches, callbacks or stubs.

TL18 records current-source execution. TL16/TL17 records and their old source
fingerprints remain unchanged; shared implementation/CMake changes make those
historical records stale. Fresh baseline reruns in TL18 cover their fixture inputs
on current code without rewriting the old evidence. Installed creature descriptors,
cell/catalog construction, creator ownership, lifecycle/removal/recycling, other
metadata/types and live scheduling/replacement remain open.
