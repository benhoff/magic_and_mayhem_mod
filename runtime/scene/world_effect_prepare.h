#pragma once
#include <stdint.h>

/* The two retained non-drawing suppliers build their trigonometric tables
 * before checking the frame dimensions. Execute them with the engine's actual
 * incoming floating state, then publish their outgoing state to the shared
 * entry frame. Callback bookkeeping must not supply the initializer's FPU. */
static inline int mnm_world_effect_fx_ready(const uint8_t fx[512]){
    const uint32_t control=fx[0]|(uint32_t)fx[1]<<8,top=(fx[3]>>3)&7;
    if((control&63)!=63||((control>>8)&3)==1)return 0;
    for(unsigned i=1;i<=3;++i)if(fx[4]&(1u<<((top+8-i)&7)))return 0;
    return 1;
}

static inline void mnm_world_effect_prepare(uint32_t entry,int source_rows,uint8_t fx[512]){
    const uint32_t empty[11]={44,0,0};
    /* Original fastcall callees preserve EBX/ESI/EDI and pop their own formal
     * stack arguments. Keep the FXSAVE destination in a preserved register.
     * One assembly region prevents inserted callback work between restore,
     * retained supplier and save. No original raster receives a nonempty frame. */
    if(source_rows){
        __asm__ volatile("fxrstor (%%esi); pushl $0; pushl $0; pushl $0; movl %%ebx,%%ecx; xorl %%edx,%%edx; call *%%edi; fxsave (%%esi)"
            ::"S"(fx),"b"(empty),"D"(entry):"eax","ecx","edx","cc","memory","st","st(1)","st(2)");
    }else{
        __asm__ volatile("fxrstor (%%esi); pushl $1; pushl $0; movl %%ebx,%%ecx; xorl %%edx,%%edx; call *%%edi; fxsave (%%esi)"
            ::"S"(fx),"b"(empty),"D"(entry):"eax","ecx","edx","cc","memory","st","st(1)","st(2)");
    }
}
