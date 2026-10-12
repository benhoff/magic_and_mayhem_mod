#pragma once
#include "canvas_producers_v3.h"
/* Recorder-only whole-update envelope; legacy replacement budgets unchanged. */
#define MNM_PRODUCER_V4_MAGIC "MNMPRO04"
#define MNM_PRODUCER_V4_MAX_RECORDS 1048576u
#define MNM_PRODUCER_V4_MAX_BYTES (512u*1024u*1024u)
#define MNM_PRODUCER_V4_MAX_QUEUES 120u
#define MNM_PRODUCER_V4_WHOLE_UPDATE 1u
#define MNM_PRODUCER_UPDATE_END 27u
