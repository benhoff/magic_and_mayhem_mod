# Cursor-adapter COPY caller baseline — 2026-11-02

The [previous V2 baseline](copy-caller-provenance-20261102.md) resolved only the
whole-opaque wrapper: five original COPY wrappers were hidden behind native
cursor protected-view detours. This batch adds the separately guarded diagnostic
handoff rather than guessing through native stack frames. Historical V2 hashes,
899 code mismatches and 99 omitted output comparisons remain unchanged.

## Implemented boundary

The [V3 contract](../formats/native-copy-callers-v3.md) binds the actual original
wrapper-entry caller to the existing active native copy scope, protected views,
provider lifetime identities, owner/thread, exact original return slot/marker
and verified detour/trampoline. Explicit PE32 thunks publish that slot and pass
the same thiscall service operands. No stack search, extra lease, source/key
mutation, forced branch, rendering admission or queue/count/readback budget
change is introduced. Default copies retain the existing direct service path.

Only the top diagnostic scope can match; native-service and indirect callers
retain their actual return PC and remain unresolved if they are not original
direct E8 calls to the selected wrapper. A queue enqueuer is never substituted.
The handoff normalizes saved instructions and callers only inside the observer's
read buffer. Both original stack/executable memory and copy operands stay on
their existing paths.

Nineteen portable context refusals cover null/stale slots, wrong wrapper/thread,
marker, destination, alias and backing lifetime identity, and nested top-scope
selection/restoration. Nine actual Wine PE32 checks execute the production
thunks for argument order, RET cleanup, exact markers/slots, nested distinct
slots and LastError propagation. Fifteen caller-reader tests include V1/V2
compatibility, V3 profile/version refusals and launcher isolation. The existing
eighteen GPU branch tests pass. The earlier fourteen-site native resolver and
publication controls execute again as part of the live producer.

## Fresh finite observation

[`cursorquick01.json`](cursor-caller-results-20261102/cursorquick01.json) runs one
diagnostic public Quick journey with skipped intros, current native builds,
unchanged 752 declared source hashes, END, normal Quit and zero graphics owners.
Original manifest bookends verify all 2,927 files. This is diagnostic observation,
not a nominal player acceptance result or original-driver comparison.

| Scope | Result |
| --- | ---: |
| Closed application-call denominator | 1,027 |
| Selected COPY callers resolved | 1,002 / 1,002 |
| Native cursor handoff | 695 calls, 21 original direct sites |
| Scene handoff | 307 calls, 20 original direct sites |
| Total original direct sites / caller-routing groups | 41 / 44 |
| Unsupported fill sites | 25 |
| Code-mismatched/unreadable/indirect frames in this run | 0 |
| Strict independently compared native COPY outputs | 1,002 |
| CPU / CPU-keyed / GPU application COPYs | 900 / 369 / 102 |
| Detailed upstream GPU output links / from cursor wrappers | 102 / 54 |
| Missing output comparisons in this run | 0 |
| Natural self / keyed GPU-source calls | 0 / 0 observed |

The cursor handoff covers all five wrappers:

| Original entry | COPY route | Calls |
| --- | --- | ---: |
| `0x58c140` | Whole keyed | 241 |
| `0x58c360` | Rectangle | 9 |
| `0x58c4a0` | Rectangle restore | 317 |
| `0x58c8a0` | Keyed rectangle restore | 123 |
| `0x58ca90` | Keyed rectangle | 5 |

All 44 caller-routing groups match retained original static E8 candidates and
complete staged original wrapper SHA256 values. Fifteen of the 41 observed sites
still have no discovered function owner. No new function boundary is inferred.
The additional GPU routes come from ordinary rectangle/restore callers; all
369 keyed copies in this finite journey have CPU sources. These observations
distinguish keying from GPU-source reachability without changing either state.

The existing strict output validator closes all 1,002 application COPY intervals,
including four retained CPU-keyed pixel cases; seven post-retirement callbacks
stay outside the active census. The new detailed per-upstream output list remains
GPU-only. Fills, arbitrary aliases/readers, recovered original flags/ABI and
driver output remain separate. The previous run's 99 omitted comparisons are
not recovered by this new complete run.

Confidence is high within the pinned NoCD build and finite observed calls.
`NP.cursor-copy-caller-provenance` gains scoped live-observation support. The older
V2 observer and broad native cursor ownership results keep their historical
hashes; changed shared sources leave their broad current support pending rather
than reusing this diagnostic as a new equivalence run.

## Smallest next research batch

[`remaining-callers01.json`](cursor-caller-results-20261102/remaining-callers01.json)
subtracts this finite observation from the existing 156 static direct candidates.
It retains all 115 unobserved sites and their old owners/instruction contexts.
These are candidate calls, not operation acceptance counts or proof of absence.

| Wrapper family | Observed / static candidates | Remaining |
| --- | ---: | ---: |
| Whole opaque | 20 / 70 | 50 |
| Whole keyed | 5 / 17 | 12 |
| Ordinary rectangle | 2 / 2 | 0 |
| Rectangle restore | 10 / 36 | 26 |
| Keyed rectangle restore | 3 / 19 | 16 |
| Keyed rectangle | 1 / 1 | 0 |
| Explicit source/destination rectangles | 0 / 11 | 11 |

Review the **11 explicit-rectangle sites and 28 unobserved keyed sites** before
another scene sweep. For each, record source/destination producer provenance,
whether canonical identities can alias, whether the source can actually be
GPU-resident, and the controlling branch/action. Dynamic operands and unknown
owners remain hypotheses until traced. Existing context alone does not establish
scene semantics or aliasing. Then choose the smallest grounded scene/action and
declare its expected positive caller/route/output witness before capture.

No user scope decision is needed for that research. Natural self/keyed-GPU
positives, original ABI/driver comparison, arbitrary resource/readers, broader
reliability, EP06 whole operations 0/36, storage 8/156 and both final verdicts
remain open. Further whole-COPY completeness in one run cannot replace them.
