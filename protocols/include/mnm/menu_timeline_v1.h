#pragma once
/* Diagnostic host journal, little-endian u32 words. One GUI writer; committed
 * records never change. This sideband is not an original-game command channel.
 * Header: magic[8], version, record bytes, capacity, committed, dropped, PID,
 * closed, clock ID (Linux CLOCK_MONOTONIC=1), tracked request, control, 4 zero.
 * Record: ordinal, event, monotonic ns low/high, request, action, cached ACK,
 * generation, screen, status, engine seqlock, client flags, request age ms,
 * state age ms, target screen, zero. No pointers or shared C++ layouts.
 */
#define MNM_MENU_TIMELINE_MAGIC "MNMQLT01"
#define MNM_MENU_TIMELINE_VERSION 1
#define MNM_MENU_TIMELINE_HEADER 64
#define MNM_MENU_TIMELINE_RECORD 64
#define MNM_MENU_TIMELINE_CAPACITY 512
#define MNM_MENU_TIMELINE_SIZE (64 + 64 * 512)
#define MNM_MTL_CREATED 1
#define MNM_MTL_REQUEST_BEFORE 2
#define MNM_MTL_REQUEST_PUBLISHED 3
#define MNM_MTL_ENGINE_READ 4
#define MNM_MTL_DESTINATION_READY 5
#define MNM_MTL_FALLBACK 6
#define MNM_MTL_RETIRE_BEFORE 7
#define MNM_MTL_RETIRE_AFTER 8
#define MNM_MTL_REQUEST_REFUSED 9
#define MNM_MTL_READ_SUPPRESSED 10
