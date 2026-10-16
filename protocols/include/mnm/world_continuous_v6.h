#ifndef MNM_WORLD_CONTINUOUS_V6_H
#define MNM_WORLD_CONTINUOUS_V6_H
#include "world_continuous_v5.h"
/* V6 synchronous resident mode adds source-only chunks between World queues.
 * Capacities and World generations remain unchanged. Slot words22..23 hold a
 * global nonzero u64 chunk ordinal only during SOURCE_REQUEST/PROCESSING/ACK.
 * Source-only slots have sourceBytes/CRC but no request, reply or return data.
 * Native applies the complete owned chunk before ACK; producer retains the
 * slot and source definitions until ACK, then resets the same World slot.
 * V3/V4/V5 peers reject this envelope rather than dropping source history. */
#define MNM_CONT_PREFIX_MAGIC "MNMCON06"
#define MNM_CONT_PREFIX_VERSION 6u
#define MNM_CONT_PREFIX_MODE 3u
#define MNM_CONT_SOURCE_REQUEST 6u
#define MNM_CONT_SOURCE_PROCESSING 7u
#define MNM_CONT_SOURCE_ACK 8u
#define MNM_CONT_PREFIX_RECORDS 16384u
#define MNM_CONT_PREFIX_BYTES (4u*1024u*1024u)
#endif
