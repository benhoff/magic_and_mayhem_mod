#pragma once
/* Independent little-endian PE32 wire version; V1 records remain unchanged.
 * No host pointer/object layout is shared. Diagnostic addresses are per-run.
 */
#define MNM_DRAW_API_V2_MAGIC "MNMDAPI2"
#define MNM_DRAW_API_V2_RECORD 800u
#define MNM_DRAW_API_V2_MAX_RECORDS 1048576u
#define MNM_DRAW_API_V2_ARGUMENTS 16u
#define MNM_DRAW_API_V2_SNAPSHOTS 4u
#define MNM_DRAW_API_V2_SNAPSHOT_BYTES 128u
/* Header/terminal retain V1 word positions with version=2, row bytes=800 and
 * the V2 budget. First32 row words retain the V1 diagnostic positions.
 * Words32..35: formal stack words (FFFFFFFF=unreviewed), sampled stack words,
 * full stack state (0 unavailable,1 read), exact declared snapshot count.
 * Words36..51: BEFORE sampled stack arguments, zero after/unavailable/unused.
 * Words52..55: receiver candidate address, saved candidate vtable, candidate
 * state (0 none,1 byte equality checked), reserved zero. A candidate requires
 * BEFORE argument0's readable first word plus the exact register-memory CALL
 * displacement to equal the captured target slot. This is byte evidence, not
 * COM/interface/type recovery. AFTER repeats the saved candidate without reads.
 * Four36-word snapshot slots begin at word56: address, requested bytes, state,
 * captured bytes, payload[32]. Unused/non-event slots are entirely zero.
 * State1 read,2 null,3 unreadable,4 arguments unavailable. Failed/null reads
 * retain no payload. Each byte extent is a separately reviewed bounded plan.
 * AFTER uses saved BEFORE pointer arguments, never re-reads cleaned-up stack
 * arguments or dereferences a released object. Release plans have no snapshots.
 * Raw bytes on a failing API do not establish a valid output or live object.
 * Formal layout is assigned only by exact import/signature or reviewed original
 * site. Unknown sites retain16 diagnostic words without semantic ABI inference.
 */
