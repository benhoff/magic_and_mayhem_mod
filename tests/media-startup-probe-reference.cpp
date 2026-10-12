// Unchanged original availability caller with scripted COM boundaries.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <limits>
#include "../reconstruction/audio/media_startup_probe.hpp"

static std::int32_t initializeResult,createResult;
static std::vector<unsigned> events;
static void check(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
static std::uint32_t __attribute__((stdcall)) release(void* object);
static std::uintptr_t table[3]={0,0,reinterpret_cast<std::uintptr_t>(&release)};
static std::uintptr_t object[2]={reinterpret_cast<std::uintptr_t>(table),0x12345678};
static std::int32_t __attribute__((stdcall)) initialize(void* reserved){
    check(!reserved,"CoInitialize reserved argument");events.push_back(1);return initializeResult;
}
static std::int32_t __attribute__((stdcall)) create(void* clsid,void* outer,std::uint32_t context,void* iid,void** output){
    check(clsid==reinterpret_cast<void*>(0x5c9330)&&iid==reinterpret_cast<void*>(0x5c9340),"FilterGraph/IGraphBuilder identities");
    check(!outer&&context==1&&output,"in-process nonaggregated creation");
    events.push_back(2);*output=object;return createResult;
}
static std::uint32_t __attribute__((stdcall)) release(void* value){
    check(value==object&&object[1]==0x12345678,"release identity/guard");events.push_back(3);return 0;
}
static void __attribute__((stdcall)) uninitialize(){events.push_back(4);}

int main(int argc,char** argv)try{
    check(argc==2,"Expected pinned original executable");map_image(read(argv[1]));
    const Bytes body(reinterpret_cast<unsigned char*>(0x483f30),reinterpret_cast<unsigned char*>(0x483f7b));
    global(0x5c5310,reinterpret_cast<std::uintptr_t>(&initialize));
    global(0x5c5318,reinterpret_cast<std::uintptr_t>(&create));
    global(0x5c5314,reinterpret_cast<std::uintptr_t>(&uninitialize));
    const std::array<std::int32_t,7> results{std::numeric_limits<std::int32_t>::min(),-2147467259,-1,0,1,17,std::numeric_limits<std::int32_t>::max()};
    unsigned cases=0,totalEvents=0;
    for(auto init:results)for(auto made:results){
        initializeResult=init;createResult=made;events.clear();
        const auto observed=reinterpret_cast<std::uint32_t(*)()>(0x483f30)();
        const bool admitted=init>=0,created=admitted&&made>=0;
        std::vector<unsigned> expected{1};
        if(admitted){expected.push_back(2);if(created)expected.push_back(3);expected.push_back(4);}
        check(observed==unsigned(mnm::reconstruction::audio::mediaStartupAvailable(init,made)),"native return gates differ");check(events==expected,"balanced COM call order");
        ++cases;totalEvents+=events.size();
    }
    check(!std::memcmp(reinterpret_cast<void*>(0x483f30),body.data(),body.size()),"original caller instructions changed");
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"events\":"<<totalEvents<<",\"caller_bytes\":"<<body.size()<<"}\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
