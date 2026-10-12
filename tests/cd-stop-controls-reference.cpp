#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "cd_stop_controls.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace mnm::reconstruction::audio;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Event {
    std::uint32_t kind,a,b,c;std::vector<std::uint32_t> values;
    bool operator==(const Event& other)const{return kind==other.kind&&a==other.a&&b==other.b&&c==other.c&&values==other.values;}
};
struct Backend:CdStopBackend {
    std::vector<Event> events;
    std::uint32_t mode=0x20e,statusError=0,stopError=31,raw=0,mixerError=0;
    int failAt=-1,sets=0,tick=0;
    std::uint32_t invoke(std::uint32_t device,std::uint32_t message,std::uint32_t flags,std::uint32_t* data){
        events.push_back({0,device,message,flags,{data,data+(message==0x814?4:1)}});
        if(message==0x814){data[1]=mode;return statusError;}
        return stopError;
    }
    std::uint32_t command(std::uint32_t device,std::uint32_t message,std::uint32_t flags,std::array<std::uint32_t,4>& data)override{return invoke(device,message,flags,data.data());}
    std::int32_t firstMixerVolume(std::uint32_t mixer,std::uint32_t& value)override{
        events.push_back({1,mixer,0,0,{}});value=raw;return std::int32_t(mixerError);
    }
    std::int32_t volume(std::int32_t level)override{
        events.push_back({2,std::uint32_t(level),0,0,{}});return sets++==failAt?-17:0;
    }
    std::int32_t ticks()override{tick+=100;events.push_back({3,std::uint32_t(tick),0,0,{}});return tick;}
};
#ifdef ORIGINAL_REFERENCE
static Backend* script;
static std::uint32_t __attribute__((stdcall)) mci(std::uint32_t d,std::uint32_t m,std::uint32_t f,std::uint32_t* p){return script->invoke(d,m,f,p);}
static std::int32_t __attribute__((thiscall)) mixer(void* self,std::uint32_t* value){return script->firstMixerVolume(std::uint32_t(reinterpret_cast<std::uintptr_t>(self)),*value);}
static std::int32_t __attribute__((thiscall)) volume(void*,std::int32_t level){return script->volume(level);}
static std::int32_t clockTicks(){return script->ticks();}
static void jump(std::uint32_t address,std::uintptr_t target){
    auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;
    const auto offset=std::uint32_t(target-address-5);std::memcpy(p+1,&offset,4);
}
static void put(std::vector<unsigned char>& bytes,unsigned at,std::uint32_t value){std::memcpy(bytes.data()+at,&value,4);}
static std::vector<unsigned char> receiver(const CdStopState& state){
    std::vector<unsigned char> bytes(1056,0xa5);const unsigned base=16;
    put(bytes,base+0x32c,state.cd.initialized);put(bytes,base+0x328,state.cd.active);put(bytes,base+0x330,state.cd.paused);
    put(bytes,base+0x33c,state.cd.device);put(bytes,base+0x340,state.mixerCount);put(bytes,base+0x344,state.firstMixer);
    for(unsigned i=0;i<4;++i){put(bytes,base+0x368+4*i,state.cd.playback[i]);put(bytes,base+0x378+4*i,state.cd.status[i]);}
    return bytes;
}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    check(argc==2,"Expected pinned original executable");map_image(read(argv[1]));
    const Bytes body(reinterpret_cast<unsigned char*>(0x482010),reinterpret_cast<unsigned char*>(0x482186));
    global(0x5c52dc,reinterpret_cast<std::uintptr_t>(&mci));
    // Script only external callee boundaries. The complete stop caller is unchanged.
    jump(0x482ac0,reinterpret_cast<std::uintptr_t>(&mixer));jump(0x482190,reinterpret_cast<std::uintptr_t>(&volume));
    jump(0x59d342,reinterpret_cast<std::uintptr_t>(&clockTicks));
    using Stop=std::uint32_t(__attribute__((thiscall))*)(void*,std::uint32_t);
#else
    (void)argc;(void)argv;
#endif
    std::uint64_t cases=0,events=0,guards=0;
    for(auto initialized:{0u,1u,0xa5a5a5a5u})for(auto active:{0u,1u,0x80000000u})
    for(auto paused:{0u,0xffffffffu})for(auto fade:{0u,1u,0xffffffffu})for(unsigned scenario=0;scenario<8;++scenario)
    for(auto count:{0u,1u,5u})for(auto raw:{0u,1u,4368u,4369u,32768u,65535u})for(auto error:{0u,29u})for(auto failAt:{-1,0,2,20}){
        CdStopState state;state.cd.initialized=initialized;state.cd.active=active;state.cd.paused=paused;
        state.cd.device=0xabcdef01;state.cd.playback={0x11223344,0x55667788,0x99aabbcc,0x44332211};
        state.cd.status={0x99887766,0x88776655,0x77665544,0x66554433};state.mixerCount=count;state.firstMixer=0x12345678;
        Backend backend;const std::uint32_t modes[]{0x20e,0x212,0x20d,0x20c,0x211,0xffffffff,0x20d,0x211};
        backend.mode=modes[scenario];backend.statusError=scenario==6?17:scenario==7?0x80004005u:0;
        backend.raw=raw;backend.mixerError=error;backend.failAt=failAt;
        const auto before=state;const auto initial=backend;const auto result=stopCd(backend,state,fade);
        check(result==0,"stop caller always returns zero");
        const bool gated=!initialized||!active;
        const bool early=!backend.statusError&&(backend.mode==0x212||backend.mode==0x20d||backend.mode==0x20c);
        if(gated)check(backend.events.empty()&&state.cd.paused==paused&&state.cd.status==before.cd.status,"inactive gate mutation");
        else if(early)check(backend.events.size()==1&&state.cd.paused==paused,"mode early return must retain paused");
        else check(state.cd.paused==0,"stop clears pause even on STOP/volume error");
#ifdef ORIGINAL_REFERENCE
        auto bytes=receiver(before);const auto expected=receiver(state);auto original=initial;script=&original;
        const auto observed=reinterpret_cast<Stop>(0x482010)(bytes.data()+16,fade);
        check(observed==result,"original stop return differs");check(original.events==backend.events,"original stop/fade command/volume/clock order differs");
        check(bytes==expected,"original stop receiver/guard bytes differ");guards+=bytes.size();
#else
        (void)initial;
#endif
        ++cases;events+=backend.events.size();
    }
#ifdef ORIGINAL_REFERENCE
    check(!std::memcmp(reinterpret_cast<void*>(0x482010),body.data(),body.size()),"original stop caller instructions changed");
#endif
    check(cases==62208,"complete stop matrix");
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"events\":"<<events<<",\"guard_bytes\":"<<guards<<"}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
