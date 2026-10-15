#ifndef MNM_RENDER_STREAM_V5_H
#define MNM_RENDER_STREAM_V5_H
#include "render_stream_v4.h"
/* V5 retains V4 and adds an explicit CPU-reader synchronization boundary.
 * Ten DWORDs: surface ID, ticket low/high, command session, width, height,
 * physical history generation low/high and nonce low/high. No pointers.
 * The consumer atomically publishes native_cpu_read_v1 before further commands. */
#define MNM_RENDER_STREAM_V5_MAGIC "MNMCMD05"
#define MNM_RENDER_STREAM_V5_VERSION 5u
#define MNM_RENDER_STREAM_V5_OPERATION_CPU_READ 21u
#define MNM_RENDER_STREAM_V5_CPU_READ_BYTES 40u
#endif
