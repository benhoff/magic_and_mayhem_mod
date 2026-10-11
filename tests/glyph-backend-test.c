#include "../reconstruction/rendering/glyph_backend.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned executions,refusals;
void mnm_glyph_fp_execute(MnmGlyphBackend *b){(void)b;++executions;}
void mnm_glyph_fp_begin(uint32_t *a,uint32_t b){(void)a;(void)b;assert(0);}
void mnm_glyph_fp_run(uint16_t *a,const uint8_t *b,uint32_t c,const uint32_t *d,const uint32_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;assert(0);}
static void reject(MnmGlyphBackend b){
    uint16_t pixels[64];uint32_t table[64];uint8_t fx[512];MnmGlyphState result=b.result;
    memcpy(pixels,b.pixels,sizeof pixels);memcpy(table,b.coverage,sizeof table);memcpy(fx,b.outgoing_fx,sizeof fx);
    assert(mnm_glyph_backend(&b)==1);assert(!memcmp(pixels,b.pixels,sizeof pixels));assert(!memcmp(table,b.coverage,sizeof table));assert(!memcmp(fx,b.outgoing_fx,sizeof fx));assert(!memcmp(&result,&b.result,sizeof result));++refusals;
}
int main(void){
    uint8_t frame[52]={52,0,0,0,2,0,0,0,1,0,0,0};frame[40]=48;frame[44]=50;frame[48]=0;frame[49]=2;frame[50]=0;frame[51]=63;
    uint16_t pixels[64];uint32_t table[64]={0};uint8_t incoming[512] __attribute__((aligned(16)))={0},outgoing[512] __attribute__((aligned(16)));
    memset(pixels,0xa5,sizeof pixels);memset(outgoing,0xd7,sizeof outgoing);incoming[0]=0x7f;incoming[1]=3;incoming[24]=0x80;incoming[25]=0x1f;
    MnmGlyphBackend good={{frame,sizeof frame,0x700000,0x6f5dac,0x100000,0x202,0,0,8,8,2,2,255,255,255,0,0},pixels,8,8,8,table,incoming,outgoing,{0}};
    MnmGlyphBackend bad=good;
    bad.width=0;reject(bad);bad=good;bad.height=2049;reject(bad);bad=good;bad.stride=7;reject(bad);bad=good;bad.state.bottom=9;reject(bad);
    bad=good;bad.incoming_fx=incoming+1;reject(bad);
    for(unsigned i=1;i<=7;++i){incoming[4]=(uint8_t)(1u<<i);reject(good);}incoming[4]=0;
    incoming[0]=0x7e;reject(good);incoming[0]=0x7f;
    incoming[1]=1;reject(good);incoming[1]=7;reject(good);incoming[1]=3;
    incoming[2]=0x80;reject(good);incoming[2]=0;
    incoming[26]=1;reject(good);incoming[26]=0;
    table[63]=0x3f800001;reject(good);table[63]=0x80000000;reject(good);table[63]=0x7fc00000;reject(good);table[63]=0;
    incoming[4]=1;incoming[40]=0xff;incoming[41]=0x7f;reject(good);incoming[40]=0;incoming[41]=0;incoming[32]=1;reject(good);incoming[32]=0;incoming[40]=1;reject(good);incoming[40]=0;incoming[4]=0;
    frame[51]=64;reject(good);frame[51]=63;
    assert(!executions);assert(!mnm_glyph_backend(&good));assert(executions==1);
    printf("%u atomic admission refusals; admitted dispatch reached once\n",refusals);return 0;
}
