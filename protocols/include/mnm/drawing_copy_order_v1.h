#pragma once
/* Pointer-free original-active copy request entry/return observation. This is
 * recorder order, not the driver's execution order or a replacement contract. */
#define MNM_COPY_ORDER_MAGIC "MNMDCQ01"
#define MNM_COPY_ORDER_VERSION 1u
#define MNM_COPY_ORDER_ROW_BYTES 128u
#define MNM_COPY_ORDER_MAX_RECORDS 65536u
/* Header: magic, version, row bytes, pinned build, row budget, ten zero words.
 * Row: ordinal, phase(1 entry/2 return/3 terminal), request, owner, hook tag,
 * tick, producer prefix, queue; words8..19 are dst/src IDs, dst width/height/
 * stride, dst x/y, source rectangle and raw key. Word20 caller RVA, word21
 * emitted producer return sequence, words22/23 zero, word24 active requests,
 * words25..31 zero. Terminal request is total requests; other identities and
 * descriptor words are zero. A terminal with active requests is incomplete. */
