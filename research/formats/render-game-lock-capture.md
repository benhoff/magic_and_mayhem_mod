# Game-owned Lock/Unlock capture

Opt-in command:

```bash
./tools/run-qt-shell.sh --capture-locks
```

The disposable experiment contains `lock-capture/` and records its path plus
`capture_locks: true` and `no_readback: true` in `manifest.json`. The bridge
forces observer readback off when this directory is configured. Use the Wine
window for input. This is a bounded lifecycle experiment, not complete live
presentation: supported offscreen RGB Blt/BltFast now propagate into bounded
offline replay files. Reconstructed destinations with observed primary identity
can also publish live frames. Observed two-buffer RGB Flips are supported, and
complete indexed primary checkpoints can publish with observed palette state.
See [game-owned blits](../runtime/opengl-game-owned-blits.md).
Supported native colour fills can establish a complete destination before
transparent copies; partial fills require an existing complete checkpoint.

## Lifecycle and acceptance

The actual application Lock is forwarded once with its original arguments.
After success the bridge remembers a full-surface writable descriptor, interface,
object and owning thread. Accepted flags are within `0x4831`, excluding READONLY.
Flagged DDSD_CKSRCBLT descriptor values also establish exact source-key
provenance for owned blits; unflagged values are ignored and ranges are rejected.
Rectangular updates require a matching complete owned checkpoint; unsupported
flags/descriptors and unsafe layouts are skipped. See [partial Lock ownership](../runtime/opengl-partial-locks.md).
The descriptor must have the correct size (108 for Surface1/2, 124 for Surface4/7).

Before a matching application Unlock, on the locking thread, native rows are
copied from the still-locked pointer to owned storage. Legacy Unlock must receive
the recorded pixel pointer; modern Unlock must receive NULL for a full surface.
The original Unlock is then forwarded once. A successful Unlock retires the
pointer and commits the copy. A failed Unlock discards this attempt's copy and
retains the original lock metadata for the application's retry. No pointer is
read after the original Unlock. API arguments, HRESULT and LastError are preserved.

No extra COM Lock, Unlock, Query or retained reference is required by this
lifecycle. At most 32 descriptors are tracked. Unmatched successful Unlocks on
known components invalidate only that component's pixels and pending locks.
Missed pixel tracking queues the affected object token for component-scoped
checkpoint and pending-Lock invalidation at the next guarded entry. The queue
has 128 entries; overflow and unknown successful Unlock identities retain the
whole-pixel-epoch fallback. Validated surface metadata is preserved for later
complete checkpoint recovery. No queued token is dereferenced.
Missed identity/property tracking additionally invalidates the metadata epoch;
uncontended final Release retires its component. Cross-interface Unlock matching uses only relationships observed in
successful application QueryInterface calls for supported surface interfaces.
Transitive aliases are supported; no fixed pointer offset or extra query is used.
Unlock argument validation follows the unlocking interface's ABI. Final Release
retires the entire observed component; a contended final Release requests a full
alias reset before the next lookup. The graph has at most 256 relationships;
saturation clears provenance. New successful Locks replace earlier metadata,
preventing reuse of an older pointer at the same object address.

Captures require nonzero dimensions up to 2048x2048, 8/16/24/32-bit supported
native formats, valid nonoverlapping RGB masks, and readable rows. Negative pitch
and padding are normalized to tightly packed top-row-first bytes. Indexed bytes
are retained without querying a palette. The [owned indexed extension](../runtime/opengl-owned-indexed.md)
uses application palette calls to associate colors and publish complete primaries.
Read-only capture remains deferred. Rectangular writes now merge into a complete
owned checkpoint; the live pointer is already relative to the locked rectangle.

At most 16 snapshot files are recorded. Live checkpoints continue after that
disk budget, with 64 MiB of retained plus pending native pixel storage allowed
per process. Transient copies are freed after every Unlock attempt.
Limits and contention drop capture work, never the application's call.
A primary RGB surface with explicitly returned DDSD_CAPS /
DDSCAPS_PRIMARYSURFACE can publish a frame after successful Unlock or supported
reconstructed blits, using the existing RGBA stream. Primary identity comes from
a committed complete Lock checkpoint or supported application metadata. Offscreen
snapshots cannot publish; indexed primaries require separately observed complete
palette state. Publication is bounded by the same capture limit.

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

## Reconstructed partial snapshot

Partial commits keep the `lock-N.bin` naming/sequence but use `MNMLOCK2`, version
2, header length 80. Offsets 16–60 have the same meanings as version 1; width and
height describe the full reconstructed surface. Four signed little-endian 32-bit
rectangle coordinates follow at offsets 64, 68, 72 and 76: left, top, right,
bottom, with exclusive right/bottom. Native payload begins at offset 80, and
must have exactly the declared complete surface length.

Pixels outside the rectangle come from the previous owned checkpoint, which
may itself be reconstructed. Pixels inside were copied from the application's
live Lock pointer before original Unlock. The file is committed only after
successful Unlock and is **not a full original-engine readback**. No palette or
row padding is stored. Readers must explicitly recognize the version/magic and
reject inconsistent lengths, dimensions and rectangles. Full live copies retain
version 1 without changing their format.

## Evidence and confidence

`tools/test-render-lock-lifecycle.py` runs actual PE32 hooks under Wine. Fourteen
fixtures cover modern primary RGB, legacy negative pitch, failed Unlock retry,
offscreen RGB, indexed bytes, failed Lock, read-only Lock, partial Lock and
invalid overlapping masks, legacy interface aliases, transitive modern aliases,
failed QueryInterface with a nonnull output, unobserved aliases and address reuse
after final Release. The original Unlock poisons its buffer; exact output
bytes prove copying before invalidation. Fixtures assert exactly one original
Lock, original Unlock counts/arguments/results/LastError, no output on rejected
locks and no offscreen/indexed primary frame without observed palette state. Evidence:
`working/tests/render-lock-lifecycle/run-vhif07ea/report.json`.

Confidence: confirmed synthetic lifecycle and ABI behavior. Real-game lifecycle
captures and uninterrupted new-game play remain unvalidated. Subsequent chunks
implement full opaque initialization, observed two-buffer RGB/indexed Flips,
indexed copy propagation and primary palette updates. The [partial CPU Lock extension](../runtime/opengl-partial-locks.md)
adds bounded merges; complete Qt gameplay presentation remains outstanding.

## Lifecycle rejection diagnostics

New lock-capture experiments also record `lock-capture/lifecycle.log` (its path
is `lock_lifecycle_log` in the manifest). This explains why an application lock
was accepted or rejected, and why its Unlock produced no snapshot. It performs
no additional COM or surface-lock calls. It preserves LastError. Records are
nonblocking, deduplicated and bounded to 128 total, with at most four distinct
records per reason, so repeated successful operations cannot consume all space
before a rejection.

Each text line has a reason followed by 19 eight-digit hexadecimal fields:

```text
reason object current_thread kind argument lock_flags HRESULT owner_thread
       descriptor_size descriptor_flags width height pitch pixel_pointer
       pixel_format_flags bits red_mask green_mask blue_mask surface_caps
```

`argument` is the supplied rectangle pointer on Lock or Unlock's argument.
For `alias_observed`, it is the returned interface pointer; `object` is the
interface on which the successful application QueryInterface was called.
Descriptor fields are zero when unavailable. Pitch is the raw signed 32-bit
value represented in hex. Thread/pointer values are launch-specific.

Application-held GDI contexts can now seed owned RGB checkpoints after successful
ReleaseDC; see [the capture boundary and evidence](../runtime/opengl-game-owned-dc.md).
These are not `lock-*.bin` records. `dc_acquired`, `dc_ready`, `dc_copied`,
`dc_checkpoint`, `dc_released`, `dc_release_failed`, `dc_unmatched`,
`dc_bitmap_rejected`, `dc_memory` and `dc_read_failed` use the ordinary fields,
with the HDC in `argument`. `dc_bitmap` uses a separate 19-field schema:

```text
dc_bitmap object current_thread HDC HBITMAP GetObject_size
          bitmap_width bitmap_height bitmap_pitch bitmap_planes_bits bitmap_pointer
          info_size info_width info_height info_planes_bits compression
          red_mask green_mask blue_mask GdiFlush_result
```

The packed planes/bits fields contain 16-bit planes in the low half and bit width
in the high half. The bitmap pointer is diagnostic only; capture uses
GetBitmapBits to normalize rows because GetObject can normalize the height sign.

Specific `blit_reject_*` reasons distinguish `source_rect`, `target_rect`, `self`,
`effects`, `flags`, `clipper`, `format` and `stretch`. Their fields are:

```text
reason target current_thread flags effects_pointer
       source_left source_top source_right source_bottom
       target_left target_top target_right target_bottom
       source_width source_height target_width target_height
       clip_known clip_present source
```

Rectangles are copied arguments or full-surface defaults, with right/bottom
exclusive. Invalid/unreadable rectangles retain defaults where unavailable.
These records share the bounded log and preserve LastError; no extra surface
readback or COM query is performed to diagnose a rejected blit.

Lock reasons: `lock_accepted`, `lock_failed`, `lock_partial`, `lock_readonly`,
`lock_flags`, `lock_descriptor`, `lock_capacity`, `lock_partial_accepted`.
`lock_partial` now means a rectangular Lock was rejected, for example because
its base, rectangle, flags or layout could not be established.
Unlock reasons: `unlock_unmatched`, `unlock_owner`, `unlock_argument`,
`unlock_layout`, `unlock_masks`, `unlock_memory`, `unlock_limit`,
`unlock_allocation`, `unlock_copied`, `unlock_failed`, `unlock_succeeded`.
The copied event occurs before the original Unlock; the succeeded/failed event
occurs afterward. Success without a copied event is not evidence of a snapshot.
Records can be dropped under contention or file errors, so absence is not proof
that no application call occurred. Contention conservatively invalidates capture
provenance rather than reading a pointer whose lifecycle could have been missed.

The fourteen PE32 fixtures now assert the expected reasons, including rejected
read-only/partial/invalid-mask locks and a failed Unlock followed by success,
while retaining their pixel-poisoning and unchanged-ABI checks.

Supported RGB propagation writes `blit-00000001.bin`, etc. in this same directory.
These use `MNMCMD01`, not the Lock snapshot header described above. Their CHECK
pixels are reconstructed output rather than original-engine readbacks. See
[provenance, limits and tests](../runtime/opengl-game-owned-blits.md).

See [primary routing and offline tests](../runtime/opengl-game-owned-primary.md)
for publication conditions. The [bootstrap extension](../runtime/opengl-primary-bootstrap.md)
adds application descriptor metadata and complete opaque initialization; its
synthetic zero destination-before image is not original-driver pixel evidence.
Additional diagnostic reasons are `surface_metadata`, `surface_metadata_failed`,
`surface_metadata_rejected`, `blit_bootstrap_ready`, `blit_initialized` and
`blit_incomplete_initialization`.

[Owned two-buffer Flip routing](../runtime/opengl-owned-flips.md) uses the existing
RGBA stream and adds no snapshot format. Diagnostic reasons are `flip_attachment`,
`flip_untracked`, `flip_unsupported`, `flip_limit`, `flip_ready`, `flip_failed`,
`flip_invalidated`, `flip_presented` and `flip_presentation_skipped`. Repeated
identical diagnostics are deduplicated; their count is not an operation count.

[Owned indexed presentation](../runtime/opengl-owned-indexed.md) adds no binary
snapshot format. Palette state is separate from `MNMLOCK1` index bytes. Additional
diagnostic reasons are `indexed_presented`, `palette_attached`, `palette_caps`,
`palette_caps_rejected`, `palette_entries`, `palette_invalidated` and
`palette_ambiguous`. The [indexed propagation extension](../runtime/opengl-owned-indexed-copies.md)
now emits bounded `blit-N.bin` and `flip-N.bin` command files using the existing
palette and swap opcodes. Unknown colors skip replay/publication while retaining
accepted native indices. Unknown old-front pixels also skip Flip replay. Serial
gaps are expected for skipped files. Additional diagnostic reasons are
`blit_palette_unobserved`, `flip_recorded` and `flip_file_failed`.

Partial CPU writes can additionally produce `update-N.bin` using `MNMCMD01`.
The serial matches the merged `lock-N.bin`. Sessions contain a complete owned
base, one row UPDATE per locked row, a reconstructed CHECK, PRESENT and cleanup.
Indexed sessions use current observed palette colors; unknown colors skip the
session. Replay limits and file failures leave native commits/publication active.
See [ownership, limits and replay tests](../runtime/opengl-partial-locks.md).
Diagnostics add `update_recorded`, `update_palette_unobserved`, `update_limit`
and `update_file_failed`.

## Publication-rate journal

Capture launches also write `presentation-rate.jsonl` beside `lock-capture/`.
Each version-1 JSON line identifies `origin=frame_v1_header_sampling`, window and
elapsed seconds, `published_updates`, `published_updates_per_second`, sample
counts (total, busy, invalid), last stable frame count and bridge status. Header
sampling uses the existing sequence contract at approximately 60 Hz; counter
deltas include publications between samples. It reads no pixel payload. This is
producer-publication cadence, not Qt paint rate or game FPS. A final short window
is emitted at runner exit. Samples crossing a busy sequence are counted as busy
and excluded from stable header observation. The sampler stops on runner exit or
PID reuse; it cannot reconstruct timing from an already completed run.
