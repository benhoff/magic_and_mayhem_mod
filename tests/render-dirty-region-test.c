#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/render/dirty_region.h"
static void check(int ok){if(!ok)abort();}
static unsigned state=0x53c143u;
static unsigned random_word(void){state=state*1664525u+1013904223u;return state;}
int main(void){
    unsigned cases=0;
    for(unsigned bytes=1;bytes<=4;++bytes)for(unsigned width=1;width<=19;++width)for(unsigned height=1;height<=13;++height){
        unsigned length=width*height*bytes;
        unsigned char before[1024],after[1024],replayed[1024];
        for(unsigned i=0;i<length;++i)before[i]=(unsigned char)random_word();
        for(unsigned pattern=0;pattern<6;++pattern){
            memcpy(after,before,length);
            if(pattern==1)after[0]^=1;
            if(pattern==2)after[length-1]^=0x80;
            if(pattern==3){after[0]^=1;after[length-1]^=0x80;}
            if(pattern==4)for(unsigned i=0;i<length;++i)after[i]^=0x55;
            if(pattern==5)for(unsigned i=0;i<length;++i)if(random_word()%13==0)after[i]^=0x40;
            unsigned r[4]={0},changed=command_dirty_region(before,after,width,height,bytes,r);
            check(changed==(memcmp(before,after,length)!=0));
            memcpy(replayed,before,length);
            if(changed){
                check(r[0]<r[2] && r[2]<=width && r[1]<r[3] && r[3]<=height);
                for(unsigned y=r[1];y<r[3];++y)memcpy(replayed+(y*width+r[0])*bytes,after+(y*width+r[0])*bytes,(r[2]-r[0])*bytes);
                /* Each edge must include a changed pixel; verify minimality
                 * independently rather than recomputing the same envelope. */
                unsigned edges=0;
                for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x)if(memcmp(before+(y*width+x)*bytes,after+(y*width+x)*bytes,bytes)){
                    if(x==r[0])edges|=1;
                    if(x+1==r[2])edges|=2;
                    if(y==r[1])edges|=4;
                    if(y+1==r[3])edges|=8;
                }
                check(edges==15);
            }
            check(!memcmp(replayed,after,length));++cases;
        }
    }
    printf("{\"success\":true,\"cases\":%u,\"native_formats\":4}\n",cases);return 0;
}
