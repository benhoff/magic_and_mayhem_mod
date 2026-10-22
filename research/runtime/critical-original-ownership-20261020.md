# Original ownership and downstream recovery

This follows the REC6 bounded recovery. The five broad Critical parents remain
open until their stated original behavior and ownership obligations are supplied.
New results use new IDs and prospective contracts; historical reports remain
unchanged. Native policy and hardware acceptance do not establish original
ownership. Original No-CD SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Prospective domains

* Surface release: unchanged 0x58b3e0, explicit external COM return boundaries,
  all object/registry bytes and ordered calls in 3,600 cells. Registry sizes
  1/2/5/128, first/middle/last positions, two independent 1/2/999/1000/1001
  zero-return thresholds, PageUnlock, palette, primary and null-Surface2 paths.
  Recover the shared retry counter and compaction without inferring COM or
  physical lifetime from numerical reference counts.
* Kind9 line: unchanged 0x4df840 and 0x580a90, plus ordinary-marker kind9
  composition through unchanged 0x5002a0. Each trace/pixel/queue matrix declares
  72,576 cells: both RGB formats, four shade values, two palette nodes, two
  independently encoded masks, three clips, 36 starts and 21 endpoint deltas.
  Trace-only execution has a separate synthetic raster binding; pixel and queue
  execution leave all three instruction bodies unchanged. Every destination
  word, including pitch/guard storage, is compared against independently
  generated native input. This does not recover fonts, arbitrary dispatch kinds,
  mutable queue callbacks, original palette construction or physical ownership.

* Kind8 ownership: six supplied seeds, 6,150 complete RNG checkpoints and
  93,312 construction/setter/destruction cases. Six default classes, both pixel
  formats, six dimension settings, nine heights, three wrapped positions,
  optional plane construction and both virtual destructor flags are compared.
  The original custom allocator runs with actual KERNEL32 imports; a checked
  forwarding delete-entry observer records frees while preserving the original
  allocator body. Terrain/cache facts are owned fixtures, not original game
  cache allocation or external writer observations. The x87 control word is
  explicitly 0x037f.
* Driver alias ownership: 18 real Wine built-in DirectDraw cohorts independently
  retain 0/1/3 credits on each Surface1/Surface2 interface, with PageLock off/on.
  Temporary canonical-IUnknown references are balanced before the test. Fill on
  one interface and a valid read lease on the other check shared bytes in both
  directions. Forwarding proxies observe actual PageUnlock and Release results.

## Executed findings

Confidence is high within these declared domains. All four final runs pass with
stable source fingerprints and original-manifest bookends. Raw inputs, binaries,
outputs, logs and failed attempts remain pinned under
`working/tests/critical-original-ownership-20261020/`. Immutable result copies
are under [the REC7 result directory](critical-original-ownership-results-20261020/).

| Behavior | Final run | Original/native comparison or observation |
| --- | --- | --- |
| RS.canvas-release-alias-lifetime | release-3 | 3,600 cases; all object/registry bytes and 1,515,384 ordered API events; 49,735,008 equal bytes |
| RS.world-kind9-line | line-3 | 72,576 cases each for ordered samples, concrete raster pixels and consumer composition; all 1,200 destination/pitch/guard words per pixel case |
| RS.kind8-owned-lifecycle | kind8-owned-2 | 6,150 full RNG checkpoints; 93,312 object/plane/lifecycle cases; 276,529,152 equal bytes |
| RS.canvas-driver-alias-retirement | driver-alias-5 | 18 canonical identity pairs; 1,728 shared pixels; 84 actual Release calls, 48 externally retained credits drained; nine successful PageUnlock calls |

### Surface release and COM ownership

Original 0x58b3e0 returns immediately for null Surface2, without registry cleanup.
Otherwise it optionally calls Surface2 PageUnlock, drains Surface1 and then
Surface2 Release results toward zero, releases a palette once, clears the primary
wrapper when matched, compacts the 128-entry registry and clears selected wrapper
fields. The two drain loops share a 1,000-attempt counter; Surface2 still gets its
first call after Surface1 consumes that budget, permitting 1,001 total interface
Release calls. PageUnlock and palette release are separate calls.

The actual Wine experiment confirms the ownership consequence: original release
consumes the extra retained credits on both interfaces. Those credits cannot be
treated as surviving owners or released again afterward. Canonical identity and
cross-interface shared pixels establish aliasing before retirement. A zero
Release return does not independently identify a physical driver allocation's
generation or prove its retirement. The native reconstruction records original
behavior; application services do not adopt this drain policy.

### Kind9 concrete pixels

Original 0x4df840 samples both line endpoints using a strict error-greater-than-
dominant-delta tie rule. Its ECX input is the first x coordinate, rather than a
generic object receiver. Each sample invokes 0x580a90, a saturating palette-add
indexed-sprite raster using the original RGB masks. Complete RGB565/555 output
matches for the declared RLE masks, shade values, palette nodes and clips.
Unchanged 0x5002a0 composition also matches for primary/software ordinary-marker
kind9 queues with one/three rows. Other helpers, fonts and finalizer pixels are
outside this result.

### Kind8 seeded construction, planes and deletion

Original 0x54e170 fills the 250-word R250 state in reverse order using the pinned
64-bit LCG high-half/carry transformation and diagonal bit constraints. Transition
0x54e200 advances two indices through the original 250-entry successor table and
XORs the old indexed words. Initialization is distinct from choosing the game's
strategic seed.

The conditional constructor's parent at 0x545e90 consumes six random values:
it chooses a component order, partitions a total of 16, then chooses three
component signs. The flag constructor consumes one value for its animation
field. All resulting object bytes and RNG state match, including seeded paths
that the earlier zero-stream fixture did not exercise.

Terrain-colour construction wraps coordinates, clamps z and dimensions, resolves
12-byte terrain cells and 356-byte block descriptors, and sign-extends the coarse
shade byte. Colour-plane construction at 0x547b70 requires a diagonal greater than
1.0, uses original float coefficients 1.3 and 0.7 and allocates width*height*6
bytes. Its colour setter compares requested integers with stored quantized words
before deciding whether to quantize and regenerate an existing plane.

The original allocator/free routines execute on real private allocations. The
observer records 103,680 frees, including plane-before-object ordering, and
93,311 consecutive object-address reuses; the original custom heap is retired
at the end. Nondeleting destruction preserves an allocated object for caller
cleanup. Deleting destruction is observed without reading the freed object.
Native storage uses owned values/vectors and logical free events, independently
of the original heap's physical layout.

### Alternate-buffer investigation

The pinned static report retains all five raw address occurrences of 0x6e1f68
and 512 nearby absolute-write candidates. Reviewing all four indexed candidates
finds bounded cleanup loops at 0x46acaf and 0x4be308 writing only
0x6de568..0x6de577, and at 0x4be838 writing 0x6ddd40..0x6ddd4f. Candidate
0x51a5ff is linear-disassembly misalignment across embedded dispatch data before
the actual 0x51a600 entry; it is not a confirmed executing write. These four
candidates do not establish an alternate-buffer writer. This scan does not
resolve arbitrary register-relative, bulk-copy, imported or indirect writes.
The live allocation/initial-clear/writer route remains unresolved.

## Remaining original recovery

The same five broad Critical parents remain partial. Their remaining work is
more specific after the four supplied children:

| Parent | Remaining recovery |
| --- | --- |
| NR.directdraw-retirement | Complete graphics callers/indirect flows and physical backing-generation, alias and writer inventory; original loss behavior across remaining driver outcomes |
| RS.canvas-create-lock | Physical allocation generations and all actual aliases/writers, held Lock/DC and other interface lifetimes; fill SURFACELOST path remains unobserved on this Wine driver |
| RS.canvas-bind-clip | Reachable original alternate-buffer allocation, initial clear and upstream writers |
| RS.world-dispatch-targets | Other concrete helpers and finalizer/font pixels, mutable queue callbacks and arbitrary object lifetimes |
| RS.world-kind8-virtual | Other classes/constructor variants, original seed selection, actor updates and original terrain-cache/external writer ownership; complete init/pixel composition |

These are outstanding recovery obligations, not merely unexecuted validation.
Full journeys, hardware acceptance and live replacement remain independent. No
broad parent completion or gameplay change is claimed.

## Preserved failed runs and environment

The first Linux attempts could not create compiler temporary files under the
host's exhausted per-user /tmp quota. The second release run executed but its
journal validator's event bound omitted PageUnlock/palette overhead; release-3
uses the corrected 3,009-DWORD ceiling. Kind8-owned-1 exposed the conditional
constructor's seeded velocity mismatch, corrected before kind8-owned-2. Line-2
passed but remains historical after that shared runner correction; line-3 is
fresh. Driver-alias-1/2 had no display; driver-alias-3/4 could not create host X
lock files. Driver-alias-5 uses an authenticated private TCP X server with a
private user/mount namespace and writable experiment temp directory. These
failures are retained separately and do not grant status credit.
