# Concrete screen ownership and World retirement

This continuation recovers the resource-release callers behind every concrete
table in the earlier 22-table shared-tick census, normal enclosing lifetimes
for the core menus and result screens, and the complete local World exit.
The original is No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Models use original 32-bit wire identities and independent owned storage. They
install no hook and make no gameplay changes.

## Concrete release providers

`reconstruction/rendering/screen_resource_ownership.hpp` models the whole local
slot+20 callers below. The original tables bind each caller to shared tick
5595d0; the runner verifies both slots against the pinned executable. Table
5c6564 has purecall59b823 at+20. It is an abstract boundary, not a no-op owner.
This closes the release-caller denominator of the existing static census;
it does not establish that the census contains every possible dynamic class.

| Table | Release entry | Distinguishing ownership behavior |
| --- | --- | --- |
| 5c5ebc, campaign report | 473c10 | Frees its 21-label and one-button pointer arrays; no initialized guard |
| 5c5ef4, Quick results | 474fe0 | Retains 26-label, three-button and four-other-control pointer arrays |
| 5c6120 | 486260 | Retains one-button and one-receiver arrays |
| 5c63e4, Main | 4a7030 | Retains six-button array; disposes label and receiver |
| 5c641c, Quick Battle | 4a7d60 | Retains four-button array; disposes label and receiver |
| 5c6454, class11 | 4a84e0 | Disposes callback+47; no surface alias clear in this caller |
| 5c648c | 4a8d00 | Disposes receivers and five cookie-backed arrays; no initialized guard |
| 5c64c4 | 4a9e20 | Two buttons, two labels, list/text/dialog providers and five receivers |
| 5c64fc | 4aae70 | Three buttons, two labels, list/text/dialog providers and six receivers |
| 5c6534 | 4ac510 | 40 labels, three buttons, four controls, 17 controls, three receivers and six controls; embedded resource+48b |
| 5c659c | 4af070 | Multiple retained control arrays; clears embedded service+5f and rewrites initialized after callbacks |
| 5c65d4 | 4b0060 | Label/text providers, two buttons and two receivers |
| 5c660c | 4b0f50 | Retained arrays and embedded service+5f |
| 5c6644, Mini | 4b2010 | Signed dynamic button count+57, reread after each iteration; two receivers and optional dialog |
| 5c667c | 4b2ae0 | Four distinct cookie-backed arrays and one receiver |
| 5c66b4 | 4b40c0 | Unguarded bitmap, surface, receiver and seven cookie-array cleanup; conditional+8b clear |
| 5c66ec | 4b70d0 | Base lobby cleanup, including 40 labels, eight controls and five receivers |
| 5c67f8 | 4b8730 | Base cleanup followed by five derived resources even if base initialized guard skips |
| 5c689c | 4b95b0 | Base cleanup followed by four derived arrays and one cookie array regardless of base guard |
| 5c6940 | 4bb580 | Two buttons, one label, text provider and two receivers |
| 5c6978 | 4bc1c0 | Two buttons, one label and one receiver array |

Resource destructor internals, real allocators, bitmap providers and the CRT
array-destructor iterator remain explicit external endpoints in these caller
comparisons. A named destructor target does not establish its transitive
semantics. Virtual disposal gets argument1; it owns its own deleting behavior.
The caller must not add another unconditional free after that virtual call.

Most initialized guards clear+8 before calling providers. Their surface cleanup
calls58b3e0, rereads owner+13, calls58b3e0 again on the reread receiver, frees
that saved second receiver, then clears+13. This double call is original behavior.
Pointer-array entry cleanup likewise rereads the owner array field after each
destructor/free before clearing an element. Each next iteration loads it again.
The tests retarget arrays and surface pointers to check this ordering.

The shared vector clear4cacb0 collapses its admitted nonempty end to begin and
clears+10, retaining the backing allocation and pointed-to elements. Destruction
4cab70 separately frees the backing allocation, even when null, then clears its
pointer fields. This distinction accounts for reusable menu arrays and storage
that survives a screen transition.

## Enclosing Main, Quick, Mini and result lifetimes

Normal constructors4a6a60,4a7970 and4b1d80 initialize screen IDs3,22 and17 and
allocate six-,four- and five-entry button arrays respectively. Mini also
allocates two receiver entries and disables its fade-allowed byte. Complete
owned patterned bytes compare, including untouched bytes and the vector
initializer's original ECX-high-byte copy. Allocations are immediately written
without a null check. These normal-return comparisons admit successful writable
allocations; they do not invent allocation-failure recovery or validate original
SEH unwinding. The fixture supplies a private i386 FS segment and verifies that
normal returns restore its exception-chain head.

Destructors4a6b00,4a7a10 and4b1e30 reset the concrete vtable, execute the actual
recovered release caller, free the retained button arrays (plus Mini's receiver
array), then retire the shared vector. Tests compare populated destruction and
two complete construction/destruction cycles.

Campaign report exit4748a0 clears initialized, retires/free its optional bitmap
at+2a5, retires embedded canvas+25f, invokes resource cleanup, then writes
return+43=1 and clears+53. Enclosing destructor473b00 conditionally executes
that exit, repeats resource cleanup, retires static resource5fcc80, retires the
embedded canvas again unconditionally and destroys the vector. Quick-result
destructor474d70 releases resources, frees three retained pointer arrays,
retires embedded bitmap+5b and destroys the vector. These complete local
callers now have independent models and unchanged-original comparisons.

## Complete World exit

`reconstruction/rendering/world_screen_retirement.hpp` models complete
46ab30..46ae8c. This screen's table5c5dd8 has a different tick; it was outside
the shared5595d0 census. The model includes the one-shot shutdown flag,
conditional service/registry retirement, bounded iterator traversal, both
bitmap stores, the segmented pointer queue and its block/map frees, two canvas
detach calls, final World owner clear and conditional clock snapshot.

The registry counts are reread around callbacks. The pointer queue advances one
DWORD per removed element, retires an exhausted block at its boundary or when
the count reaches zero, resets the original queue fields, and frees the map
when empty. Two-block, first-boundary and last-item cases compare at every
endpoint, rather than merely checking a final zero count. Original SETE AL
preserves EAX's upper24 bits for56d0a0's argument; the model and fixture retain
that DWORD instead of normalizing it to a host boolean.

Simulation, asset, network/audio, heap, bitmap and platform internals called by
this exit remain authored endpoints. The local comparison does not claim their
semantic recovery, full restored simulation or a physical allocator lifecycle.

## Evidence, confidence and limits

Run `tools/test-screen-resource-ownership.py --suite screens` or `--suite world`
with new owned `--output` and immutable `--report` paths. The runner brackets
original-consuming experiments with manifest verification, pins the executable,
checks complete local byte/branch censuses and fingerprints compiler inputs.
The fixture preserves and rechecks every compared original body. Only named
external endpoint adapters change in its private mapping. No original file is
modified and no game process runs.

Comparisons check complete owned bytes and full storage snapshots at every
endpoint, including patterned bytes, sparse resources, signed counts, repeated
release, receiver retargeting, callback mutations and normal enclosing cycles.
Optimized i386, i386 ASan/UBSan and native64 ASan/UBSan must have identical
semantic checksums. Final source-bound report identities and case counts are
recorded in the coverage ledger. Confidence is high within these admitted local
domains.

Retained early attempts include compiler tmpfs quota failure, two fixture
storage-admission failures, a pass made historical by adding enclosing lifetimes,
a compiler lambda diagnostic and a retained narrower pass before strengthening the array-retarget controls. No failed or stale attempt grants acceptance.

**Still separate:** other classes' complete construction/enclosing destruction,
resource initialization/prepare internals, World entry and transitive simulation
providers, full dynamic registration and computed aliases/writers, real heap and
fault reachability, asynchronous retirement, installed semantic takeover and
end-to-end presented-session equivalence. Screen transitions, object destruction
and full-game replacement remain distinct milestones.

The existing58b3e0 recovery clears canvas+28 only on its non-null Surface2 path;
the null-Surface2 path returns before that clear. Linking this fact to class11's
uncleared callback alias does not close later-use reachability. No new use-after-
free finding or general alias-lifetime acceptance is asserted.


The final [screen comparison](screen-ownership-results-20261025/release08.json)
passes3,429 release,1,215 menu and405 result schedules,including738 actual
array-retarget schedules. The [World comparison](screen-ownership-results-20261025/world03.json)
passes1,536 schedules. Each passes all three configurations with matching
checksums and complete current source bindings. The [audit](screen-ownership-results-20261025/audit02.json)
has no errors;the [dashboard reader](screen-ownership-results-20261025/ui01.json)
requires current comparison support for both new contracts. The
[committed-history review](screen-ownership-results-20261025/history01.json)
checks233af3b3..d77015d4:6 commits,2,516 file transitions and no missing exact
receipts. Broader historical staleness remains visible independently.
