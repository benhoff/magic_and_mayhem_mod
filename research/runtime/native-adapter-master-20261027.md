# Native adapter latency integration, 2026-10-27

Integrates adapter58132cda with master0357068d. The resolved bridge retains
master's prepared CPU clipping/fill plans and partial-failure admission, while
preserving guarded post-original snapshot borrows and bounded pooled storage.
Cleanup releases plans and borrowed references independently. Plan-bearing
operations remain on the prepared copy path and do not enter direct live-only
admission or redundant archive recording. The authored register and append-only
review journal preserve both parents.

Eight of107 production inputs differ from the source diagnostic closure:
- runtime/native-draw/gpu_copy_plan.h
- runtime/render/bridge.c
- runtime/render/command_queue.h
- runtime/render/command_scheduler.h
- runtime/render/input_polling.h
- runtime/render/input_selftest.h
- runtime/render/lock_surfaces.h
- runtime/render/owned_session.h

Parent source measurements were19.898 and19.924 fresh World updates/s, with
resumed quiet20.00 and stress19.92FPS. These are historical bounded measurements
using the previous Qt diagnostic context. They do not validate all-current main
application latency or whole-game original-driver pixel equivalence. Historical
execution hashes remain unchanged; no independent behavior stage is promoted.

Fresh merged-tree execution passes15 CPU and23
GPU clipping/copy/fill cases, compiling the production PE32 bridge and comparing
native command replay with independent CPU facade output. Coverage includes
empty/clipped/256-piece plans, self/keyed policies and partial source failure.
Launcher controls pass18 cases with real Bash routing and mocked
Wine/prefix/owner orchestration. Combined live gameplay, original-driver
comparison and current application performance remain pending.

The first fresh attempt timed out during Wine prefix setup before rendering
cases ran; its negative report and raw logs remain retained. A bounded follow-up
established Wine startup with an explicitly addressed TCP Xvfb display and
preinitialized fixture prefixes with optional HTML/Mono installers disabled.
The second controls run records this environment and passes with stable sources.
No engine change was made to work around that test-environment failure.

History02 reviews12 incoming reachable commits since8c0c4ea4,
2856 first-parent file transitions, with zero unresolved exact
receipts and zero unavailable payloads. Separate retrospective receipts retain
220 adapter and92 native-player raw artifact identities; these are storage
accounting, not assertions of past gate passes or renewed engine validation.
The prior interrupted2026-10-26 integration reports remain preserved as history.

The user explicitly approved a temporary87GiB main-drive reserve for this merge
on2026-10-27 after concurrent writes exhausted the earlier88GiB exception; the repository's ordinary90GiB policy is unchanged. The secondary
reserve remains100GiB. Builds and Wine fixtures run in private RAM. Evidence and
review databases remain on durable storage with verified local backups. No
remote publication or upload is part of this merge. Unrelated main-checkout work
is preserved separately from this owned integration batch.
