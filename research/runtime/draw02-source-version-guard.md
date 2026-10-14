# Observed source versions through selected worker execution

`--wine-source-version` enables a bounded diagnostic write-version check. It
requires bounded mapped sources, exact packet/worker binding, resource map and
copy observers. The separate client-storage-point observer is incompatible
with this mode because both would own the same descriptor-rebind IAT slot.
Default-off execution does not install that additional hook.

Each exact public resource pointer has a process-local write epoch and active
writer count. Distinct texture aliases resolving to that resource share this
state. Epochs start at one and never reset when an address is reused; texture
generations are checked separately. Addresses never enter the portable journal.
The fixed resource and pending-snapshot tables each have 2,048 entries.

The observer takes the read epoch after the successful synchronous public READ
map has returned, before copying rows. After unmap and owned-file retention it
requires the same epoch, no observed active writer and the original texture
generation, then registers the snapshot's payload and submission identity.
The selected emitter binds that snapshot to the exact packet command. Worker
entry checks the packet resource/subresource, command, original generation and
write epoch before the original callback. Worker return checks them again.
Only then is that pending snapshot retired. Missing, duplicate or changed
identity, changed generation/epoch, active writes, pending snapshots at close
and capacity/overflow conditions refuse with exit 99.

These observed paths advance write epochs on both entry and exit:

- Public resource-map attempts whose flags are not exactly READ; successful
  writable maps remain active until successful original unmap.
- Public resource-copy destination attempts, including failed attempts.
- Every selected opcode 42 worker destination, around its original callback.
- Public texture descriptor/client-memory rebind attempts through the pinned
  DirectDraw IAT slot, including failed attempts.

Versions are coarse for the whole resource. Writes to another subresource can
therefore conservatively invalidate a snapshot. An original worker copying
within its own source resource is explicit: entry validates the source first,
then the worker's own destination mutation advances that epoch twice. No other
observed source write may overlap that interval. This permits the known own
write; it does not assert unchanged source bytes after a self-copy.

The additional rebind hook reuses the previously inspected Wine 11.16 public
cdecl four-argument ABI, DirectDraw IAT RVA 0x6d700 and wined3d export
RVA 0x221460. Installation checks the original slot, export address and exact
16-byte prefix before modification. Wrapper entry/return FP and LastError are
preserved. After capture close it forwards directly without extra queries.

The worker does not call the public resource-map API. Wine's public map path
delegates to the device context, whose map emitter invokes the command-stream
thread restriction and waits for resource idle and map-queue completion.
See the pinned primary [resource wrapper](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/resource.c)
and [command-stream implementation](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/cs.c).
These source leads explain the design; installed binary checks and executions
remain separate evidence.

## Capture boundaries and confirmed limitations

Wire 1 originally required every global write lease to close with the capture.
[Original 01](draw02-source-version-original-01.json) checked 3,066 snapshot
workers before refusing at close: the original game opened another writable
map after the final checked frame. The unclosed-prefix counts are diagnostic
only; missing `session.done` prevents acquisition admission, and the forced
exit does not provide complete update-counter or normal-Quit evidence.
[The close failure](draw02-source-version-close-negative-01.json) preserves this
finding. It agrees with the independently retained prior resource-map evidence,
which also contains one writable map at the declared capture boundary.

Wire 2 preserves those open leases as explicit boundary records. It still
refuses any pending snapshot and any conflicting write during a checked read
interval. A closed snapshot-to-worker interval is separate from a global
claim that all application maps have ended; the latter remains false when
boundary leases exist. Unknown data is retained rather than waived.

This guard covers observed mutation paths. Direct CPU writes bypassing those
paths, different resource objects sharing backing storage, unobserved command
opcodes and asynchronous GPU consumption remain outside its proof. The owned
direct-write case intentionally changes a consumed byte from 3 to 2 after the
snapshot while the version guard still accepts. That is executable evidence
of this limitation. Neither source atomicity nor all-alias ownership is
promoted by successful observed-version checks.

Confidence is high for the tested observer state machine and selected observed
event relationships; completeness for original source writers is unproved.
Original simulation and rasterization remain active. These source files remain
external diagnostic inputs, with no production native renderer or replacement
claim. Source-only changes leave earlier fingerprints historical.


## Observed source write-version checks — 2026-10-13

The [source-version guard](draw02-source-version-guard.md) checks resource write
epochs and texture generations from a mapped snapshot through the exact emitted
packet's worker entry and return. Aliases resolving to the same public resource
share an epoch. Observed writable maps, copy attempts, selected worker destinations
and descriptor rebind attempts invalidate older reads. The guard defaults to off;
original simulation and rasterization continue to execute.

[Owned fixture 05](draw02-source-version-fixture-05.json) passes 13 PE32 cases and
8 decoder tests. These cover concurrent writes, generation reuse, failed rebinds,
wrong subresources, mutation during copying, valid self-copies, unrelated writes,
open leases at capture close and default-off forwarding. FP and error state are
preserved. Both existing mapped-source regression suites also pass. Failed builds,
fixtures, prefix startup and capture close remain separate historical records.

[Quick Battle](draw02-source-version-original-02.json) checks 3,083 sources across
225 complete advancing updates; [its analysis](draw02-source-version-egl-analysis-01.json)
matches 507 whole images. [Campaign](draw02-source-version-campaign-original-01.json)
checks 1,402 sources across 50 complete updates; [its analysis](draw02-source-version-campaign-egl-analysis-01.json)
matches 194 whole images. Combined, these are 4,485 source-to-worker checks and
701 matching whole images containing 336,480,000 RGB pixels. Both runs verify
unchanged original manifests and sections, and normal original Quit.
[Selected campaign state](draw02-source-version-campaign-pause-analysis-01.json)
stays stable during two Mini dwells of 2,479 and 3,114 ms and a Preferences dwell
of 2,513 ms; 22 resumed samples show actual World advancement. This covers the
selected state, with animation, camera and full pause equivalence still open.

[The timing comparison](draw02-source-version-cadence-comparison-01.json) covers
120 common queued updates. Relative to the same counter-only control, median
cost is 2.061× and p95 cost is 3.187×. Combined diagnostic acquisition therefore
remains intrusive; the comparison does not isolate guard cost or establish
identical randomized work or native parity.

The owned direct-write case changes the worker's consumed byte from 3 to 2
without an observed Wine mutation, and the guard accepts it. This confirms a
remaining gap: unhooked direct writes, other opcodes, separate resource objects
sharing backing and asynchronous GPU reads remain outside the check. Original
source atomicity and complete alias ownership remain unproved. Steps 3–7 and
admission for all 47 journeys and 21 operations remain outstanding.

History from 72a357b0 through b030d7fc was reviewed against 38 exact file receipts
with zero gaps. The source census and exact current review accompany this change.
