# Save capture and native storage ownership recovery

The specific CSR1 Save-menu blocker is fixed. A fresh Apprentice Celtic1 native
journey completes Save, Load, both original display rebuilds, a two-minute Main
Load pause, reload, resume and normal Quit. Four selected World rasters match
unchanged original rendering code exactly. This is bounded rendering and host
ownership validation; complete restored simulation and whole-session equivalence
remain separate.

## Confirmed causes

The retained CSR1 terminal files first report checkpoint failure 11 on World 93,
then close at the finite 131,072-record source limit. Capture had selected the
finite V4 channel and disabled the ordinary V6 source-only drain. Metadata-only
checkpoints also incorrectly inherited the 9,999-file oracle limit, although
those checkpoints neither read pixels nor produce oracle files. The
[diagnosis](save-capture-recovery-results-20261025/diagnosis01.json) preserves the
original terminal hashes and decoded fields. Confidence: confirmed diagnostic
lifetime defect; this does not establish a broken original Save callback.

After repairing that lifetime, the next
[live attempt](save-capture-recovery-results-20261025/live02-failure.json) writes
its save and resumes, but stalls at native Load. The original channel reports
ready Load, generation 27, ACK 6 and the correct Mini parent/depth. The host
stored that ready ACK as its return baseline, then required another generation
change before restoring native ownership. The requesting Mini was generation
23. Confidence: confirmed host acknowledgement/readiness race.

The host now remembers the requesting Mini generation. It accepts either a
coalesced ready destination or readiness arriving later at the acknowledged
generation. Destination, Mini parent/depth, thread, exact request ACK and fresh
visit checks remain mandatory. No callback retry or extra request ID is added. A refused second launch leaves
the pending handoff intact.

## Implementation and controls

Save/Load diagnostic capture explicitly enables V6 source draining and compressed
V3 storage. Source chunks retain both their next-World generation and an
independent ordinal. They preserve native resources/history and never count as
completed Worlds. Legacy V1/V2 capture and independent/asynchronous V5 remain
separate. Flat benchmark normalization refuses V3 rather than dropping history.

The archive retains finite limits: 8,192 total records, 16 GiB raw and stored,
and a 256 MiB worker queue. Queue/storage failure remains explicit. The source
reader validates complete archives, including excluded tails, before native
checkpoint replay. Only metadata checkpoints bypass file-oracle limits; finite
pixel-oracle capture keeps its original limits. See the
[format contract](../formats/resident-profile-corpus.md).

The fresh [component execution](save-capture-recovery-results-20261025/component08.json)
passes the actual Qt writer and independent Python reader, record-cap and
malformed-stream controls, source-prefix/resource continuity, real PE32 producer
checkpoint regression, and 14 native Mini-to-storage schedules. The latter
cover both destinations, coalesced and delayed readiness, unchanged-generation
readiness, stale generation, wrong destination/ACK/thread and media deferral. The delayed-readiness case also refuses a second launch
without clearing pending ownership.
The producer fixture exceeds the old oracle count while using unreadable pixel
storage and confirms that metadata capture never touches it. Existing finite
oracle and transaction/admission controls still pass. Compiler dependencies are
explicitly checked against the declared source hashes.

## End-to-end and original comparison

The [final live execution](save-capture-recovery-results-20261025/live05.json)
passes with 544 imports, completed Worlds and verified returns; 23 source chunks;
376,832 records in the longest source-only interval; and a 120.008-second Main
Load pause. Canvas/context ownership changes across the original 800→640→800
rebuild. Capture retires completely with no truncation. There are zero ordinary
World readbacks, pixel forwards or history refusals. FPS is diagnostic. The
profile uses llvmpipe, skipped movies and legacy audio.

The [independent comparison](save-capture-recovery-results-20261025/comparison03.json)
checks four selected draws:

| Checkpoint | World generation | RGB565 words | Differences |
| --- | ---: | ---: | ---: |
| Before Save | 36 | 480,000 | 0 |
| After Save | 136 | 480,000 | 0 |
| After Load | 295 | 480,000 | 0 |
| After display rebuild and Main Load | 407 | 480,000 | 0 |

The original reference receives a diagnostic native entry canvas. Two additional
authored original baselines also agree at these checkpoints; that is not general
baseline independence. Native rendering receives no original oracle. All 544
captured Worlds and 23 chunks pass archive validation; replay preserves every
preceding source chunk through the selected checkpoints. Original manifest
checks pass before and after the experiments.

The preceding [60-second run](save-capture-recovery-results-20261025/live03-short-stress.json)
completes the functional journey, but its 114,688 records do not exceed the old
131,072-record limit. Its aggregate stress result remains failed. The longer run
meets the unchanged denominator. An earlier
[comparison admission refusal](save-capture-recovery-results-20261025/comparison01-admission.json)
retains an undeclared shared grayscale test header; the final declaration names
that dependency. Earlier build/provenance failures and exact intermediate sources
are retained, rather than replacing their hashes.

The [Python controls](save-capture-recovery-results-20261025/unit-controls02.json)
pass 19 storage and 26 dashboard tests. The dashboard's old native-launch
expectation fails identically at both committed baselines, as retained in the
[baseline control](save-capture-recovery-results-20261025/ui-baseline-proof01.json).
Its expectation now reflects the already-delivered bounded launch milestone;
broader ownership and complete replacement gates remain pending.

## Remaining boundary

There is no remaining implementation or validation step for this specific
captured Save/Load blocker within the recorded profile. Other difficulties,
fresh-process restore, complete simulation/save-state equivalence, hardware and
all failure/navigation schedules are not established by this run.

Original screen service recovery remains separate: direct targets 0x5571e0 and
0x4a39a0 and virtual slots +00/+04/+08/+0c/+28/+2c in the registered external
service frontier remain unassigned. Whole-screen class selection, control and
resource lifetime are not recovered by fixing native capture or host handoff.

Canonical reports remain in the local SQLite evidence store. Raw captures,
isolated saves, manifests and exact source variants are pinned in this batch's
owned worktree on the second drive. Verified local database backups preserve
these records. Ignored compiler trees are reusable execution output. Nothing is
published, and original media remains unchanged.

The earlier successful [live04](save-capture-recovery-results-20261025/live04.json)
and [comparison02](save-capture-recovery-results-20261025/comparison02.json) remain
unchanged. Final records above rerun the exact source after the launch-reset
guard review; their historical hashes are not refreshed.

The later [screen-service recovery](screen-services-recovery-20261025.md) closes
the local5571e0 selection and4a39a0 cursor-selector knowledge gaps listed above,
and recovers the4a5070 input dispatcher. [Concrete class lifetime recovery](screen-class-lifetime-recovery-20261025.md)
resolves the named tick slots for22 static tables and compares class11's
construction,vector retirement and callback invocation/deletion. Whole dynamic
screen selection,other class lifetimes and indirect alias closure remain open;
the Save/Load evidence itself is unchanged.

The [incoming-history review](save-capture-recovery-results-20261025/incoming-history02.json)
checks all 29 first-parent file transitions across `618cc26c..a50d0375` against
exact retained receipts, with no gaps. It adds no execution claim. The current
[ordinary dashboard reader](save-capture-recovery-results-20261025/ui-snapshot02.json)
accepts 1,376 registered behaviors and the separate Medium diagnostic rating.
The [focused summary](coverage/summary-save-capture-recovery-02-20261025.md)
retains historical staleness and the independent implementation, comparison,
integration and replacement axes.
