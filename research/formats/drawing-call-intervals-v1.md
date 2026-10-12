# Original-active drawing observer call intervals V1

This diagnostic is emitted only in physical API mode. It is additive to
`MNMDAPI1`; older physical runs without this artifact remain historical witnesses.
It records observer order, not hardware completion or an owned producer timeline.

The header and each record are64bytes, little-endian32-bit words. The header has
`MNMDINT1`, version1, record size64, pinned build prefix`0x40209ca7`, maximum786433
records and ten zero reserved words. The bounded journal uses less than49MiB.

| Word | Meaning |
| --- | --- |
| 0 | Consecutive event ordinal |
| 1 | 1 entry,2 original-return observation,3 observer completion,4 terminal |
| 2 | Original API entry ID |
| 3–6 | Original record type, method, destination generation, input-related generation |
| 7–8 | Original thread ID and caller address (diagnostic identity only) |
| 9 | GetTickCount observation (wrap allowed; ordering uses event ordinals) |
| 10 | Original result for return/completion, zero at entry |
| 11–12 | Physical plane ordinal and API journal ordinal at completion only |
| 13 | Pending observer entries before completion releases ownership |
| 14–15 | Zero |

An entry snapshots the input-related identity before QueryInterface/factory outputs
can overwrite the main journal's related field. All points are appended under the
existing journal critical section. The mutex is released across original driver
calls, classification, GdiFlush and ACK waiting. Completion with a nonzero plane
is emitted only after reading the exact physical peer ACK. Other completions carry
plane zero. Original HRESULT/LastError/x87/SSE remain restored by the forwarding
observer. Terminal words2,7,9,10–15 contain reason, thread, clock, final API record
count including terminal, plane count, entry count, pending count, drops and gaps;
words3–6 and8 are zero. Admission requires all entries to have exactly ordered
entry/return/completion points, exact journal bindings and zero pending/drop/gap.

The conservative mutation classification includes reference release, attachment,
overlay, blits, Flip, Lock/GetDC, restore, palette/key/clipping and unlock/DC release.
For each physical witness the parser reports mutations whose original-call
interval intersects its entry-to-ACK interval, including those on other threads.
Lists are capped at256entries with explicit full counts/truncation flags. This is
a diagnostic restriction, never complete reference admission.

Successful Lock/GetDC opens a CPU/DC lease for the local COM generation; successful
Unlock/ReleaseDC closes it. Failed releases keep the lease. Missing/duplicate
acquisitions and leases remaining at terminal stay explicit. Aliases may share
storage and raw CPU writes are not observed by this journal. These facts prevent
lease bookkeeping from asserting source immutability or complete lifetimes.

Confidence: high within the guarded wire/observer scope after the recorded tests;
original producer ownership, every presentation route, physical scanout ordering
and all47journey/21operation closure requirements remain unproved.

## Temporal query-bound lifetime analysis

`drawing_surface_lifetimes.py` groups local generations only when the original
returns success from an observed QueryInterface. Failed queries, attachment
acquisition and pointer/generation reuse cannot create an alias link. Links take
effect at the return observation; a later query never repairs an earlier unmatched
release. Existing leases migrate with the group. Two leases merged together,
duplicate acquires, failed/missing releases and zero-refcount Release while a
lease remains open stay explicit and cannot establish source ownership.

The read-only `analyze-drawing-lifetimes.py` validates a registered original report's
hash/provenance and its retained journal/interval hashes, then executes the
ordering/lifetime guards and analysis without reading any original pixel plane.
Reports retain the original evidence identity independently from current analyzer
sources/claims. This is offline execution, not original reexecution or fresh
engine/native pixel comparison.

Per witness, global lease exposure includes leases released before shutdown.
Known source/destination exposure uses the query groups available in event order.
Lists are bounded with explicit count/completeness flags. Zero known exposure does
not prove all shared backing aliases, raw CPU writes or immutable producers.
Admission additionally requires consecutive entry IDs and refuses an observer
completion when an earlier main-journal call lacks its return observation.
