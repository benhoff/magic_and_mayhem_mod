#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "cd_track_controls.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace mnm::reconstruction::audio;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct Call {
    std::uint32_t device,message,flags;std::vector<std::uint32_t> parameters;
    bool operator==(const Call& other)const{return device==other.device&&message==other.message&&flags==other.flags&&parameters==other.parameters;}
};
struct Backend:CdCommandBackend {
    std::vector<Call> calls;
    std::uint32_t first=0,statusError=0,statusValue=0,retry=0;
    std::uint32_t invoke(std::uint32_t device,std::uint32_t message,std::uint32_t flags,std::uint32_t* parameters){
        const unsigned count=message==0x806?3:message==0x814?4:1;
        calls.push_back({device,message,flags,{parameters,parameters+count}});
        if(message==0x814){parameters[1]=statusValue;return statusError;}
        return calls.size()==1?first:retry;
    }
    std::uint32_t command(std::uint32_t device,std::uint32_t message,std::uint32_t flags,std::array<std::uint32_t,4>& parameters)override{return invoke(device,message,flags,parameters.data());}
};
#ifdef ORIGINAL_REFERENCE
static Backend* originalBackend;
static std::uint32_t __attribute__((stdcall)) mci(std::uint32_t device,std::uint32_t message,std::uint32_t flags,std::uint32_t* parameters){return originalBackend->invoke(device,message,flags,parameters);}
static void put(std::vector<unsigned char>& bytes,unsigned at,std::uint32_t value){std::memcpy(bytes.data()+at,&value,4);}
static std::vector<unsigned char> receiver(const CdTrackState& state){
    std::vector<unsigned char> bytes(1024+32,0xa5);const unsigned base=16;
    put(bytes,base+0x32c,state.initialized);put(bytes,base+0x328,state.active);put(bytes,base+0x330,state.paused);put(bytes,base+0x33c,state.device);
    for(unsigned i=0;i<4;++i){put(bytes,base+0x368+i*4,state.playback[i]);put(bytes,base+0x378+i*4,state.status[i]);}
    return bytes;
}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    check(argc==2,"Expected pinned executable");map_image(read(argv[1]));global(0x5c52dc,reinterpret_cast<std::uintptr_t>(&mci));
    const std::vector<std::pair<std::uint32_t,std::uint32_t>> ranges{{0x481dc0,0x481e8b},{0x481ed0,0x481f21},{0x481930,0x4819a5},{0x481e90,0x481ecb}};
    std::vector<Bytes> instructions;for(auto range:ranges)instructions.emplace_back(reinterpret_cast<unsigned char*>(range.first),reinterpret_cast<unsigned char*>(range.second));
    using Arg=std::uint32_t(__attribute__((thiscall)) *)(void*,std::uint32_t);
    using NoArg=std::uint32_t(__attribute__((thiscall)) *)(void*);
#else
    (void)argc;(void)argv;
#endif
    std::uint64_t cases=0,calls=0,retrySuccessPreservedPause=0,guardBytes=0;
    for(unsigned operation=0;operation<4;++operation)
    for(auto initialized:{0u,1u,0xa5a5a5a5u})for(auto active:{0u,1u,0x80000000u})for(auto paused:{0u,1u,0xffffffffu})
    for(auto track:{0u,1u,2u,13u,255u,256u,0xffffffffu,0x12345678u})for(unsigned scenario=0;scenario<8;++scenario){
        CdTrackState state;state.initialized=initialized;state.active=active;state.paused=paused;state.device=0xabcdef01;
        state.playback={0x11223344,0x55667788,0x99aabbcc,0x44332211};state.status={0x99887766,0x88776655,0x77665544,0x66554433};
        Backend backend;backend.first=scenario==0?0:scenario==7?0x80004005u:17;
        backend.statusError=scenario==1?29:0;backend.statusValue=scenario==2?track+1:track;
        backend.retry=scenario==3?33:scenario==4?0x88780096u:0;
        if(operation==3){backend.statusError=scenario==7?0x80004005u:scenario%2?17:0;backend.statusValue=track;}
        const auto before=state;const auto initialBackend=backend;
        const auto result=operation==0?playCdTrack(backend,state,track):operation==1?playCdFromTrack(backend,state,track):operation==2?pauseResumeCd(backend,state):cdLengthByte(backend,state);
        if(operation<3&&(!initialized||!active)){check(!result&&backend.calls.empty(),"inactive gate");check(state.playback==before.playback&&state.status==before.status&&state.paused==before.paused,"inactive state changed");}
        if(operation==0&&initialized&&active){
            check(state.playback[1]==(track&255u)&&state.playback[2]==((track+1u)&255u),"track byte arithmetic");
            if(scenario==0)check(!result&&!state.paused&&backend.calls.size()==1,"successful play clears pause");
            if(scenario==5&&paused){check(!result&&state.paused==paused&&backend.calls.size()==3,"retry success must preserve original pause annotation");++retrySuccessPreservedPause;}
        }
        if(operation==3){check(backend.calls.size()==1&&backend.calls[0].message==0x814&&backend.calls[0].flags==0x102&&state.status[2]==1,"length query bypasses active gate");check(result==(backend.statusError?0:track&255u),"length byte policy");}
#ifdef ORIGINAL_REFERENCE
        auto bytes=receiver(before);const auto expected=receiver(state);Backend original=initialBackend;originalBackend=&original;
        const auto originalResult=operation==0?reinterpret_cast<Arg>(0x481dc0)(bytes.data()+16,track):operation==1?reinterpret_cast<Arg>(0x481ed0)(bytes.data()+16,track):operation==2?reinterpret_cast<NoArg>(0x481930)(bytes.data()+16):reinterpret_cast<NoArg>(0x481e90)(bytes.data()+16);
        check(originalResult==result,"original return differs");check(original.calls==backend.calls,"original MCI command/argument order differs");check(bytes==expected,"original receiver/guard mutations differ");guardBytes+=bytes.size();
#else
        (void)initialBackend;
#endif
        ++cases;calls+=backend.calls.size();
    }
#ifdef ORIGINAL_REFERENCE
    for(unsigned i=0;i<ranges.size();++i)check(!std::memcmp(reinterpret_cast<void*>(ranges[i].first),instructions[i].data(),instructions[i].size()),"original control instructions changed");
#endif
    check(cases==6912&&retrySuccessPreservedPause==64,"complete matrix");
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"calls\":"<<calls<<",\"retry_success_preserved_pause\":"<<retrySuccessPreservedPause<<",\"guard_bytes\":"<<guardBytes<<"}\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
