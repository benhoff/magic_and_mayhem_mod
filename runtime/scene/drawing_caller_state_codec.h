#pragma once
#include <stdint.h>
#include "../../protocols/include/mnm/drawing_caller_state_v1.h"

static inline void mnm_caller_state_fx(uint8_t out[MNM_CALLER_STATE_FX_BYTES],const uint8_t fx[512]){
    for(unsigned i=0;i<MNM_CALLER_STATE_FX_BYTES;++i)out[i]=0;
    for(unsigned i=0;i<5;++i)out[i]=fx[i];
    for(unsigned i=0;i<4;++i)out[8+i]=fx[24+i];
    const unsigned top=(fx[3]>>3)&7;
    for(unsigned i=0;i<8;++i)if(fx[4]&(1u<<((top+i)&7)))
        for(unsigned j=0;j<10;++j)out[12+i*10+j]=fx[32+i*16+j];
    for(unsigned i=0;i<128;++i)out[92+i]=fx[160+i];
}
