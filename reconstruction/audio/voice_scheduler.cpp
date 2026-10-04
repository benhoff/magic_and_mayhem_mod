#include "voice_scheduler.hpp"
namespace mnm::reconstruction::audio {
namespace {
void detach(ScheduleNode& n){n.previous->next=n.next;n.next->previous=n.previous;}
void insertBefore(ScheduleNode& n,ScheduleNode& at){
    n.previous=at.previous;n.next=&at;at.previous->next=&n;at.previous=&n;
}
void clear(ScheduleNode& n){if(n.record.outputAddress && n.output)*n.output=0;clearSchedule(n.record);}
void stopSelected(SchedulerBackend& b,ScheduleNode& n,bool enabled){
    if(!n.record.voiceAddress)return;
    const bool selected=n.record.deadline==0xffffffffu || !retirementDue(b.tickCount(),n.record.deadline);
    if(selected && enabled)stopAndReset(b,b.bufferForVoice(n.record.voiceAddress),true);
}
}
ScheduleNode& evictSchedule(SchedulerBackend& b,ScheduleNode*& head,ScheduleNode& requested,bool enabled){
    auto* selected=head->previous;
    if(&requested==head)head=selected;
    else if(&requested!=selected){detach(*selected);insertBefore(*selected,requested);}
    stopSelected(b,*selected,enabled);
    // Original skips output-slot clearing if voiceAddress is already null.
    if(selected->record.voiceAddress)clear(*selected);else clearSchedule(selected->record);
    return *selected;
}
ScheduleNode* selectSchedule(SchedulerBackend& b,ScheduleNode*& head,std::int32_t volume,bool enabled){
    auto* cursor=head;
    for(;;){
        if(!cursor->record.voiceAddress)return cursor;
        if(cursor->record.deadline!=0xffffffffu && retirementDue(b.tickCount(),cursor->record.deadline)){
            const bool tail=cursor==head->previous;auto* next=cursor->next;
            retireSchedule(head,*cursor);
            if(tail)return cursor;
            cursor=next;continue;
        }
        if(cursor->record.volume<volume)return &evictSchedule(b,head,*cursor,enabled);
        cursor=cursor->next;if(cursor==head)return nullptr;
    }
}
void assignSchedule(SchedulerBackend& b,ScheduleNode& n,const VoiceWrapper& v,std::uint32_t address,
                    std::uint32_t* output,bool loop,std::int32_t x,std::int32_t y){
    n.record.outputAddress=address;n.output=output;n.record.voiceAddress=v.identity;
    n.record.deadline=loop?0xffffffffu:b.tickCount()+v.duration;
    n.record.volume=v.cachedVolume;n.record.x=x;n.record.y=y;
}
void updateScheduleVolume(ScheduleNode*& head,ScheduleNode& n,std::int32_t volume){
    const auto old=n.record.volume;
    if(old==volume)return;
    if(old>volume){
        auto* at=n.next;
        while(at!=head && at->record.volume>old)at=at->next;
        if(at==head){
            if(head->previous!=&n){
                if(head==&n)head=n.next;
                else{detach(n);insertBefore(n,*head);}
            }
        }else if(at->previous!=&n){
            if(head==&n)head=n.next;
            detach(n);insertBefore(n,*at);
        }
    }else{
        auto* tail=head->previous;auto* at=n.previous;
        while(at!=tail && at->record.volume<old)at=at->previous;
        if(at==tail){
            if(head!=&n){if(tail!=&n){detach(n);insertBefore(n,*head);}head=&n;}
        }else if(at->next!=&n){auto* next=at->next;detach(n);insertBefore(n,*next);}
    }
    n.record.volume=volume;
}
void clearSchedules(SchedulerBackend& b,ScheduleNode* head,bool enabled){
    auto* n=head;
    do{stopSelected(b,*n,enabled);if(n->record.voiceAddress)clear(*n);else clearSchedule(n->record);n=n->next;}while(n!=head);
}
}
