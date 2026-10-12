# DRAW-02 canvas storage-generation boundaries

Step 2 remains in progress. Review `RS.canvas-create-lock`,
`DRAW.canvas-lock-recovery` and `DRAW.canvas-release-client` before extending
original wrapper storage observations. Successful startup pointers and synthetic
clients do not establish real allocation generations, COM aliases, all restore
paths or unrestricted use after unlock. No ownership status is promoted here.

## Pinned static inspection

`draw02-canvas-storage-static-01.json` preserves the broad disassembly export.
Its release window starts at `0x58b3e1`, one byte after the registered entry;
do not use that window alone to infer the entry instruction sequence.
`draw02-canvas-storage-aligned-01.json` separately checks the pinned NoCD hash,
PE file-backed entry offsets and exact prologues before decoding each routine
from its entry. These reports are static evidence, not engine executions.

At `0x58b660`, the initial Lock result is stored at `0x6f8150`.
In the lost-surface branch Restore overwrites that global; the retried Lock
result is not stored there. The shared result therefore cannot prove retry
success. The common tail reads descriptor pitch and pixels, optionally calls
auxiliary Unlock, and returns the saved pixel pointer even after that call.
A cached returned pointer does not prove that a map remains open.

At `0x58b3e0`, a null interface at wrapper `+8` exits before clearing other
fields. The `+4` and `+8` Release loops share one counter: after 1,000 calls
in the first loop, the second still executes once. The existing unchanged
original-client fixture verifies this ordering, optional auxiliary/clipper calls,
current-wrapper retirement, populated registry compaction and wrapper writes
across 5,184 scripted cases; see `drawing-canvas-release-original-20261011.json`.
The scripted interfaces do not model real aliases, shared refcounts, physical
allocation destruction, external mutation or reentrancy.

## Next observation boundary

Observe create, release and returned-lock transitions with process-local logical
wrapper generations, storage epochs and explicit invalidation. Retain entry and
return identities, caller, thread and cached dimensions. Reuse of a numerical
address must not silently retain an old identity. A changed pointer, interface
or pitch invalidates an earlier observed storage epoch. The observations remain
partial: unchanged addresses cannot establish unchanged allocation ownership,
all alias writers, successful upload or complete post-unmap lifetime.

The existing producer cache only changes its pixel address at Lock return and
retires wrappers at create/release return. A release/create in progress can
therefore leave a prior cached address visible. Any new query must expose this
uncertainty or refuse it; pointer equality alone must not admit ownership.
