#include "voice_admission.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
Status start(AdmissionBackend& b,VoiceWrapper& v,int volume,int pan,bool loop){
    if(v.cachedVolume!=volume){
        v.requestedVolume=volume;v.cachedVolume=volume;
        const auto result=b.volume(v.buffer,volume);if(result)return result;
    }
    auto result=b.pan(v.buffer,pan);if(result)return result;
    return b.play(v.buffer,0,0,loop?1:0);
}
void promote(VoiceWrapper*& head,VoiceWrapper& v){
    if(head==&v)return;
    if(head->sourceIndex!=v.sourceIndex){
        v.previous->next=v.next;v.next->previous=v.previous;
        v.previous=head->previous;v.next=head;head->previous->next=&v;head->previous=&v;
    }
    head=&v;
}
void rotateDuplicates(VoiceWrapper*& source){
    auto* old=source;auto* first=old->duplicate;auto* tail=first;
    while(tail->duplicate)tail=tail->duplicate;
    tail->duplicate=old;old->duplicate=nullptr;
    first->previous=old->previous;first->next=old->next;
    first->previous->next=first;first->next->previous=first;
    source=first;
}
}
std::int32_t admissionSoundId(AdmissionBackend& b,const AdmissionCatalog& c,std::int32_t requested){
    const auto group=std::lower_bound(c.groups.begin(),c.groups.end(),requested,
        [](const SoundGroup& g,int id){return g.id<id;});
    if(group!=c.groups.end() && group->id==requested){
        if(group->members.empty())throw std::invalid_argument("Original group divisor must be nonzero");
        requested=group->members[(b.randomWord()&0x7fffffffu)%group->members.size()];
    }
    const auto source=std::lower_bound(c.sourceIds.begin(),c.sourceIds.end(),requested);
    if(source==c.sourceIds.end() || *source!=requested)return 0x19a;
    return requested;
}
Status duplicateAndStart(AdmissionBackend& b,VoiceWrapper& source,int volume,int pan,bool loop,VoiceWrapper*& output){
    std::uint32_t buffer=0;auto result=b.duplicateBuffer(source.buffer,buffer);
    if(result)return result;
    auto& child=b.allocateWrapper();child.buffer=buffer;
    result=start(b,child,volume,pan,loop);
    child.sourceIndex=source.sourceIndex;child.field08=source.field08;
    child.field0c=source.field0c;child.duration=source.duration;
    auto* tail=&source;while(tail->duplicate)tail=tail->duplicate;
    tail->duplicate=&child;source.requestedVolume=volume;output=&child;
    return result;
}
Status admitVoice(AdmissionBackend& b,const ManagerState& manager,const AdmissionCatalog& catalog,
                 VoiceWrapper*& head,ScheduleNode*& schedules,std::uint32_t disabled,const AdmissionRequest& request){
    if(!manager.initialized || !manager.active){if(request.output)*request.output=disabled;return 0;}
    auto* slot=selectSchedule(b,schedules,request.volume,true);
    if(!slot)return 0;
    const auto index=admissionSoundId(b,catalog,request.sound);
    auto* source=head;bool found=false;
    do{if(source->sourceIndex==index){found=true;break;}source=source->next;}while(source!=head);
    bool usable=false;
    if(found){
        std::uint32_t flags=0;
        if(!b.getStatus(source->buffer,flags)){
            if(flags&2)source->field0c=2;else usable=true;
        }
    }
    if(!usable){const auto result=b.loadSource(index,source);if(result)return result;}
    std::uint32_t flags=0;
    const bool duplicate=source->buffer && (b.getStatus(source->buffer,flags)!=0 || (flags&1));
    Status result=0;
    if(duplicate){
        VoiceWrapper* child=nullptr;
        result=duplicateAndStart(b,*source,request.volume,request.pan,request.looping,child);
        if(result)return result;
        assignSchedule(b,*slot,*child,request.outputAddress,request.output,request.looping,request.x,request.y);
        if(request.output)*request.output=child->identity;
    }else{
        result=start(b,*source,request.volume,request.pan,request.looping);
        if(!result){
            assignSchedule(b,*slot,*source,request.outputAddress,request.output,request.looping,request.x,request.y);
            auto* played=source;
            if(source->duplicate){rotateDuplicates(source);source->requestedVolume=request.volume;}
            if(request.output)*request.output=played->identity;
        }
    }
    // Direct-start failure still promotes; duplicate failure returns above.
    promote(head,*source);
    return result;
}
}
