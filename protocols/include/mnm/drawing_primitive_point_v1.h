#pragma once
#define MNM_PRIMITIVE_POINT_VERSION 1u
#define MNM_PRIMITIVE_POINT_BYTES 192u
/* Read-only, in-process diagnostic query. No pointers or pixel bytes cross it.
 * DWORDs: version, thread, state (0 no active call, 1 active, 2 stopped,
 * 3 recorder unavailable), call, parent, tag, caller, primitive, depth,
 * producer prefix, queue prefix, same-thread active calls, request present,
 * reserved, QPC low/high; words16..39 canonical request header; words40..47
 * known canvas ID/width/height/stride/dirty/sampled/has CPU storage/reserved.
 * Identity is local to this recorder invocation. A point never leases storage.
 */
#define MNM_PRIMITIVE_API_MAGIC "MNMPAPI1"
#define MNM_PRIMITIVE_API_VERSION 1u
#define MNM_PRIMITIVE_API_ROW_BYTES 256u
#define MNM_PRIMITIVE_API_MAX_ROWS 786433u
/* Header64: magic/version/row bytes/build40209ca7/row budget/point bytes/zero.
 * Row64: ordinal, phase (1 entry,2 original return,3 completion,4 terminal),
 * API entry, thread, method, destination identity, source identity, result,
 * API completion-record ordinal (phase3 only), reserved9..15, point16..63.
 * Terminal: word2 reason, word3 thread, word4 total entry count; rest zero.
 */
