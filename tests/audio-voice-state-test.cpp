#include "voice_contract.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::audio;
namespace rec=mnm::reconstruction::audio;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void ok(Error e){check(e==Error::ok,"unexpected native audio error");}
BufferId make(Device& device,std::size_t frames=5,unsigned channels=1,unsigned bits=16){
    const auto alignment=std::uint16_t(channels*(bits/8));
    Wave wave{{1,std::uint16_t(channels),22050,std::uint32_t(22050*alignment),alignment,std::uint16_t(bits),0},
              std::vector<std::uint8_t>(frames*alignment,42)};
    auto upload=rec::uploadStatic(device,wave);ok(upload.error);return upload.buffer;
}
void oneShot(){
    Device d;auto id=make(d);AdvanceResult a{99,true};
    auto v=d.voice(id);check(v && v->frame==0 && v->frames==5 && v->status()==0,"initial state");
    ok(d.advanceFrames(id,3,a));check(a.consumed==0 && !a.completed,"stopped does not advance");
    ok(d.play(id));ok(d.advanceFrames(id,2,a));check(a.consumed==2 && !a.completed && d.voice(id)->frame==2,"first frames");
    ok(d.play(id));check(d.voice(id)->frame==2,"play while playing preserves cursor");
    ok(d.stop(id));ok(d.advanceFrames(id,4,a));check(a.consumed==0 && d.voice(id)->frame==2,"stop preserves cursor");
    ok(d.play(id));ok(d.advanceFrames(id,2,a));check(d.voice(id)->frame==4 && !a.completed,"resume");
    ok(d.advanceFrames(id,100,a));check(a.consumed==1 && a.completed,"consume only remaining frame");
    v=d.voice(id);check(v->playback==Playback::completed && v->frame==5 && v->status()==0,"completion boundary");
    ok(d.advanceFrames(id,1,a));check(a.consumed==0 && !a.completed,"completion only reported once");
    ok(d.play(id));check(d.voice(id)->frame==0 && d.voice(id)->status()==1,"replay completed one-shot");
    ok(d.advanceFrames(id,1,a));ok(d.resetPosition(id));check(d.voice(id)->frame==0 && d.voice(id)->status()==1,"reset playing voice");
    ok(d.stop(id));ok(d.resetPosition(id));check(d.voice(id)->frame==0 && d.voice(id)->status()==0,"stop then reset");
    ok(d.play(id));ok(d.advanceFrames(id,5,a));ok(d.resetPosition(id));
    check(d.voice(id)->playback==Playback::stopped && d.voice(id)->frame==0,"reset completed voice");
}
void looping(){
    Device d;auto id=make(d,7);AdvanceResult a;
    ok(d.play(id,1));ok(d.advanceFrames(id,6,a));check(d.voice(id)->frame==6 && d.voice(id)->status()==5,"loop state");
    ok(d.advanceFrames(id,std::numeric_limits<std::uint64_t>::max(),a));
    check(d.voice(id)->frame==0 && a.consumed==std::numeric_limits<std::uint64_t>::max() && !a.completed,"huge loop advance without overflow");
    ok(d.advanceFrames(id,21,a));check(d.voice(id)->frame==0 && a.consumed==21,"whole loops");
    ok(d.advanceFrames(id,3,a));ok(d.play(id,0));check(d.voice(id)->frame==3 && d.voice(id)->status()==1,"change loop flag without restart");
    ok(d.advanceFrames(id,4,a));check(a.completed && d.voice(id)->status()==0,"finish after disabling loop");
    ok(d.play(id,1));ok(d.advanceFrames(id,8,a));check(d.voice(id)->frame==1,"replay as loop");
    ok(d.stop(id));check(d.voice(id)->looping && d.voice(id)->status()==0,"stopped loop configuration vs status");
}
void duplicatesAndWrites(){
    Device d;auto source=make(d);ok(d.setVolume(source,-1200));ok(d.setPan(source,3333));ok(d.play(source,1));
    AdvanceResult a;ok(d.advanceFrames(source,2,a));BufferId child=0;ok(d.duplicate(source,child));
    check(d.voice(child)->frame==0 && d.voice(child)->status()==0 && d.voice(child)->volume==-1200 && d.voice(child)->pan==3333,"duplicate controls, independent activity");
    ok(d.setVolume(child,-600));ok(d.setPan(child,-3333));ok(d.play(child));ok(d.advanceFrames(child,1,a));
    check(d.voice(source)->frame==2 && d.voice(source)->volume==-1200 && d.voice(source)->pan==3333,"child controls independent");
    ok(d.advanceFrames(source,4,a));check(d.voice(source)->frame==1 && d.voice(child)->frame==1,"independent advances");
    WriteLock lock;ok(d.lock(source,0,2,0,lock));std::fill_n(lock.first.data,2,99);
    check(d.samples(child)[0]==42,"pending bytes hidden while playing");
    ok(d.unlock(lock,2,0));check(d.samples(child)[0]==99 && d.voice(child)->frame==1,"commit does not reset playback");
    ok(d.lock(source,0,2,0,lock));std::fill_n(lock.first.data,2,12);ok(d.release(source));
    check(!d.voice(source) && d.samples(child)[0]==99 && d.voice(child)->status()==1,"release playing writer preserves survivor");
    check(d.unlock(lock,2,0)==Error::invalid,"released writer cannot commit");
    ok(d.advanceFrames(child,4,a));check(a.completed,"survivor finishes after source release");
    ok(d.play(child,1));ok(d.release(child));check(d.count()==0 && !d.voice(child),"retire playing last owner");
    a={77,true};check(d.advanceFrames(child,1,a)==Error::invalid && a.consumed==77 && a.completed,"retired id cannot advance");
    auto replacement=make(d);check(replacement!=source && replacement!=child,"retired identities not reused");
    check(d.play(child)==Error::invalid,"stale voice stays invalid");
}
void rejectsAndFormats(){
    Device d;auto id=make(d);BufferId primary=0;ok(d.createPrimary(0x81,primary));AdvanceResult a{77,true};
    check(!d.voice(primary) && d.play(primary)==Error::unsupported && d.stop(primary)==Error::unsupported &&
          d.resetPosition(primary)==Error::unsupported && d.setVolume(primary,0)==Error::unsupported &&
          d.setPan(primary,0)==Error::unsupported && d.advanceFrames(primary,1,a)==Error::unsupported,"primary metadata excluded");
    check(a.consumed==77 && a.completed,"failed advance leaves output untouched");
    ok(d.setVolume(id,-10000));ok(d.setPan(id,10000));ok(d.play(id,1));
    check(d.setVolume(id,1)==Error::invalid && d.setVolume(id,-10001)==Error::invalid &&
          d.setPan(id,10001)==Error::invalid && d.setPan(id,-10001)==Error::invalid && d.play(id,2)==Error::unsupported,"controls and flags bounded");
    check(d.voice(id)->volume==-10000 && d.voice(id)->pan==10000 && d.voice(id)->status()==5,"rejection preserves state");
    ok(d.setVolume(id,0));ok(d.setPan(id,-10000));check(d.voice(id)->frame==0,"control limits do not advance");
    check(d.stop(999)==Error::invalid && d.resetPosition(999)==Error::invalid && d.setVolume(999,0)==Error::invalid && d.setPan(999,0)==Error::invalid,"missing identities");
    for(unsigned channels:{1u,2u})for(unsigned bits:{8u,16u}){
        auto sample=make(d,3,channels,bits);ok(d.play(sample));ok(d.advanceFrames(sample,2,a));
        check(d.voice(sample)->frames==3 && d.voice(sample)->frame==2,"frames include all channels");
        ok(d.advanceFrames(sample,1,a));check(a.completed,"whole PCM frame completion");
    }
}
// Independent oracle: step one source frame at a time, rather than modular math.
void advancementOracle(){
    for(std::size_t length=1;length<=9;++length)for(bool loop:{false,true})for(unsigned split=0;split<17;++split){
        Device d;auto id=make(d,length);ok(d.play(id,loop?1:0));
        std::uint64_t position=0;bool playing=true;
        for(unsigned amount:{split,0u,17u-split,13u}){
            std::uint64_t consumed=0;bool completed=false;
            for(unsigned n=0;n<amount && playing;++n){++position;++consumed;
                if(position==length){if(loop)position=0;else{playing=false;completed=true;}}
            }
            AdvanceResult a;ok(d.advanceFrames(id,amount,a));auto v=d.voice(id);
            check(a.consumed==consumed && a.completed==completed && v->frame==position &&
                  (v->playback==Playback::playing)==playing,"independent frame-step oracle");
        }
    }
}
// Fixture adapter only: exercise reconstructed call order on real native state.
struct NativeBackend : rec::VoiceBackend {
    Device& d;explicit NativeBackend(Device& device):d(device){}
    static rec::Status result(Error e){return e==Error::ok?0:-1;}
    rec::Status getStatus(std::uint32_t id,std::uint32_t& flags) override {
        auto v=d.voice(id);if(!v)return -1;flags=v->status();return 0;
    }
    rec::Status stop(std::uint32_t id) override{return result(d.stop(id));}
    rec::Status position(std::uint32_t id,std::uint32_t byte) override{return byte?-1:result(d.resetPosition(id));}
    rec::Status volume(std::uint32_t id,std::int32_t value) override{return result(d.setVolume(id,value));}
    rec::Status pan(std::uint32_t id,std::int32_t value) override{return result(d.setPan(id,value));}
    rec::Status play(std::uint32_t id,std::uint32_t r1,std::uint32_t r2,std::uint32_t flags) override {
        return r1 || r2?-1:result(d.play(id,flags));
    }
    void volumeRecord(rec::VoiceContract&,std::int32_t) override{}
};
void engineContracts(){
    Device d;auto id=make(d);NativeBackend b(d);rec::VoiceContract v{std::uint32_t(id),99,99,nullptr};
    check(rec::startVoice(b,v,-1500,3333,true,true)==0 && rec::voiceBusy(b,v,false),"engine start uses native state");
    AdvanceResult a;ok(d.advanceFrames(id,3,a));check(rec::stopAndReset(b,v.buffer,true)==0,"engine stop/reset");
    check(d.voice(id)->frame==0 && d.voice(id)->status()==0 && d.voice(id)->volume==-1500,"engine reset separate from controls");
    check(rec::startVoice(b,v,-1500,-3333,false,true)==0,"engine restart cached volume");
    ok(d.advanceFrames(id,5,a));check(!rec::voiceBusy(b,v,false),"engine sees completion");
    check(rec::setVoiceVolume(b,v,1,false,false,true)!=0 && v.cachedVolume==1 && d.voice(id)->volume==-1500,"engine cache distinct from accepted native control");
}
}
int main(){try{oneShot();looping();duplicatesAndWrites();rejectsAndFormats();advancementOracle();engineContracts();
    std::cout<<"Native voice state passed; 306 independent advancement scenarios; no output or hooks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
