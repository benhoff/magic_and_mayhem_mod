#pragma once
#include "../canvas-writers/raster_workspace.h"

/* Selected original generic/black/displacement state. The owned command's
 * palette marker is canonical, so indexing and the guest frame identity are
 * explicit inputs. No guest address below is dereferenced by this model. */
typedef struct {
    const uint8_t* frame;
    uint32_t bytes,frame_address,canvas_address,right,bottom,stride,family,indexed;
    int32_t clip_left,clip_top,x,y;
} MnmWorldGenericInput;
typedef struct {
    uint32_t words[16],eax,flags,clear_c1;
    int32_t x,y;
} MnmWorldGenericState;

static inline uint32_t mnm_world_compare_flags(uint32_t left,uint32_t right){
    const uint32_t result=left-right;
    uint32_t parity=result&255u;parity^=parity>>4;parity^=parity>>2;parity^=parity>>1;
    return (left<right?1u:0u)|(!(parity&1)?4u:0u)|((left^right^result)&16u)|
        (!result?64u:0u)|(result&0x80000000u?128u:0u)|
        (((left^right)&(left^result)&0x80000000u)?0x800u:0u);
}

static inline int mnm_world_generic_frame(const MnmWorldGenericInput* in,uint32_t width,uint32_t height){
    const uint32_t table=40+height*8;
    if(table>in->bytes)return 0;
    uint32_t end=in->bytes;
    for(unsigned i=0;i<2;++i){
        const uint32_t extra=mnm_raster_word(in->frame+32+i*4);
        if(extra&&(extra<table||extra>in->bytes))return 0;
        if(extra&&extra<end)end=extra;
    }
    const uint32_t colour_start=mnm_raster_word(in->frame+44);
    if(colour_start<table||colour_start>end)return 0;
    uint32_t previous_control=0,previous_pixel=0;
    for(uint32_t row=0;row<height;++row){
        uint32_t control=mnm_raster_word(in->frame+40+row*8),pixel=mnm_raster_word(in->frame+44+row*8);
        if(control<table||control>=colour_start||pixel<colour_start||pixel>end||
           (in->family!=4&&row&&(control!=previous_control||pixel!=previous_pixel)))return 0;
        uint32_t x=0,opaque=0;
        while(x<width){
            if(control>=colour_start)return 0;
            const uint32_t count=in->frame[control++];
            if(count>width-x||(opaque&&!count))return 0;
            if(opaque){const uint32_t step=in->indexed?1u:2u;if(count>(end-pixel)/step)return 0;pixel+=count*step;}
            x+=count;opaque=!opaque;
        }
        previous_control=control;previous_pixel=pixel;
    }
    return 1;
}

static inline int mnm_world_generic_prepare(const MnmWorldGenericInput* in,const uint32_t before[16],
                                          const uint8_t fx[512],MnmWorldGenericState* output){
    if(!in||!before||!fx||!output||!in->frame||in->bytes<40||in->bytes>1048576||
       mnm_raster_word(in->frame)!=in->bytes||in->family>6||in->family==5||in->indexed>1||
       !in->right||in->right>2048||!in->bottom||in->bottom>2048||
       in->stride<in->right||in->stride>4096||in->clip_left<0||in->clip_top<0||
       in->clip_left>=(int32_t)in->right||in->clip_top>=(int32_t)in->bottom||
       in->frame_address>UINT32_MAX-in->bytes)return 1;
    MnmWorldGenericState state;
    for(unsigned i=0;i<16;++i)state.words[i]=before[i];
    state.eax=0;state.flags=0x44;state.clear_c1=0;state.x=in->x;state.y=in->y;
    if(in->family!=4)state.words[10]=in->frame_address;
    const uint32_t width=mnm_raster_word(in->frame+4),height=mnm_raster_word(in->frame+8);
    if(width>2048||height>2048)return 1;
    if(width&&height){
        const int64_t left=(int64_t)in->x-(int32_t)mnm_raster_word(in->frame+12);
        const int64_t top=(int64_t)in->y-(int32_t)mnm_raster_word(in->frame+16);
        if(left<-4096||left>4096||top<-4096||top>4096)return 1;
        state.x=(int32_t)left;
        const int refuse=in->family==4||in->family==6;
        if(refuse&&left<in->clip_left){
            state.eax=1;state.flags=mnm_world_compare_flags((uint32_t)left,(uint32_t)in->clip_left);
        }else{
            state.words[5]=width;
            if(refuse&&left+width>=in->right){
                state.eax=1;state.flags=mnm_world_compare_flags((uint32_t)(left+width),in->right);
            }else{
                if(in->family<4&&!in->indexed)return 1;
                if(!mnm_world_generic_frame(in,width,height))return 1;
                const uint32_t control=fx[0]|(uint32_t)fx[1]<<8,top_slot=(fx[3]>>3)&7;
                if((control&63)!=63||((control>>8)&3)==1||(fx[4]&(1u<<((top_slot+7)&7))))return 1;
                state.y=(int32_t)top;state.clear_c1=1;
                const int clipped=left<in->clip_left||left+width>=in->right;
                if(clipped){state.words[12]=left<in->clip_left?(uint32_t)(in->clip_left-left):0;
                    state.words[13]=left+width>=in->right?(uint32_t)(in->right-left):width;}
                const uint32_t first=top<in->clip_top?(uint32_t)(in->clip_top-top):0;
                const int32_t last=top+height>=in->bottom?(int32_t)(in->bottom-top):(int32_t)height;
                state.words[0]=first;state.words[1]=(uint32_t)last;
                state.words[2]=(in->stride-width)*2;
                state.words[6]=(uint32_t)(top<in->clip_top?in->clip_top:top)*in->stride;
                state.words[7]=in->frame_address+40+first*8;
                struct MnmRasterPixels raster;
                for(unsigned i=0;i<sizeof(raster);++i)((uint8_t*)&raster)[i]=0;
                raster.frame=in->frame;raster.left=(int32_t)left;raster.top=(int32_t)top;
                raster.clip_left=in->clip_left;raster.clip_top=in->clip_top;
                raster.clip_right=(int32_t)in->right;raster.clip_bottom=(int32_t)in->bottom;
                const struct MnmPixelPlane plane={(uint16_t*)(uintptr_t)in->canvas_address,in->right,in->bottom,in->stride,0};
                mnm_raster_workspace(state.words,&raster,&plane,in->frame_address,in->family,clipped);
            }
        }
    }
    for(unsigned i=0;i<16;++i)output->words[i]=state.words[i];
    output->eax=state.eax;output->flags=state.flags;output->clear_c1=state.clear_c1;
    output->x=state.x;output->y=state.y;
    return 0;
}

static inline void mnm_world_generic_commit(const MnmWorldGenericState* state,uint32_t workspace[16],
                                         uint32_t saved[13],uint8_t fx[512]){
    for(unsigned i=0;i<16;++i)workspace[i]=state->words[i];
    saved[7]=state->eax;saved[8]=(saved[8]&~0x8d5u)|state->flags;
    saved[11]=(uint32_t)state->x;saved[12]=(uint32_t)state->y;
    if(state->clear_c1)fx[3]&=(uint8_t)~2u;
}
