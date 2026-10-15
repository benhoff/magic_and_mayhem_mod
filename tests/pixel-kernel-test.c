#include "../reconstruction/rendering/pixel_kernel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    uint16_t pixels[64], before[64];uint8_t encoded[132]={0}, palette[768]={0}, indices[12];
    struct MnmPixelPlane p={pixels,4,3,6,0};
    struct MnmPcxInput s={encoded,palette,sizeof(encoded),4,1};
    encoded[128]=0xc4;encoded[129]=255;palette[765]=255;palette[766]=128;palette[767]=7;
    for(unsigned i=0;i<64;++i)pixels[i]=0x1234;
    assert(mnm_pcx_words(&p,&s));for(unsigned i=0;i<4;++i)assert(pixels[i]==0xfc00);
    assert(pixels[4]==0x1234);memcpy(before,pixels,sizeof(pixels));
    encoded[128]=0xc5;assert(!mnm_pcx_words(&p,&s));assert(!memcmp(before,pixels,sizeof(pixels)));
    encoded[128]=0xc0;assert(!mnm_pcx_words(&p,&s));assert(!memcmp(before,pixels,sizeof(pixels)));
    encoded[128]=0xc4;s.bytes=129;assert(!mnm_pcx_words(&p,&s));s.bytes=132;
    memset(indices,99,sizeof(indices));assert(mnm_pcx_indices(indices,sizeof(indices),6,&s));assert(indices[0]==255 && indices[3]==255 && indices[4]==99);
    uint8_t info[1064]={40,0,0,0,3,0,0,0,2,0,0,0,1,0,8}, bits[8]={0,1,2,77,2,1,0,88};
    info[40+1*4+2]=255;info[40+2*4+1]=255;
    struct MnmDibInput d={info,bits,sizeof(info),sizeof(bits),0};
    assert(mnm_dib_words(&p,&d,1,1));assert(pixels[7]==0x7e0 && pixels[8]==0xf800 && pixels[9]==0);
    memcpy(before,pixels,sizeof(pixels));d.pixel_bytes=7;assert(!mnm_dib_words(&p,&d,0,0));assert(!memcmp(before,pixels,sizeof(pixels)));
    d.pixel_bytes=8;info[16]=1;assert(!mnm_dib_words(&p,&d,0,0));assert(!memcmp(before,pixels,sizeof(pixels)));
    info[16]=0;assert(mnm_dib_words(&p,&d,INT32_MIN,INT32_MAX));assert(!memcmp(before,pixels,sizeof(pixels)));
    unsigned checked=0;
    for(unsigned format=0;format<2;++format)for(unsigned old=0;old<65536;++old){p.format=format;pixels[0]=(uint16_t)old;
        assert(mnm_rgb_add(&p,0,0,1,1,65535,65535,65535));
        const unsigned shift=format?10:11,gm=format?31:63;
        const unsigned r=(((old>>shift)&31)-1)&65535,g=(((old>>5)&gm)-1)&65535,b=((old&31)-1)&65535;
        assert(pixels[0]==((r>31?31:r)<<shift | (g>gm?gm:g)<<5 | (b>31?31:b)));++checked;}
    printf("{\"success\":true,\"word_wrap_cases\":%u,\"atomic_refusal_cases\":6}\n",checked);
}
