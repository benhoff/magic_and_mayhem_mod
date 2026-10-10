#pragma once
#include "../../renderer/sprites/word_raster.h"

/* Workspace writes for the strictly in-bounds native admission only.
 * Retain fields not written by the selected original branch. */
static inline void mnm_word_unclipped_workspace(uint32_t s[16],const MnmWordDraw *d,
                                               uint32_t frame,uint32_t destination,
                                               uint32_t stride,uint32_t backend) {
    s[0]=0;s[1]=d->height;s[2]=(stride-d->width)*2;s[5]=d->width;
    s[6]=(uint32_t)d->top*stride;s[7]=frame+40;s[8]=0;s[10]=frame;
    s[9]=backend==0x197086?
        (d->last_run&&d->last_run_y==d->height-1?
         d->width-d->last_run_x-d->last_run:d->width):0;
    if(backend==0x197086&&d->last_run){
        uint32_t at=destination+((d->top+d->last_run_y)*stride+d->left+d->last_run_x)*2;
        s[11]=d->last_run-((at&2)!=0);
    }
}
