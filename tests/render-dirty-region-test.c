#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../runtime/render/dirty_region.h"
static void check(int ok){if(!ok)abort();}
static unsigned state=0x53c143u;
static unsigned random_word(void){state=state*1664525u+1013904223u;return state;}
static unsigned split_cases,dense_cases,empty_cases;
static void partition_check(const unsigned char* before,const unsigned char* after,unsigned width,unsigned height,unsigned bytes){
    unsigned length=width*height*bytes,regions[COMMAND_DIRTY_REGIONS][4],whole[4];
    unsigned count=command_dirty_regions(before,after,width,height,bytes,regions);
    unsigned char* replayed=malloc(length);unsigned char* covered=calloc(width*height,1);check(replayed && covered);
    memcpy(replayed,before,length);check(count<=32 && (count!=0)==(memcmp(before,after,length)!=0));
    unsigned cost=0;
    for(unsigned i=0;i<count;++i){unsigned* r=regions[i];
        check(r[0]<r[2] && r[2]<=width && r[1]<r[3] && r[3]<=height);
        cost+=(r[2]-r[0])*(r[3]-r[1])*bytes+32;
        for(unsigned y=r[1];y<r[3];++y){
            memcpy(replayed+(y*width+r[0])*bytes,after+(y*width+r[0])*bytes,(r[2]-r[0])*bytes);
            for(unsigned x=r[0];x<r[2];++x)check(!covered[y*width+x]++);
        }
    }
    if(count){check(command_dirty_region(before,after,width,height,bytes,whole));
        unsigned single=(whole[2]-whole[0])*(whole[3]-whole[1])*bytes+32;
        check(cost<=single && (count==1 || cost<single));
    }
    check(!memcmp(replayed,after,length));
    if(count>1)++split_cases;else if(count)++dense_cases;else ++empty_cases;
    free(covered);free(replayed);
}
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
            check(!memcmp(replayed,after,length));partition_check(before,after,width,height,bytes);++cases;
        }
    }
    /* Larger odd dimensions, distant changes, every grid cell, unused RGB32
     * high bytes, dense fallback and unrelated random sparse input. */
    for(unsigned bytes=1;bytes<=4;++bytes)for(unsigned pattern=0;pattern<5;++pattern){
        unsigned width=137,height=83,length=width*height*bytes;
        unsigned char* before=malloc(length);unsigned char* after=malloc(length);check(before && after);
        for(unsigned i=0;i<length;++i)before[i]=(unsigned char)random_word();
        memcpy(after,before,length);
        if(pattern==1){after[bytes-1]^=0x80;after[length-1]^=0x80;}
        if(pattern==2)for(unsigned i=0;i<length;++i)after[i]^=0x55;
        if(pattern==3)for(unsigned i=0;i<length;++i)if(random_word()%103==0)after[i]^=1;
        if(pattern==4)for(unsigned y=0;y<8;++y)for(unsigned x=0;x<4;++x)after[((y*10+3)*width+x*34+2)*bytes+bytes-1]^=0x80;
        partition_check(before,after,width,height,bytes);free(after);free(before);++cases;
    }
    check(split_cases && dense_cases && empty_cases);
    printf("{\"success\":true,\"cases\":%u,\"native_formats\":4,\"split_cases\":%u,\"single_cases\":%u,\"unchanged_cases\":%u}\n",cases,split_cases,dense_cases,empty_cases);return 0;
}
