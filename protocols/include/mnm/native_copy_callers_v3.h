#ifndef MNM_NATIVE_COPY_CALLERS_V3_H
#define MNM_NATIVE_COPY_CALLERS_V3_H
#include "native_copy_callers_v2.h"
/* Same physical rows as V2; new magic/version admits mode3 only after the
 * separately verified native cursor active-scope handoff. V1/V2 stay immutable. */
#define MNM_COPY_CALLERS_V3_BYTES MNM_COPY_CALLERS_V2_BYTES
typedef struct MnmCopyCallersV2 MnmCopyCallersV3;
#define mnm_copy_callers_v3_record mnm_copy_callers_v2_record
#endif
