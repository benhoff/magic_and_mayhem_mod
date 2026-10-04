#include "manager_contract.hpp"
#include "voice_bridge.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
using namespace mnm::audio;
namespace rec=mnm::reconstruction::audio;
void check(bool v,const char* text){if(!v)throw std::runtime_error(text);}
struct Fake:rec::ManagerBackend {
    std::string calls;rec::ManagerState* manager=nullptr;int getFault=0,playFault=0,volumeFault=0,primaryStopFault=0,stopFault=0,positionFault=0;
    unsigned clock=10;bool playing=true;std::int32_t lastVolume=0;
    rec::Status getVolume(std::uint32_t,std::int32_t& v) override{calls+='G';v=-1200;return getFault;}
    rec::Status play(std::uint32_t id,std::uint32_t a,std::uint32_t b,std::uint32_t flags) override{check(id==99 && !a && !b && flags==1,"primary looping arguments");calls+='P';return playFault;}
    rec::Status volume(std::uint32_t,std::int32_t v) override{calls+='V';lastVolume=v;return volumeFault;}
    rec::Status stop(std::uint32_t id) override{if(id==99){check(manager && !manager->active,"active flag cleared before primary Stop");calls+='S';return primaryStopFault;}calls+='s';return stopFault;}
    rec::Status getStatus(std::uint32_t,std::uint32_t& f) override{calls+='Q';f=playing?1:0;return 0;}
    rec::Status position(std::uint32_t,std::uint32_t byte) override{check(!byte,"zero reset");calls+='Z';return positionFault;}
    rec::Status pan(std::uint32_t,std::int32_t) override{return 0;}
    void volumeRecord(rec::VoiceContract&,std::int32_t) override{}
    std::uint32_t tickCount() override{calls+='T';return clock;}
    void retireVoice(std::uint32_t) override{calls+='D';}
    void releaseVoice(std::uint32_t) override{calls+='R';}
    void releaseDevice(std::uint32_t) override{calls+='E';}
};
void managerMatrix(){
    for(bool initialized:{false,true})for(bool active:{false,true})for(int get:{0,17,-1})for(int play:{0,23,-1}){
        rec::ManagerState m{initialized,active,99,88,0,0};Fake f;f.manager=&m;f.getFault=get;f.playFault=play;
        const auto result=rec::initializePrimaryControls(f,m);
        const std::string expected=!initialized?"":get || active?"G":"GP";
        check(f.calls==expected,"startup gates and order");check(result==(!initialized?0:get?get:active?0:play),"nonzero startup failures");
        check(m.active==(active || (initialized && !get && !play)),"active only after successful Play");
        std::int32_t untouched=77;f.calls.clear();rec::getPrimaryVolume(f,m,untouched);
        check(f.calls==(initialized?"G":"") && untouched==(initialized?-1200:77),"getter gates on initialized, not active");
        f.calls.clear();f.volumeFault=5;check(rec::setPrimaryVolume(f,m,-500)==(initialized?5:0) && f.calls==(initialized?"V":""),"setter gate/result");
    }
    for(bool initialized:{false,true})for(bool active:{false,true})for(auto deadline:{9u,10u,11u,0xffffffffu})for(int stop:{0,7}){
        rec::ManagerState m{initialized,active,99,88,-1200,0};Fake f;f.manager=&m;f.stopFault=stop;f.primaryStopFault=13;
        rec::Schedule32 r{123,456,deadline,-5000,42,43,4,5};std::uint32_t slot=42;
        const auto result=rec::disableAudio(f,m,{{&r,55,&slot}});
        const bool enabled=initialized && active,selected=deadline>10;
        std::string expected;if(enabled){if(deadline!=0xffffffffu)expected+='T';if(selected)expected+=stop?"Qs":"QsZ";expected+='S';}
        check(f.calls==expected && result==(enabled?13:0),"global disable selection and secondary error ordering");
        check(m.active==(enabled?false:active),"disable gate and pre-call flag clearing");
        check(enabled?(!r.voiceAddress && !r.deadline && !slot && r.x==-1 && r.y==-1 && r.next==123 && r.previous==456):r.voiceAddress==42,"record clearing retains list links");
    }
    for(bool initialized:{false,true})for(bool active:{false,true})for(int restore:{0,19}){
        rec::ManagerState m{initialized,active,99,88,-1200,0};Fake f;f.manager=&m;f.volumeFault=restore;f.primaryStopFault=29;
        rec::Schedule32 r{1,2,0xffffffffu,-5000,42,43,0,0};std::uint32_t slot=42;
        check(rec::shutdownAudio(f,m,{{&r,55,&slot}},{55,56})==0,"shutdown ignores restore/stop errors and returns zero");
        check(f.calls==(!initialized?"":active?"VDSRRE":"VRRE"),"restore precedes retirement, Stop and device release");
        if(initialized)check(!m.initialized && !m.device && f.lastVolume==-1200,"shutdown lifecycle fields");
        if(initialized && active)check(!slot && m.lastPrimaryStop==29,"shutdown retains primary error and clears slot");
    }
}
void nativePcm(){
    Device d;VoiceCommands commands(d);std::uint32_t w[16]{};
    auto rpc=[&](unsigned op,unsigned value=0){w[5]=op;w[7]=value;return commands.execute(w,{});};
    check(!rpc(MNM_AUDIO_PRIMARY_VOLUME,std::uint32_t(-2000)).status && rpc(MNM_AUDIO_PRIMARY_GET_VOLUME).value==std::uint32_t(-2000),"wire primary volume round trip");
    w[9]=22050;w[10]=2;w[11]=16;w[12]=4;w[13]=88200;w[14]=1;
    check(!rpc(MNM_AUDIO_PRIMARY_FORMAT).status && d.primaryState().format->rate==22050 && d.outputRate()==48000,"requested format independent of negotiated output clock");
    w[12]=2;check(rpc(MNM_AUDIO_PRIMARY_FORMAT).status && d.primaryState().format->alignment==4,"malformed primary format retains metadata");
    check(rpc(MNM_AUDIO_PRIMARY_PLAY,0).status && !rpc(MNM_AUDIO_PRIMARY_PLAY,1).status && rpc(MNM_AUDIO_PRIMARY_STATUS).value==5,"primary requires looping");
    Wave wav{{1,1,48000,96000,2,16,0},{0x20,0x4e}};
    auto uploaded=rec::uploadStatic(d,wav);check(uploaded.error==Error::ok,"upload");const auto id=uploaded.buffer;d.play(id,1);
    BufferId child=0;d.duplicate(id,child);d.play(child,1);
    std::vector<std::int16_t> pcm;d.mixStereo(1,pcm);check(pcm==std::vector<std::int16_t>({4000,4000}),"master applied to sum before clipping");
    rpc(MNM_AUDIO_PRIMARY_STOP);d.mixStereo(1,pcm);check(pcm[0]==4000 && !rpc(MNM_AUDIO_PRIMARY_STATUS).value && d.voice(id)->status()==5,"primary Stop leaves secondary mixing active");
    rpc(MNM_AUDIO_PRIMARY_VOLUME,std::uint32_t(-10000));d.mixStereo(1,pcm);check(pcm[0]==0 && d.voice(id)->status()==5,"master mute does not stop voices");
    check(rpc(MNM_AUDIO_PRIMARY_VOLUME,1).status && d.primaryState().volume==-10000,"invalid master volume retains state");
    rpc(MNM_AUDIO_PRIMARY_VOLUME,0);d.mixStereo(1,pcm);check(pcm[0]==32767,"restore master volume clips normally");
    d.release(id);d.release(child);d.stopPrimary();d.mixStereo(1,pcm);check(pcm[0]==0 && d.count()==0,"released voices leave silence");
}
int main(){try{managerMatrix();nativePcm();std::cout<<"Primary controls passed; manager gates/failure/order matrices and wire-to-PCM master gain\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
