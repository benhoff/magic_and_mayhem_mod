# Rendering chunks 5–6: alias leases and graphics retirement

The native facade already implements the admitted lifetime behavior. This batch
adds independently executable provider comparisons and allocation-failure
controls; it does not change production rendering code. Confidence is high
within the boundaries below. Broader game-session integration remains pending.

All original addresses use the No-CD image SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Original-consuming executions verify the immutable manifest before and after.
The real compatibility provider is Wine 11.16, with installed DirectDraw and
WineD3D module hashes in the execution record. Wine is not a legacy Windows
physical-driver acceptance result.

## 5. Surface aliases and leases

The same freestanding PE32 caller runs against Wine DirectDraw, the unchanged
native DLL, and the native guarded dual-view profile. It uses all 16 ordered
Surface1/2/3/4 interface pairs and checks canonical IUnknown identity.

* **128 offscreen cohorts per endpoint:** Lock or GetDC through one alias;
  nested acquisition and release through another; optional creator Release
  while the lease is held; both final alias-release orders; and a fresh,
  independently seeded surface while the old generation remains alive.
  Five complete 13×9 RGB565 snapshots per cohort compare initial, modified,
  cross-alias and allocation-separated storage. The caller records repeat
  Unlock/ReleaseDC errors, not just successful operations.
* **64 focused double-buffer cohorts per endpoint:** front/back Lock/DC through
  each ordered alias pair, Flip while held, releases against both logical
  objects, and eventual DC release after flip-back. Both complete 640×480
  planes are checked word by word against independently authored uniform
  patterns before their hashes are compared: 39,321,600 final pixel words per
  endpoint. This proves the admitted physical backing/lease exchange, including
  a busy Flip that has already exchanged backing. It does not justify making
  busy Flip atomic.
* Every native cohort explicitly releases its public COM credits and checks
  device, surface, DC, clipper and palette counters before process reclamation.
  Final DLL retirement receipts also require zero retained storage bytes.

Guarded mode 3 deliberately refuses GetDC during an active CPU Lock. The runner
requires exactly 64 such admission differences and otherwise identical guarded
caller/pixel results. This policy is preserved separately from plain Wine
compatibility. No lease test treats a COM pointer value as a generation ID.

## 6. Graphics shutdown and failure cleanup

**192 sessions per endpoint** execute complete unchanged original `58acf0`
shutdown and `58b3e0` wrapper retirement. Wrappers are built by complete original
`58ad90`, calling actual Wine/native CreateSurface, QI and optional PageLock.
The matrix includes 0/1/2/3 registered wrappers, page hints, window/fullscreen
state, optional clippers, normal shutdown or explicit caller-triggered shutdown
after refused creation, and extra Surface3/4 caller credits. It repeats these
sessions in one process and invokes the draw2-null idempotent shutdown exit.

Assertions check the cleared device globals, primary publication, registry count,
fullscreen state and six retired wrapper ownership fields. Complete 13×9 pixels
of every retained later-interface alias survive shutdown. A supervisor device
credit remains valid during those post-shutdown reads; the fixture does not use
a dead device. Native intermediate counters must equal exactly the authored
external surface/device credits and the original-retained fullscreen clipper.
The caller explicitly releases these credits before requiring final zero owners.
Thus original retention is not silently rewritten into automatic native cleanup.

The failed-width descriptor has intentionally different HRESULT policy: the
existing native facade returns E_OUTOFMEMORY while Wine returns a failed result.
Both exact results are retained; only this explicit policy field is excluded
from byte equality. The fixture calls shutdown itself afterward. This is not
proof that a naturally failing original startup reaches the same cleanup path.
The fullscreen flag is authored; no physical fullscreen device is established
by the session fixture's normal-cooperative-level setup.

**29 native allocation-failure controls** run the unchanged production DLL:
10 plain and 19 guarded dual-view cases. A checked, test-only in-memory import
adapter fails one named allocation ordinal and forwards all other calls to the
real host API. It verifies the original IAT entry against GetProcAddress before
substitution and restores the original page protection. No DLL file or original
artifact is patched, and no production test export is added.

The controls cover device allocation; each front/back surface state and ordinary
backing allocation; guarded mapping handle, both views and dirty-map allocations;
and DC, DIB and bitmap-selection creation. Each failure must be reached exactly
once, return failure without publishing an output, restore the exact count of
tracked real heap/virtual/mapping/view/DC/bitmap resources and native owners, and
permit an immediate successful retry. Failed DC creation preserves every owned
pixel. Caller credits and all tracked resources return to zero before exit.
These are native failure policies, not original allocation-failure equivalence.

## Evidence and remaining boundaries

[The final execution record](graphics-leases-shutdown-results-20261024/execution06.json)
is a SQLite-backed immutable report. Raw PE callers, production DLL, complete
comparison bytes, logs, compiler dependencies and retirement files are retained
under `working/tests/rendering-leases-shutdown-20261024/`. Historical attempts
remain separate: execution01 had matching offscreen/session outputs but a
crashed display server; execution02 failed display startup; execution03 failed
physical display-mode setup; execution04 passed the plain provider matrix;
execution05 passed the new failure controls but its runner changed during the
run, so it is not admitted as current-source validation. The final fixture uses
an authenticated private TCP X server with Mesa software rendering and a Wine
virtual desktop for focused Flip. It checks display-server survival.

Reproduce from the owned worktree, supplying an idle, serial-owned Wine prefix
and its marker-owning checkout from the same Git repository:

```sh
python3 /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py run \
  --reserve-gib 0.08 -- python3 /absolute/checkout/tools/test-graphics-leases-shutdown.py \
  --output /absolute/checkout/working/tests/graphics-lifetime-next \
  --report research/runtime/graphics-lifetime-results-next/execution.json \
  --prefix-owner-root /absolute/prefix-owner \
  --wine-prefix /absolute/serial-owned-prefix
```

The runner pins prospective behavior and scenario contracts, recursively declares
local includes, checks actual compiler dependencies, verifies source stability,
locks the shared fixture prefix, and stops only its owned Wine clients/display.
Use an isolated temporary filesystem when host temporary-file quotas require it.
No prefix copy, release publication or game launch is needed.

The two new native-policy contracts have scoped implementation and headless
integration evidence. Their original-comparison and live-replacement statuses
remain `none`. Existing aggregate retirement and original-model contracts keep
their previous scopes and evidence; this batch does not promote them wholesale.

Still pending: natural whole-game Quit/restart and startup-failure reachability,
arbitrary callbacks/threads/unknown aliases and writers, unadmitted flip/lease
interleavings, legacy physical-driver loss/restore, complete game pixel and
performance acceptance. Alternate-canvas ownership is the separate chunk 3/4
frontier and is not closed by ordinary DirectDraw surface retirement.
