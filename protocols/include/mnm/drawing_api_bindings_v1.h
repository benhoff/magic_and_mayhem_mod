#pragma once
/* Per-run PE32 allocation/module identities, never shared host pointers. */
#define MNM_DRAW_API_BINDINGS "MNMDAB01"
#define MNM_DRAW_API_BINDINGS_END "MNMDABEN"
/* Header (128 bytes,32 words): magic[2],version=1,size=128,row size=48,
 * count,original base,API module base/size,core module base/size/leave,
 * menu module base/size,reserved[2],original SHA256[8],reserved[8].
 * Row (48 bytes,12 words): tag,original CALL site,splice size,CALL size,
 * trampoline base/size,remainder PC,BEFORE stub,AFTER stub,trampoline CALL
 * return PC,original CALL return PC,reserved.
 * Terminal (64 bytes): magic[2],version=1,row count,reserved[12].
 * Null core/menu modules are explicit. Nonnull module bounds and an exact
 * exported core leave identity do not infer original/logical function ownership.
 * A closed map records prepared allocations, not successful installation.
 */
