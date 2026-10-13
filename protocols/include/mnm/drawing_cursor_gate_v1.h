#ifndef MNM_DRAWING_CURSOR_GATE_V1_H
#define MNM_DRAWING_CURSOR_GATE_V1_H
/* Versioned little-endian128byte in-process diagnostic output. No handles or
 * host pointers. Original imports continue once. Owned implementation tested;
 * production original integration remains pending.
 * DWORD0version,1bytes,2build,3phase,4reason,5token,6owner,7outerID,
 * 8innerID,9outerWait,10innerWait,11outerRelease,12innerRelease,
 * 13outstandingLevels,14retiredBits,15flags;16..31reservedzero.
 * Raw outcomes must remain distinct from policy reasons. Begin's scalar result
 * is the admitted lease token, or zero. DWORD5 may retain a nonzero attempted
 * reservation ID after a failed wait and reservation drainage; that ID alone
 * does not establish active ownership. */
#define MNM_DRAWING_CURSOR_GATE_V1_VERSION 1u
#define MNM_DRAWING_CURSOR_GATE_V1_BYTES 128u
#endif
