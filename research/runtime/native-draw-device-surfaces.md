# Native graphics compatibility device and owned surfaces

The [2026-10-19 retirement follow-up](directdraw-retirement-20261019.md) supplies
the pinned optional PageLock/PageUnlock route and repairs the original DirectX
checker's lost Surface3/4 references through two guarded Release-site adapters.
Strict native-only normal Quit and Hard restart now have positive zero-owner
evidence; NVIDIA standalone startup also passes. Earlier sections below retain
their historical scope, including the former instruction-byte preservation
claim. Current staging additionally changes two ten-byte checker regions.
Full required caller/alias, physical-loss, pixel-equivalence and performance
acceptance remains an unresolved aggregate.

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

The2026-10-11 cache/validity increment preserves
`native-draw-cache128-performance-failure-20261011.json`:128 DirectDraw-disabled
frames complete on TITAN RTX, cache335 and RSS growth1,474,560 bytes pass, but
16.9312 original FPS/17.1461 Qt FPS still fail19FPS. Cropped CPU opaque validity
and metadata-LRU atlas changes pass independent native pixel/refusal/teardown
checks and retained32-queue GPU history comparisons; they do not prove original
pixel equivalence or semantic game teardown. Actual scene raster counts differ
between launches, so cross-run stage medians are observations, not controlled
speedup measurements.


The2026-10-11 [public gameplay proof](native-draw-public-gameplay-pass-20261011.json) completes a difficulty3 campaign through native menus/command presentation with real ddraw disabled and absent. Successful Zombie casting, player melee/lethal enemy depletion, physical movement, blocked spell attempts, player Fireball, nine portrait recenter cycles, Mini/resume and normal original Quit pass. The19FPS aggregate stress gate passes: combat command/paint36.7755/25.6537FPS over115.539seconds; resumed59.8363/34.6316 over60.465seconds. Worst whole-flow window11.9266FPS remains explicit. Stable portrait interior17.99seconds gives20.0111FPS, worst19.2484 and maximum channel error1. Compiler/source/input fingerprints remain stable;18 validator checks and original manifests before/after pass. Forced termination does not supply this normal-Quit proof, and normal Quit does not assert semantic COM reference cleanup.

The new `test-native-campaign.py --hardware` creates and stops a private NVIDIA display and refuses software fallback. [Hardware failures](native-draw-public-hardware-negative-artifacts-20261011.json) remain failures: difficulty1 native and real-DD control both fail summoning with stopped World snapshots; their activation cause is unconfirmed. Difficulty3 advances World but reaches a50ms bounded copy-order timeout (observed64/65ms), invalidates history and refuses GAP/falls back. No native graphics unknown-method record appears; absence alone cannot establish successful gameplay. NVIDIA public gameplay/defaults and the other OpenGL launch route remain pending. This public route still executes original CPU drawing; the independently proven V4 World pixel-roundtrip removal remains a separate selected19.54FPS milestone.

## Default owned native launch routes — 2026-10-11

The combined native Qt menus/command route defaults to MnmDraw, and the
standalone OpenGL launcher now stages the same guarded factory/surface services
and disables real ddraw. Explicit MNM_NATIVE_DRAW=0 remains a legacy diagnostic.
The standalone [startup proof](native-opengl-owned-startup-pass-20261011.json)
shows20 native main-menu frames on the actual Qt zink/NVIDIA TITAN RTX backend,
zero ordinary GPU readbacks/viewport uploads/recoveries, module absence, bounded
resource counters and zero consumer resources after abort. It checks35 native
compiler inputs, actual PE32 compiler/builder and protocol-generator inputs,
1111 immutable installed graphics inputs and original manifests before/after.
This proves bounded startup, excluding gameplay/FPS/movies/native Qt menu
replacement/normal Quit/semantic COM release. Original CPU drawing remains active.

The probe permits120seconds for first publication while a private Wine prefix
starts; recovery remains disallowed by the proof. Failed early cancellation and
undeclared compiler/schema inputs are preserved separately. The bridge builder
now emits actual compiler dependencies, retaining existing manifest fields.
The fresh full default gameplay attempt remains a recorded native presentation
failure; it is not promoted to a pass because another selected journey passed.
Universal19FPS,60FPS and whole-engine retirement remain outside these proofs.

The fresh [full default NVIDIA journey](native-draw-public-default-full-nvidia-pass-20261011.json)
passes casting/combat/movement, Fireball and refused spells, nine portrait
cycles, Mini/resume and normal Quit. Actual Qt backend identifies TITAN RTX;
real ddraw remains disabled/absent, with no unknown call/recovery/fallback.
Combat command/paint rates are36.8339/25.2133FPS over118.152seconds; resumed
57.9501/31.1704FPS over59.672seconds. Worst measured gameplay paint window is
19.9456FPS; portrait17.694seconds yields20.0068FPS with maximum channel error1.
All compiler/source/1111 installed input fingerprints and original manifests
remain stable. This is a selected full default journey, not a controlled
speedup or all-scene19/60FPS assertion. Previously recorded address-wait long
BltFast ownership failures remain failures. Additional diagnostic stage stores
alone cannot establish a causal scheduling fix. Original CPU drawing remains
active in this public campaign; V4 World delivery has its own evidence.

The [fully bound default rerun](native-draw-public-default-full-bound-nvidia-pass-20261011.json)
also passes the full campaign, requiring44 actual render bridge compiler/builder
inputs and28 generator/schema inputs in the prelaunch726-source declaration.
It measures25.1780 combat paint FPS and30.9131 resumed paint FPS; worst measured
active gameplay window19.8610FPS. The standalone shell uses the same120second
first-frame allowance as native menus, avoiding cancellation during fresh prefix
startup. Current eight copy/lock/DC/flip fixtures validate added stage diagnostics
separately; phase labels distinguish preparation(2), original call(3), derived
history commit(4), postprocessing(5), capture(6) and completion(7).

The fresh [4096-frame World result](native-draw-final-4096-performance-failure-20261011.json)
still fails full-path19FPS despite completed delivery/paint and resource bounds.
Median World40ms, between-World14ms and8941 readability region queries identify
remaining costs. This performance failure is preserved alongside earlier selected
passes; neither public UI callback throughput nor GPU readback removal hides it.

## Session-owned diagnostic file (2026-10-11)

Repeated public TITAN transitions reproduced copy-order refusal at the unchanged
50ms admission budget. Read-only launch-local stage inspection identified the
holder inside native `nd_status`'s `CloseHandle`: BltFast/original phase3, native
status stage5, held76/140/205/271ms. A separate earlier run held `surface_lock`
phase11 for74ms and longer. This locates diagnostic handle closure, not a measured
GPU raster stall; why the Wine call blocks remains unassigned. Retain the exact
[close-stage failure](native-draw-public-status-close-failure-20261011.json).

The optional status file now opens once at DLL startup. A nonwaiting diagnostic
writer lease rewinds and overwrites the same96-byte v2 record; it closes only at
DLL detach. Each call still checks real ddraw module presence and preserves
LastError. Open/seek/short-write failures retain terminal100, module presence101,
oversized path102. A diagnostic collision can omit a snapshot but cannot bypass
module refusal. This does not add native graphics API thread safety or durability.
Stage access performs an atomic read only and is resolved once during adapter load.

The [independent sink fixture](native-draw-status-fixtures-20261011.json) verifies
1024 complete overwrite records, one open, zero hot closes, one teardown close,
no growth, exact resource/peak/counter fields, LastError, module checks, collisions
and terminal errors. Assertions and ASan/UBSan pass with actual header dependencies.
The [current full public run](native-draw-public-status-file-full-nvidia-pass-20261011.json)
passes default native menus, casting/combat/movement/Fireball/refusals, nine stable
portrait cycles, resume and original Quit on TITAN RTX. Paints average25.7352FPS
in combat and31.3169FPS after resume; portrait20.0071FPS/channel error1. No real
ddraw, unknown call, recovery or fallback. Prior failures remain; repeated
transition/copy/DC/flip regression and current4096 World/fault revalidation are
pending before this paragraph is superseded by their source-bound records.
Original CPU drawing remains in public command mode; public V4 fusion, universal
19/60FPS, independent original pixels and semantic COM reference cleanup remain
separate milestones.

The source-bound [three-cycle TITAN repeat](native-draw-public-status-file-repeat-pass-20261011.json)
now passes the same previously failing transition route,19FPS and normal Quit,
with zero timeout/recovery/fallback. Its prospective claims include the diagnostic
file policy, bounded copy admission and default native menu route. The current
[eight contention fixtures](native-copy-status-stage-fixtures-20261011.json) pass
13 independent complete-frame comparisons plus expected50ms GAP refusals,
original counts/HRESULT/LastError and zero ordinary readbacks/uploads/resources.
Their actual compiler closure is30 native and42 selftest bridge inputs, plus28
protocol inputs. Older stage-fixture prose said44; its immutable mapping actually
has42. Production bridge's44-input closure is separately bound by public tests.
This is a count correction, not a historical hash refresh or new past execution.
Current [64 services cases](native-draw-status-services-pass-20261011.json) compare
2048 caller and22464 pixel words against Wine DirectDraw, plus351 native-policy
words and terminal unknown calls; native module absence and zero resources pass.
The current4096/fault World revalidation remains pending after diagnostic changes.

Current diagnostic-file [4096 World](native-draw-status-world4096-pass-20261011.json)
passes every unchanged budget at19.9697 original/19.9731 Qt FPS,zero World pixel
roundtrip,all4096 paints and zero native GPU teardown resources,ddraw disabled/
absent. Both peer-deaths at generation34 and subsequent fresh128 recovery pass.
The actual standalone default OpenGL launcher reaches21 native menu frames on
TITAN with zero ordinary readback/upload/recovery and zero bounded-abort consumer
resources. These supersede the pending revalidation notes above within their
recorded scopes. Full standalone gameplay/movies and public V4 fusion/60FPS remain
separate boundaries; public casting/combat and normalQuit are separately proven.

## Mapped diagnostics outside drawing-path file IO (2026-10-11)

The [full campaign failure](native-draw-public-status-write-failure-20261011.json)
reached combat/portraits but fell back during resumed gameplay. Its lifecycle
record identifies BltFast phase3 with82/146ms held duration and native status
stage4, the old `WriteFile`. The startup-owned handle avoided prior CloseHandle
stalls but every-call overwrite was still blocking inside the50ms copy lease.

Startup now creates a96-byte file-backed read/write view and initializes it.
Admitted drawing callbacks check real ddraw on every call, preserve LastError
and store24 bounded32-bit words under nonwaiting diagnostic admission. They call no
CreateFile/seek/WriteFile/flush/unmap/close. DLL teardown flushes/unmaps and closes
both owners. The v2 layout,terminal100/101/102 policy and resource peaks remain;
unknown-method terminal refusal logs are separate. Concurrent external readers
have no atomic-snapshot guarantee; crash durability and OS paging/writeback
latency are outside scope. No extra worker/thread/unload policy is introduced.

[Independent fixtures](native-draw-mapped-status-fixtures-20261011.json) pass1024
exact guarded snapshots,no drawing file IO,LastError,module checks,collision and
mapping/flush/unmap failures with assertions+ASan/UBSan. The
[actual Wine/native services](native-draw-mapped-status-services-20261011.json)
pass64 caller cases/22,464 pixels,16 startup cases/256 words and351 policy words,
with a readable96-byte final mapped file and zero retained resources. Current
public gameplay/World long-run/faults/standalone startup still require fresh
executions after this DLL change; no historical fingerprints are refreshed.

The current [full default public TITAN campaign](native-draw-mapped-status-full-nvidia-pass-20261011.json)
passes after mapped diagnostics: successful casting/combat/movement,spell damage
and both refusals,nine portraits(maximum channel error1),Mini Quit No/resume and
normal original Quit. All requested19FPS checks pass,real ddraw disabled/absent,
no fallback/recovery/unknown call and current8 native/44 renderer/28 protocol
inputs are bound. Earlier WriteFile failure remains recorded; no universal
causal speedup,publicV4fusion,60FPS or semantic COM teardown is claimed.

## Default-placement public offscreen request,2026-10-11

The resident public camera/selection run reaches a108-byte descriptor with
flags7,510x180 and caps0x800. The previous selected policy rejects this descriptor
withE_NOTIMPL; historical main-thread snapshots place the original constructor
0x58ad90 in its error MessageBox after COM slot6. The new native policy selects
owned system-memory RGB565 for offscreen requests without a placement bit,
reporting caps0x840. Explicit formats retain strict565 admission.

`native-default-offscreen-pass-20261011.json` retains the prospective source
claim, unchanged64-case Wine caller/full-word comparison and4native default
policy cases on108/124-byte descriptors. Unsupported extra caps and RGB555
remain refused. The policy has no physical driver placement or original-game
pixel-equivalence claim. Public resident throughput and bounded concurrent
primary-screen cursor-copy traffic remain pending.

## Resource/state extension — 2026-10-15

The [resource/state contract](native-resource-state.md) adds owned indexed8,
RGB555, RGB565, BGR24 and BGR32 storage; retained flagged8BIT|ALLOW256 palettes;
DC color snapshots and selected-bitmap admission; canonical clip lists and current
HWND visibility; and ordered clipped Blt failure effects. WORD producer lookup
admits565/555 only and never reinterprets indexed or wider pixels. Palette and
index ownership remain distinct. Idle flips, healthy host-memory Restore and
strict borrowed access retain their explicit native policies. No in-place
resident GPU recovery or held-lease physical driver flip equivalence is claimed.
Fresh resource/clip/DC/format and existing surface/services comparison records
are linked from that contract. Earlier paragraphs retain their historical scopes.
