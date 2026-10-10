#include "glyph_backend.h"
#if defined(__i386__) || defined(_M_IX86)
typedef char incoming_fx_offset[(offsetof(MnmGlyphBackend,incoming_fx)==88)?1:-1];
typedef char outgoing_fx_offset[(offsetof(MnmGlyphBackend,outgoing_fx)==92)?1:-1];
#endif
static uint32_t word(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t half(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8;}
/* Assembly executes under the admitted caller environment, then restores the
 * host environment before returning. Integer traversal never uses host FP. */
void mnm_glyph_fp_execute(MnmGlyphBackend *);
void mnm_glyph_fp_begin(uint32_t *,uint32_t);
void mnm_glyph_fp_run(uint16_t *,const uint8_t *,uint32_t,const uint32_t *,const uint32_t *);
int mnm_glyph_backend(MnmGlyphBackend *b){
    MnmGlyphState result;
    if(!b||!b->pixels||!b->coverage||!b->incoming_fx||!b->outgoing_fx||
       ((uintptr_t)b->incoming_fx&15)||((uintptr_t)b->outgoing_fx&15)||
       !b->width||!b->height||b->width>2048||b->height>2048||
       b->stride<b->width||b->stride>2048||
       b->state.right>(int32_t)b->width||b->state.bottom>(int32_t)b->height||
       mnm_glyph_backend_state(&b->state,&result))return 1;
    const uint8_t *fx=b->incoming_fx;
    uint32_t cw=half(fx),status=half(fx+2),top=(status>>11)&7;
    if((cw&63)!=63||((cw>>8)&3)==1||((cw>>10)&3)!=b->state.rounding||
       (status&0x80)||(word(fx+24)&~0xffbfu))return 1;
    /* Every physical slot used by the deepest seven-push sequence must be
     * empty. A count of occupied registers alone cannot establish this. */
    for(uint32_t i=1;i<=7;++i)if(fx[4]&(1u<<((top+8-i)&7)))return 1;
    if(fx[4]&(1u<<top)){
        uint32_t exponent=half(fx+40)&0x7fff;
        if(exponent==0x7fff||(!exponent&&(word(fx+32)||word(fx+36)))||
           (exponent&&!(word(fx+36)&0x80000000u)))return 1;
    }
    /* Warm tables obey the established finite unit-coverage policy. Cold
     * tables use recovered x87 generation, including the upward-rounded
     * endpoint0x3f800001; it is not admitted as arbitrary warm coverage. */
    if(!b->state.cold)for(uint32_t i=0;i<64;++i)if(b->coverage[i]>0x3f800000u)return 1;
    b->result=result;
    mnm_glyph_fp_execute(b);
    return 0;
}
/* Called only by the environment-saving assembly wrapper after all validation.
 * Extents and source indices for hidden rows were checked by the state model. */
void mnm_glyph_backend_execute(MnmGlyphBackend *b){
    mnm_glyph_fp_begin(b->coverage,b->state.cold);
    if(b->result.branch!=MNM_GLYPH_ROWS)return;
    const uint8_t *frame=b->state.frame;
    int32_t x=b->state.x-(int32_t)word(frame+12),y=b->state.y-(int32_t)word(frame+16);
    uint32_t start=y<b->state.top?(uint32_t)(b->state.top-y):0;
    uint32_t end=start+b->result.visible_rows,width=word(frame+4);
    for(uint32_t row=start;row<end;++row){
        uint32_t control=word(frame+40+row*8),colour=word(frame+44+row*8),at=0,on=0;
        uint16_t *destination=b->pixels+(uint32_t)(y+(int32_t)row)*b->stride+(uint32_t)x;
        while(at<width){
            uint32_t count=frame[control++];
            if(on){mnm_glyph_fp_run(destination+at,frame+colour,count,b->coverage,b->result.arguments+1);colour+=count;}
            at+=count;on=!on;
        }
    }
}
