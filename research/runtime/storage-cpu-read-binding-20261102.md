# Retained storage CPU-read identity and completion binding — 2026-11-02

This bounded offline batch starts at bde2f6115. It reuses the four independently
compared nominal storage executions from the worker/storage matrix. It runs no
game, original executable, new GL replay or pixel oracle. Existing successful
pixel comparisons remain at their original hashes and execution phase.

The reader joins each V5 CPU_READ request to its captured full reply and the
existing independent comparison's exact bytes/digest. Command sequence,
monotonic ticket, command session, live surface creation/format/extent, history
generation/nonce and complete reply header must agree. Surface deletion and END
remain ordered. The separately retained terminal reply must equal the last
captured reply. Extra, duplicate, missing, stale or mutated inputs refuse proof.

The shutdown MNMRDC01 counter file has twelve little-endian u32 words: magic8,
version1, size48, last completed ticket, four completion counts, three reserved
zeros. It carries no session ID. Its binding is the immutable co-retained
artifact in one private acquisition, rather than a self-authenticating nonce.
Counts must sum to the contiguous request/reply denominator. Completion kinds
come from the captured producer source:

| Kind | Meaning within the native policy |
| --- | --- |
| 0 | Non-Lock materialization used by GetDC or explicit panel preflight |
| 1 | Explicit owned-Lock reader with family/route/caller information |
| 2 | Implicit owned-Lock reader with zero family/route/caller |
| 3 | Explicit reader outside an owned Lock |

Uniform aggregate kind counts can classify the observed completion category.
They do not recover individual original call sites or distinguish GetDC from
Preferences/Save/Load preflight. In particular, the generic lifecycle logger's
`values[2]=0` is a different field, not this completion-kind enum.

Lifecycle details are optional: at most128 entries, at most four per reason,
duplicate suppression and nonwaiting contention drops. Missing entries do not
prove missing callbacks. Selected details bind ticket, logical surface and
byte extent within one execution. Object addresses and thread IDs remain
process-local observations; they are not stable caller/resource identities.
These records contain no original return PC for kind0 materialization.

The new tooling policy may gain scoped headless support from fresh decoding
and refusal controls. Original CPU authority/reader contracts, per-call physical
identity/lease restoration, DC acquire/release lifetime, arbitrary readers,
masked-write semantics, whole operations and final decisions stay separate.
The report explicitly distinguishes reused pixel proof from new reader work.

## Recorded result

The [fresh binding](cpu-read-binding-results-20261102/binding01.json) reconciles
all 212 requests, complete replies, existing independent comparison digests,
terminal reply files and aggregate completion counts:

| Configuration | World extent | Requests / replies / completions | Detailed lifecycle entries |
| --- | --- | ---: | ---: |
| Default | 800x600 | 37 | 4 |
| Default | 640x480 | 59 | 4 |
| Explicit software | 640x480 | 61 | 4 |
| Explicit software | 800x600 | 55 | 4 |

All completion counts are kind0 (GetDC or panel preflight). This is a result for
these finite executions, not proof that other reader classes are unreachable.
The 16 detailed records bind ticket, logical surface and payload size. Another
196 requests have no detailed lifecycle record. No original return PC is
recovered for any request. Object/thread observations are retained per execution
without equating addresses between launches.

Ten focused controls pass, including mutation of every reply-header word,
changed payloads, duplicate/orphan replies, stale terminal files, counter sum/
reserved-field errors, conflicting lifecycle identity and sparse-log boundaries.
A control with completion kind3 and generic lifecycle kind0 proves those two
fields are kept distinct. Original-manifest bookends verify 2927 files.

Run from an isolated owned checkout with a new immutable result name:

```sh
TMPDIR="$TASK_TMP" python3 "$MAIN/tools/working-retention.py" run --reserve-gib 2 -- \
  python3 "$REPO/tools/bind-storage-cpu-reads.py" \
  --report research/runtime/cpu-read-binding-results-20261102/NEW.json
```

The source comparison and pixel checks are reused historical execution proof;
the new result validates decoding and identity joins only. The selected storage
acceptance remains 8/156, whole-operation acceptance 0/36 and both final decisions
pending. A future reader/lifetime capture needs an explicit per-request caller
and GetDC-versus-panel context, with DC acquire/release and physical lease events
as separate observations. The capped lifecycle log cannot supply that evidence.
