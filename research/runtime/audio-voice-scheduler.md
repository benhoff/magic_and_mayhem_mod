# Voice selection, eviction and scheduler ordering

Reviewed 2026-10-04. Selected static engine blocks reconstructed and tested with
synthetic clocks, host rings and native PCM. No live game or injected scheduler.

## Evidence and recovered behavior

Reference: No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Saved evidence: `working/decompiled/audio-support-jhnwov_f/voice_scheduler.asm`,
covering `0x00571ab0` through `0x00571f79`. Its bytes were checked against the
artifact hash in the export manifest. This reuses the previously hash-guarded
export with original-manifest verification before/after; no executable was
read, modified or launched for this chunk.

| Entry | Reconstructed contract |
| --- | --- |
| `0x00571ab0` | Scan from ring head: return first free record, retire expired records, or evict when occupied volume is strictly below requested volume. Equal volumes do not qualify. Return null after a complete unsuccessful scan. |
| `0x00571be0` | Assign voice, output address, deadline, cached volume and coordinates. Loop deadline is `0xffffffff`; one-shot deadline is wrapping unsigned clock + duration. |
| `0x00571c30` | Reuse the former tail, optionally moving it before the requested candidate. Stop/reset its voice if selected by deadline and manager gates; clear it despite control failures; return that former tail. |
| `0x00571d80` | Reposition a record using old-volume comparisons and direction determined by old versus new volume; write the new volume after traversal/link changes. |
| `0x00571ec0` | Clear every record without changing topology. Stop/reset only looping or not-yet-expired voices when both manager gates are set. |

The selection scan checks unsigned `now >= deadline`, separately for each
nonloop occupied record. Expired head records rotate to the tail; expired middle
records move to the tail and scanning continues from their saved successor.
An expired tail is immediately returned after clearing. Expiration retirement
does not query status or Stop. A free record returns without clearing its
payload or calling the clock. A complete unsuccessful scan returns null.

Eviction does not necessarily stop the candidate that qualified on volume.
At `0x00571c30`, the former tail is the reusable victim. For a head candidate,
the tail becomes the new head; for a middle candidate, the tail moves immediately
before it; for a tail candidate, topology stays unchanged. Status/Stop/reset
operate on the returned tail voice. Any nonzero query result attempts Stop;
nonzero Stop skips reset. Control failures do not prevent reuse or clearing.
No buffer is released. This surprising tail behavior is preserved, not replaced
with a conventional lowest-volume victim policy.

Eviction and whole-ring clearing clear the caller output only when the old
voice address was nonzero. Bufferless wrappers skip controls. An expired voice
skips Stop even if its native playback is still active. These blocks rely on
original deadline/ownership assumptions; they are not an improved playback
completion policy. Disabled controls still permit record clearing and topology
changes; nonloop occupied records still query the clock.

Assignment copies cached wrapper volume (+0x14), not requested volume (+0x10).
It stores the output-slot address without writing the caller's output slot, and
does not reorder the ring. A looping assignment makes no clock call. The deadline
comparison deliberately retains ordinary unsigned x86 behavior across wrap;
no wrap-safe modern clock policy is substituted.

Volume updates preserve the inspected old-value scans, including cases where
the result is not sorted by the newly requested volume. Equal old/new values
skip all work. An old-to-lower change scans forward against the old value; an
old-to-higher change scans backward against the old value, with specific head
and tail rotation branches. This is confirmed static behavior, not evidence
that every possible live ring is accepted by the original engine.

## Implementation and boundaries

`reconstruction/audio/voice_scheduler.*` uses the host-linked ScheduleNode from
the lifetime reconstruction. Host next/previous pointers are authoritative;
raw Schedule32 link words are not rewritten as process addresses. Valid,
nonempty circular rings and member records are preconditions. Backends resolve
session-local wrapper identities into buffer handles and supply clocks/controls;
callbacks must not mutate ring topology. Existing control primitives preserve
status-error, Stop and zero-position ordering. Original scratch-global results,
engine memory ownership and allocator behavior are not reproduced.

Confidence is high for exported comparisons, field writes and ring branches.
Full manager start/duplicate selection, source-list admission, positional audio,
initialization unwind and live thread/cadence compatibility remain separate.
This model does not change native mixer policy or select voices in the live
COM adapter. It is a foundation for later validated integration.

## Reproduce and validation

```bash
python3 tools/test-audio-scheduler.py
```

Evidence: `working/tests/audio-scheduler/run-d1vr4dud/report.json` and logs.
All 14 audio/asset CTests passed. Independent vector oracles check 5,184
selection/clock/slot/order cases and 972 old-volume ordering cases, including
unsorted score arrangements. Another 64 eviction combinations exercise gates,
missing voices, status, Stop and reset failures. Fixtures check strict volume
comparison, wrapping assignment, loop clock suppression, cached-volume fields,
repeated clearing and bidirectional ring integrity.

A native Device fixture checks exact stereo PCM: when a head candidate qualifies,
its tail duplicate is stopped/reset, its sample storage remains owned, and the
head voice continues producing 10000-valued samples. Whole-ring clearing then
produces silence while retaining both buffers. The scheduler fixture passed
AddressSanitizer, UndefinedBehaviorSanitizer and LeakSanitizer as
`working/build/audio-output/audio-scheduler-sanitize`, outside sandbox tracing.
These tests use synthetic samples and no audio device, Wine or game assets.
No x86 runtime bridge code changed.
