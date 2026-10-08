#pragma once
/* Fixed little-endian64-byte request/reply header followed by tight RGB565
 * reply words. Request identity words4..10 must match exactly. Reply uses
 * payload bytes at11, FNV-1a32 over payload at12, success1 at13;14 original
 * signature-checked generic entry address,15 zero. Reply14 must match request.
 * Only the already admitted native producer record named at6 is executable.
 * Publish reply by atomic rename; runtime validates before any writeback. */
#define MNM_WORLD_BYPASS_REQUEST "MNMWBP01"
#define MNM_WORLD_BYPASS_REPLY "MNMWBR01"
#define MNM_WORLD_BYPASS_VERSION 1u
#define MNM_WORLD_BYPASS_HEADER 64u
#define MNM_WORLD_BYPASS_MAX 8u
#define MNM_WORLD_BYPASS_TIMEOUT 120000u

/* V2 complete-queue handshake has the same64-byte extent.15 is the recovered
 * low16 AX result (0/1 in the admitted fixture), echoed in replies. Distinct
 * six-digit world-raster filenames preserve the bounded V1 experiment. */
#define MNM_WORLD_RASTER_REQUEST "MNMWBP02"
#define MNM_WORLD_RASTER_REPLY "MNMWBR02"
#define MNM_WORLD_RASTER_MAX 65536u
