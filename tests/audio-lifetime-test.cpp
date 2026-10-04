#include "voice_lifetime.hpp"
#include "buffers.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace rec=mnm::reconstruction::audio;
using namespace mnm::audio;
void check(bool v,const char* message){if(!v)throw std::runtime_error(message);}
struct Fake:rec::LifetimeBackend {
    std::vector<std::string> calls;int query=0,stopFault=0,resetFault=0;bool playing=true;
    void log(char op,unsigned id){calls.push_back(std::string(1,op)+std::to_string(id));}
    rec::Status getStatus(unsigned id,unsigned& f)override{log('Q',id);f=playing?1:0;return query;}
    rec::Status stop(unsigned id)override{log('S',id);return stopFault;}
    rec::Status position(unsigned id,unsigned p)override{check(!p,"zero reset");log('Z',id);return resetFault;}
    rec::Status volume(unsigned,int)override{return 0;}
    rec::Status pan(unsigned,int)override{return 0;}
    rec::Status play(unsigned,unsigned,unsigned,unsigned)override{return 0;}
    void volumeRecord(rec::VoiceContract&,int)override{}
    void clearVoiceSchedule(unsigned id)override{log('C',id);}
    void releaseBuffer(unsigned id)override{log('R',id);}
    void freeWrapper(unsigned id)override{log('F',id);}
};
rec::VoiceWrapper wrapper(unsigned id,unsigned buffer,rec::VoiceWrapper* child=nullptr){
    rec::VoiceWrapper v;v.identity=id;v.buffer=buffer;v.duplicate=child;v.duration=345;v.sourceIndex=7;v.cachedVolume=-123;return v;
}
void retirementMatrix(){
    for(bool enabled:{false,true})for(bool buffer:{false,true})for(bool playing:{false,true})
    for(int query:{0,9,-1})for(int stop:{0,17,-1})for(int reset:{0,23,-1})for(bool clear:{false,true}){
        Fake b;b.playing=playing;b.query=query;b.stopFault=stop;b.resetFault=reset;
        auto v=wrapper(1,buffer?11:0);const auto result=rec::retireVoice(b,v,false,clear,enabled);
        std::vector<std::string> expected;int wanted=0;
        if(enabled && buffer){expected.push_back("Q11");if(query || playing){expected.push_back("S11");if(clear)expected.push_back("C1");wanted=stop;if(!stop){expected.push_back("Z11");wanted=reset;}}}
        check(b.calls==expected && result==wanted,"retirement gates and nonzero failure ordering");
        check(v.buffer==(buffer?11u:0u) && v.duration==345,"retirement retains ownership and wrapper metadata");
    }
    Fake b;auto tail=wrapper(3,33);auto child=wrapper(2,22,&tail);auto source=wrapper(1,0,&child);
    check(!rec::retireVoice(b,source,true,true,true),"null source still visits children");
    check(b.calls==std::vector<std::string>({"Q22","S22","C2","Z22","C2","Q33","S33","C3","Z33","C3"}),"child forced clear and outer duplicate notification");
    b.calls.clear();b.stopFault=17;
    check(rec::retireVoice(b,source,true,false,true)==17 && b.calls==std::vector<std::string>({"Q22","S22","C2"}),"child failure prevents later children even without outer clear");
    b.calls.clear();b.playing=false;b.stopFault=0;
    rec::retireVoice(b,source,true,true,true);
    check(b.calls==std::vector<std::string>({"Q22","C2","Q33","C3"}),"idle children receive outer notification only");
}
void destruction(){
    for(unsigned flags:{0u,1u,2u,3u}){
        Fake b;auto tail=wrapper(3,33);auto child=wrapper(2,22,&tail);auto source=wrapper(1,11,&child);
        rec::destroyWrapper(b,source,flags);
        std::vector<std::string> expected={"R33","F3","R22","F2","R11"};if(flags&1)expected.push_back("F1");
        check(b.calls==expected,"postorder release/free and deleting flag bit");
        check(!source.buffer && !source.duplicate && source.sourceIndex==-1 && source.cachedVolume==99 && !source.duration,"reset defaults");
        if(!(flags&1)){b.calls.clear();rec::destroyWrapper(b,source,0);check(b.calls.empty(),"repeat contents cleanup releases nothing twice");}
    }
    Fake b;auto child=wrapper(2,22);auto source=wrapper(1,0,&child);rec::releaseSourceContents(b,source);
    check(b.calls==std::vector<std::string>({"R22","F2"}) && !source.duplicate && source.duration==345,"absent buffer skips reset but destroys children");
}
void rings(){
    for(unsigned count=1;count<=8;++count)for(unsigned first=0;first<count;++first)for(unsigned target=0;target<count;++target){
        std::vector<rec::ScheduleNode> nodes(count);std::vector<unsigned> slots(count,55),expected;
        for(unsigned i=0;i<count;++i){nodes[i].next=&nodes[(i+1)%count];nodes[i].previous=&nodes[(i+count-1)%count];nodes[i].record={0,0,123,-5,50+i,1,2,3};nodes[i].output=&slots[i];}
        for(unsigned i=0;i<count;++i)if((first+i)%count!=target)expected.push_back((first+i)%count);
        expected.push_back(target);
        auto* head=&nodes[first];rec::retireSchedule(head,nodes[target]);auto* cursor=head;
        for(auto index:expected){check(cursor==&nodes[index] && cursor->next->previous==cursor && cursor->previous->next==cursor,"ring ordering and bidirectional integrity");cursor=cursor->next;}
        check(cursor==head && slots[target]==0 && !nodes[target].record.voiceAddress && nodes[target].record.x==-1,"retirement clears record/slot but retains ring");
        for(unsigned i=0;i<count;++i)if(i!=target)check(slots[i]==55 && nodes[i].record.voiceAddress==50+i,"unrelated records retained");
        rec::retireSchedule(head,nodes[target]);check(head==&nodes[expected.front()],"repeated tail retirement retains order");
    }
}
struct Native:Fake {
    Device& device;explicit Native(Device& d):device(d){}
    rec::Status getStatus(unsigned id,unsigned& f)override{auto v=device.voice(id);if(!v)return -1;f=v->status();return 0;}
    rec::Status stop(unsigned id)override{return device.stop(id)==Error::ok?0:-1;}
    rec::Status position(unsigned id,unsigned p)override{return !p && device.resetPosition(id)==Error::ok?0:-1;}
    void releaseBuffer(unsigned id)override{check(device.release(id)==Error::ok,"native release succeeds once");Fake::releaseBuffer(id);}
};
void nativeOwnership(){
    Device d;PcmFormat fmt{1,1,48000,96000,2,16,0};BufferId id=0,childId=0;
    check(d.createStatic(0xea,fmt,2,id)==Error::ok,"create native sample");WriteLock lock;
    check(d.lock(id,0,2,0,lock)==Error::ok,"lock");lock.first.data[0]=0x10;lock.first.data[1]=0x27;
    check(d.unlock(lock,2,0)==Error::ok && d.duplicate(id,childId)==Error::ok,"upload and duplicate");
    d.play(id,1);d.play(childId,1);Native b(d);auto child=wrapper(2,childId);auto source=wrapper(1,id,&child);
    rec::retireVoice(b,source,false,false,true);check(!d.voice(id)->status() && d.voice(childId)->status()==3 && d.count()==2,"retire source leaves duplicate playing and samples owned");
    d.release(id);source.buffer=0;std::vector<std::int16_t> pcm;d.mixStereo(1,pcm);
    check(pcm[0]==10000 && d.voice(childId)->status()==3,"source release leaves independently owned duplicate samples");
    rec::releaseSourceContents(b,source);check(d.count()==0 && !source.duplicate,"recursive cleanup retires final duplicate");
    b.calls.clear();rec::releaseSourceContents(b,source);check(b.calls.empty(),"repeated native contents cleanup");
}
int main(){try{retirementMatrix();destruction();rings();nativeOwnership();std::cout<<"Voice lifetimes passed: 432 retirement combinations, 204 ring cases, ownership and postorder cleanup\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
