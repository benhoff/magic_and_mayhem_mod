#pragma once
/* Little-endian private observation file, no host pointers or render-wire edits.
 * Stream header64, frame header128, actor256, matched draw80, terminal64.
 * Every frame token is local to that frame. Slot/pool epochs are observational,
 * never claims about allocation generations. Producers never call game helpers.
 */
#define MNM_SAO_MAGIC "MNMSAO01"
#define MNM_SAO_END "MNMSAOE1"
#define MNM_SAO_VERSION 1u
#define MNM_SAO_BUILD 0x40209ca7u
#define MNM_SAO_HEADER 64u
#define MNM_SAO_FRAME 128u
#define MNM_SAO_ACTOR 256u
#define MNM_SAO_DRAW 80u
#define MNM_SAO_ACTORS 256u
#define MNM_SAO_DRAWS 4096u
#define MNM_SAO_FRAMES 8192u
#define MNM_SAO_BYTES (256u*1024u*1024u)
