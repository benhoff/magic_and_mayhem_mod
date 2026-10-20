#include "attachment.hpp"
#include <limits>
#include <stdexcept>
#include <utility>
namespace mnm::reconstruction {
ModeOneSelection modeOneSelection(std::uint32_t base,std::uint32_t assetIndex,std::uint32_t facing){
    if(facing>7 || base>std::numeric_limits<std::uint32_t>::max()-facing)
        throw std::runtime_error("Mode-one attachment sequence outside native bounds");
    return {assetIndex,base+facing};
}
bool modeOneVisible(std::int32_t health,std::uint32_t mode){return health!=0 && mode==1;}
bool modeOneTicks(std::uint32_t mode){return mode==1;}
bool ModeOneAttachment::change(std::uint32_t requested,std::uint32_t base,
        std::uint32_t asset,std::uint32_t facing,const ModeOneSequenceSource& source){
    if(requested==mode_)return false;
    if(requested>1)throw std::runtime_error("Unsupported attachment mode transition");
    if(requested==0){
        if(player_)player_->stop();
        mode_=0;return true;
    }
    const auto selected=modeOneSelection(base,asset,facing);
    if(!source)throw std::runtime_error("Attachment requires an owned animation sequence");
    NoCdAnimationPlayer next(source(selected.assetIndex,selected.sequence));
    next.start();
    player_=std::move(next);selection_=selected;mode_=1;return true;
}
std::int32_t ModeOneAttachment::tick(){return modeOneTicks(mode_)&&player_?player_->tick():0;}
}
