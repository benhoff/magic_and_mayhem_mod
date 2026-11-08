/* Exercise actual shared production entry assembly and the state transaction.
 * The provider adapter supplies an owned CPU plane; native pixels are computed
 * independently of the original result stored in the corpus. */
#define _word_enter_scalar _producer_enter_22
#define _word_enter_forward _producer_enter_36
#define MNM_WORD_STATE_PRECISION_TEST
#define main world_word_reference_main
#include "word-backend-state-reference.cpp"
#undef main
#include "../runtime/scene/world_word_state.h"
#include "../renderer/sprites/word_clipped.h"

extern "C" {
std::uint32_t _producer_trampolines[61];
static unsigned admissions;
void _producer_leave_observe(std::uint32_t*) { throw std::runtime_error("Unexpected return observer"); }
int _producer_enter_observe(std::uint32_t* saved,std::uint32_t tag,std::uint8_t* fx) {
    if(tag!=22&&tag!=36)throw std::runtime_error("Unexpected World word tag");
    auto* arguments=saved+10;
    auto* frame=reinterpret_cast<const std::uint8_t*>(arguments[0]);
    std::uint32_t size;std::memcpy(&size,frame,4);
    const auto backend=tag==22?0x197086u:0x196cb8u;
    MnmWordBackendInput input{frame,size,arguments[0],*reinterpret_cast<std::uint32_t*>(0x658174),
        *reinterpret_cast<std::uint32_t*>(0x6a49b8),*reinterpret_cast<std::uint32_t*>(0x656618),
        *reinterpret_cast<std::uint32_t*>(0x6a2dc8),backend,
        *reinterpret_cast<std::int32_t*>(0x6e0008),*reinterpret_cast<std::int32_t*>(0x6cbb6c),
        std::int32_t(arguments[1]),std::int32_t(arguments[2])};
    MnmWorldWordState state{};
    auto* workspace=reinterpret_cast<std::uint32_t*>(0x5f1e50);
    if(mnm_world_word_prepare(&input,workspace,fx,&state))throw std::runtime_error("State preflight refused corpus");
    MnmWordCanvas canvas{reinterpret_cast<std::uint8_t*>(input.canvas_address),
        std::size_t(input.stride_words)*input.bottom*2,input.right,input.bottom,input.stride_words};
    MnmWordClip clip{input.clip_left,input.clip_top,std::int32_t(input.right),std::int32_t(input.bottom)};
    MnmWordClippedDraw draw{};
    if(mnm_word_sprite_clip_draw(frame,size,&canvas,&clip,input.anchor_x,input.anchor_y,&draw))
        throw std::runtime_error("Native pixel admission refused corpus");
#ifdef MNM_WORLD_WORD_STATE_BASELINE
    saved[7]=0; /* Negative control: the former pixel-only publication policy. */
#else
    mnm_world_word_commit(&state,workspace,saved,fx);
#endif
    ++admissions;
    return 1;
}
}

int main(int argc,char** argv) {
    const int result=world_word_reference_main(argc,argv);
    if(!result&&!admissions)throw std::runtime_error("No production bypass admission");
    if(!result)std::cout<<admissions<<" actual shared World word entry admissions\n";
    return result;
}
