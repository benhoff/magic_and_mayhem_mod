#ifndef MNM_DRAWING_CURSOR_GATE_V1_H
#define MNM_DRAWING_CURSOR_GATE_V1_H
/* Proposed little-endian128byte in-process diagnostic output. No handles or
 * host pointers. Original imports continue once. Not implemented/live yet.
 * DWORD0version,1bytes,2build,3phase,4reason,5token,6owner,7outerID,
 * 8innerID,9outerWait,10innerWait,11outerRelease,12innerRelease,
 * 13outstandingLevels,14retiredBits,15flags;16..31reservedzero.
 * Raw outcomes must remain distinct from policy reasons. */
#define MNM_DRAWING_CURSOR_GATE_V1_VERSION 1u
#define MNM_DRAWING_CURSOR_GATE_V1_BYTES 128u
#endif
