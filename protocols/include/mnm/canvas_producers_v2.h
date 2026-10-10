#pragma once
#include "canvas_producers_v1.h"
// Opt-in owned minimap input extension; V1 operation numbers stay unchanged.
#define MNM_PRODUCER_V2_MAGIC "MNMPRO02"
#define MNM_PRODUCER_OWNED_MINIMAP 24u
#define MNM_MINIMAP_META_WORDS 37u
#define MNM_PRODUCER_V2_MAX_RECORDS 262144u
#define MNM_PRODUCER_V2_MAX_BYTES (512u*1024u*1024u)
