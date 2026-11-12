#pragma once
/* All integer fields are little endian. Guest addresses are numeric identities,
 * never host pointers. This optional observation stream changes no draw input.
 * Header: magic[8], version, header bytes, row bytes, maximum rows, build, then
 * nine reserved DWORDs. Rows contain 16 metadata DWORDs, incoming/outgoing nine
 * saved register DWORDs, incoming/outgoing six argument DWORDs, incoming/outgoing
 * sixteen workspace DWORDs, two semantic FX blocks, and eight reserved bytes.
 * Metadata: ID, completion kind (1 native bypass, 2 retained entry, 3 footer),
 * tag, thread, original caller, entry SP, return SP, incoming queue/sequence,
 * outgoing queue/sequence, argument count, incoming/outgoing LastError, flags,
 * canvas address. Flag1 means outgoing formal arguments remain above return SP.
 * Footer: ID=rows including footer, tag=starts, thread=completed,
 * caller=active, entrySP=failed, remaining fields zero. No drops are permitted.
 * FX: CW[2], SW[2], physical FTW[1], zero[3], MXCSR[4], eight logical 80-bit
 * registers[80] with empty entries zeroed, XMM0..7[128], zero[4]. Instruction/
 * data history and empty physical payloads are deliberately outside this view. */
#define MNM_CALLER_STATE_MAGIC "MNMCSA01"
#define MNM_CALLER_STATE_VERSION 1u
#define MNM_CALLER_STATE_HEADER_BYTES 64u
#define MNM_CALLER_STATE_ROW_BYTES 768u
#define MNM_CALLER_STATE_FX_BYTES 224u
#define MNM_CALLER_STATE_MAX_ROWS 262144u
#define MNM_CALLER_STATE_NATIVE 1u
#define MNM_CALLER_STATE_RETAINED 2u
#define MNM_CALLER_STATE_FOOTER 3u
