#ifndef MNM_NATIVE_CPU_READ_V1_H
#define MNM_NATIVE_CPU_READ_V1_H
/* Variable-length atomic reply file: 64-byte header plus tightly packed RGB565.
 * Header DWORDs: 8-byte magic, version, header bytes, command session, ticket
 * low/high, surface ID, width, height, payload bytes, history generation low/high,
 * nonce low/high, ready=1. Reserved/extra bytes are forbidden. One retained reply
 * per private launch; atomic rename prevents partially published input. */
#define MNM_NATIVE_CPU_READ_V1_MAGIC "MNMCREAD"
#define MNM_NATIVE_CPU_READ_V1_VERSION 1u
#define MNM_NATIVE_CPU_READ_V1_HEADER 64u
#define MNM_NATIVE_CPU_READ_V1_MAX_PIXELS (2048u*2048u)
#define MNM_NATIVE_CPU_READ_V1_TIMEOUT_MS 10000u
#include <stdint.h>
/* Header words are decoded little-endian before calling. Request has the ten
 * CPU_READ words; no process pointer or host struct layout is accepted. */
static inline int mnm_native_cpu_read_reply(const uint32_t* h,uint32_t bytes,const uint32_t* r){
 if(!h || !r || !r[0] || !r[1] || r[2] || !r[3] || !r[4] || r[4]>2048 || !r[5] || r[5]>2048 ||
    (!r[6]&&!r[7]) || (!r[8]&&!r[9]))return 0;
 const uint32_t payload=r[4]*r[5]*2;
 return h[0]==0x434d4e4du && h[1]==0x44414552u && h[2]==1 && h[3]==64 && h[4]==r[3] &&
    h[5]==r[1] && h[6]==r[2] && h[7]==r[0] && h[8]==r[4] && h[9]==r[5] && h[10]==payload &&
    h[11]==r[6] && h[12]==r[7] && h[13]==r[8] && h[14]==r[9] && h[15]==1 && bytes==64+payload;
}
#endif
