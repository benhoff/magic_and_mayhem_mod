#include "fade_kernel.h"
void mnm_fade_words(uint16_t *words,uint32_t count,uint32_t format) {
    const uint32_t mask=format ? 0x7defu : 0x7befu;
    for(uint32_t i=0;i<count;++i)words[i]=(uint16_t)((words[i]>>1)&mask);
}
