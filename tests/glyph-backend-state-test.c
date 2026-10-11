#include "../reconstruction/rendering/glyph_backend_state.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void put(unsigned char *p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(v>>(8*i));}
int main(void){
    unsigned char f[64]={0};put(f,64);put(f+4,2);put(f+8,2);put(f+40,56);put(f+44,60);put(f+48,58);put(f+52,62);
    f[56]=0;f[57]=2;f[58]=0;f[59]=2;f[60]=0;f[61]=63;f[62]=1;f[63]=2;
    MnmGlyphStateInput in={f,sizeof(f),0x100000,0x6f5dac,0x800004,0x600,0,0,4,4,1,1,173,91,237,0,0};
    MnmGlyphState out;assert(!mnm_glyph_backend_state(&in,&out));assert(out.eax==0&&out.ecx==11&&out.edx==0x100038&&out.arguments[0]==0&&out.arguments[1]==21&&out.arguments[2]==22&&out.arguments[3]==29&&out.visible_rows==2&&out.opaque_pixels==4);
    unsigned char aux[104]={0};memcpy(aux,f,sizeof(f));put(aux,sizeof(aux));put(aux+32,72);put(aux+36,88);
    MnmGlyphStateInput auxiliary=in;auxiliary.frame=aux;auxiliary.frame_bytes=sizeof(aux);
    assert(!mnm_glyph_backend_state(&auxiliary,&out)&&out.opaque_pixels==4);
    unsigned refusals=0;
    for(unsigned test=0;test<15;++test){
        MnmGlyphStateInput bad=in;unsigned char copy[64];memcpy(copy,f,64);bad.frame=copy;
        switch(test){
        case 0:copy[63]=64;bad.y=-100;break; // Invalid hidden row is still refused.
        case 1:put(copy+48,64);break;
        case 2:copy[59]=3;break;
        case 3:copy[59]=0;break;
        case 4:put(copy,63);break;
        case 5:put(copy+28,0xffffffffu);break;
        case 6:bad.red=256;break;
        case 7:bad.cold=2;break;
        case 8:bad.rounding=4;break;
        case 9:bad.right=bad.left;break;
        case 10:bad.top=-1;break;
        case 11:bad.x=4097;break;
        case 12:bad.frame_address=0xfffffff0u;break;
        case 13:bad.coverage_address=0xffffff00u;break;
        case 14:bad.entry_sp=0;break;
        }
        memset(&out,0xa5,sizeof(out));MnmGlyphState before=out;
        assert(mnm_glyph_backend_state(&bad,&out));assert(!memcmp(&out,&before,sizeof(out)));++refusals;
    }
    uint32_t table[64];assert(!mnm_glyph_coverage_bits(0,table)&&table[0]==0&&table[1]==0x3c820821&&table[63]==0x3f800000);
    assert(!mnm_glyph_coverage_bits(2,table)&&table[63]==0x3f800001);
    uint32_t old[64];memcpy(old,table,sizeof(table));assert(mnm_glyph_coverage_bits(4,table)&&!memcmp(old,table,sizeof(table)));++refusals;
    printf("16 atomic state/table refusals; selected integer state and directed coverage rounding pass\n");return refusals==16?0:1;
}
