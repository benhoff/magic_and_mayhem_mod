#ifndef MNM_WORLD_CONTINUOUS_V1_H
#define MNM_WORLD_CONTINUOUS_V1_H
/* Little-endian byte offsets. Ownership uses aligned lock-free u32 states;
 * generation is u64 but read/written only under exclusive slot ownership. */
#define MNM_CONT_MAGIC "MNMCON01"
#define MNM_CONT_HEADER 128u
#define MNM_CONT_SLOT_HEADER 128u
#define MNM_CONT_SOURCE_BYTES (16u*1024u*1024u)
#define MNM_CONT_REQUEST_BYTES (64u+12320u*16u)
#define MNM_CONT_REPLY_BYTES (64u+800u*600u*2u)
#define MNM_CONT_SLOT_BYTES (MNM_CONT_SLOT_HEADER+MNM_CONT_SOURCE_BYTES+MNM_CONT_REQUEST_BYTES+MNM_CONT_REPLY_BYTES)
#define MNM_CONT_BYTES (MNM_CONT_HEADER+2u*MNM_CONT_SLOT_BYTES)
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
 * requestBytes5, sourceFNV6, requestFNV7, returnedBytes8, reserved9,
 * replyBytes10, replyFNV11, queueStartTick12, publishTick13,
 * writebackTick14, returnTick15, reserved16..31.
 * Each data area starts after its fixed-capacity predecessor. */
#endif
