#include "fade_route.h"
#include "../../reconstruction/rendering/fade_kernel.h"
MnmFadeResolve mnm_fade_resolve;
uintptr_t mnm_fade_original,mnm_fade_resume;
uint32_t mnm_fade_native,mnm_fade_forwarded;
int mnm_fade_route(uint32_t *r) {
    struct MnmFadePlane p={0};
    if(!mnm_fade_resolve || !mnm_fade_resolve(r,&p) || !p.pixels || !p.width ||
       p.width>2048 || !p.height || p.height>2048 || p.stride<p.width || p.stride>4096 ||
       r[2]!=0 || r[6]!=r[7] || r[7]!=(uintptr_t)p.pixels || r[5]!=p.height ||
       r[1]!=(p.format ? 0xfdef7defu : 0xfbef7befu)) {
        ++mnm_fade_forwarded;return 0;
    }
    const uint32_t count=(p.width/2)*2*p.height;
    if((uintptr_t)p.pixels>UINT32_MAX-count*2u){++mnm_fade_forwarded;return 0;}
    mnm_fade_words(p.pixels,count,p.format);
    /* Final original outer-loop CMP(height,height); other flags remain intact.
     * Original tail reloads the lock pointer and restores its saved EBX. */
    r[2]=p.height;r[5]=0;r[6]=r[7]+count*2;
    r[8]=(r[8]&~0x8d5u)|0x44u;
    ++mnm_fade_native;return 1;
}
