# Pixel helper virtual Unlock recovery — 2026-10-26

All three outgoing calls retained by the five-helper investigation are
`IDirectDrawSurface::Unlock(void *pixels)` through vtable byte offset `0x80`
(slot32). They use the original Surface1 interface, not the queried Surface2
interface used by the Lock helper. This closes these three service identities;
it does not discover natural incoming callers into the five helpers.

The pinned No-CD input is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
No original file or selected instruction is patched. Understanding confidence
is high within the normal wrapper-origin receiver scope below. Driver addresses
and observations are explicitly limited to the measured Wine build.

| Original call | Receiver | Argument | Ordering / return |
| --- | --- | --- | --- |
| `0x58d487` | source wrapper `+4`, Surface1 | base pointer returned by source `0x58b660` | First row-copy Unlock; result is unchecked |
| `0x58d494` | destination wrapper `+4`, Surface1 | base pointer returned by destination Lock | Runs even after source Unlock failure; its HRESULT remains in EAX on return |
| `0x58d8a8` | brighten wrapper `+4`, Surface1 | original Lock base, not the advanced pixel cursor | Always follows the loop; its HRESULT remains in EAX on return |

## Receiver and ABI evidence

`0x58adbb..0x58adc2` passes wrapper `+4` as the output of DirectDraw
CreateSurface (slot6). `0x58add7..0x58ade3` queries IID
`57805885-6eec-11cf-9441-a82303c10e27`, storing Surface2 at wrapper `+8`.
The original successful creation path is executed in both fixtures, including
its Surface1 GetSurfaceDesc endpoint. Both fields remain independently tested:
substituting Surface2 for the retirement receiver fails the scripted checks.

`0x58b67f` loads wrapper `+8`; `0x58b69d` calls slot25 Lock with null rectangle,
108-byte cleared DDSURFACEDESC, `DDLOCK_WAIT=1`, and null event. Returned pitch
is cached at wrapper `+0x18`; the base pixels come from descriptor `+0x24`.
The caller later reads wrapper `+4` and calls slot32 with two stack arguments:
Surface1 receiver followed by the base pointer. The callee uses stdcall and
returns HRESULT in EAX. The local installed Wine `ddraw.h` explicitly names
Surface1 slot32 `Unlock(void *data)`; the original IID, argument flow and real
interface observations supply independent checks of this identification.

The complete row-copy and brighten functions execute unchanged, as do the
original creation and Lock bodies and the row-copy x87 conversion helper.
There are no substitute Lock or x87 bodies in this experiment. Scripted COM
endpoints model only the admitted successful creation/Lock boundary and chosen
Unlock results. No claim covers arbitrary or corrupted object pointers.

## Observed details

Both zero width and zero height controls still reach the tail Unlock calls.
The zero-sized wrapper dimensions are authored after successful nonzero fixture
creation; these are caller controls, not naturally observed zero-sized surfaces.
Row-copy continues to destination retirement for both successful and failing
source Unlock results. Neither wrapper reports, retries or restores after
these tail results. Their incidental returned EAX is the final Unlock HRESULT;
this does not establish that unknown callers consume that result.

When `0x6e1f1c` is nonzero, `0x58b806` already calls Surface1 Unlock before Lock
returns its retained pointer. The later helper-tail calls remain present. Real
Wine returns `0x88760248` (`DDERR_NOTLOCKED`) at those later calls. The early
Unlock itself succeeds. The helpers leave that HRESULT unchecked.

The tests preserve two previously modeled pixel-loop details: row-copy uses
the cached byte-pitch field as a WORD index stride; brighten traverses a flat
`width*height` WORD sequence, including row padding. The scripted buffers cover
the row-copy accesses with complete guards. Real-driver row-copy is bounded to
one copied row; real brighten expectations use independently measured pitch to
identify which logical pixels lie in the flat span. These tests do not justify
correcting those loops as part of service identification.

## Execution evidence

[Scripted comparison](pixel-virtual-services-results-20261026/scripted04.json)
(`PVIRT.scripted-20261026`) passes 512 cases per optimized32 and
ASan/UBSan32 build: two helpers, RGB565/RGB555 selector, early-Unlock off/on,
four widths, four heights and two results for each owned surface. Each variant
executes 1024 successful factory paths, 768 selected tail dispatches and compares
4,194,304 physical buffer WORDs including guards. Exact return PCs, receiver
version/owner, base pointer, ordering, results and untouched original text agree.

[Real-driver comparison](pixel-virtual-services-results-20261026/driver06.json)
(`PVIRT.driver-20261026`) executes 64 cells with 128 real created Surface1/2
alias pairs, 96 selected tail dispatches and 48 additional early Unlocks. Original
creation, Lock and helper instructions stay active. Owned Surface1 vtable shadows
observe only Unlock and forward the same receiver/pointer to the real target.
The 32, 768 compared WORD slots include fixed packet sentinels;5,184 slots are
readable logical before/after pixels. No padding pixel equivalence or complete
physical frame is claimed for this driver probe.

All selected tail calls resolve to Wine 11.16 built-in `ddraw.dll+0x29f70`,
DLL SHA-256
`24c8c8fbbe0bac100efa5a716ebc78506e0f42e80d2af019b641911c0847ab28`.
VirtualQuery allocation-base and module identity checks bind that RVA to the
loaded DLL. It is an external COM implementation, not a missing function at a
fixed Chaos.exe address. Other driver builds and launches require their own
module/target observations. Both fixture modes verify all 2,927 immutable inputs
before and after and recheck declared source/input hashes.

The first scripted attempt retained a missing supplied GetSurfaceDesc endpoint;
the second retained a sanitizer callback stack-alignment failure. The successful
fixture explicitly aligns its host callback stack without changing original
instructions. Driver attempts1/2 retain host tmpfs quota failures; attempt3
retains private-X startup/teardown failure; attempt4 passes the actual matrix but
fails its overly strict already-stopped wineserver check. Attempt5 passes the
older packet-count label. The final result names logical pixels and packet slots
separately. None of these earlier records or source hashes is overwritten.

## Accounting and remaining scope

`RE.pixel-helper-unlock-dispatch` owns this recovered contract. The central
`five-pixel-helper-outgoing-virtual-services` entry is complete only for these
three admitted outgoing service sites. Its historical discovery inventory and
incomplete global indirect-flow census remain unchanged. No new function/range
boundary or native implementation, native equivalence, integration, live
replacement or gameplay milestone is claimed. The existing native pixel splices
already leave these original Unlock tails active and are unchanged.

Natural incoming callers for all five helpers remain unresolved. Retain the
43-route denominator, 38-route historical observed union and provisional
unused-candidate classification. Failed/missing Lock pointers, loss/Restore,
receiver mutation/reentrancy, asynchronous COM lifetime, foreign drivers and
full installed gameplay acceptance are outside this recovery.

## Reproduction and retention

Declare `RE.pixel-helper-unlock-dispatch` with
`tools/draft-coverage-claims.py`, then run `tools/test-pixel-virtual-services.py`
with `--mode scripted` or `--mode driver`, `--claims` and a new `--report`.
Driver mode supports an exclusively leased, marked idle fixture prefix with
`--shared-fixture-prefix`; it snapshots its registries and waits for retirement.
It also supports cloning a stopped `--prefix-template` to a new `--prefix-path`.
Use a private mount namespace with a task-owned `/tmp` binding when host tmpfs
quotas prevent Wine/X startup. The final run uses the existing marked fixture
suite, not a game installation. The common Git checkout, owner marker and
exclusive lease are checked before use. Normal90/100GiB storage reserves apply.

Generated reports, compiled probes, raw packets and logs are retained through
the SQLite writer; research paths are references. Disposable execution trees
remain under this task's ignored `working/tests/pixel-virtual-services/`.
Owned temporary PE zero extents were sparsified with unchanged byte hashes;
duplicate immutable gzip fixtures in this checkout and the completed size-one
checkout from this conversation were shared with byte-identical main fixtures.
Every fixture path and exact payload is retained; no original or other checkout
bytes are changed or removed. The sharing receipts' allocated-byte amounts
measure displaced checkout copies; their original link counts were not retained,
so those amounts are not asserted as actual filesystem space reclaimed.

The final closeout uses the normal 90 GiB main-drive reserve after free space
recovers; the user's temporary 88 GiB permission is not needed. Local evidence
and review databases receive verified, pinned SQLite backups before commit.
No publication or merge into master is part of this batch.

The [source-retention receipt](pixel-virtual-services-results-20261026/source-retention01.json)
retains all 95 reported source bindings, including historical failed-attempt
variants reconstructed only when their bytes match the already recorded hash.
No historical hash or outcome is refreshed. The
[committed-history review](pixel-virtual-services-results-20261026/committed-history01.json)
checks all 1,801 first-parent file transitions across the six commits in
`c5137b9c..1ffeb53e`: 1,757 match exact receipts and 44 are retained journal or
nonwatched artifact transitions, with no unresolved watched transitions. This
is history accounting, separate from final working/staged validation.

The final full audit has zero errors, no source-census drift and current source
fingerprints for both PVIRT results. Existing unrelated historical staleness and
discovery gaps remain explicit. The planning controls pass 38 tests and the
existing computed-caller controls pass 34; no planning weights are changed.

The verified local input snapshot is
`working/coverage/backups/6db110dace1eeb524672a0143862554f442e5ec00a98dbb6d2487eb0e351f502.sqlite3`
(2,780 hash-verified objects; its filename is its verified database SHA-256).
It retains the copied parent metadata and all inputs available at sealing.
The separate active `working/coverage/evidence-pending.sqlite3` contains 93
unique owned artifact payloads, including the retained source variants.
Set `MM_EVIDENCE_DB` to the input snapshot for local working/staged/base audits;
keep that snapshot, the active store and `backups/catalog.sqlite3` pinned.
The normal finisher separately snapshots and verifies the active evidence and
review databases before staging and the normal hook. This split avoids another
full copy of already retained parent metadata; it changes no evidence hashes.
