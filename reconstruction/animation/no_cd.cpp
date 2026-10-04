#include "no_cd.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
NoCdAnimationPlayer::NoCdAnimationPlayer(std::vector<assets::AnimationRecord> sequence):records_(std::move(sequence)){
    if(records_.empty() || records_.size()>65536 || records_.back().opcode!=6)throw std::runtime_error("Invalid bounded animation sequence");
}
void NoCdAnimationPlayer::start(){
    state_={};state_.active=true;
    if(records_[0].opcode==0)state_.displayedRecord=0;
    else dispatch(true);
}
void NoCdAnimationPlayer::switchSequence(std::vector<assets::AnimationRecord> sequence){
    if(!state_.active || !state_.displayedRecord || sequence.empty() || sequence.size()>65536 ||
       sequence.back().opcode!=6 || state_.pc>=sequence.size() || *state_.displayedRecord>=sequence.size() ||
       sequence[*state_.displayedRecord].opcode!=0 || sequence[*state_.displayedRecord].argument<0)
        throw std::runtime_error("Animation switch has no compatible active record positions");
    records_=std::move(sequence);
}
std::int32_t NoCdAnimationPlayer::dispatch(bool initial){
    std::int32_t event=0;
    for(unsigned budget=0;budget<65536;++budget){
        if(state_.pc>=records_.size())throw std::runtime_error("Animation control escaped selected sequence");
        const auto at=state_.pc;const auto& r=records_[at];const auto word=static_cast<std::uint32_t>(r.argument);
        bool stop=false,jump=false;
        switch(r.opcode){
        case 0:state_.displayedRecord=at;stop=true;break;
        case 1:state_.delay=word;break;
        case 2:state_.repeats=word;break;
        case 3:if(state_.repeats){--state_.repeats;jump=true;}break;
        case 4:if(!state_.breakFlag)jump=true;else state_.breakFlag=0;break;
        case 5:event=r.argument;stop=!initial;break;
        case 6:state_.active=false;state_.displayedRecord.reset();event=1;stop=true;break;
        default:break; // Original dispatch skips unknown opcodes.
        }
        if(jump){const auto target=std::int64_t(at)+r.argument;
            if(target<0 || std::uint64_t(target)>=records_.size())throw std::runtime_error("Animation relative jump outside selected sequence");
            state_.pc=static_cast<std::size_t>(target);
        }else ++state_.pc;
        if(stop)return event;
    }
    throw std::runtime_error("Animation dispatch exceeds instruction budget");
}
std::int32_t NoCdAnimationPlayer::tick(){
    if(!state_.active)return 0;
    if(state_.elapsed<state_.delay){++state_.elapsed;return 0;}
    const auto event=dispatch(false);state_.elapsed=0;return event;
}
std::optional<std::uint32_t> NoCdAnimationPlayer::sprite() const{
    if(!state_.displayedRecord)return {};
    const auto arg=records_[*state_.displayedRecord].argument;
    if(arg<0)throw std::runtime_error("Negative sprite index in animation");
    return static_cast<std::uint32_t>(arg);
}
std::optional<assets::AnimationRecord> NoCdAnimationPlayer::displayedRecord() const{
    if(!state_.displayedRecord)return {};
    return records_[*state_.displayedRecord];
}
}
