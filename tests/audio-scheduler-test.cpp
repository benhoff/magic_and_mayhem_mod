#include "voice_scheduler.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
struct Backend:r::SchedulerBackend {
    unsigned clock=10,ticks=0;int query=0,stopFault=0,resetFault=0;bool playing=true;
    std::string calls;
    unsigned tickCount()override{++ticks;return clock;}
    unsigned bufferForVoice(unsigned id)override{calls+='B';return id+10;}
    r::Status getStatus(unsigned,unsigned& f)override{calls+='Q';f=playing?1:0;return query;}
    r::Status stop(unsigned)override{calls+='S';return stopFault;}
    r::Status position(unsigned,unsigned p)override{check(!p,"zero position");calls+='Z';return resetFault;}
    r::Status volume(unsigned,int)override{return 0;}
    r::Status pan(unsigned,int)override{return 0;}
    r::Status play(unsigned,unsigned,unsigned,unsigned)override{return 0;}
    void volumeRecord(r::VoiceContract&,int)override{}
};
struct Pool {
    std::vector<r::ScheduleNode> nodes;std::vector<unsigned> slots;r::ScheduleNode* head;
    explicit Pool(unsigned size):nodes(size),slots(size,99),head(&nodes[0]){
        for(unsigned i=0;i<size;++i){nodes[i].next=&nodes[(i+1)%size];nodes[i].previous=&nodes[(i+size-1)%size];nodes[i].output=&slots[i];nodes[i].record={0,0,0xffffffffu,0,i+1,1,5,6};}
    }
    std::vector<unsigned> order()const{
        std::vector<unsigned> result;auto* p=head;
        do{check(p->next->previous==p && p->previous->next==p,"bidirectional topology");result.push_back(unsigned(p-nodes.data()));p=p->next;check(result.size()<=nodes.size(),"bounded ring");}while(p!=head);
        check(result.size()==nodes.size(),"all records retained");return result;
    }
};
void selectionOracle(){
    for(unsigned states=0;states<64;++states)for(unsigned scores=0;scores<27;++scores)for(int request:{-2,0,2}){
        Pool p(3);Backend b;unsigned stateCode=states,scoreCode=scores;std::vector<unsigned> state(3),expectedSlots(3,99),order={0,1,2};std::vector<int> score(3);unsigned expectedTicks=0;std::string calls;int chosen=-1;
        for(unsigned i=0;i<3;++i){state[i]=stateCode%4;stateCode/=4;score[i]=int(scoreCode%3)-1;scoreCode/=3;
            p.nodes[i].record.voiceAddress=state[i]?i+1:0;p.nodes[i].record.deadline=state[i]==1?10:state[i]==2?11:0xffffffffu;p.nodes[i].record.volume=score[i];}
        unsigned index=0;
        for(;;){const auto id=order[index];if(!state[id]){chosen=int(id);break;}
            if(state[id]==1 || state[id]==2)++expectedTicks;
            if(state[id]==1){expectedSlots[id]=0;state[id]=0;const bool tail=index==order.size()-1;
                order.erase(order.begin()+index);order.push_back(id);if(tail){chosen=int(id);break;}continue;}
            if(score[id]<request){const auto victim=order.back();
                if(index==0)std::rotate(order.begin(),order.end()-1,order.end());
                else if(index!=order.size()-1){order.pop_back();order.insert(order.begin()+index,victim);}
                if(state[victim]==1 || state[victim]==2)++expectedTicks;
                if(state[victim] && state[victim]!=1)calls="BQSZ";
                if(state[victim])expectedSlots[victim]=0;
                state[victim]=0;chosen=int(victim);break;}
            if(++index==order.size())break;
        }
        auto* selected=r::selectSchedule(b,p.head,request,true);
        check((selected?int(selected-p.nodes.data()):-1)==chosen,"independent vector selection oracle");
        check(p.order()==order && p.slots==expectedSlots && b.ticks==expectedTicks && b.calls==calls,"selection topology, slots, clocks and calls");
        for(unsigned i=0;i<3;++i)check(bool(p.nodes[i].record.voiceAddress)==bool(state[i]),"cleared candidate payload");
    }
}
void evictionFailures(){
    for(bool enabled:{false,true})for(bool present:{false,true})for(bool playing:{false,true})for(int query:{0,9})for(int stop:{0,7})for(int reset:{0,5}){
        Pool p(3);Backend b;b.playing=playing;b.query=query;b.stopFault=stop;b.resetFault=reset;
        if(!present)p.nodes[2].record.voiceAddress=0;
        auto& result=r::evictSchedule(b,p.head,p.nodes[1],enabled);
        std::string expected;if(enabled && present){expected="BQ";if(query || playing){expected+='S';if(!stop)expected+='Z';}}
        check(&result==&p.nodes[2] && p.order()==std::vector<unsigned>({0,2,1}),"eviction returns former tail and inserts before candidate");
        check(b.calls==expected && !result.record.voiceAddress && p.slots[2]==(present?0u:99u),"failed stop/reset still clears, absent voice retains slot");
    }
    Pool p(2);Backend b;p.nodes[0].record.volume=0;p.nodes[1].record.volume=0;
    check(!r::selectSchedule(b,p.head,0,true) && b.calls.empty(),"equal scores do not evict");
}
void reprioritization(){
    // Independent vector/register oracle for the old-score scans in 0x571d80.
    for(unsigned code=0;code<81;++code)for(unsigned target=0;target<4;++target)for(int value:{-2,0,2}){
        Pool p(4);unsigned digits=code;std::vector<int> score(4);std::vector<unsigned> order={0,1,2,3};
        for(unsigned i=0;i<4;++i){score[i]=int(digits%3)-1;digits/=3;p.nodes[i].record.volume=score[i];}
        const auto old=score[target];
        if(old>value){unsigned at=(target+1)%4;while(at!=0 && score[at]>old)at=(at+1)%4;
            if(at==0){if(target!=3){order.erase(order.begin()+target);order.push_back(target);}}
            else if(at-1!=target){order.erase(order.begin()+target);auto pos=std::find(order.begin(),order.end(),at);order.insert(pos,target);}
        }else if(old<value){unsigned at=(target+3)%4;while(at!=3 && score[at]<old)at=(at+3)%4;
            if(at==3){if(target!=0){order.erase(order.begin()+target);order.insert(order.begin(),target);}}
            else if(at+1!=target){order.erase(order.begin()+target);auto pos=std::find(order.begin(),order.end(),at);order.insert(pos+1,target);}
        }
        r::updateScheduleVolume(p.head,p.nodes[target],value);
        check(p.order()==order && p.nodes[target].record.volume==value,"old-volume ordering oracle");
        for(unsigned i=0;i<4;++i)if(i!=target)check(p.nodes[i].record.volume==score[i],"unrelated volume retained");
    }
    Pool p(3);p.nodes[0].record.volume=0;p.nodes[1].record.volume=10;p.nodes[2].record.volume=20;
    r::updateScheduleVolume(p.head,p.nodes[1],-100);
    check(p.order()==std::vector<unsigned>({0,2,1}),"scan uses old volume, not newly requested value");
}
void assignmentAndClear(){
    Pool p(3);Backend b;b.clock=0xfffffff0u;r::VoiceWrapper v;v.identity=55;v.cachedVolume=-123;v.duration=32;
    auto* next=p.nodes[0].next;r::assignSchedule(b,p.nodes[0],v,1,&p.slots[0],false,8,9);
    check(p.nodes[0].record.deadline==16 && p.nodes[0].record.volume==-123 && p.slots[0]==99 && p.nodes[0].next==next,"assignment wraps clock, stores fields, does not publish output or reorder");
    const auto ticks=b.ticks;r::assignSchedule(b,p.nodes[0],v,1,&p.slots[0],true,8,9);check(b.ticks==ticks && p.nodes[0].record.deadline==0xffffffffu,"loop assignment avoids clock");
    b.clock=10;b.calls.clear();b.ticks=0;p.nodes[1].record.deadline=10;p.nodes[2].record.deadline=11;
    r::clearSchedules(b,p.head,true);
    check(b.calls=="BQSZBQSZ" && b.ticks==2 && p.order()==std::vector<unsigned>({0,1,2}),"whole clear stops loops/future only, clock per record, no reorder");
    check(p.slots==std::vector<unsigned>({0,0,0}),"whole clear slot retirement");
    b.calls.clear();b.ticks=0;r::clearSchedules(b,p.head,true);check(b.calls.empty() && !b.ticks,"repeat clear has no controls or clocks");
}
void nativeEviction(){
    using namespace mnm::audio;
    struct Native:Backend {
        Device& d;explicit Native(Device& device):d(device){}
        unsigned bufferForVoice(unsigned id)override{return id;}
        r::Status getStatus(unsigned id,unsigned& flags)override{auto voice=d.voice(id);if(!voice)return -1;flags=voice->status();return 0;}
        r::Status stop(unsigned id)override{return d.stop(id)==Error::ok?0:-1;}
        r::Status position(unsigned id,unsigned byte)override{return !byte && d.resetPosition(id)==Error::ok?0:-1;}
    };
    Device d;BufferId first=0,last=0;PcmFormat f{1,1,48000,96000,2,16,0};
    check(d.createStatic(0xea,f,2,first)==Error::ok,"native source");WriteLock write;
    check(d.lock(first,0,2,0,write)==Error::ok,"native lock");write.first.data[0]=0x10;write.first.data[1]=0x27;
    check(d.unlock(write,2,0)==Error::ok && d.duplicate(first,last)==Error::ok,"native committed duplicates");
    d.play(first,1);d.play(last,1);Pool p(2);p.nodes[0].record.voiceAddress=unsigned(first);p.nodes[1].record.voiceAddress=unsigned(last);
    p.nodes[0].record.volume=-100;p.nodes[1].record.volume=-200;Native b(d);
    auto* selected=r::selectSchedule(b,p.head,0,true);
    check(selected==&p.nodes[1] && d.voice(first)->status()==5 && !d.voice(last)->status() && d.voice(last)->frame==0,"tail reuse stops tail voice rather than candidate voice");
    std::vector<std::int16_t> pcm;d.mixStereo(1,pcm);check(pcm==std::vector<std::int16_t>({10000,10000}) && d.count()==2,"eviction removes tail from mix without releasing samples");
    r::clearSchedules(b,p.head,true);d.mixStereo(1,pcm);check(pcm[0]==0 && d.count()==2,"whole-ring clear retains ownership and produces silence");
}
int main(){try{selectionOracle();evictionFailures();reprioritization();assignmentAndClear();nativeEviction();std::cout<<"Scheduler passed: 5184 selection, 972 old-volume ordering, 64 failure scenarios; assignment and whole-ring clearing\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
