#ifndef MNM_WORLD_CONTINUOUS_V3_H
#define MNM_WORLD_CONTINUOUS_V3_H
/* Little-endian byte offsets. Ownership uses aligned lock-free u32 states;
 * generation is u64 but read/written only under exclusive slot ownership. */
#define MNM_CONT_VERSION 3u
#define MNM_CONT_MAGIC "MNMCON03"
#define MNM_CONT_HEADER 128u
#define MNM_CONT_SLOT_HEADER 128u
#define MNM_CONT_SOURCE_BYTES (16u * 1024u * 1024u)
#define MNM_CONT_REQUEST_BYTES (64u + 12320u * 16u)
#define MNM_CONT_REPLY_BYTES (64u + 800u * 600u * 2u)
#define MNM_CONT_SLOT_BYTES                                                    \
  (MNM_CONT_SLOT_HEADER + MNM_CONT_SOURCE_BYTES + MNM_CONT_REQUEST_BYTES +     \
   MNM_CONT_REPLY_BYTES)
#define MNM_CONT_BYTES (MNM_CONT_HEADER + 2u * MNM_CONT_SLOT_BYTES)
/* Continuous requests/replies use batch version5 inside the V1 envelope.
 * Descriptor source digest is FNV(canonical96-byte header || little-endian
 * CRC32C(exact owned payload)); inner V5 reply pixel digest is CRC32C.
 * Legacy finite batch V3 remains distinct with its FNV pixel digest. */
#define MNM_CONT_BATCH_VERSION 5u
#define MNM_CONT_FREE 0u
#define MNM_CONT_WRITING 1u
#define MNM_CONT_REQUEST 2u
#define MNM_CONT_PROCESSING 3u
#define MNM_CONT_REPLY 4u
#define MNM_CONT_RETURNED 5u
/* Header words: magic0..1, version2, header3, bytes4, reserved5,
 * nonce6..7, nativeRole8, originalRole9, cancelled10, finished11,
 * targetQueues12..13 (0 means no test cap), reserved14..31.
 * Slot words: state0, reserved1, generation2..3, sourceBytes4,
 * requestBytes5, sourceCRC32C6, requestCRC32C7, returnedBytes8, reserved9,
 * replyBytes10, replyCRC32C11, queueStartTick12, publishTick13,
 * writebackTick14, returnTick15, priorPacerStart16, priorPacerEnd17, priorPacerMask18,
 * priorScreenCopyMs19, priorScreenCopyCalls20, sourceAdmissionRegionQueries21, reserved22..31.
 * Pacer mask is0 when absent/disabled or3 for a completed guarded
 * original wait. Copy and pacing observations precede this queue entry.
 * Each data area starts after its fixed-capacity predecessor. */
#endif
