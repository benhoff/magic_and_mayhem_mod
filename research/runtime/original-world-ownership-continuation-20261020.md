# Original World ownership recovery continuation

This work follows the five Critical parent contracts without changing their
completion requirements. Initial discovery identifies World queue storage owner
`4bfda0`, teardown `4bdd80`, auxiliary occlusion-mask allocation `501350`, base
registry construction/removal `4a4df0`/`4a4f70`, signed complete-row sort `4fff60`,
and occlusion writers `5013c0`/`5015f0`. The auxiliary mask is distinct from the
unresolved alternate canvas at `6e1f6c`; its allocation does not close alternate
canvas reachability or ownership.

Prospective comparison contracts are registered before execution. Allocation
providers are explicit owned boundaries; original ctor, destructor, base registry,
animation initialization, auxiliary clear and sort instructions are retained
unchanged. A null row allocation and null auxiliary allocation have different
original behavior and must not be replaced by invented symmetric refusal.

Raw discovery, read-only decompilation, failed attempts and comparison outputs
are pinned under `working/tests/original-ownership-continuation-20261020/`.
## Executed recovery

Seven independent models now match the unchanged pinned original bodies in
176,802 owned cases, comparing 1,298,569,036 output bytes. Each run declares its
scoped child contract and complete repository source closure before execution,
checks compiler dependencies and source stability, and verifies all 2,927
immutable files before and after. Native execution never receives original
output as input. Durable reports are in
`original-world-ownership-results-20261020/`; raw compressed outputs, binaries,
discovery and failed attempts remain in the pinned working directory above.

| Scoped contract | Original entry | Cases | Confirmed boundary |
| --- | --- | ---: | --- |
| `RS.world-queue-sort` | `4fff60` | 46,570 | Signed complete 36-byte row ordering, including unstable ties and 12,000-row capacity. |
| `RS.world-queue-owner-storage` | `4bfda0`, `4bdd80` | 360 | Row/mask allocation, base registry removal and ordered frees. |
| `RS.world-occlusion-mask` | `5013c0` | 49,664 | Existing-mask query and subsequent occlusion stamping. |
| `RS.canvas-startup-initializer` | `4e40d0`, `58d540` | 46,656 | Complete ordinary startup descriptors, call order, padding and pixel-format globals at owned provider boundaries. |
| `RS.world-occlusion-actor-writers` | `5015f0`, `5017e0` | 27,648 | Reverse traversal, row suppression and borrowed actor flag writes. |
| `RS.particle-owner-slot-lifecycle` | `5060a0`, `549820`, `549c90` | 1,296 | Slot reset and attached particle updates across authored reuse at the same address. |
| `RS.drawing-session-release` | `58acf0`, `58b3e0` | 4,608 | Full graphics shutdown with controlled COM callbacks and registry retirement. |

Confidence is high within these scopes. Their understanding and implementation
are scoped, and original comparison is recorded. Integration and replacement
remain none. The five broader Critical parents retain their partial statuses.

## Queue storage, borrowed rows and occlusion writers

The owner allocates 432,000 bytes for 12,000 rows without clearing row storage.
Offsets `+229/+22d/+231/+235` hold row base/cursor/count/capacity; `+239` holds a
separate 14,577-byte occlusion mask. The mask is fully cleared, with geometry
width344, height339 and pitch43. Destructor `4bdd80` frees row storage even when
null, optionally frees and clears the mask pointer, and removes its base registry
entry through unchanged `4a4f70`. It leaves the freed row base, cursor and count
stored. It never destroys objects borrowed by rows. Registry removal compacts
indices and selection while retaining the unused last physical pointer.

Null first allocation is admitted while the mask is constructed. Null mask
allocation faults during unconditional clear: a separate child process confirms
SIGSEGV with core dumps disabled. This is not a recovered safe refusal. Native
invalid extent/count refusals are explicit separate policies.

Sort `4fff60` exchanges whole rows and compares the first DWORD as signed. Its
middle-pivot partition has observable unstable equal-key order. Exhaustive
permutations through length8, signed extrema, duplicate keys, ordered/reversed,
random, organ-pipe and full-capacity controls compare every physical row,
including untouched inactive rows. Borrowed object words are never dereferenced.

Mask query `5013c0` reads sprite offsets32/36, performs wrapped origin subtraction
and exact mode-specific viewport tests, then transforms coordinates into the
43-byte-pitch mask. Its unaligned DWORD query and byte-swap precede stamping.
Kinds0/33 stamp every admitted height row even when the query stopped early;
the result describes prior coverage. Kinds1/8/9 and null/missing coverage have
separate early exits; admitted zero height returns true. All mask bytes and
guards are compared. Source sprite bytes remain borrowed and unchanged.

Sweep `5015f0` traverses backwards and skips row0. Covered rows become kind-2.
Actor pointers come from row DWORD5; the sweep sets/clears WORD5 bit0x40 and
WORD4 bits0x4/0x8000. Three class/clock-selected frames use first-match order;
catalog fallback is strictly index greater than lastFrame, admitting equality.
Shared actors, missing actors, duplicate mappings, both video sizes and viewport
gates compare full rows, actors and masks. This supplies specific writers, not
ownership of arbitrary external actors or sprite resources.

## Ordinary startup and alternate storage

The complete initializer requests primary and offscreen 108-byte descriptors,
clears the offscreen canvas and runs the actual pixel-format wrapper. Mode setup,
surface creation/fill and COM format query are declared normal-return providers.
Both resolutions and window/fullscreen branches cover raw flags, depth words
and unsigned padding wrap. Nonzero `6e1f78` resets padding to96/0; otherwise the
previous values remain. Alternate rendering-base pointer `6e1f68` and copy-source
pointer `6e1f6c` remain untouched, including nonzero sentinels. The complete
36-byte neighboring global region is compared.

The actual format wrapper publishes low WORDs of DDPIXELFORMAT DWORDs4/5/6
(RGB masks) to `6e1f7c/7e/80`, with the observed indicator bit from their inverted
OR. An initial mistaken decompiler-local interpretation failed at case69 and
was corrected before the final matrix; that failed run is preserved. Overlapping
recursive-disassembly roots must not be treated as one linear instruction stream.

The separately recovered alternate x87 copy and loader-zeroed globals are
documented in `alternate-canvas-recovery-20261020.md`. Neither that work nor the
occlusion allocation establishes alternate pixel allocation, initial pixel clear,
rendering-base/copy-source publication or actual session reachability.

## Actor slot reset and raw particle aliases

Unchanged `5060a0` clears exactly0xe4b bytes while preserving the slot address and
restoring its index. Actual callback deletion `52cbe0/52cc00` and list-node erasure
`52cc70` free old owned subobjects. It installs a16-byte callback (aliased at
`+708/+cf3`), a12-byte list header and a48-byte self-linked sentinel. Only the
low index BYTE of the list header is written; upper bytes retain allocator data.
Null callback allocation is accepted; separately executed null list and sentinel
allocations both SIGSEGV. Lifetime events distinguish allocation generations and
normalize pointers while alive, before frees or address reuse.

Pool-active `+4` and attached-follow gate `+e4` are independent. After reset,
retained particles keep their raw owner address. Actual `549820/549c90` updates
observe newly authored coordinates when that same address is reactivated. There
is no generation check or external alias repair in these bodies. Four snapshots
per case compare complete slot, particle, subobject and lifetime events. The
fixture authors reactivation facts: it does not execute the original activation
manager, full release cascade or pool allocation. Those remain recovery work.

## Full graphics session cleanup and the physical driver boundary

Unchanged `58acf0` drains a valid registry using actual canvas retirement,
performs mode-specific clipper/RestoreDisplayMode cleanup, then releases Draw1/2/4.
A null Draw2 takes an early exit without cleanup. Eight controlled callback modes
show that the original re-reads interface globals after earlier callbacks;
later bindings and fullscreen changes matter. An explicit null overwrites a
clipper published during its release. RestoreDisplayMode's HRESULT is ignored
while the attempted mode flag is cleared; a later callback can set fullscreen
again. Fullscreen cleanup can leave the clipper field stored. These are owned
COM controls, not actual driver result credits or complete alias closure.

A separate inconsistent registry control has a front wrapper with null Surface2.
Canvas retirement then makes no progress and original shutdown spins until the
one-second child timeout. The native model refuses atomically. No claim is made
that a normal original session reaches this malformed state.

The final `d98cur` and `d95cur` native-only loader probes both load DDRAW16 and
DIBENG, keep X alive and fail DDRAW LoadLibrary with error998/c0000005 before
DirectDrawCreate or surfaces. The supplied Win95/Win98 archives produce identical
DDRAW.DLL, DDRAW16.DLL, DDRAW.VXD and DDHELP.EXE hashes, so changing between them
does not supply an alternate runtime. Longer diagnostic output paths produced
an earlier1114 failure; shortened paths restore the dependency-assisted outcome.
An exact path-length limit is not established. A timed-out intermediate run is
also retained rather than counted as a completed admission result.

The read-only NE inspector maps one recorded failure's runtime selector to
DIBENG segment2 at file offset0x1740. Fault PCs0xce/0xd7 both contain `INT31h`,
with AX0x000b/0x000c: Get Descriptor and Set Descriptor, separated by a descriptor
flag change. Selectors are matched per run and are not stable addresses. See
`../formats/legacy-ne-dibeng.md` for fields, specification links and evidence.
This narrows the environment blocker; it does not identify a verified Wine fix,
establish publisher authenticity of the archived DLL, or prove all hosts fail.

## Remaining Critical recovery obligations

| Parent | Supplied by this continuation | Still unrecovered or unvalidated |
| --- | --- | --- |
| `NR.directdraw-retirement` | Complete controlled session cleanup and callback ordering. | Required route/interface inventory, external CPU/GPU aliases/writers, actual legacy driver outcomes and whole-session retirement/acceptance. |
| `RS.canvas-create-lock` | Complete ordinary initializer and pixel-format publication. | Physical provider failures/loss, external/imported writers and non-CPU storage ownership. |
| `RS.canvas-bind-clip` | Ordinary startup preserves both alternate pointers; occlusion storage identity is separated. | Alternate allocation, initial pixels, both pointer publishers and supported session reachability. |
| `RS.world-dispatch-targets` | Queue owner/allocation/free, signed sort, occlusion mask and actor writers. | External producer/resource reclamation during traversal and all required helper/session compositions. |
| `RS.world-kind8-virtual` | Real actor reset and raw alias behavior across authored slot reuse. | Original activation/full release cascades, external actor/terrain/resource writers, unresolved classes/exceptions/lazy providers and complete session composition. |

Fresh scoped comparisons do not renew historical wider evidence or assert native
production integration. No gameplay balance changes or live replacement were
made. Original recovery, current-code validation, supported-session acceptance
and replacement remain separate milestones.


REC11 continuation: the original activation/cleanup/release roots, map membership,
reference repair and pool admission now have five explicit owned-provider scoped
comparisons (47,520 cases). Full physical backing and downstream service ownership
remain independent. REC11 also supersedes the terminal DIBENG diagnosis above:
execution continues after both descriptor-service dispatches and fails later in
32-bit Wine code. No compatible legacy host or specific fix is validated. See
[the current closeout](open-recovery-closeout-20261021.md).
