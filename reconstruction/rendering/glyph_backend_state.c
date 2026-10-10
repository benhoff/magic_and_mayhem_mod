#include "glyph_backend_state.h"
static uint32_t word(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
int mnm_glyph_coverage_bits(uint32_t rounding,uint32_t out[64]){
    if(rounding>3||!out)return 1;
    out[0]=0;
    for(uint32_t i=1;i<64;++i){
        uint32_t n=i*8521761u,bits=0;
        for(uint32_t v=n;v>>=1;)++bits;
        uint32_t mantissa,exponent=bits+98;
        if(bits<=23)mantissa=n<<(23-bits);
        else{
            uint32_t shift=bits-23,remainder=n&((1u<<shift)-1),half=1u<<(shift-1);
            mantissa=n>>shift;
            if((rounding==2&&remainder)||(rounding==0&&(remainder>half||(remainder==half&&(mantissa&1)))))++mantissa;
            if(mantissa==0x1000000u){mantissa>>=1;++exponent;}
        }
        out[i]=(exponent<<23)|(mantissa&0x7fffffu);
    }
    return 0;
}
static uint32_t return_flags(uint32_t entry_sp,uint32_t incoming){
    uint32_t a=entry_sp-40,result=entry_sp,flags=incoming&0x400;
    if(result<a)flags|=1;
    uint32_t parity=result&255;parity^=parity>>4;parity^=parity>>2;parity^=parity>>1;
    if(!(parity&1))flags|=4;
    if((a^40u^result)&16)flags|=16;
    if(!result)flags|=64;
    if(result&0x80000000u)flags|=128;
    if((~(a^40u)&(a^result))&0x80000000u)flags|=0x800;
    return flags;
}
static int valid(const MnmGlyphStateInput *in,uint32_t width,uint32_t height,int *opaque){
    size_t table=40+(size_t)height*8,bound=in->frame_bytes;
    if(table>bound)return 0;
    for(unsigned j=0;j<2;++j){uint32_t plane=word(in->frame+32+j*4);if(plane&&(plane<table||plane>in->frame_bytes))return 0;if(plane&&plane<bound)bound=plane;}
    for(uint32_t y=0;y<height;++y){
        size_t control=word(in->frame+40+y*8),colour=word(in->frame+44+y*8);
        if(control<table||colour<table||control>bound||colour>bound)return 0;
        uint32_t x=0,on=0;opaque[y]=0;
        while(x<width){
            if(control>=colour||control>=bound)return 0;
            uint32_t n=in->frame[control++];if(n>width-x||(on&&!n))return 0;
            if(on){if(n>bound-colour)return 0;for(uint32_t k=0;k<n;++k)if(in->frame[colour+k]>=64)return 0;colour+=n;opaque[y]+=(int)n;}
            x+=n;on=!on;
        }
    }
    return 1;
}
int mnm_glyph_backend_state(const MnmGlyphStateInput *in,MnmGlyphState *out){
    if(!in||!out||!in->frame||in->frame_bytes<40||in->frame_bytes>1048576||
       word(in->frame)!=in->frame_bytes||word(in->frame+28)==UINT32_MAX||
       in->frame_address>UINT32_MAX-in->frame_bytes||in->coverage_address>UINT32_MAX-256||
       !in->entry_sp||in->red>255||in->green>255||in->blue>255||in->cold>1||in->rounding>3||
       in->left<0||in->top<0||in->right<=in->left||in->bottom<=in->top||
       in->right>2048||in->bottom>2048)return 1;
    uint32_t width=word(in->frame+4),height=word(in->frame+8);if(width>2048||height>128)return 1;
    int opaque[128];if(!valid(in,width,height,opaque))return 1;
    int64_t x=(int64_t)in->x-(int32_t)word(in->frame+12),y=(int64_t)in->y-(int32_t)word(in->frame+16);
    if(x<-4096||x>4096||y<-4096||y>4096)return 1;
    MnmGlyphState s={0};s.ecx=11;s.edx=height;
    s.arguments[0]=(uint32_t)in->y;s.arguments[1]=in->red/8;s.arguments[2]=in->green/4;s.arguments[3]=in->blue/8;
    s.flags=return_flags(in->entry_sp,in->incoming_flags);
    if(!width&&!height){s.eax=in->cold?(in->coverage_address+256)&0xffff0000u:0;s.branch=MNM_GLYPH_EMPTY;goto done;}
    s.eax=(uint32_t)y&0xffff0000u;
    if(x+width<=in->left){s.branch=MNM_GLYPH_LEFT_OUT;goto done;}
    if(y+height<=in->top){s.branch=MNM_GLYPH_TOP_OUT;goto done;}
    if(x>=in->right){s.branch=MNM_GLYPH_RIGHT_OUT;goto done;}
    if(y>=in->bottom){s.branch=MNM_GLYPH_BOTTOM_OUT;goto done;}
    uint32_t start=y<in->top?(uint32_t)(in->top-y):0;
    uint32_t end=y+height>in->bottom?(uint32_t)(in->bottom-y):height;
    s.arguments[0]=end;s.edx=in->frame_address+40+start*8;s.eax=start>=height?4:0;
    if(x<in->left||x+width>in->right){s.branch=MNM_GLYPH_HORIZONTAL;goto done;}
    if(start>=end){s.branch=MNM_GLYPH_NO_ROWS;goto done;}
    s.branch=MNM_GLYPH_ROWS;s.visible_rows=end-start;s.edx=in->frame_address+40+end*8;
    for(uint32_t row=start;row<end;++row){s.opaque_pixels+=(uint32_t)opaque[row];if(opaque[row])s.arguments[0]=0;}
 done:
    *out=s;return 0;
}
