# Game-owned Lock/Unlock capture

Opt-in command:

```bash
./tools/run-qt-shell.sh --capture-locks
```

The disposable experiment contains `lock-capture/` and records its path plus
`capture_locks: true` and `no_readback: true` in `manifest.json`. The bridge
forces observer readback off when this directory is configured. Use the Wine
window for input. This is a bounded lifecycle experiment, not complete live
presentation: it does not propagate offscreen buffers through Blt/Flip.

## Lifecycle and acceptance

The actual application Lock is forwarded once with its original arguments.
After success the bridge remembers a full-surface writable descriptor, interface,
object and owning thread. Accepted flags are within `0x4831`, excluding READONLY.
Partial locks, unsupported flags/descriptors and unsafe layouts are skipped.
The descriptor must have the correct size (108 for Surface1/2, 124 for Surface4/7).

Before a matching application Unlock, on the locking thread, native rows are
copied from the still-locked pointer to owned storage. Legacy Unlock must receive
the recorded pixel pointer; modern Unlock must receive NULL for a full surface.
The original Unlock is then forwarded once. A successful Unlock retires the
pointer and commits the copy. A failed Unlock discards this attempt's copy and
retains the original lock metadata for the application's retry. No pointer is
read after the original Unlock. API arguments, HRESULT and LastError are preserved.

No extra COM Lock, Unlock, Query or retained reference is required by this
lifecycle. At most 32 descriptors are tracked. Unknown successful Unlocks,
final releases and tracker contention conservatively invalidate the capture
epoch; missed/aliased events do not establish valid provenance. Cross-interface
Unlock matching is not implemented. New successful Locks replace earlier
metadata, preventing reuse of an older pointer at the same object address.

Captures require nonzero dimensions up to 2048x2048, 8/16/24/32-bit supported
native formats, valid nonoverlapping RGB masks, and readable rows. Negative pitch
and padding are normalized to tightly packed top-row-first bytes. Indexed bytes
are retained without querying a palette; indexed color/palette association is a
later chunk. Read-only and rectangular updates are also deferred.

At most 16 committed snapshots and 64 MiB of committed plus pending pixel storage
are allowed per process. Transient copies are freed after every Unlock attempt.
Limits and contention drop capture work, never the application's call.
Only a directly locked primary RGB surface with explicitly returned DDSD_CAPS /
DDSCAPS_PRIMARYSURFACE can publish a frame, after successful Unlock, using the
existing RGBA stream. Offscreen or indexed snapshots cannot publish guessed
frames. Publication is bounded by the same capture limit.

## Snapshot file

`lock-00000001.bin`, etc. contains a 64-byte little-endian header followed by
native pixels. Reject incomplete files (the declared payload must be present).

| Offset | Meaning |
|---|---|
| 0 | Eight bytes `MNMLOCK1` |
| 8 | Version 1 |
| 12 | Header length 64 |
| 16 | Sequence, starting at 1 |
| 20 | Observed surface interface pointer |
| 24 | Locking thread ID |
| 28 | Interface kind: 11/12/14/17 |
| 32 | Application Lock flags |
| 36 | Width |
| 40 | Height |
| 44 | Bits per pixel |
| 48 | Red mask |
| 52 | Green mask |
| 56 | Blue mask |
| 60 | Payload length: width * height * (bits / 8) |

Pointers/thread IDs apply only to the captured process. No palette or original
row padding is stored.

## Evidence and confidence

`tools/test-render-lock-lifecycle.py` runs actual PE32 hooks under Wine. Nine
fixtures cover modern primary RGB, legacy negative pitch, failed Unlock retry,
offscreen RGB, indexed bytes, failed Lock, read-only Lock, partial Lock and
invalid overlapping masks. The original Unlock poisons its buffer; exact output
bytes prove copying before invalidation. Fixtures assert exactly one original
Lock, original Unlock counts/arguments/results/LastError, no output on rejected
locks and no offscreen/indexed primary frame. Evidence:
`working/tests/render-lock-lifecycle/run-sk_p4mn8/report.json`.

Confidence: confirmed synthetic lifecycle and ABI behavior. Real-game lifecycle
captures and uninterrupted new-game play remain unvalidated. Blit/flip propagation,
partial updates, indexed palettes and complete Qt gameplay presentation remain
outstanding.
