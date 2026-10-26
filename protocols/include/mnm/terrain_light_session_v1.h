#pragma once
#include "terrain_light_v1.h"
/* Fixed little-endian session file. Only byte offsets and u32 words cross the
 * boundary. One engine-thread request at a time; phase uses acquire/release.
 * Header128, input[MAX], reply[MAX], observed result[MAX]. Each length has an
 * independent bound. The original result lane is used only by diagnostics.
 * Host creates BOOT, publishes IDLE. Guest publishes REQUEST; host publishes
 * REPLY or REFUSED. Guest either returns IDLE, or publishes RESULT for a checked
 * diagnostic; host ACKs that result before the guest returns IDLE. Timeout or
 * invalid response latches CLOSED: a late host response cannot revive it.
 */
#define MNM_LIGHT_SESSION_MAGIC "MNMLS001"
#define MNM_LIGHT_SESSION_BYTES (128u+3u*MNM_LIGHT_V1_MAX_BYTES)
#define MNM_LS_IDLE 0u
#define MNM_LS_REQUEST 1u
#define MNM_LS_REPLY 2u
#define MNM_LS_RESULT 3u
#define MNM_LS_ACK 4u
#define MNM_LS_DECLINED 5u
#define MNM_LS_CLOSED 6u
#define MNM_LS_BOOT 7u
#define MNM_LS_WAIT_MS 100u
#define MNM_LS_TRACE_SAMPLES 32u
#define MNM_LS_TRACE_BYTES (64u*1024u*1024u)
/* Word indices; the magic occupies words0..1. Mutable lanes have one writer.
 * MODE:0 observes,1 replaces. TRACE:0 none,1 first samples,2 named reload.
 * ARM belongs to the host journey. Guest starts reload tracing only at472201.
 * Guest stats:CALLS/APPLIED/FALLBACK/REFUSED/DISABLED/MAX_WAIT/CALLER/CLOCK/CHANGED.
 * Host stats:PROCESSED/REJECTED/COMPARED/MISMATCHES/MAX_COMPUTE/RECORDS.
 */
#define MNM_LS_VERSION 2u
#define MNM_LS_CAPACITY 3u
#define MNM_LS_PHASE 4u
#define MNM_LS_INPUT_LENGTH 5u
#define MNM_LS_REPLY_LENGTH 6u
#define MNM_LS_RESULT_LENGTH 7u
#define MNM_LS_SEQUENCE 8u
#define MNM_LS_NONCE_LO 9u
#define MNM_LS_NONCE_HI 10u
#define MNM_LS_MODE 11u
#define MNM_LS_TRACE 12u
#define MNM_LS_ARM 13u
#define MNM_LS_CALLS 14u
#define MNM_LS_APPLIED 15u
#define MNM_LS_FALLBACK 16u
#define MNM_LS_REFUSED 17u
#define MNM_LS_DISABLED 18u
#define MNM_LS_PROCESSED 19u
#define MNM_LS_REJECTED 20u
#define MNM_LS_COMPARED 21u
#define MNM_LS_MISMATCHES 22u
#define MNM_LS_MAX_WAIT 23u
#define MNM_LS_MAX_COMPUTE 24u
#define MNM_LS_CALLER 25u
#define MNM_LS_CLOCK 26u
#define MNM_LS_CHANGED 27u
#define MNM_LS_ERROR 28u
#define MNM_LS_RECORDS 29u
#define MNM_LS_TRACE_ACTIVE 30u

/* Diagnostic only after CLOSED: host requests1, guest publishes2 after one
 * original fallback source/result capture, or3 when no snapshot is admitted.
 * No worker request or native replacement is restarted by this mailbox. */
#define MNM_LS_FALLBACK_SAMPLE 31u
