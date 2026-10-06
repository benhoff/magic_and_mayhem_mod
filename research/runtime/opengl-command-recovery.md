# Explicit fresh-session producer recovery

2026-10-06. Native policy `NR.command-recovery` adds `RenderRecover(const char*)`
to the PE32 bridge. Recovery creates a new stream after the previous stream is
retired; it never resumes a failed cursor or reuses the old ring. Original drawing
remains installed. This is a controlled native policy, separate from original
engine recovery behavior, real-driver equivalence and automatic Qt negotiation.

## Caller contract

The caller must quiesce original rendering callbacks and return application-owned
locks/DCs, then call `RenderShutdown`. A false shutdown result can still mean the
failed stream's worker joined and storage closed; a join/close that retained
worker-visible storage cannot recover. Recovery rejects a live/unjoined worker,
remaining queue/mapping, tracker contention, observed active locks/DCs or reserved
in-flight snapshots. Tracking cannot prove that an unobserved original lease was
returned, so caller quiescence is a precondition, not an inferred fact.

The caller creates a separate fresh v2 file and consumer. Its nonzero channel
session ID must exceed every previously spent ID in this process. It then calls
`RenderRecover` locally in the PE32 process with a NUL-terminated ANSI path of
1..511 bytes. The export preserves LastError and returns readiness. It performs
no observer COM calls. Successful recovery starts one new retry worker outside
DllMain; the existing startup and guarded ordinary exit paths apply again.

The main-image exit interceptor and installed original drawing callbacks remain
installed. There is no hook uninstall or process-wide restart implicit in this
export. Default bounded/v1 producers cannot invoke recovery.

## Admission and ownership

v2 startup and recovery require `GetFileInformationByHandle` to report the exact
v2 size, including zero high size word, and a nonzero file index. The volume
serial/file index pair identifies the mapped file, so an old file through a
hard-link alias cannot become a new session. Filesystems without usable file
identity refuse v2 admission. A sixteen-entry process history includes the
initial claimed file and any subsequently spent candidate. After that bound, or
UINT32 session-ID exhaustion, use a new process rather than recycle history.

A candidate must have a valid immutable v2 identity, READY state and zero
publication, acknowledgement, cancellation, failure and reserved fields. Invalid
paths, size/version/identity, reused files/IDs, already claimed/cancelled channels
and ownership/lifecycle contention refuse. Before claim, candidate resources are
private temporary allocations; rejection frees/unmaps them and leaves the
candidate and old producer unchanged. Claim is a CAS after guarded admission.
An identity change observed during claim spends the candidate file/highest
observed ID and refuses it without enabling a producer. Peer identity mutation
remains protocol failure, not a route to reopening the old stream.

The lifecycle guard protects worker handoff, the tracker protects checkpoint
retirement, and the drain guard excludes callback-side pumps during queue/cursor
publication. New state is fully initialized before worker launch. If thread
creation fails, the claimed candidate fails and closes; its identity remains
spent. There is no wait for peer acknowledgement in recovery admission.
Filesystem opens/metadata/mapping and allocation remain synchronous host work;
recovery belongs at an explicit quiescent orchestration boundary, not in drawing
callbacks. Worker join and shutdown retain their existing bounded policy.

## Fresh state

As of the 2026-10-06 metadata correction, ordinary recovery frees every owned
pixel checkpoint and detached lock base. It first synchronizes observed missed
operations and retirement/metadata epochs. It then preserves only current
application lifetime metadata: verified interface relationships, descriptors,
clipping, color keys, attachments and palette bindings/colors whose epochs remain
current. These observations continue during failed publication; consumer failure
alone is not an application object lifetime boundary. Pending missed retirement
still clears aliases; metadata uncertainty still retires affected descriptors.
No COM calls or references are added. New complete pixels must come from admitted
original operations; partial updates and keyed copies cannot invent a missing base.

Recovery resets primary-frame identity, session sequences, byte/pixel/resource
counters, wire palette membership, queue/failure/deadline counters and worker flags
under the new identity. Resource IDs start at one in the new stream and never
repeat within it. CPU generations and diagnostic budgets remain monotonic. Strict
CHECKPOINT admission remains separate: every observed resource must already have
complete owned pixels and current metadata, with all finite limits unchanged.
See [the current diagnosis](opengl-command-recovery-metadata.md) for evidence and
remaining actual-game limits. Earlier records below describe their historical
clear-all implementation; their hashes have not been refreshed.

Diagnostic archives use an incrementing eight-hex-digit session suffix:
`session-00000001.bin`, `session-00000002.bin`, etc. New sessions do not truncate
prior archives. Failed prefixes remain incomplete; complete accepted sessions
retain DELETE/END. Other capture/log budgets continue across recovery rather
than being reset to unbounded file production.

## Validation and remaining boundaries

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-recovery.py working/build/render-ring
python3 tools/test-render-command-queue.py
```

The independent PE32 fixture retains the same original surface addresses and
original pixels across shutdown/recovery. Borrowed rows are poisoned at Unlock.
A fresh native streaming/GPU consumer independently verifies every complete frame
per channel. It checks original call counts/HRESULT/LastError, full ACK of valid
streams, reset CPU checkpoint/alias/palette/session state, native cleanup and
ordinary zero readback/upload counters. Old terminal ring and archive hashes are
checked throughout all subsequent handoffs.

Cases cover orderly, cancelled, stalled, invalid-ACK and borrowed-lock failure
recovery; live recovery and held-lock/tracker rejection; invalid/missing/v1/
reserved/claimed/cancelled candidates, old files and hard links, repeated handoffs
and the sixteen-file history limit. New candidate rejection and a later valid
retry are separate assertions. The high-word/wrong-low-size and unusable-file-ID
admission cases also run under native queue fakes and ASan/UBSan.

Automatic consumer replacement/control-channel negotiation in Qt, actual
original-game recovery, in-flight callback stress, concurrent candidate corruption
execution, allocation/thread/metadata/mapping failure injection, full UINT32
counter exhaustion and partial/indexed/keyed re-observation scenarios remain
pending. The synthetic test uses a new consumer process for each channel; it does
not establish same-context Qt texture handoff or a user-facing retry action.
Original comparison and live replacement remain none for this policy.

Early three-case and nine-case runs remain under `working/tests/render-recovery/`;
the first native sanitizer sandbox attempt hit LeakSanitizer's tracing restriction
and was rerun outside that restriction. Final-source records are added separately
without refreshing historical evidence fingerprints.

The committed range `bc29824..4eb3977` was reviewed against exact receipts for all
25 files and twelve affected behavior contracts, with no unresolved accounting
gaps. See [the history report](coverage/committed-history-orchestration-20261006.json).

## Recorded final-source result

[Nine recovery scenarios](opengl-command-recovery-pe32.json) pass across 33
sessions and 66 independent complete GPU frames. The initial four failed
sessions preserve cancellation3, stall4, invalid ACK5 and ownership GAP2; their
fresh successors complete normally. Complete streams reach full ACK and
DELETE/END. Same original objects receive IDs1/2 in each new channel namespace.
The held lock and live/tracker/candidate guards refuse before claim; valid retries
succeed. The budget fixture admits sixteen total files and leaves the seventeenth
READY. Reset checkpoint/alias/palette/session counters are zero before new work.
All prior ring and archive hashes remain stable through later handoffs.

[Native/sanitized queue checks](opengl-command-recovery-queue.json) pass 25 queue/
configuration/file admission cases and eleven writer cases each, including exact
high/low size words and usable identity. [Eight pressure cases](opengl-command-recovery-pressure.json),
[five idle cases](opengl-command-recovery-idle.json),
[eleven startup/exit cases / eight frames](opengl-command-recovery-exit.json), and
[two continuous GPU cases /72 frames](opengl-command-recovery-gpu.json) pass.
Production and selftest PE32 builds pass. These are final-source native policy
checks; historical original comparisons keep their old fingerprints and are not
promoted by this result.
