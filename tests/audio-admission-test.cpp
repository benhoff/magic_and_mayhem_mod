#include "voice_admission.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
struct Backend:r::AdmissionBackend {
    std::string calls;std::vector<std::pair<int,unsigned>> queries;
    std::size_t queryAt=0;unsigned random=0xffffffffu,clock=0xfffffff0u;
    int loadFault=0,duplicateFault=0,volumeFault=0,panFault=0,playFault=0;
    r::VoiceWrapper* loaded=nullptr;int loadedIndex=-1;
    std::vector<std::unique_ptr<r::VoiceWrapper>> owned;
    unsigned tickCount()override{calls+='T';return clock;}
    unsigned randomWord()override{calls+='R';return random;}
    unsigned bufferForVoice(unsigned id)override{calls+='B';return id+10;}
    r::Status getStatus(unsigned,unsigned& flags)override{
        calls+='Q';check(queryAt<queries.size(),"unexpected status call");
        auto [result,value]=queries[queryAt++];flags=value;return result;
    }
    r::Status loadSource(int index,r::VoiceWrapper*& out)override{calls+='L';loadedIndex=index;if(!loadFault)out=loaded;return loadFault;}
    r::Status duplicateBuffer(unsigned source,unsigned& out)override{calls+='D';check(source==11,"duplicate source buffer");if(!duplicateFault)out=100;return duplicateFault;}
    r::VoiceWrapper& allocateWrapper()override{calls+='A';auto v=std::make_unique<r::VoiceWrapper>();v->identity=1000;owned.push_back(std::move(v));return *owned.back();}
    r::Status stop(unsigned)override{calls+='S';return 0;}
    r::Status position(unsigned,unsigned p)override{calls+='Z';check(!p,"zero reset");return 0;}
    r::Status volume(unsigned,int v)override{calls+='V';check(v==-100,"volume");return volumeFault;}
    r::Status pan(unsigned,int p)override{calls+='N';check(p==20,"pan");return panFault;}
    r::Status play(unsigned,unsigned a,unsigned b,unsigned flags)override{calls+='P';check(a==0 && b==0 && flags<=1,"play arguments");return playFault;}
    void volumeRecord(r::VoiceContract&,int)override{throw std::runtime_error("no volume scheduler notification on start");}
};
struct Scene {
    r::VoiceWrapper source,other,d1,d2; r::VoiceWrapper* head=&other;
    r::ScheduleNode slot; r::ScheduleNode* schedules=&slot;
    r::ManagerState manager; r::AdmissionCatalog catalog{{},{10,20}};
    unsigned output=77; r::AdmissionRequest request;
    Scene(){
        source.identity=1;source.buffer=11;source.sourceIndex=10;source.duration=32;source.field08=42;source.field0c=3;
        other.identity=2;other.buffer=12;other.sourceIndex=20;
        source.previous=source.next=&other;other.previous=other.next=&source;
        d1.identity=3;d1.buffer=13;d1.sourceIndex=10;d2.identity=4;d2.buffer=14;d2.sourceIndex=10;
        slot.next=slot.previous=&slot;manager.initialized=manager.active=true;
        request.sound=10;request.volume=-100;request.pan=20;request.x=8;request.y=9;
        request.outputAddress=123;request.output=&output;
    }
    int run(Backend& b){return r::admitVoice(b,manager,catalog,head,schedules,99,request);}
    void assigned(unsigned id,bool loop){check(slot.record.voiceAddress==id && slot.record.outputAddress==123 && slot.output==&output &&
        slot.record.volume==-100 && slot.record.x==8 && slot.record.y==9 && slot.record.deadline==(loop?0xffffffffu:16),"scheduler publication");}
};
void gatesAndCatalog(){
    for(bool initialized:{false,true})for(bool active:{false,true}){
        if(initialized && active)continue;
        Scene s;Backend b;s.manager.initialized=initialized;s.manager.active=active;
        check(!s.run(b) && b.calls.empty() && s.output==99,"disabled manager output and no work");
    }
    Scene s;Backend b;s.slot.record.voiceAddress=8;s.slot.record.deadline=0xffffffffu;s.slot.record.volume=0;
    check(!s.run(b) && b.calls.empty() && s.output==77,"no slot skips source resolution");
    r::AdmissionCatalog c{{{5,{10,20,30}}},{10,20,30}};
    for(unsigned random:{0u,1u,2u,3u,0x80000001u,0xffffffffu}){
        Backend rng;rng.random=random;
        check(r::admissionSoundId(rng,c,5)==c.groups[0].members[(random&0x7fffffffu)%3] && rng.calls=="R","group masked random selection");
    }
    check(r::admissionSoundId(b,c,20)==20 && r::admissionSoundId(b,c,999)==0x19a,"sorted ID and fallback");
    c.groups[0].members.clear();bool rejected=false;try{r::admissionSoundId(b,c,5);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"empty alias divisor rejected");
}
void duplicates(){
    for(int fault=0;fault<4;++fault){
        Scene s;Backend b;s.source.duplicate=&s.d1;s.d1.duplicate=&s.d2;
        b.volumeFault=fault==1?11:0;b.panFault=fault==2?12:0;b.playFault=fault==3?13:0;
        r::VoiceWrapper* output=&s.other;
        const auto result=r::duplicateAndStart(b,s.source,-100,20,false,output);
        check(result==(fault?10+fault:0),"duplicate control error propagation");
        check(b.calls==(fault==1?"DAV":fault==2?"DAVN":"DAVNP"),"duplicate control ordering");
        auto* child=b.owned[0].get();
        check(output==child && s.d2.duplicate==child && s.source.requestedVolume==-100 && s.source.cachedVolume==99,"append/publication persists on control failure");
        check(child->cachedVolume==-100 && child->sourceIndex==10 && child->field08==42 && child->field0c==3 && child->duration==32,"copied duplicate metadata");
    }
    Scene s;Backend b;b.duplicateFault=19;r::VoiceWrapper* output=&s.other;
    check(r::duplicateAndStart(b,s.source,-100,20,false,output)==19 && b.calls=="D" && output==&s.other && !s.source.duplicate,"duplicate API failure leaves chain and output");
}
void managerMatrix(){
    for(bool busy:{false,true})for(bool loop:{false,true})for(int fault=0;fault<4;++fault){
        Scene s;Backend b;s.request.looping=loop;b.queries={{0,0},{0,busy?1u:0u}};
        b.volumeFault=fault==1?11:0;b.panFault=fault==2?12:0;b.playFault=fault==3?13:0;
        check(s.run(b)==(fault?10+fault:0),"manager result");
        const std::string controls=fault==1?"V":fault==2?"VN":"VNP";
        check(b.calls=="QQ"+std::string(busy?"DA":"")+controls+(!fault && !loop?"T":""),"manager call ordering and loop clock suppression");
        if(fault){check(s.output==77 && !s.slot.record.voiceAddress,"control failure skips slot publication");
            check(s.head==(busy?&s.other:&s.source),"direct failure promotes, duplicate failure does not");
        }else{s.assigned(busy?1000:1,loop);check(s.output==(busy?1000u:1u) && s.head==&s.source,"successful publication and source promotion");}
        check(bool(s.source.duplicate)==busy,"duplicate chain retained even on failure");
        check(s.head->next->previous==s.head && s.head->previous->next==s.head,"source ring integrity");
    }
}
void sourceRingAndCache(){
    for(bool sameIndex:{false,true}){
        Scene s;Backend b;r::VoiceWrapper third;third.identity=5;third.sourceIndex=2;
        s.other.next=&s.source;s.source.previous=&s.other;s.source.next=&third;
        third.previous=&s.source;third.next=&s.other;s.other.previous=&third;
        if(sameIndex){s.other.sourceIndex=10;b.loaded=&s.source;b.queries={{7,0},{0,0}};}
        else b.queries={{0,0},{0,0}};
        check(!s.run(b) && s.head==&s.source,"source becomes head");
        auto* expectedNext=sameIndex?&third:&s.other;
        check(s.head->next==expectedNext && s.head->next->next==(sameIndex?&s.other:&third),"same-index promotion rotates only, other index detaches and prepends");
        auto* node=s.head;for(unsigned i=0;i<3;++i){check(node->next->previous==node && node->previous->next==node,"three-member bidirectional source ring");node=node->next;}
        check(node==s.head,"source ring remains bounded");
    }
    Scene cached;Backend b;cached.source.cachedVolume=-100;cached.source.requestedVolume=42;
    cached.request.output=nullptr;cached.request.outputAddress=0;b.queries={{0,0},{0,0}};
    check(!cached.run(b) && b.calls=="QQNPT" && cached.source.requestedVolume==42 && !cached.slot.output && !cached.slot.record.outputAddress,"cache suppresses volume writes and absent output is supported");
}
void loadingAndRotation(){
    for(unsigned firstFlags:{0u,1u,2u,3u})for(int queryFault:{0,7}){
        Scene s;Backend b;b.loaded=&s.source;b.queries={{queryFault,firstFlags},{0,0}};
        check(!s.run(b),"status/loading start");
        const bool reload=queryFault || (firstFlags&2);
        check(b.calls==std::string(reload?"QLQVNPT":"QQVNPT"),"loading gate calls");
        check((b.loadedIndex==10)==reload,"first-query error or buffer-lost flag reloads");
        check(s.source.field0c==(!queryFault && (firstFlags&2)?2u:3u),"field0c writes only on successful bit2 query");
    }
    Scene failed;Backend loader;loader.queries={{8,0}};loader.loadFault=18;
    check(failed.run(loader)==18 && loader.calls=="QL" && failed.output==77 && failed.head==&failed.other,"load failure leaves publication/head");
    Scene missing;Backend load;missing.request.sound=999;load.loaded=&missing.source;load.queries={{0,0}};
    check(!missing.run(load) && load.loadedIndex==0x19a && load.calls=="LQVNPT","missing ID falls back through loader");
    Scene error;Backend status;status.queries={{0,0},{9,0}};
    check(!error.run(status) && status.calls=="QQDAVNPT" && error.output==1000,"second status error duplicates");
    Scene rejected;Backend duplicate;duplicate.queries={{0,0},{0,1}};duplicate.duplicateFault=19;
    check(rejected.run(duplicate)==19 && duplicate.calls=="QQD" && rejected.head==&rejected.other && rejected.output==77 && !rejected.slot.record.voiceAddress,"manager duplicate API failure skips promotion/publication");
    Scene evicted;Backend failAfterEviction;unsigned formerOutput=88;
    evicted.slot.record.voiceAddress=42;evicted.slot.record.deadline=0xffffffffu;
    evicted.slot.record.volume=-200;evicted.slot.record.outputAddress=456;evicted.slot.output=&formerOutput;
    failAfterEviction.queries={{0,1},{7,0}};failAfterEviction.loadFault=18;
    check(evicted.run(failAfterEviction)==18 && failAfterEviction.calls=="BQSZQL" && formerOutput==0 && !evicted.slot.record.voiceAddress && evicted.output==77,"eviction commits before failed load, with no rollback");
    for(bool atHead:{false,true})for(bool two:{false,true}){
        Scene s;Backend b;if(atHead)s.head=&s.source;
        s.source.duplicate=&s.d1;if(two)s.d1.duplicate=&s.d2;
        b.queries={{0,0},{0,0}};
        check(!s.run(b) && s.head==&s.d1 && s.output==1,"played original published while first duplicate becomes source head");
        check(s.d1.next==&s.other && s.d1.previous==&s.other && s.other.next==&s.d1 && s.other.previous==&s.d1,"duplicate replaces source ring member");
        check((two?s.d2.duplicate:s.d1.duplicate)==&s.source && !s.source.duplicate && s.d1.requestedVolume==-100,"old source rotates to duplicate tail");
        s.assigned(1,false);
    }
}
struct Native:Backend {
    mnm::audio::Device device{1024,8,22050};
    r::Status getStatus(unsigned id,unsigned& flags)override{auto voice=device.voice(id);if(!voice)return 1;flags=voice->status();return 0;}
    r::Status duplicateBuffer(unsigned source,unsigned& out)override{
        mnm::audio::BufferId id=0;auto result=device.duplicate(source,id);out=unsigned(id);return int(result);
    }
    r::Status volume(unsigned id,int v)override{return int(device.setVolume(id,v));}
    r::Status pan(unsigned id,int v)override{return int(device.setPan(id,v));}
    r::Status play(unsigned id,unsigned a,unsigned b,unsigned flags)override{check(!a && !b,"native play reserved arguments");return int(device.play(id,flags));}
};
void nativeOverlap(){
    for(bool looping:{false,true}){
    Scene s;Native b;b.clock=100;mnm::audio::PcmFormat format{1,1,22050,44100,2,16,0};mnm::audio::BufferId id=0;
    check(b.device.createStatic(0xea,format,128,id)==mnm::audio::Error::ok,"native source allocation");
    mnm::audio::WriteLock lock;check(b.device.lock(id,0,128,0,lock)==mnm::audio::Error::ok,"sample lock");
    for(unsigned i=0;i<128;i+=2){lock.first.data[i]=0xa0;lock.first.data[i+1]=0x0f;} // mono 4000
    check(b.device.unlock(lock,128,0)==mnm::audio::Error::ok,"sample commit");
    s.source.buffer=unsigned(id);s.request.volume=0;s.request.pan=0;s.request.looping=looping;
    r::ScheduleNode second;second.previous=second.next=&s.slot;s.slot.previous=s.slot.next=&second;
    check(!s.run(b) && s.output==1 && s.slot.record.voiceAddress==1,"native original admission");
    check(!s.run(b) && s.output==1000 && second.record.voiceAddress==1000,"native overlap duplicate admission");
    const auto childBuffer=b.owned[0]->buffer;
    check(b.device.voice(id)->status()==(looping?5u:1u) && b.device.voice(childBuffer)->status()==(looping?5u:1u),"playing/looping status does not set buffer-lost bit");
    check(b.device.samples(id)==b.device.samples(childBuffer) && b.device.count()==2,"duplicate shared committed sample content");
    std::vector<std::int16_t> pcm;check(b.device.mixStereo(1,pcm)==mnm::audio::Error::ok && pcm==std::vector<std::int16_t>({8000,8000}),"exact overlapping stereo samples");
    check(b.device.stop(childBuffer)==mnm::audio::Error::ok,"stop duplicate independently");
    check(b.device.mixStereo(1,pcm)==mnm::audio::Error::ok && pcm==std::vector<std::int16_t>({4000,4000}),"original continues after duplicate stop");
    }
}
int main(){gatesAndCatalog();duplicates();managerMatrix();sourceRingAndCache();loadingAndRotation();nativeOverlap();std::cout<<"Audio admission gates, remapping, loading, duplicates, failure ordering, ring rotation and native PCM passed\n";}
