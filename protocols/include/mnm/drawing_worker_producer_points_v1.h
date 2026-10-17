#pragma once
#define MNM_WORKER_PRODUCER_MAGIC "MNMWPP01"
#define MNM_WORKER_PRODUCER_VERSION 1u
#define MNM_WORKER_PRODUCER_ROW_BYTES 128u
#define MNM_WORKER_PRODUCER_MAX_ROWS 131072u
/* Header64: magic/version/row128/build40209ca7/budget131072/point64/zero.
 * Row32 DWORDs: ordinal, phase1entry/2return/3terminal, worker, thread, tick,
 * getter0success/1unavailable/2refused, legacy worker entry record, reserved0;
 * words8..23 are the pointer-free SourcePointV1 queried with a null COM object;
 * words24..31 zero. Terminal worker=entries, word6=returns, word7=pending;
 * getter and point words zero. Prefixes bound the observed worker body; they
 * do not establish atomic source bytes or a general replay execution order. */
