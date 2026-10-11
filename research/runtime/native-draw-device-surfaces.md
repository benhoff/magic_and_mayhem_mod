# Native graphics compatibility device and owned surfaces

The native PE32 factory and storage import no DirectDraw implementation. This is
an incremental compatibility boundary; isolated admission is not game retirement.
Unregistered device/surface methods terminate with exit97/98 rather than forward.
Logical display modes do not change the desktop. GPU World history remains owned
by the native renderer; this storage does not export or synthesize that history.

Static pinned NoCD evidence (SHA256
40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168):
fullscreen setup0x58a9b0 and windowed setup0x58aa90 create Draw1 and query
IID_IDirectDraw2 at0x5c7960 and IID_IDirectDraw4 at0x5c7970. Windowed caller
0x58aac3 invokes SetCooperativeLevel slot20 with flag8; fullscreen uses0x11.
The fullscreen Draw2 caller sets a five-argument mode at0x58aa51. Windowed
0x58ab3d creates a clipper (slot4), then calls its SetHWnd slot8 at0x58ab59.
Cleanup releases all three device aliases and restores the mode through Draw2
slot19. Confidence: high for these static call sites and GUID bytes; no claim
of dynamic reachability or whole-device recovery. The retained disassembly is
in the device discovery report. Original input manifests were checked before
and after discovery.

The isolated slice owns bounded VirtualAlloc RGB565 storage, eight-byte-aligned
pitch, selected108/124-byte descriptors, Surface1/2/4 aliases, rectangular
Lock/Unlock, source keys, opaque/keyed Blt, BltFast, color fill and frozen aliasing
copies. Explicit system-memory/offscreen requests are admitted; unknown flags,
formats, stretching, primary surfaces and flip chains are refused. Surface
identity/reference semantics and full output pixels must be compared against
unchanged Wine callers. Failed comparisons remain evidence; they do not justify
changing old source hashes or promoting game replacement status.

Remaining boundaries: primary/attached/flip storage, clipping, DC/palette
services, imported entry redirection, actual game execution with ddraw disabled,
CPU reads that require GPU World history, public menus/gameplay, teardown,
19FPS and60FPS. The prior4,096-frame GPU acknowledgement delivery passes but
its16.18FPS performance result fails the19FPS budget.

The first64-case execution matched all37,440 RGB565 words but exposed a28-byte
native versus32-byte Wine pitch and shared versus per-interface reference counts.
The correction uses eight-byte pitch and per-interface Surface1/2/4 counters;
IUnknown resolves to Surface1, with canonical identity across aliases. The
original outputs show AddRef4 for a created Surface1 and AddRef2 for a created
Surface4 after two IUnknown acquisitions. These observations are build/runtime
specific. Initial failures and the subsequent pass are retained separately.

The bounded native policy allows at most64 devices,512 surfaces and256MiB owned
pixel bytes. Its refusal corpus covers unsupported stretching/flags, invalid
rectangles/overflow, missing source key, bad descriptor size, repeated lock and
repeated unlock; failures must preserve all117 destination words. Capacity and
final owner counts are checked separately from original error equivalence.

The import preparation independently guards SHA256 of all four original file
sections plus the exact graphics descriptor and lookup words. It appends a
MnmDraw.dll/NativeDrawAnchor import and redirects only the copied graphics DLL
name RVA, preserving original IAT addresses and bytes. The offline proof covers
zero through three existing menu/scene/render imports, six byte mutations and
four duplicate staging refusals. This is preparation only: a loaded-module check
with Wine ddraw disabled and actual game execution are still required.

The current expanded64-case storage proof passes. A fresh128-lifetime factory
regression attempt was refused by the global capture reservation because another
Wine game started; its failure report is retained. Earlier128-case factory
validation remains historical after this source change. The current storage proof
covers Draw1/4 creation/query/cleanup only and makes no new Draw2 factory corpus
equivalence claim. The separate unchanged128-case regression remains to rerun.

Subsequent current-source factory and storage regressions pass in the next
services milestone:128 factory lifetimes/2,176 selected caller words and64
storage cases/37,440 pixel words, with ddraw disabled and no module or owners.
The earlier reservation refusal remains historical. See
[native DC/clipper/primary storage admission](native-draw-services-20261010.md).

## Backend-disabled startup and resident World admission

The selected128-queue original-engine session now completes with native owned
PE32 graphics storage and TITAN RTX Qt GPU presentation. Wine ddraw is disabled,
its module remains absent, the native DLL imports no DirectDraw, and no unknown
method refusal occurs. World delivery exports no pixels and writes none back to
the original World canvas. Original outside-World CPU drawing remains active.
This is bounded live observation, not independent original pixel equivalence,
whole-game compatibility or semantic quit. The private process stops while its
normal game owners remain active; process cleanup is not a COM teardown proof.

Startup had two dynamic graphics lookups beyond its static imports: the DirectX
checker0x4bd906 uses uppercase DDRAW.DLL at0x5e1264, and adapter enumeration
0x58f0c0 uses lowercase ddraw.dll at0x5f1a84. Both original12-byte regions
include two zero padding bytes. Guarded staging replaces only these24 data bytes
with MnmDraw.dll strings, leaving instructions and neighboring bytes unchanged.
Offline proof covers eight import compositions, ten mutation guards and eight
duplicate refusals. The separate LoadLibraryA/GetProcAddress bindings and
DirectX-failure modal were observed in a bounded private API trace; the static
startup discovery report records the original hashes and call sites.

The unchanged16-case startup corpus matches256 caller words against Wine for
normal cooperative level with a NULL HWND, primary creation, Surface3/4 identity
and per-interface reference cleanup. Native Surface3 uses the old108-byte
descriptor and separate refs. Current clipper/DC64-case full-pixel comparisons
pass with no retained devices/surfaces/DCs/clippers. Native policies separately
check two logical565 modes, enumeration cancellation, a256MiB owned host-memory
budget and allocation/refusal accounting. These are not original mode or GPU
memory equivalence. The actual session uses a saved device path, with zero
adapter enumeration callbacks; enumeration reachability remains a separate case.

Optional local diagnostic v2 is96 bytes, magic MNMDRW01, version2/size96,
module/resource counters, accepted creations/locks/copies/flips, peak bounds,
factory attempts, enumeration calls and last creation HRESULT. Reserved words
remain zero. Unknown device/surface/clipper methods write32-byte MNMDREF1 v1
records with family/interface-kind/slot/launch-local caller, then exit97/98/99.
Three isolated cases prove these terminal routes. No fallback is used.

The actual128-frame run fails the unchanged performance policy:17.6600 original
FPS,17.8759 Qt FPS and560 peak GPU cache frames against the512 limit. Median
World time39ms/p95 53ms/max72ms and RSS growth114,688 bytes pass their gates.
The native stage still costs23.53ms median; producer capture11ms and original
between-World work14ms also remain. No19FPS or60FPS claim follows. Historical
failed startup attempts and the rejected mandatory-enumeration assertion are
retained separately, without changing their recorded source hashes.

Inventory accounting retains0x58f0c0 as an unassigned static discovery gap.
Its documented entry bytes are not an original-execution recovered-range
exception. The central original code links cover the registered factory startup, surface
wrapper and DirectX-check branches. The newly observed enumeration entry/thunk
remain documented gaps rather than fabricated function coverage.
Fresh source execution reports retain pre-correction contract identities; no
historical claim hashes are refreshed to hide this accounting correction.
