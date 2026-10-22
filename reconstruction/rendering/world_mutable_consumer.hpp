#pragma once
#include "world_consumer.hpp"
#include <memory>
#include <stdexcept>

namespace mnm::reconstruction {
// Rebinding the envelope does not change the cached entry backing. Its owner
// must retain that generation until traversal returns. Count is reread after
// every row; capacity is the extent of that retained allocation, not count.
struct MutableWorldQueue {
    std::shared_ptr<std::vector<ConsumerRow>> backing;
    std::uint32_t count{};
};
template<class Backend,class Clip,class Finalize>
void consumeMutableWorldRows(MutableWorldQueue& queue,ConsumerState& state,
        Backend&& backend,Clip&& clip,Finalize&& finalize){
    const auto backing=queue.backing;
    if(!backing||queue.count>backing->size())throw std::invalid_argument("World entry backing extent");
    const auto hardware=state.hardware; // Selected once before either loop.
    const auto signedDword=[](std::uint32_t value){return value<=0x7fffffffu?std::int32_t(value):-1-std::int32_t(0xffffffffu-value);};
    const auto signedWord=[](std::uint16_t value){return value<0x8000?std::int32_t(value):std::int32_t(value)-0x10000;};
    auto right=[&]{return std::uint32_t(state.base.right)+2*state.padding;};
    auto setClip=[&](std::uint32_t l,std::uint32_t t,std::uint32_t r,std::uint32_t b){
        state.active={signedDword(l),signedDword(t),signedDword(r),signedDword(b)};clip(state.active);
    };
    auto draw=[&](ConsumerRow& row,QueueTable table){
        // Argument registers/stack values are cached before external calls.
        // Kind9 zeroes metadata before entering its concrete helper.
        const auto arguments=row.draw;
        if(arguments.kind==9&&table!=QueueTable::auxiliary)row.metadata=0;
        const auto result=dispatchQueueRow(table,arguments,backend);
        if(result&&arguments.kind!=9)row.metadata=*result;
    };
    constexpr std::uint32_t unset=0x88ca6c00u;
    for(std::uint32_t index=0;index<queue.count;++index){
        if(index>=backing->size())throw std::invalid_argument("World callback grew beyond retained backing");
        auto& row=backing->at(index);
        if(!hardware){draw(row,QueueTable::software);continue;}
        auto lower=unset,upper=unset;
        if(row.auxiliaryMarker!=0x8ad0){
            lower=row.draw.y+row.sourceHeight-row.sourceOriginY;
            const auto edge=signedDword(lower+std::uint32_t(signedWord(row.auxiliaryMarker)));
            if(edge<state.base.bottom&&edge>state.base.top)
                setClip(std::uint32_t(state.base.left)+state.padding,state.base.top,right(),std::uint32_t(std::max(edge,0)));
        }
        if(row.metadata!=0x8ad0){
            upper=row.draw.y+row.sourceHeight-row.sourceOriginY;
            row.draw.y+=row.sourceHeight-std::uint32_t(signedWord(row.metadata));
            const auto edge=signedDword(upper);
            if(edge<state.base.bottom&&edge>state.base.top)
                setClip(std::uint32_t(state.base.left)+state.padding,state.base.top,right(),upper);
        }
        draw(row,QueueTable::primary);
        // Auxiliary marker, video size, base clip and padding reload here.
        if(lower!=unset&&row.auxiliaryMarker!=0x8ad0){
            const auto edge=signedDword(lower+std::uint32_t(signedWord(row.auxiliaryMarker)));
            if(edge<(state.tallVideo?600:480)&&edge<state.base.bottom){
                setClip(state.base.left,std::uint32_t(std::max(edge,0)),right(),state.base.bottom);
                draw(row,QueueTable::auxiliary);
            }
        }
        if(lower!=unset||upper!=unset)setClip(state.base.left,state.base.top,right(),state.base.bottom);
    }
    if(state.finalize){const auto arguments=state.finalArguments;finalize(arguments);state.finalArguments={};}
}
}
