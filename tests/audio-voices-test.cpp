#include "voice_contract.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace mnm::reconstruction::audio;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Fake : VoiceBackend {
    std::vector<std::string> calls;
    Status query=0,stopResult=0,resetResult=0,volumeResult=0,panResult=0,playResult=0;
    std::uint32_t flags=0,failVolumeId=0,failPanId=0,busyId=0;
    Status getStatus(std::uint32_t id,std::uint32_t& out) override {
        calls.push_back("status:"+std::to_string(id));out=id==busyId?1:flags;return query;
    }
    Status stop(std::uint32_t id) override {calls.push_back("stop:"+std::to_string(id));return stopResult;}
    Status position(std::uint32_t id,std::uint32_t byte) override {
        calls.push_back("reset:"+std::to_string(id)+":"+std::to_string(byte));return resetResult;
    }
    Status volume(std::uint32_t id,std::int32_t value) override {
        calls.push_back("volume:"+std::to_string(id)+":"+std::to_string(value));
        return failVolumeId && id!=failVolumeId?0:volumeResult;
    }
    Status pan(std::uint32_t id,std::int32_t value) override {
        calls.push_back("pan:"+std::to_string(id)+":"+std::to_string(value));
        return failPanId && id!=failPanId?0:panResult;
    }
    Status play(std::uint32_t id,std::uint32_t r1,std::uint32_t r2,std::uint32_t loop) override {
        check(r1==0 && r2==0,"reserved play arguments");
        calls.push_back("play:"+std::to_string(id)+":"+std::to_string(loop));return playResult;
    }
    void volumeRecord(VoiceContract& v,std::int32_t value) override {
        calls.push_back("record:"+std::to_string(v.buffer)+":"+std::to_string(value));
    }
};
void expect(const Fake& f,std::initializer_list<std::string> expected){
    check(f.calls==std::vector<std::string>(expected),"unexpected call sequence");
}
void stopCases(){
    for(auto query:{Status(0),Status(7),Status(-9)})for(auto flags:{0u,1u,2u,3u}){
        Fake f;f.query=query;f.flags=flags;
        check(stopAndReset(f,11,true)==0,"stop success");
        if(query || (flags&1))expect(f,{"status:11","stop:11","reset:11:0"});
        else expect(f,{"status:11"});
    }
    Fake f;check(stopAndReset(f,0,true)==0 && stopAndReset(f,11,false)==0,"stop guards");expect(f,{});
    f.flags=1;f.stopResult=13;check(stopAndReset(f,11,true)==13,"positive stop failure");
    expect(f,{"status:11","stop:11"});
    f={};f.flags=1;f.resetResult=-17;check(stopAndReset(f,11,true)==-17,"reset failure");
    expect(f,{"status:11","stop:11","reset:11:0"});
}
void volumeCases(){
    VoiceContract c{3,99,99,nullptr},b{2,99,99,&c},a{1,99,99,&b};Fake f;
    check(setVoiceVolume(f,a,-1200,true,true,true)==0,"volume chain");
    expect(f,{"volume:1:-1200","record:1:-1200","volume:2:-1200","record:2:-1200",
              "record:2:-1200","volume:3:-1200","record:3:-1200","record:3:-1200"});
    f.calls.clear();c.cachedVolume=99;
    check(setVoiceVolume(f,a,-1200,true,true,true)==0,"cache suppression");expect(f,{});
    check(c.cachedVolume==99,"equal root skips children");
    f.volumeResult=19;
    check(setVoiceVolume(f,a,-700,true,true,true)==19,"volume failure");expect(f,{"volume:1:-700"});
    check(a.requestedVolume==-700 && a.cachedVolume==-700 && b.cachedVolume==-1200,"cache precedes failure");
    f.calls.clear();check(setVoiceVolume(f,a,-700,true,true,true)==0,"failed cached value not retried");expect(f,{});
    f={};f.volumeResult=-23;f.failVolumeId=2;
    check(setVoiceVolume(f,a,-800,true,false,true)==-23,"child failure");
    expect(f,{"volume:1:-800","volume:2:-800"});check(c.cachedVolume==99,"later child untouched");
    f={};check(setVoiceVolume(f,a,-600,true,true,false)==0,"disabled volume cache");expect(f,{});
    check(a.cachedVolume==-600 && b.cachedVolume==-800,"disabled does not propagate");
    f={};b.cachedVolume=-400;c.cachedVolume=-400;
    check(setVoiceVolume(f,a,-400,true,true,true)==0,"equal child cache notification");
    expect(f,{"volume:1:-400","record:1:-400","record:2:-400","record:3:-400"});
}
void panAndStartCases(){
    VoiceContract c{3,99,99,nullptr},b{2,99,99,&c},a{1,99,99,&b};Fake f;
    check(setVoicePan(f,a,3333,true,true)==0,"pan chain");expect(f,{"pan:1:3333","pan:2:3333","pan:3:3333"});
    f={};f.panResult=29;f.failPanId=2;
    check(setVoicePan(f,a,-3333,true,true)==29,"pan child failure");expect(f,{"pan:1:-3333","pan:2:-3333"});
    f={};check(setVoicePan(f,a,0,true,false)==0,"disabled pan");expect(f,{});
    check(setVoicePan(f,a,0,false,true)==0 && setVoicePan(f,a,0,false,true)==0,"pan has no cache");
    expect(f,{"pan:1:0","pan:1:0"});
    for(bool loop:{false,true}){
        f={};a.cachedVolume=99;
        check(startVoice(f,a,-900,100,loop,true)==0,"start success");
        expect(f,{"volume:1:-900","pan:1:100",loop?"play:1:1":"play:1:0"});
    }
    f={};f.volumeResult=-31;a.cachedVolume=99;
    check(startVoice(f,a,-900,100,false,true)==-31,"start volume failure");expect(f,{"volume:1:-900"});
    f={};f.panResult=37;check(startVoice(f,a,-900,100,false,true)==37,"start pan failure");expect(f,{"pan:1:100"});
    f={};f.playResult=-41;check(startVoice(f,a,-900,100,true,true)==-41,"start play failure");expect(f,{"pan:1:100","play:1:1"});
    f={};check(startVoice(f,a,0,0,true,false)==0,"disabled start");expect(f,{});check(a.cachedVolume==-900,"disabled start cache unchanged");
}
void busyAndScheduleCases(){
    VoiceContract b{2,99,99,nullptr},a{1,99,99,&b};Fake f;
    check(!voiceBusy(f,a,true),"idle chain");expect(f,{"status:1","status:2"});
    f={};f.busyId=2;check(voiceBusy(f,a,true),"busy duplicate");
    f={};f.query=43;check(voiceBusy(f,a,true),"query failure is busy");expect(f,{"status:1"});
    f={};a.buffer=0;check(!voiceBusy(f,a,true),"null root short circuit");expect(f,{});
    check(retirementDeadline(100,37,false)==137 && retirementDeadline(100,37,true)==0xffffffffu,"deadline");
    check(wavDuration(22050,22050)==1000 && wavDuration(1,22050)==0,"duration integer truncation");
    check(wavDuration(0x00418938u,1000)==0,"duration x86 product wraps");
    check(!retirementDue(136,137) && retirementDue(137,137) && retirementDue(138,137),"unsigned comparison boundary");
    check(!retirementDue(0xffffffffu,0xffffffffu),"loop sentinel");
    check(retirementDeadline(0xfffffff0u,32,false)==16 && retirementDue(0xfffffff0u,16),"preserve original wrap behavior");
    Schedule32 s{0x10,0x20,137,-900,0x12345678,0x87654321,4,5};
    check(clearSchedule(s)==0x87654321 && s.next==0x10 && s.previous==0x20,"retain links, return output slot");
    check(s.deadline==0 && s.volume==-5000 && s.voiceAddress==0 && s.outputAddress==0 && s.x==-1 && s.y==-1,"clear record");
}
void panMath(){
    check(positionalPan(0,20)==0 && positionalPan(10,20)==1666 && positionalPan(-10,20)==-1666,"pan interior");
    check(positionalPan(20,20)==3333 && positionalPan(-21,20)==-3333,"pan saturation");
    check(positionalPan(1,2)==1666 && positionalPan(-1,2)==-1666,"signed truncation");
    bool rejected=false;try{positionalPan(1,0);}catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"pan width guard");
}
}
int main(){try{stopCases();volumeCases();panAndStartCases();busyAndScheduleCases();panMath();
    std::cout<<"Voice contracts passed; no playback or game hooks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
