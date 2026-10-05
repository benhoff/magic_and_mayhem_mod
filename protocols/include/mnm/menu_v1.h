#pragma once
/* Little-endian u32 words. Separate single-writer seqlock lanes; no host pointers.
 * Host: sequence, alive, heartbeat counter, request ID, action, expected generation.
 * Engine: sequence, generation, screen ID, ready, acknowledgement, status, thread.
 * One request outstanding. IDs strictly increase. ACK means callback dispatched,
 * not transition completed: presentation follows screen/ready. No retries.
 * Lease expires after 2000 ms without heartbeat change; retirement is permanent.
 */
#define MNM_MENU_V1_MAGIC "MNMMCMD1"
#define MNM_MENU_V1_SIZE 128
#define MNM_MENU_V1_VERSION 1
#define MNM_MENU_V1_HOST_WORD 4
#define MNM_MENU_V1_ENGINE_WORD 16
#define MNM_MENU_V1_LEASE_MS 2000
#define MNM_MENU_OPEN_QUICK 1
#define MNM_MENU_BACK 2
#define MNM_MENU_QUIT 3 /* Main menu original callback index 4. */
#define MNM_MENU_OK 1
#define MNM_MENU_STALE 2
#define MNM_MENU_UNAVAILABLE 3
#define MNM_MENU_UNSUPPORTED 4
#define MNM_MENU_RETIRED 5
