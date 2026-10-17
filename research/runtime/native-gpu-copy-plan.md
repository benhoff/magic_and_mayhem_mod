# Owned GPU copy plans

The owned live bridge now admits RGB565 GPU-source Blt with current canonical
clip snapshots and aliased/self source objects. The new process-local
`NativeDrawGpuCopyPlanV1` service returns scalar metadata for up to 256 ordered
pieces and the original HRESULT separately from successful plan admission.
A later source-bounds failure retains earlier pieces. Empty lists cause no writes
or authority change. Bad identity, ABI, flags, leases, formats, missing/range keys
and non-GPU source history refuse without output-plan or CPU storage mutation.
Legacy `NativeDrawGpuCopyV1` remains a disjoint/unclipped bootstrap service.

This is intentional owned-facade policy. Its CPU implementation freezes the whole
aliased source before any piece, including keyed overlap. The renderer's general
Surface2 path instead implements ordered driver piece semantics; it is not reused
as an ownership shortcut. The bridge creates a bounded zero-initialized temporary,
fully overwrites it with a GPU COPY of the owned source, emits every clipped COPY
from that immutable temporary, then deletes it before returning. No stale CPU
image initializes that snapshot. Resource IDs advance monotonically and source,
destination, leases, generation and session order remain checked. Temporary
capacity/publication failure terminates before application continuation.

Known destination CPU allocations remain unchanged and stale whenever any piece
executes, including an error prefix. Partial/keyed/clipped writes cannot bootstrap
an unknown destination. Full un-clipped opaque bootstrap keeps its separate private
write-mask baseline. Explicit CPU reads retain their FIFO materialization contract;
no per-World pixel export, readback or original fallback is introduced.

Implementation/build checks are separate from headless execution, original-driver
comparison and installed-game replacement. The focused controls execute the actual
production facade and bridge functions using authored GPU source images, poisoned
CPU allocations, separate current-CPU facade expectations, and the real native GL
command consumer. They do not run the original game. Full-session pixel regressions
remain independent. Arbitrary raw CPU readers, stretched/range-key GPU copies,
resource-capacity refusals, async World source readback, global routes, physical
context loss, hardware performance and complete renderer replacement remain open.

The [source-bound headless result](native-gpu-copy-plan-controls-20261016.json)
passes all 23 copy cases, 25 prepublication refusal controls and three publication
failures. Cases include each overlap direction, clipped cross-piece/keyed overlap,
distinct interface aliases, empty clips, late source errors, integer extremes and
256 pieces. The legacy baseline/CPU restore probe and both command-consumer and
resident-World compatibility executables also pass. Original manifests verify
before and after (2,927 files). Expected pixels enter only the external comparator.
The tested GL backend is software llvmpipe, not hardware performance evidence.

Independent full-session regressions also pass: all 222 corrected-profile storage
frames and their source/default-framebuffer witnesses, and all 323 Quick Battle
GPU/X11 frames. Final RGB tolerance remains one channel value; exact diagnostic
witness checks remain exact. The offline compositor does not exercise this new
live bridge path. These results preserve the existing observed-EGL scope and
leave complete presented-journey/replacement acceptance false. Results:
[storage](native-gpu-copy-plan-storage-20261016.json),
[Quick Battle](native-gpu-copy-plan-quick-20261016.json).

Failed setup attempts remain retained under
`working/tests/native-gpu-copy-plan-20261016`: an unrelated active Wine-session
guard, fresh-prefix startup timeout, fixture link correction and stale cached
command-consumer rejection. The final run uses a separately owned, locked prefix
and a freshly built current consumer. The first storage suite run was discarded
when coverage metadata changed during execution; its stable-metadata rerun passes.
None of those attempts contributes passing evidence.

The [2026-10-17 live follow-up](native-gpu-copy-live-validation.md) now validates
867 opaque disjoint GPU-route calls at three actual engine return sites through
Save/Load, display rebuild and Quit, with independent copy destinations. It
observes zero clipped/self/keyed GPU calls; those branches retain headless evidence
only. The [final-source 23-case rerun](native-gpu-copy-plan-controls-20261017.json)
passes after adding the opt-in observer. This does not promote complete original
or physical presented-session equivalence.

The [active route follow-up](native-copy-route-reachability.md) distinguishes
1,828 keyed CPU-backed calls from 1,666 opaque GPU-source calls. Clipped/self/keyed
GPU-source plans remain unexercised live; all 23 final-observer-source headless
cases pass. Seven post-retirement callbacks are explicit exclusions.
