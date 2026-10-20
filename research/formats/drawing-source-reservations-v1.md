# COPY read reservation journal, version 1

`wine-source-reservations.bin` is a bounded diagnostic wire journal. It records
exclusion at the observed Wine resource writer entry points; it does not describe
physical allocations or close unobserved writers and aliases.

All fields are little-endian DWORDs. The 64-byte header begins `MNMSRG01`.
Words2..7 are version1, record size128, build0x40209ca7, record limit131072,
reservation capacity2048 and four observed mutation kinds. Words8..15 are zero.

The 128-byte rows have these fields:

| Words | Meaning |
|---|---|
| 0..3 | Record ordinal, phase, thread ID, tick |
| 4..10 | Submission, surface API entry, texture ID, generation, public resource ID, subresource, write epoch |
| 11..14 | Payload ordinal, command ID, worker ID, reservation state |
| 15..18 | Reason, reason detail, total acquired, total released |
| 19..31 | Zero |

Phases1..6 advance strictly through acquire, mapped snapshot, emitted packet,
worker entry, signaled current-context fence and release. Acquisition occurs
before the extra synchronous READ map. The pointer returned by that map is
used only inside its valid interval, then discarded on unmap.

Phase7 is terminal. Source/consumer/state fields and mutation kind are zero;
reason is1or2 and acquired equals released. No pending reservation is allowed.
Phase8 is a refusal followed by ExitProcess(99), with reason1write conflict,
2invalid acquisition,3duplicate submission,4capacity,5missing reservation,
6identity/order/epoch mismatch,7unsignaled fence,9conservative texture decref,
10unsupported mapped source,11pending close or12active/unresolved retirement. A refused/incomplete journal is
never admitted as a complete execution.

For write-conflict reason1, detail kinds1..4 identify writable public map, public resource copy,
opcode42 worker destination and public texture descriptor rebind. A conflict
is rejected before that observed mutation forwards. Public resource identity
is process-local and does not identify an allocation across separate resource
objects, lifetimes or launches. An acquired reservation conservatively covers
all subresources of that resource; same-resource self-copies are unsupported.
Other refusal details identify the failed phase/status; detail5 at close/capacity
can identify the separate retirement guard. Retirement leases are private
observer state, not a new source-version mutation kind. Acquisitions refuse
while an unresolved or aliasing texture retirement is in flight, and close
refuses outstanding retirement bodies.

Evidence and limitations: [implementation](../runtime/drawing-source-read-reservations.md).


Consumer-side admission independently decodes the complete raw source-version
and GPU-completion journals, including headers, reserved fields and terminals.
Their decoded fields and SHA-256 must match the supplied reports. Reservation
phase4 records establish the worker thread; both the source-version consumer and
the signaled GPU completion must name that same thread as well as the exact
submission, command and worker. This is a same-context observed-path join and
does not prove physical alias exclusion or all-context completion.
