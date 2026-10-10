#ifndef MNM_WORLD_ROLLING_V1_H
#define MNM_WORLD_ROLLING_V1_H
/* Source-only ordered native policy. Explicit little-endian words; no pointers.
 * One producer and one consumer claim a fresh session once. Slot ownership is
 * FREE -> WRITING -> READY -> READING -> COMPLETE -> FREE. No supersession.
 * Cancellation is terminal; recovery requires a new file and session identity.
 */
#define MNM_ROLL_MAGIC "MNMROLL1"
#define MNM_ROLL_VERSION 1u
#define MNM_ROLL_HEADER 64u
#define MNM_ROLL_SLOT_HEADER 64u
#define MNM_ROLL_SLOTS 2u
#define MNM_ROLL_INPUT (16u*1024u*1024u)
#define MNM_ROLL_REPLY (8u*1024u*1024u)
#define MNM_ROLL_SLOT (MNM_ROLL_SLOT_HEADER+MNM_ROLL_INPUT+MNM_ROLL_REPLY)
#define MNM_ROLL_SIZE (MNM_ROLL_HEADER+MNM_ROLL_SLOTS*MNM_ROLL_SLOT)
#define MNM_ROLL_FREE 0u
#define MNM_ROLL_WRITING 1u
#define MNM_ROLL_READY 2u
#define MNM_ROLL_READING 3u
#define MNM_ROLL_COMPLETE 4u
#endif
