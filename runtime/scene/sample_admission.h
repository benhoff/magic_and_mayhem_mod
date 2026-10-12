#include "../../protocols/include/mnm/canvas_producers_v4.h"
/* Explicit diagnostic policy, independent of original simulation limits. */
static int scene_sample_admitted(u32 samples,u32 whole,u32 producer,u32 trace,u32 batch,u32 binding){
 if(!samples||whole>1||producer>1||trace>1||batch>1||binding>1)return 0;
 if(whole)return samples<=MNM_PRODUCER_V4_MAX_QUEUES&&producer&&trace&&binding&&!batch;
 if(samples<=16)return 1;return samples==MNM_PRODUCER_V3_QUEUES&&producer&&(trace||batch);
}
