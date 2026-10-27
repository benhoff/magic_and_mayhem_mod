# Original alternate owner diagnostic record, version 1

This authored format belongs to `runtime/alternate-canvas/owner.c`; it is not a
recovered game file. It is an optional, fresh Windows file created by the opt-in
owner. All integers are unsigned little-endian 32-bit values. Guest addresses
are launch-local observations, never reusable host pointers. The decoder is
`tools/original_alternate_owner.py`.

The 256-byte header contains 64 DWORDs. Indices 0..33 are, in order:

```text
magic version header_bytes reserved
allocations publications retirements copies world_bindings error
active_base active_source width height padding_x padding_y
ordinary_pitch backing_width backing_height guard_checks pixel_checks pixel_words
rows thread enabled initializations shutdowns creation_repairs entry_checks
padding_resets red_mask green_mask blue_mask nonzero_words
```

Magic is 0x414e4d4d, version 1 and header_bytes 256. Remaining header DWORDs are
reserved zero. ordinary_pitch and backing_width are WORD counts; pixel_words
counts sampled WORD comparisons before UI overlays. nonzero_words counts their
nonzero source values. Counters are DWORDs and may wrap outside the bounded runs
accepted here. The header's active pointers are sampled from the original slots.

Each event occupies 80 bytes (20 DWORDs): sequence,kind,generation,CPU base,
centered source,visible width,visible height,paddingX,paddingY,CPU stride in WORDs,
payload bytes,caller,active drawing pointer,active pitch,active height,published
base,published source,copy count,World-bind count,error. Sequence starts at 1.
At most 2048 events are written, bounding a normal record to 164,096 bytes. After
saturation, new event rows stop while header counters and rendering continue.

| Kind | Meaning |
| --- | --- |
| 1 | Guarded payload allocated and cleared, not yet published |
| 2 | Source published, then branch-selecting base published |
| 3 | Sampled original copy equals the CPU source |
| 4 | Owned generation retired after closing publication and invalidating bindings |
| 5 | Initialization destination descriptor repaired to nominal dimensions |
| 6 | Original shutdown entered and generation retired |
| 7 | Original World alternate binding observed (first four only) |
| 8 | Original initialization completed with alternate option disabled |
| 9 | Refusal, error in header and event |

Error 0 means no detected error. Codes 1..13 respectively mean wrong thread,
guard damage, foreign publication, release failure, geometry/format rejection,
allocation failure, copy binding rejection, copied-word mismatch, reentrant
initialization, World binding rejection, return-frame bookkeeping overflow or
mismatch, original initialization failure, and memory-protection/cache failure.
A diagnosed refusal clears only pointers into owned storage, frees its own
allocation and terminates with exit `170 + code`. Log I/O failure exits 191.
Entry-signature/preexisting-owner refusals occur before normal installation and
may produce no log. Logging is diagnostic; its absence does not disable ownership.

Evidence and confidence: the field layout is directly authored in owner.c and
used by all nine isolated controls and four final live sessions documented in
[the repair record](../runtime/original-alternate-allocation-repair-20261026.md).
The file contains compatibility-policy observations, not proof of a historical
allocator or arbitrary lifetime safety.
