#include "../reconstruction/rendering/fade_kernel.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    uint16_t pixels[65538];
    for(unsigned format=0;format<4;++format){
        pixels[0]=pixels[65537]=0xb37d;
        for(unsigned i=0;i<65536;++i)pixels[i+1]=(uint16_t)i;
        mnm_fade_words(pixels+1,65536,format);
        for(unsigned i=0;i<65536;++i)assert(pixels[i+1]==((i>>1)&(format?0x7def:0x7bef)));
        assert(pixels[0]==0xb37d && pixels[65537]==0xb37d);
    }
    mnm_fade_words(0,0,0);
    puts("262144 independently checked words; guards and empty span pass");
    return 0;
}
