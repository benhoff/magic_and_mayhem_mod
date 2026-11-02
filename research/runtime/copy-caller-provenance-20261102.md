# Upstream COPY caller provenance — 2026-11-02

The previous COPY tail work proves native destination self-consistency for the
finite retained histories. Natural GPU self-copy and keyed GPU-source callers
remain unobserved. The older route census identifies shared DirectDraw wrapper
return sites; it does not distinguish their upstream game callers. This batch
adds bounded diagnostic provenance before choosing more scene sweeps.

## Pinned original frame finding

`copy-caller-results-20261102/layouts01.json` derives seven original wrapper
layouts from the supported NoCD executable, SHA256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The existing [wrapper export](native-copy-route-static-20261017.json) retains
their complete instruction windows and 156 direct-call candidates, including
55 without discovered owners. Original manifest checks bracket the new byte
inspection. These are static findings, not new function boundaries or driver
execution.

| Wrapper entry | Local bytes | Saved registers | Distance from API return slot |
| --- | ---: | ---: | ---: |
| `0x58bf40`, whole opaque | 48 | 4 | 92 |
| `0x58c140`, whole keyed | 48 | 4 | 92 |
| `0x58c360`, rectangle | 24 | 4 | 68 |
| `0x58c4a0`, rectangle restore | 32 | 4 | 76 |
| `0x58c8a0`, keyed rectangle restore | 32 | 4 | 76 |
| `0x58ca90`, keyed rectangle | 24 | 4 | 68 |
| `0x58cbc0`, explicit rectangles | 16 | 4 | 60 |

At the fourteen selected Blt/BltFast call sites, the offset is local bytes plus
16 saved-register bytes, 24 bytes of six stdcall API arguments and the API
call's four-byte return address. The next word is the original wrapper's
destination argument. The opt-in observer requires a matching complete wrapper
code guard, the expected API bytes, readable return/destination slots, an
upstream direct E8 back to that wrapper, and a destination wrapper whose Surface
field at +8 equals the actual API target. The producer checks complete staged
wrapper SHA256 values and upstream call bytes again after capture.

This uses fixed reviewed slots, not an original frame-pointer chain. Unsupported
API sites, code drift, unreadable/mismatched slots and indirect upstream calls
remain explicit states. They are never guessed into a known caller. See the
[versioned wire contract](../formats/native-copy-callers-v1.md).

## Controls and acquisition scope

The native controls cover all fourteen table entries and reject wrong code,
methods, destination identities, direct-call targets, unreadable/overflowing
slots and exhausted/invalid publication. Thirteen Python methods check exact
companion/census binding, frame identities, unknown-state preservation,
counter/tail corruption, versioned Scene metadata, positive output-credit
boundaries and diagnostic versus nominal launcher admission. The existing
eighteen GPU branch proof tests also pass.

The observer is selected by `--copy-callers` on the existing public diagnostic
Quick Battle route. It adds no game action, key/alias mutation, drawing input,
COM query, timeout extension or admission fallback. Original API results and
GetLastError remain on their existing paths. The hook retains its own compiler
frame to locate its API return slot; original EBP is not interpreted as a chain.
The 8,192-COPY/32-GiB native output budget and its refusal behavior are unchanged.
No nominal acceptance or complete-operation claim is made by this diagnostic.

The first attempt, `quick01.json`, was refused before native build or game
execution: the new diagnostic accidentally selected the nominal census flag.
The unchanged nominal guard rejected that mixed, skip-intro route. The initial
producer then reported the absent acquisition as a join error. Its exact source
preimages and launch log are retained. The corrected route and its guard tests
are distinct from this failed attempt.

The next attempt, `quick02.json`, stopped during staging because this isolated
checkout lacked its audio asset manifest. No original game execution occurred.
`assets-setup01.json` records the subsequent fixture preparation with manifest
bookends. `quick03.json` then reached normal Quit/zero owners and complete native
COPY output, but all 855 selected wrappers failed the original-body guard:
the already installed Scene/cursor hooks were still present. That guard failure
is retained; the observer never read arbitrary stack candidates to fill it in.

Version 2 adds the read-only [Scene handoff](../formats/native-copy-callers-v1.md).
It validates the existing detour/trampoline and exact active frame, substitutes
saved original bytes and caller only inside the diagnostic reader, and checks
both wrapper Surface identities against actual API operands. The cursor adapter
has five additional protected-view detours; its entries remain code mismatches.
Original instructions, frames, operands, cursor lifetime and rendering admission
are unchanged. The opt-in build and live runs validate this bounded integration;
the old version 1 result and its complete output proof stay separate.

`quick04.json` resolved 324 calls but retained 833 code mismatches and 25 fills.
Its strict output join failed because the unchanged 512-MiB worker queue dropped
seven comparisons. Its exact old host sources are preserved in the paired store
under `copy_caller_scene_guard_preimages/quick04`; previous failed-source roots
also remain pinned. Neither those missing witnesses nor the result was rewritten.

## Reviewed bounded result

[`quick05.json`](copy-caller-results-20261102/quick05.json) executes a distinct
prospective scope: preserve the complete closed metadata denominator, then credit
only resolved GPU callers with complete independent output intervals. It does
not require unresolved frames or missing output to disappear for a positive
individual-call observation. Strict whole-output validation still fails.

| Result | Observed scope |
| --- | --- |
| Closed companion/V2 census | 1,269 application calls, no active misses |
| Resolved original callers | 345 calls, 20 E8 sites, 21 caller/routing groups |
| Resolved CPU / GPU routing | 267 / 78 calls, all through whole-opaque wrapper |
| Complete output linked to resolved GPU caller | 77 calls |
| Unresolved cursor-adapter code guards | 899 calls |
| Unsupported fill sites | 25 calls |
| Natural self / keyed GPU-source metadata | 0 / 0 observed; no absence claim |
| Lifecycle | END, normal Quit, zero graphics owners; original manifest verified before/after |

All resolved upstream instruction bytes match the staged original executable
and existing static direct-call candidates. Nine of the twenty call sites still
have no discovered owner; no function boundary was inferred from a caller alone.
GPU output on this route comes from original call instruction RVA `0x6b9fc`
(return RVA `0x6ba01`, wrapper RVA `0x18bf40`); the same caller also occurs twice
with a CPU source. This is a concrete caller/route distinction concealed by the
previous shared API return PC.

The separate native output proof checks 161 of 162 original GPU intervals and
replays retained pixels independently in Python. One resolved GPU interval is
missing its output and receives no credit. The report lists every missing CPU
and GPU interval; diagnostic truncation and whole-output refusal remain visible.
CPU upstream provenance earns no new per-caller pixel proof. Neither the
8,192-COPY count cap nor the 512-MiB queue/32-GiB readback budgets changed.

Confidence is high for the pinned byte/layout derivation and these observed
direct caller identities. Reachability outside this finite journey is unknown.
The new native policy `NP.copy-caller-provenance` records only this integration;
recovered original COPY comparison and live replacement remain unchanged.

## Next bounded work

1. Add a separate opt-in, versioned read-only handoff at the existing native
   cursor copy scope. Capture the actual wrapper-entry caller and bind the
   active protected views, original trampoline and exact physical return marker.
   Preserve native-service/indirect callers as unknown; never substitute a queue
   enqueuer for the caller that actually executed the copy.
2. Test stale/wrong wrapper, thread, generation, marker and nested-scope refusals,
   then run one bounded Quick diagnostic to resolve the five adapter-owned
   wrappers. Avoid another broad scene sweep before that provenance exists.
3. Use the recovered caller/branch map to select a scene or document the exact
   missing natural self/keyed-GPU prerequisite. Synthetic branches stay separate
   from natural reachability and original-driver comparison.

No user scope decision is required for that diagnostic follow-up. Natural
self/keyed-GPU positives, indirect/unknown ownership, arbitrary alias/readers,
original-driver semantics, recovered caller ABI, whole operations and both final
verdicts remain separate obligations. EP06 whole operations remain 0/36 and
storage obligations remain 8/156. Historical incomplete captures stay immutable.
