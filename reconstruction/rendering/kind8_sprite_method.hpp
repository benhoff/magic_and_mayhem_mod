#pragma once
#include "world_dispatch.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace mnm::reconstruction {
enum class Kind8Sprite { flies, cure };
inline Kind8Sprite kind8SpriteClass(std::uint32_t table,std::uint32_t method){
    if(table==0x5c74e4 && method==0x5492b0)return Kind8Sprite::flies;
    if(table==0x5c74fc && method==0x549730)return Kind8Sprite::cure;
    throw std::invalid_argument("Unrecovered kind8 sprite class/method");
}
struct Kind8SpriteFrame {std::uint32_t token{};bool words{};};
struct Kind8SpriteBank {
    bool present{};
    std::array<std::uint32_t,8> players{};
    std::vector<Kind8SpriteFrame> frames;
};
// Owned source tokens and player values only. Construction, source lifetime,
// RNG implementation and actual raster admission remain caller contracts.
// Positive bank sizes are a bounded native policy, not a recovered size limit.
template<class Random,class Raster>
std::uint16_t drawKind8Sprite(Kind8Sprite type,const Kind8SpriteBank& bank,
        std::int32_t player,std::int32_t x,std::int32_t y,Random&& random,Raster&& raster){
    if(type!=Kind8Sprite::flies && type!=Kind8Sprite::cure)
        throw std::invalid_argument("Unknown kind8 sprite method");
    if(!bank.present || player<0 || player>=8 || !bank.players[unsigned(player)] || bank.frames.empty())return 0;
    if(bank.frames.size()>4096)throw std::invalid_argument("Kind8 sprite bank exceeds native policy");
    if(type==Kind8Sprite::cure && (std::uint32_t(random())&0x7fffffffu)%100<=50)return 0;
    const auto index=(std::uint32_t(random())&0x7fffffffu)%bank.frames.size();
    const auto& frame=bank.frames[index];
    auto call=[&](QueueBackend backend,std::initializer_list<std::uint32_t> values){
        QueueCall request{backend,{},unsigned(values.size())};unsigned i=0;
        for(auto v:values)request.values[i++]=v;
        return std::uint16_t(raster(request));
    };
    if(frame.words)call(QueueBackend::wordCopy,{frame.token,std::uint32_t(x),std::uint32_t(y)});
    else if(call(QueueBackend::copy,{frame.token,std::uint32_t(x),std::uint32_t(y),0,0})==1)
        call(QueueBackend::clippedCopy,{frame.token,std::uint32_t(x),std::uint32_t(y),0,0});
    return 0; // Both original wrappers clear AX after every path.
}
}
