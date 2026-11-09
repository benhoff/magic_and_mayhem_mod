#include "../runtime/scene/drawing_caller_state_codec.h"
#include <stdio.h>
#include <string.h>
int main(void){
    unsigned count=0;uint8_t fx[512],out[MNM_CALLER_STATE_FX_BYTES+2];
    for(unsigned top=0;top<8;++top)for(unsigned tag=0;tag<256;++tag){
        for(unsigned i=0;i<512;++i)fx[i]=(uint8_t)(i*37+top*71+tag);
        fx[3]=(uint8_t)((fx[3]&~0x38u)|(top<<3));fx[4]=(uint8_t)tag;
        const uint8_t before[2]={0xa7,0x5c};memset(out,0xff,sizeof(out));out[0]=before[0];out[sizeof(out)-1]=before[1];
        mnm_caller_state_fx(out+1,fx);
        if(out[0]!=before[0]||out[sizeof(out)-1]!=before[1])return 1;
        const uint8_t* packed=out+1;
        if(memcmp(packed,fx,5)||memcmp(packed+8,fx+24,4)||memcmp(packed+92,fx+160,128))return 2;
        for(unsigned i=0;i<8;++i)for(unsigned j=0;j<10;++j)
            if(packed[12+i*10+j]!=((tag&(1u<<((top+i)&7)))?fx[32+i*16+j]:0))return 3;
        for(unsigned i=5;i<8;++i)if(packed[i])return 4;
        for(unsigned i=220;i<224;++i)if(packed[i])return 5;
        ++count;
    }
    printf("%u TOP/tag combinations; active values, empty normalization, control/status, XMM and canaries verified\n",count);
    return count==2048?0:6;
}
