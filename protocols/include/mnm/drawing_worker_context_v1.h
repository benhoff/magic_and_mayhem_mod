#pragma once
/* Explicit little-endian u32wire,64bytes,neverhostpointers/objectlayouts. */
#define MNM_DRAW_WORKER_CONTEXT_MAGIC "MNMWCX01"
#define MNM_DRAW_WORKER_CONTEXT_BYTES 64u
#define MNM_DRAW_WORKER_CONTEXT_VERSION 1u
/* words0..1magic,2version,3bytes,4seqlock,5active,6worker,7command,
 * 8submission,9Win32thread,10lifetimeprefix,11flags,12..15reservedzero. */
