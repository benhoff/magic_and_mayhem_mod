#include "word_backend_state.h"

/* Explicit word copies keep the recovered model usable in a no-CRT PE32 hook. */
static void store(MnmWordBackendState *out,const MnmWordBackendState *in) {
    for(unsigned i=0;i<16;++i)out->words[i]=in->words[i];
    out->argument_x=in->argument_x;out->argument_y=in->argument_y;
    out->return_eax=in->return_eax;
}

static uint32_t word(const uint8_t *p) {
    return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static int valid_frame(const uint8_t *f,size_t bytes,uint32_t width,uint32_t height) {
    size_t table=40+(size_t)height*8;
    if(table>bytes)return 0;
    size_t plane_end=bytes;
    for(unsigned i=0;i<2;++i){
        uint32_t auxiliary=word(f+32+i*4);
        if(auxiliary&&(auxiliary<table||auxiliary>bytes))return 0;
        if(auxiliary&&auxiliary<plane_end)plane_end=auxiliary;
    }
    size_t control=word(f+40),pixel=word(f+44),pixel_base=pixel;
    if(control<table||pixel<control||pixel>plane_end)return 0;
    for(uint32_t y=0;y<height;++y){
        if(word(f+40+y*8)!=control||word(f+44+y*8)!=pixel)return 0;
        uint32_t x=0,opaque=0;
        while(x<width){
            if(control>=pixel_base)return 0;
            uint32_t n=f[control++];
            if(n>width-x||(opaque&&!n))return 0;
            if(opaque){if(pixel>plane_end||n>(plane_end-pixel)/2)return 0;pixel+=n*2;}
            x+=n;opaque=!opaque;
        }
    }
    return 1;
}
int mnm_word_backend_state(const MnmWordBackendInput *in,const uint32_t before[16],
                           MnmWordBackendState *out) {
    if(!in||!before||!out||!in->frame||in->frame_bytes<40||in->frame_bytes>4*1024*1024||
       word(in->frame)!=in->frame_bytes||word(in->frame+28)!=UINT32_MAX||
       (in->backend_rva!=0x196cb8&&in->backend_rva!=0x197086)||
       !in->right||!in->bottom||in->right>2048||in->bottom>2048||
       in->stride_words<in->right||in->stride_words>4096||
       in->clip_left<0||in->clip_top<0||in->clip_left>=(int32_t)in->right||
       in->clip_top>=(int32_t)in->bottom||
       in->frame_address>UINT32_MAX-in->frame_bytes)return 1;
    const uint8_t *f=in->frame;
    uint32_t width=word(f+4),height=word(f+8);
    if(width>2048||height>2048)return 1;
    MnmWordBackendState result;
    for(unsigned i=0;i<16;++i)result.words[i]=before[i];
    result.argument_x=in->anchor_x;result.argument_y=in->anchor_y;result.return_eax=0;
    result.words[10]=in->frame_address;
    if(!width||!height){store(out,&result);return 0;}
    if(!valid_frame(f,in->frame_bytes,width,height))return 1;
    int64_t left=(int64_t)in->anchor_x-(int32_t)word(f+12);
    int64_t top=(int64_t)in->anchor_y-(int32_t)word(f+16);
    if(left<-4096||left>4096||top<-4096||top>4096)return 1;
    result.argument_x=(int32_t)left;result.argument_y=(int32_t)top;
    uint32_t *s=result.words;
    int horizontal=left<in->clip_left||left+width>=in->right;
    int32_t start_x=left<in->clip_left?in->clip_left-(int32_t)left:0;
    int32_t end_x=left+width>=(int64_t)in->right?(int32_t)(in->right-left):(int32_t)width;
    if(horizontal){s[12]=(uint32_t)start_x;s[13]=(uint32_t)end_x;}
    uint32_t start_y=top<in->clip_top?(uint32_t)(in->clip_top-top):0;
    int32_t end_y=top+height>=(int64_t)in->bottom?(int32_t)(in->bottom-top):(int32_t)height;
    s[0]=start_y;s[1]=(uint32_t)end_y;s[2]=(in->stride_words-width)*2;s[5]=width;
    s[6]=(uint32_t)(top<in->clip_top?in->clip_top:top)*in->stride_words;
    s[7]=in->frame_address+40+start_y*8;
    if(end_y<=(int32_t)start_y){store(out,&result);return 0;}
    int scalar=in->backend_rva==0x197086;
    s[8]=(uint32_t)end_y-start_y;
    for(uint32_t y=start_y;y<(uint32_t)end_y;++y){
        size_t control=word(f+40+y*8);
        uint32_t x=0;s[9]=width;
        if(horizontal)s[14]=0;
        while(x<width){
            uint32_t skip=f[control++];x+=skip;
            if(!scalar){s[9]-=skip;if(horizontal)s[14]+=skip;}
            if(x==width)break;
            uint32_t run=f[control++];
            s[9]=width-x-run;
            if(horizontal){
                s[15]=0;
                int32_t begin=(int32_t)x;
                if(start_x>begin){
                    if(start_x-begin>=(int32_t)run){x+=run;s[14]=x;continue;}
                    begin=start_x;
                }
                if(begin>=end_x){x+=run;s[14]=x;continue;}
                int32_t finish=(int32_t)(x+run);
                if(finish>end_x){s[15]=(uint32_t)(finish-end_x);finish=end_x;}
                if(scalar){
                    uint32_t destination=in->canvas_address+((uint32_t)(top+y)*in->stride_words+in->anchor_x-(int32_t)word(f+12)+(uint32_t)begin)*2;
                    s[11]=(uint32_t)(finish-begin)-((destination&2)!=0);
                }
                x+=run;s[14]=x;
            }else{
                if(scalar){
                    uint32_t destination=in->canvas_address+((uint32_t)(top+y)*in->stride_words+(uint32_t)left+x)*2;
                    s[11]=run-((destination&2)!=0);
                }
                x+=run;
            }
        }
        --s[8];
    }
    store(out,&result);return 0;
}
