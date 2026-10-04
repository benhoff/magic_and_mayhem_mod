#include "voice_contract.hpp"
#include <limits>
#include <stdexcept>

namespace mnm::reconstruction::audio {
std::uint32_t clearSchedule(Schedule32& record){
    const auto output=record.outputAddress;
    record.deadline=0;record.volume=-5000;record.voiceAddress=0;record.outputAddress=0;
    record.x=-1;record.y=-1;return output;
}
bool voiceBusy(VoiceBackend& backend,const VoiceContract& voice,bool children){
    if(!voice.buffer)return false;
    std::uint32_t flags=0;
    if(backend.getStatus(voice.buffer,flags)!=0 || (flags&1))return true;
    if(children)for(auto* child=voice.duplicate;child;child=child->duplicate)
        if(voiceBusy(backend,*child,false))return true;
    return false;
}
Status stopAndReset(VoiceBackend& backend,std::uint32_t buffer,bool enabled){
    if(!enabled || !buffer)return 0;
    std::uint32_t flags=0;
    if(backend.getStatus(buffer,flags)==0 && !(flags&1))return 0;
    const auto result=backend.stop(buffer);
    return result?result:backend.position(buffer,0);
}
Status setVoiceVolume(VoiceBackend& backend,VoiceContract& voice,std::int32_t value,
                     bool children,bool notify,bool enabled){
    if(voice.cachedVolume==value)return 0;
    voice.requestedVolume=value;voice.cachedVolume=value;
    if(!enabled)return 0;
    const auto result=backend.volume(voice.buffer,value);
    if(result)return result;
    if(notify)backend.volumeRecord(voice,value);
    if(children)for(auto* child=voice.duplicate;child;child=child->duplicate){
        const auto childResult=setVoiceVolume(backend,*child,value,false,true,enabled);
        if(childResult)return childResult;
        // Original calls the record helper AGAIN when the outer notify flag is set.
        if(notify)backend.volumeRecord(*child,value);
    }
    return 0;
}
Status setVoicePan(VoiceBackend& backend,VoiceContract& voice,std::int32_t value,
                  bool children,bool enabled){
    if(!enabled)return 0;
    const auto result=backend.pan(voice.buffer,value);
    if(result)return result;
    if(children)for(auto* child=voice.duplicate;child;child=child->duplicate){
        const auto childResult=setVoicePan(backend,*child,value,false,enabled);
        if(childResult)return childResult;
    }
    return 0;
}
Status startVoice(VoiceBackend& backend,VoiceContract& voice,std::int32_t volume,
                  std::int32_t pan,bool looping,bool enabled){
    if(!enabled)return 0; // Outer manager gate precedes the entire selected block.
    auto result=setVoiceVolume(backend,voice,volume,false,false,enabled);
    if(result)return result;
    result=setVoicePan(backend,voice,pan,false,enabled);
    if(result)return result;
    return backend.play(voice.buffer,0,0,looping?1:0);
}
std::uint32_t retirementDeadline(std::uint32_t now,std::uint32_t duration,bool looping){
    return looping?0xffffffffu:now+duration;
}
bool retirementDue(std::uint32_t now,std::uint32_t deadline){
    return deadline!=0xffffffffu && now>=deadline;
}
std::uint32_t wavDuration(std::uint32_t bytes,std::uint32_t rate){
    if(!rate)throw std::invalid_argument("WAV byte rate must be nonzero");
    return (bytes*std::uint32_t(1000))/rate;
}
std::int32_t positionalPan(std::int32_t lateral,std::int32_t width){
    if(width<=0)throw std::invalid_argument("Pan width must be positive");
    const auto distance=lateral<0?-std::int64_t(lateral):std::int64_t(lateral);
    if(distance>=width)return lateral>0?3333:-3333;
    const auto product=std::int64_t(lateral)*3333;
    if(product<std::numeric_limits<std::int32_t>::min() || product>std::numeric_limits<std::int32_t>::max())
        throw std::out_of_range("Outside inspected signed pan arithmetic domain");
    return std::int32_t(product/width);
}
}
