#include "voice_lifetime.hpp"
namespace mnm::reconstruction::audio {
void resetWrapper(VoiceWrapper& v){
    v.buffer=0;v.sourceIndex=-1;v.field08=0;v.field0c=2;
    v.requestedVolume=99;v.cachedVolume=99;v.duplicate=nullptr;v.duration=0;
}
void destroyWrapper(LifetimeBackend& b,VoiceWrapper& v,std::uint32_t flags){
    if(v.duplicate){destroyWrapper(b,*v.duplicate,1);v.duplicate=nullptr;}
    if(v.buffer){b.releaseBuffer(v.buffer);resetWrapper(v);}
    if(flags&1)b.freeWrapper(v.identity);
}
void releaseSourceContents(LifetimeBackend& b,VoiceWrapper& v){
    if(v.duplicate){destroyWrapper(b,*v.duplicate,1);v.duplicate=nullptr;}
    if(v.buffer){b.releaseBuffer(v.buffer);resetWrapper(v);}
}
}
namespace mnm::reconstruction::audio {
Status retireVoice(LifetimeBackend& b,VoiceWrapper& v,bool children,bool clear,bool enabled){
    if(!enabled)return 0;
    if(v.buffer){
        std::uint32_t flags=0;
        if(b.getStatus(v.buffer,flags)!=0 || (flags&1)){
            auto result=b.stop(v.buffer);
            if(clear)b.clearVoiceSchedule(v.identity); // Even when Stop fails.
            if(result)return result;
            result=b.position(v.buffer,0);if(result)return result;
        }
    }
    if(children)for(auto* child=v.duplicate;child;child=child->duplicate){
        const auto result=retireVoice(b,*child,false,true,enabled);
        if(clear)b.clearVoiceSchedule(child->identity); // Original second notification.
        if(result)return result;
    }
    return 0;
}
void retireSchedule(ScheduleNode*& head,ScheduleNode& n){
    auto* tail=head->previous;
    if(&n==head)head=head->next;
    else if(&n!=tail){
        n.previous->next=n.next;n.next->previous=n.previous;
        n.previous=tail;n.next=head;tail->next=&n;head->previous=&n;
    }
    if(n.record.outputAddress && n.output)*n.output=0;
    clearSchedule(n.record);
}
}
