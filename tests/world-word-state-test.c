#include "../runtime/scene/world_word_state.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void put(uint8_t* p,uint32_t value){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(value>>(8*i));}
int main(void){
    uint8_t frame[52]={0};
    put(frame,52);put(frame+4,1);put(frame+8,1);put(frame+28,UINT32_MAX);
    put(frame+40,48);put(frame+44,50);frame[48]=0;frame[49]=1;frame[50]=0x34;frame[51]=0x12;
    uint32_t workspace[16],saved[16],before_saved[16];
    uint8_t fx[512]={0};fx[0]=0x7f;fx[1]=3;fx[3]=2;
    for(unsigned i=0;i<16;++i){workspace[i]=0xabc00000+i;saved[i]=0x12340000+i;}
    memcpy(before_saved,saved,sizeof(saved));
    MnmWordBackendInput in={frame,sizeof(frame),0x21000000,0x22000000,8,8,11,0x197086,0,0,2,3};
    MnmWorldWordState state;
    assert(!mnm_world_word_prepare(&in,workspace,fx,&state));
    assert(state.positive&&state.backend.argument_x==2&&state.backend.argument_y==3);
    mnm_world_word_commit(&state,workspace,saved,fx);
    assert(!memcmp(workspace,state.backend.words,64));
    assert(saved[7]==0&&saved[11]==2&&saved[12]==3&&!(fx[3]&2));
    assert((saved[8]&0x8d5)==0x44);
    for(unsigned i=0;i<16;++i)if(i!=7&&i!=8&&i!=11&&i!=12)assert(saved[i]==before_saved[i]);
    unsigned refusals=0;
    for(unsigned mode=0;mode<4;++mode){
        memset(&state,0xa5,sizeof(state));MnmWorldWordState before=state;
        uint8_t bad[512];memcpy(bad,fx,512);
        if(mode==0)bad[0]&=(uint8_t)~1u;
        if(mode==1)bad[1]=(bad[1]&0xfcu)|1u;
        if(mode==2)bad[4]=0x80;
        if(mode==3)in.backend_rva=0;
        uint32_t ws[16],regs[16];memcpy(ws,workspace,64);memcpy(regs,saved,sizeof(saved));
        const int result=mnm_world_word_prepare(&in,workspace,bad,&state);
        assert(result&& !memcmp(&state,&before,sizeof(state)));
        assert(!memcmp(workspace,ws,64)&&!memcmp(saved,regs,sizeof(saved)));
        ++refusals;in.backend_rva=0x197086;
    }
    put(frame+4,0);put(frame+8,0);fx[3]=2;fx[4]=255;fx[0]=0;
    assert(!mnm_world_word_prepare(&in,workspace,fx,&state)&&!state.positive);
    mnm_world_word_commit(&state,workspace,saved,fx);assert(fx[3]==2);
    printf("%u atomic state refusals; positive and empty commits pass\n",refusals);
    return 0;
}
