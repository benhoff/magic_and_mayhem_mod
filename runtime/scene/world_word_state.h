#pragma once
#include "../../reconstruction/rendering/word_backend_state.h"

/* Whole-entry World bypass state, separate from the queued pixel command.
 * Preparation is atomic: no original workspace, arguments or saved processor
 * state changes until the complete request has been admitted. */
typedef struct {
    MnmWordBackendState backend;
    uint32_t positive;
} MnmWorldWordState;

static inline int mnm_world_word_prepare(const MnmWordBackendInput* input,
                                        const uint32_t before[16],
                                        const uint8_t fx[512],
                                        MnmWorldWordState* output) {
    if(!input || !before || !fx || !output || !input->frame || input->frame_bytes<40)return 1;
    const uint8_t* frame=input->frame;
    uint32_t width=0,height=0;
    for(unsigned i=0;i<4;++i){width|=(uint32_t)frame[4+i]<<(i*8);height|=(uint32_t)frame[8+i]<<(i*8);}
    const uint32_t positive=width&&height;
    if(positive){
        /* Original positive preparation pushes one exact bounded integer.
         * Unsupported exception controls or an occupied push slot require a
         * separate provider contract, rather than suppressing its exception. */
        const uint32_t control=fx[0]|(uint32_t)fx[1]<<8;
        const uint32_t top=(fx[3]>>3)&7;
        if((control&63)!=63 || ((control>>8)&3)==1 || (fx[4]&(1u<<((top+7)&7))))return 1;
    }
    MnmWorldWordState state;
    if(mnm_word_backend_state(input,before,&state.backend))return 1;
    state.positive=positive;
    for(unsigned i=0;i<16;++i)output->backend.words[i]=state.backend.words[i];
    output->backend.argument_x=state.backend.argument_x;
    output->backend.argument_y=state.backend.argument_y;
    output->backend.return_eax=state.backend.return_eax;
    output->positive=state.positive;
    return 0;
}

static inline void mnm_world_word_commit(const MnmWorldWordState* state,
                                       uint32_t workspace[16],uint32_t saved[13],
                                       uint8_t fx[512]) {
    for(unsigned i=0;i<16;++i)workspace[i]=state->backend.words[i];
    saved[11]=(uint32_t)state->backend.argument_x;
    saved[12]=(uint32_t)state->backend.argument_y;
    saved[7]=state->backend.return_eax;
    /* Original final XOR EAX,EAX: AF is undefined; choose its native zero. */
    saved[8]=(saved[8]&~0x8d5u)|0x44u;
    if(state->positive)fx[3]&=(uint8_t)~2u; /* x87 C1, bit9 of status. */
}
