#ifndef MNM_WORLD_COLOUR_H
#define MNM_WORLD_COLOUR_H
#include <stdint.h>
/* Read-only diagnostics, never a native drawing input or writer attribution. */
struct mnm_colour_checkpoint {
    uint32_t tag,caller,count,positions,left,top,right,bottom;
};
static inline struct mnm_colour_checkpoint mnm_colour_scan(const uint8_t* pixels,
        uint32_t width,uint32_t height,uint32_t stride,uint32_t colour){
    struct mnm_colour_checkpoint result={0,0,0,2166136261u,width,height,0,0};
    for(uint32_t y=0;y<height;++y)for(uint32_t x=0;x<width;++x){
        const uint8_t* p=pixels+2*(y*stride+x);
        if(((uint32_t)p[0]|(uint32_t)p[1]<<8)!=colour)continue;
        ++result.count;uint32_t position=y*width+x;
        for(unsigned byte=0;byte<4;++byte){result.positions^=(position>>(byte*8))&255;result.positions*=16777619u;}
        if(x<result.left)result.left=x;
        if(y<result.top)result.top=y;
        if(x+1>result.right)result.right=x+1;
        if(y+1>result.bottom)result.bottom=y+1;
    }
    if(!result.count)result.left=result.top=result.right=result.bottom=0;
    return result;
}
#endif
