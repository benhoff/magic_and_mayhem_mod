#include "glyph_route.h"
MnmGlyphRouteContext *mnm_glyph_route_context;
int mnm_glyph_route(uint32_t *saved,uint8_t *fx){
    MnmGlyphRouteContext *context=mnm_glyph_route_context;
    if(!context)return 0;
    MnmGlyphBackend request;
    /* A large aggregate assignment imports memcpy on the PE32 target. Keep
     * the adapter freestanding without supplying a CRT helper. */
    for(uint32_t i=0;i<sizeof request;++i)((uint8_t *)&request)[i]=((const volatile uint8_t *)&context->backend)[i];
    uint8_t outgoing[512] __attribute__((aligned(16)));
    if((uint32_t)(uintptr_t)request.state.frame!=saved[6]){++context->forwarded;return 0;}
    request.state.frame_address=saved[6];
    request.state.entry_sp=(uint32_t)(uintptr_t)(saved+9);
    request.state.incoming_flags=saved[8];
    request.state.x=(int32_t)saved[5];request.state.y=(int32_t)saved[10];
    request.state.red=saved[11];request.state.green=saved[12];request.state.blue=saved[13];
    request.state.rounding=(fx[1]>>2)&3;
    request.incoming_fx=fx;request.outgoing_fx=outgoing;
    if(mnm_glyph_backend(&request)){++context->forwarded;return 0;}
    saved[7]=request.result.eax;saved[6]=request.result.ecx;saved[5]=request.result.edx;
    saved[8]=(saved[8]&~0xcd5u)|request.result.flags;
    for(uint32_t i=0;i<4;++i)saved[10+i]=request.result.arguments[i];
    for(uint32_t i=0;i<512;++i)fx[i]=outgoing[i];
    ++context->handled;
    return 1;
}
