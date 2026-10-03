# Owned partial CPU Lock updates

The capture bridge merges writable rectangular application Locks into a previously
owned complete native checkpoint. It does not issue extra COM calls, retain COM
references or read driver pixels after Unlock. This extends the opt-in
`--capture-locks` lifecycle; legacy observer/history capture remains separate.

## Supported contract

- Supported Surface1/2/4/7 interfaces (kinds 11/12/14/17), existing writable flags
  and native 8/16/24/32-bit layouts, with dimensions at most 2048x2048.
- One active tracked Lock per observed interface component. A second successful
  Lock invalidates the earlier record; concurrent rectangular Locks are deferred.
- A complete retained checkpoint must exist before a partial Lock. The rectangle
  is copied before forwarding Lock and must remain unchanged on return, be nonempty
  and fit inside the checkpoint. Returned descriptor dimensions, format and masks
  must match the full checkpoint. Region-sized descriptors are rejected.
- The returned pixel pointer already addresses the rectangle's top-left pixel.
  Copy only the rectangle's row width and height, with the returned signed pitch;
  do not add the rectangle offset to that pointer again. Pitch must accommodate
  the full surface row. Padding and negative pitch are normalized.
- Legacy Unlock must receive the recorded pixel pointer. Modern Unlock must use
  the original rectangle pointer with unchanged rectangle values. A NULL modern
  Unlock is supported only for a full Lock. Cross-interface matching uses observed
  application QueryInterface relationships and the unlocking interface's ABI.

Microsoft documents the rectangle-relative pointer and its validity only until
[Unlock](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-lock),
and the rectangle argument required for
[modern Unlock](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-unlock).
[Wine's implementation](https://github.com/wine-mirror/wine/blob/master/dlls/ddraw/surface.c)
(`ddraw_surface7_lock`, viewed 2026-10-03) maps the selected rectangle but copies
full surface metadata into the returned descriptor. That supports the conservative
full-descriptor contract here; behavior of every Windows driver is not established.

## Ownership and commit

After successful partial Lock, detach the complete checkpoint from the normal
surface cache into the Lock record. Count its allocation as pending storage.
Blt sources and palette publication cannot read or present that old checkpoint
while the application is writing. Surface properties and palette association
remain on the surface identity.

Before the matching Unlock, allocate a candidate full image, copy the detached
base, then overwrite only the observed rows. Forward original Unlock unchanged.
Success commits the candidate and frees the detached base. Failure frees the
candidate but retains the base and live descriptor for retry; retry recopies the
latest live bytes. Original Unlock may immediately poison or invalidate its
pointer. Successful invalidation, conflicting alias provenance, epoch changes
and final Release also release detached bases. Captures rejected after a
successful Unlock cannot leave a reusable old checkpoint.

The existing 32-record, 16-snapshot and 64 MiB bounds remain. Detached bases and
transient merged candidates both count toward pending memory; cumulative snapshot
bytes also constrain capture. Partial updates consume a snapshot slot and store
a complete image. This is bounded evidence collection, not continuous streaming.
Successful reconstruction feeds subsequent native copies and the existing primary
publication rules. Indexed updates retain raw indices; publication still requires
a separately observed complete palette.

## Evidence format

Full live checkpoints remain `MNMLOCK1`. Partial merged checkpoints use
`MNMLOCK2`, with an 80-byte header and the observed rectangle appended to the
existing metadata fields. The payload is a complete reconstructed image:
outside pixels are inherited, inside pixels are copied before original Unlock.
It is not an independent full driver readback. See the
[format specification](../formats/render-game-lock-capture.md).

## Native UPDATE replay

After a partial Unlock commits, `runtime/render/lock_updates.h` can write
`lock-capture/update-N.bin`. Its serial matches the corresponding `lock-N.bin`.
It uses the existing `MNMCMD01` format and requires no consumer protocol change:
CREATE the detached complete base as session-local ID 1, optionally attach a
complete observed palette, UPDATE each normalized row at its original x/y,
CHECK the full reconstructed result, PRESENT, DESTROY and END.

One row per UPDATE avoids another packed-region allocation. Each record has
height 1; logical order and bytes are independent of driver pitch and padding.
Only bytes inside the observed rectangle are uploaded after CREATE. CHECK bytes
are comparison data and are never uploads. CREATE and CHECK derive from owned
reconstruction state; they are not independent driver readbacks. CREATE/DESTROY
here delimit a replay checkpoint, not the application's COM lifetime.

For indexed surfaces, replay requires a complete application-observed palette.
Use its colors at successful Unlock for the entire replay, including inherited
indices outside the rectangle. A palette update while the surface is locked
must not publish the detached old checkpoint. Unknown colors skip replay while
retaining the accepted raw index snapshot. Offscreen updates can produce replay
PRESENT records without publishing a live primary frame.

Replay files have a separate cumulative 64 MiB allowance, including every header,
record and palette byte. No more than 15 partial records can follow an initial
checkpoint within the existing 16-snapshot bound. At most 2048 row updates keep
a session below the consumer's 4096-record bound. File/byte limits, palette gaps
and exclusive-create failures skip recording while native commit and application
HRESULT/LastError continue normally. A failed or rejected Unlock emits no update
session. File serial gaps are expected. Interrupted writes lack a complete END
and fail decoding.

Diagnostic reasons are `update_recorded`, `update_palette_unobserved`,
`update_limit` and `update_file_failed`. The latter also covers a failed write.

## Offline validation and confidence

```bash
./tools/test-render-partial-locks.py
```

Twenty-five synthetic PE32 cases run actual bridge hooks under Wine, without Chaos.exe
or original artifacts. They cover Surface1/2/4/7 Unlock ABIs, RGB16/24/32,
negative padded pitch, raw indices without observed palette, sequential overlapping
updates, failed Lock, failed Unlock with a changed live region before retry,
missing base, read-only/discard flags, mismatched layout, changed rectangle,
wrong Unlock argument, Restore invalidation and capture saturation. A separate
8-MiB fixture reaches 64 MiB of cumulative checkpoints before attempting a
rectangular update, checking that detached storage cannot wrap budget arithmetic.
A following opaque blit initializes a primary from the reconstructed offscreen surface;
its recorded native replay must match the fake engine's independent pixels.

Each original Unlock poisons all exposed lock memory. Exact complete checkpoint
bytes and each ordered primary RGBA stream frame are compared with independent
engine storage, including unchanged borders and no publication while locked or
on a failed attempt. Original call counts, forwarded arguments, HRESULT and
LastError are checked. Published images also pass framebuffer readback through
Qt's actual OpenGL viewport under Xvfb/Mesa software rendering.

Every replay file is decoded by an independent Python UPDATE oracle and the
native integer OpenGL renderer. CREATE/base, row coordinates and payloads must
match independent fake-engine bytes; full native output and palette-resolved
RGBA hashes must agree. Upload statistics prove only CREATE plus the recorded
rows were uploaded. A poisoned CHECK must fail with no native export. Added
fixtures cover observed indexed palettes, a palette change during Lock, a
pre-existing output file and 2048-row updates. The replay-byte limit fixture
continues to commit all seven 8-MiB checkpoints while allowing only three
UPDATE sessions under 64 MiB; later replay rejection cannot stop native commits.


Initial merge evidence: all 21 cases passed in
`working/tests/render-partial-locks/run-3g1xqchh/report.json`. The report records
PE32 DLL SHA-256, snapshot/frame counts and rejection reasons. All 14 previous
lifecycle cases passed in
`working/tests/render-lock-lifecycle/run-sqms26ld/report.json`; all 24 indexed
copy/Flip cases passed in
`working/tests/render-indexed-copies/run-eotp38qw/report.json`; all 22 owned
palette cases passed in
`working/tests/render-owned-palettes/run-j7xse0pt/report.json`.
The production DLL builds as PE32 Intel i386; hashes and source provenance are
recorded in `working/build/render/manifest.json`.

UPDATE replay evidence: all 25 cases and 32 native replay sessions passed in
`working/tests/render-partial-locks/run-vzihgfbg/report.json`. Each of the three
maximum-height replays has 2053 commands and 2049 uploads (CREATE plus 2048 rows).
The report records native output, RGBA and command hashes, OpenGL driver details,
frame counts and diagnostic reasons. All 14 lifecycle regressions passed in
`working/tests/render-lock-lifecycle/run-sc91qe97/report.json`. RGB chain, key and
destination-alias cases passed in
`working/tests/render-lock-blits/run-_r8iuplq/report.json`; indexed opaque-copy and
rotation cases passed in
`working/tests/render-indexed-copies/run-3vcucmdm/report.json`.


Confidence: confirmed synthetic x86 ownership, merge, native UPDATE replay and
palette-resolved publication behavior.
Real-game rectangular descriptor/pointer observations and uninterrupted gameplay
remain unvalidated. Concurrent regions, partial initialization without a full
checkpoint and continuous capture are deferred.
