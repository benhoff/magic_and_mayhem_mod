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
