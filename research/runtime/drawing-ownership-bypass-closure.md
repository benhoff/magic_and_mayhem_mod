# Public mutator and presentation resolver bypass closure

This increment removes specific observation bypasses. It does not complete the
physical backing/alias writer or global presentation-route ownership gates.
The exact retained reports below distinguish owned controls from original-game
execution. Prior evidence and source hashes remain historical.

## Public Wine mutator entries

With `MNM_DRAW_WINE_SOURCE_RESERVATIONS=1`, the first DirectDraw creation now
installs four pinned Wine11.16 public entry detours after the existing import
observers: resource map (RVA0x204490), unmap (0x204bc0), public region copy
(0x1df0d0), and texture descriptor rebind (0x221460). All four expected whole
instruction prefixes and import originals are checked before any site is changed.
The map prefix is7bytes, unmap/copy6bytes, and rebind7bytes; none contains a
relocation operand. Trampolines replay those instructions and jump back.

DirectDraw import wrappers call the trampolines, avoiding duplicate interception.
Other importers, internal callers of these public entries, and cached export
addresses now pass through the same wrappers. Writable maps, copy destinations
and rebinds therefore reach the existing pre-forward reservation guard.
Shutdown leaves the detours installed and forwards to the original bodies after
observation closes. The source reservation option remains off by default.

This covers calls entering those four functions. A raw CPU store, a private
Wine worker that writes without one of these entries, a distinct resource sharing
physical storage, and GPU work in another context remain outside this proof.
The installed module remains pinned by the launcher; no Wine file is modified.
The memory patches are scripted, prefix checked, and limited to the private run.
Installation occurs before the first original DirectDraw create is forwarded;
later installation into an active rendering process is unsupported.

## Presentation resolver refusal

`MNM_DRAW_STRICT_ROUTES=1` enables an explicit diagnostic policy. It refuses
selected `RTLD_NEXT` lookups before returning an unobserved function. It also
refuses lookup of seven alternate presentation entries through dlsym, EGL, and
both GLX proc resolvers: EGL damage swaps (KHR/EXT), NV post-sub-buffer,
NOK region swaps (both forms), GLX MESA sub-buffer copy, and OML timed swap.
Invalid flag values refuse. Ordinary caller-relative `RTLD_NEXT` forwarding and
missing-symbol errors retain their original assembly tail forwarding.

The capture launcher exposes `--strict-presentation-routes`, requires a delivery
observer, fingerprints the policy, and records the option in the private run
manifest. The child runner clears inherited policy values unless the manifest
requests the option. Default observation retains its existing behavior.

This is a resolver restriction. It does not intercept direct alternate calls,
versioned dlvsym/other loader namespaces, front-buffer writers, raw XCB/DRI paths, other GL contexts, or private driver
lookups. No closed-world presentation or scanout claim follows from it.

## Validation

Fresh execution reports and remaining original-game results are recorded below.


The [final controls](ownership-bypass-controls-20261017.json) pass9actual PE32
cached-entry cases,55resolver cases, and default/strict software EGL and GLX
comparisons (four and five selected swap routes each). All-site prefix mismatch
refuses before patching; map/copy/rebind through cached entries refuse pending
source writes before forwarding. Default-off and post-close forwarding pass.
A first provider-fixture failure is retained; its caller-relative lookup also
failed without observation until the second DSO became an explicit dependency.
The superseded passing controls retain their earlier dependency declarations.

The [fresh original run](ownership-bypass-original120-20261017.json) requests
both new policies and completes normal original Quit/child exit0 with stable
sources and2,927-file manifest bookends. Its first120accepted updates contain
120source queues,2,023input calls and298primary witnesses, with zero pending
callback frames or source-point gaps. Across the capture,2,661COPY reservations
complete through exact workers/current-context fences; all391selected EGL copy
regions and whole-source images agree at unchanged RGB tolerance1. These are
llvmpipe diagnostic reference observations, not native renderer parity.

The [unchanged interval gate](ownership-bypass-interval-gate-20261017.json)
still refuses exactly `all_original_backing_alias_writers_closed` and
`global_presentation_routes_closed`. No gate, denominator or tolerance changed.
The [existing reservation regression](ownership-bypass-reservation-regression-20261017.json)
passes19PE32 and six decoder controls, including the direct unhooked CPU write
counterexample. Public entry coverage does not make that raw store observable.
The [provenance](ownership-bypass-provenance-20261017.json) retains both failed
acquisitions, installed-module/prefix review and exact committed-history receipts.
One original acquisition failed at Wine prefix initialization before game launch;
using a different retained prefix succeeded. The loader failure's cause remains
unproved. It is not included as a successful engine run.

## Remaining ownership work

Physical closure requires an allocation/view owner and generation beneath public
resource identities, with every CPU mapping, private Wine writer, GPU writer and
cross-context dependency either covered or refused. Distinct resources sharing
storage cannot be declared disjoint from unequal object pointers, map data IDs,
or VirtualQuery regions. No unmapped pointer is retained or protected here.

Global presentation closure requires enforcement beneath resolver observation:
direct alternate/front-buffer calls, all context lifetimes and writes, XCB/DRI
and private driver delivery must be owned or refused. Adding lookup names cannot
establish that closed domain. Both global gates therefore remain false; this
increment removes specific bypasses without claiming full source ownership.


## Strict observer capability admission (2026-10-18)

Final review found that requesting strict resolver mode could previously accept
an X11-only observer. The launcher now requires the matching current EGL/GLX
policy build manifest, verifies its binary and relevant source hashes, and pins
its digest for child admission. Mixed/missing capability receipts refuse before
game execution. Default observation remains unchanged.

[Fresh controls](ownership-bypass-capability-controls-20261018.json) pass the
existing9PE32/55resolver/four graphics cases plus six build-admission cases: X11
and GLX-only observers, wrong binary, stale policy source and missing manifests
refuse; the matching EGL/GLX build admits. The
[fresh original run](ownership-bypass-capability-original120-20261018.json)
executes this handoff and completes normal original Quit/child exit0, stable
sources and2,927-file bookends. First120updates contain120source queues,
2108inputs and284primary witnesses, with zero pending frames or source-point
gaps. Across the capture,2556COPY reservations complete through current-context
fences and379selected EGL regions and whole-source images match RGB tolerance1.
The unchanged gate still refuses physical backing/alias writers and global
presentation routes. Prior results and hashes remain historical.


## Native output and backing ownership profile (2026-10-18)

`tools/run-native-owned.py` adds an explicit native-profile ownership boundary.
Qt keeps its host display; Wine runs on a fresh authenticated private Xvfb
server with MIT-SHM, DRI3 and GLX disabled. Before any Wine bootstrap, the final
runner starts a separate user/PID/network/IPC namespace with an empty device
tree, private temporary directories and masked `/run/user`. Only the private
X11 filesystem socket is exposed. Host abstract X11 sockets are inaccessible
in the new network namespace; host X11/Wayland filesystem sockets and graphics
devices are absent. Inherited loader, Wayland and session-bus selectors are
removed. The root filesystem is read-only except the separate disposable game
session and admitted Wine prefix. No original file is modified.

The private server cookie and socket identity are checked against a live
ancestor-owned receipt. Invalid or stale receipts refuse. The native Qt input
adapter authenticates separately to this private server; a wrong cookie refuses
without falling back to the host connection. Private XIDs cannot be embedded on
the host display. Original calls that bypass public presentation resolvers can
therefore only target the contained legacy display through the admitted output
endpoints. This is an operating-system boundary for the trusted game/Wine
profile, not a claim of security against arbitrary hostile filesystem IPC.

Admission requires resident native commands, owned drawing and all six producer
families in no-forward mode. A current native-draw compiler receipt is checked
before execution. `MNM_NATIVE_OWNERSHIP_REQUIRED=1` then refuses guard modes0–2,
invalid selectors and nonresident operation before any facade object is created.
Mode3 uses the existing allocation owner: anonymous section-backed public and
private views, protected stale public pages even during writable Lock, private
scalar-store emulation and exact typed history restoration. Allocation/protection
failure refuses rather than silently using an unowned fallback. This increment
requires that backing implementation; it does not take credit for its separate
CPU reader and GPU history validation.

Use a stopped admitted disposable prefix; the launcher refuses an active prefix.
For example, with a hash-pinned working installation:

```sh
MNM_MENU_GAME_SOURCE=/absolute/path/under/working/source-game \
  ./tools/run-native-owned.py --prefix /absolute/path/under/working/wineprefix
```

The wrapper records its server receipt, exact session admission and process
result under `working/tests/native-output-owner/`, then retires its own Qt and
server process groups. This remains an opt-in profile. The ordinary launcher,
original-reference physical alias/global route gates, original pixel parity,
complete-native release and hardware acceptance retain their separate scopes.


### Original image-row copy boundary

The first movement investigation under this profile retained a terminal mode3
record at No-CD `0x533af7`: an original `REP MOVSD` destination store during a
writable Lock, not an unknown GPU reader. The surrounding `0x533a00` clipped
image routine prepares EAX=source, EDX=destination and EBP=row bytes at `0x533aec`,
then copies DWORDs and residual bytes at `0x533af7`/`0x533afe`. This finding is
confirmed for the retained staged build; other image/format branches remain
unproved.

The opt-in adapter now checks the exact eight original prefix bytes before a
scripted in-memory jump. It restores current CPU history for either owned row
operand through the existing typed lease service before replaying that prefix
and executing the original copy. Bounds, flat ownership and forward direction
are checked; private-view operands and unsupported spans refuse. GPRs, flags,
LastError and FX state are preserved around preparation. No page is made writable
inside the exception handler, and no new instruction is accepted by its store
decoder. The original pixel algorithm remains in use and is outside the43route
native producer denominator. This boundary is an explicit CPU ownership service,
not a claim that all original drawing has been replaced.


### Current execution and remaining boundary

[Current controls](native-output-ownership-joined-kernel-20261018.json) pass10
Python admission tests,120C selector cases, kernel namespace/device/socket
exclusion, independently unchanged host pixels versus private drawing, and two
actual Qt input cases including wrong-cookie refusal.
[Nine actual PE32 schedules](native-output-ownership-joined-pe32-20261018.json)
pass section-backed protection, exact restore and original DWORD/byte row copies;
wrong prefixes and out-of-owned-range rows refuse before copying.

The [bounded game journey](native-output-ownership-campaign-07-20261018.json)
passes New Game/Enter, gameplay, Mini Cancel, Quit No/resume/Yes and normal
original exit0. All7Worlds complete/import/return in balance;1029native producer
calls have zero forwarding. Host embedding fallback is absent, sources stay
stable and all2927original files verify before/after execution. This is software
live observation, with movies and route pixel comparisons omitted.

Movement/HUD stress is still **not accepted**. After the fixed row-copy boundary,
[campaign06](native-output-ownership-campaign-06-20261018.json) stalls at an
original DirectDraw error dialog contained on the private server. Its retained
surface log reports `application_bltfast 80004001` with flags`10`; the precise
unsupported surface/format condition is unproved. Containment prevents host
presentation leakage but can leave Qt showing the last frame while an original
modal waits. This is a concrete remaining compatibility/failure-reporting task,
not evidence of a completed session. Full media/save/restart, hardware performance,
original pixel parity and complete-native release remain pending.

[Provenance](native-output-ownership-provenance-20261018.json) retains failed and
superseded attempts, fixture/setup corrections and incoming history accounting.
Prior hashes are not refreshed. The original-reference ownership gates remain
false; the implemented policy owns admitted native backing and contains the
named host output endpoints instead of claiming original Wine closure.


### Subsequent shared-source edits and current validation limit

The ownership control suites and campaign07 above passed for their exact recorded
snapshot. A concurrent native-only admission change subsequently edited shared
Qt, launcher and backing sources. Two combined retries again completed the
original gameplay/Mini/No/resume/Yes/normal Quit flow, but their source-stability
checks correctly refused acceptance: first `tools/native_only_contract.py`, then
that module and `apps/qt-shell/native_only_session.hpp` changed during execution.
The current game attempt completes7balanced Worlds and948native producer calls
with zero forwards; its aggregate success is **false** because sources changed.
The separate `--native-only` option was not exercised by these ownership retries.

[Concurrent provenance](native-output-ownership-concurrent-provenance-20261018.json)
preserves both attempts and the successful intermediate kernel/PE32 controls.
Current combined-tree validation remains pending until the shared code settles
and the declared runs are repeated. Historical stages/evidence remain recorded;
no hashes are refreshed and these moving-source runs do not promote acceptance.
The final change-gate attempt also detected the concurrent source/index drift;
the earlier audit's zero-error result belongs to its retained snapshot.
