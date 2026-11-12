// Compile twice: unchanged original PE32 and a separate portable reconstruction.
// All scripted endpoint results and mutations are fixture inputs, never pixels.
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#ifdef MNM_NATIVE_MODEL
#include "../reconstruction/rendering/surface_copy_retry_geometry.hpp"
#include "../reconstruction/rendering/surface_retry.hpp"
#else
#include <sys/mman.h>
#endif
using U=std::uint32_t;using I=std::int32_t;
struct Rect {I l,t,r,b;};struct Point {I x,y;};
struct Object {std::array<U,16> words{};};
struct Interface {void** table;U id;};
static std::vector<std::vector<std::int64_t>> events;
static U fastMode,noWait,keyEnabled,windowMode,mutation,attempt,losses,restoreFailure,keyResult;
static Rect sourceRect;static Object sourceObject,targetObject;
static Interface surfaces[3],windowInterface;
static bool keyed;
#ifdef MNM_NATIVE_MODEL
static mnm::reconstruction::SurfaceCopyRetryGeometry* geometry;
#else
// Original Win32 callers guarantee four-byte stack alignment. Realign before
// entering compiler-generated C++/SSE code in the recording endpoints.
#define ABI __attribute__((stdcall,force_align_arg_pointer))
#endif
#ifndef ABI
#define ABI
#endif
static void publish() {
#ifndef MNM_NATIVE_MODEL
    *reinterpret_cast<U*>(0x6f68d8)=fastMode;*reinterpret_cast<U*>(0x6f98d8)=noWait;
    *reinterpret_cast<U*>(0x6de6dd)=keyEnabled;
    *reinterpret_cast<Object**>(0x6f98c0)=windowMode ? &targetObject : nullptr;
    *reinterpret_cast<Interface**>(0x6f98d0)=windowMode==1 ? &windowInterface : nullptr;
#endif
}
static void mutate() {
    if(mutation&1) fastMode^=1;
    if(mutation&2) noWait^=1;
    if(mutation&4) {windowMode=windowMode ? 0 : 1;sourceRect={-3,2,7,8};}
    publish();
}
static U draw(U op,Rect destination,Rect source,U flags) {
    if(attempt>=3) throw std::runtime_error("Original draw budget exceeded");
    const U result=attempt<losses ? (losses==2 && attempt==1 ? 0x887601aeu : 0x887601c2u) : 0;
    events.push_back({op,result,flags,source.l,source.t,source.r,source.b,
                      destination.l,destination.t,destination.r,destination.b});
    ++attempt;if(result && attempt==1) mutate();return result;
}
static I ABI blt(void* receiver,Rect* destination,void* source,Rect* rect,U flags,void* effects) {
    if(receiver!=&surfaces[2] || source!=&surfaces[1] || effects) throw std::runtime_error("COM copy identity");
    return draw(1,*destination,*rect,flags);
}
static I ABI fast(void* receiver,I x,I y,void* source,Rect* rect,U flags) {
    if(receiver!=&surfaces[2] || source!=&surfaces[1]) throw std::runtime_error("COM fast identity");
    return draw(2,{x,y,0,0},*rect,flags);
}
static I ABI restore(void* receiver) {
    const U id=static_cast<Interface*>(receiver)->id;
    const U result=(restoreFailure&(1u<<(id-1))) ? 0x80004005u : 0;
    events.push_back({3,id,result});return result;
}
static I ABI setKey(void* receiver,U flags,U* range) {
    const U id=static_cast<Interface*>(receiver)->id;
    if(flags!=8 || range[0]!=range[1]) throw std::runtime_error("COM key arguments");
    events.push_back({4,id,range[0],keyResult});return keyResult;
}
static I ABI getWindow(void*,U* hwnd) {*hwnd=0x1234;events.push_back({5});return 0;}
static I ABI clientToScreen(U hwnd,Point* p) {
    events.push_back({6,hwnd});if(hwnd==0x1234){p->x=-19;p->y=33;return 1;}return 0;
}
static I ABI message(void*,const char*,const char*,U) {events.push_back({7});return 1;}
static I ABI destroy(void*) {events.push_back({8});return 0;}
#ifndef MNM_NATIVE_MODEL
static U word(const std::vector<unsigned char>& b,std::size_t at) {
    if(at>b.size() || b.size()-at<4)throw std::runtime_error("PE read extent");
    U value;std::memcpy(&value,b.data()+at,4);return value;
}
static void mapImage(const char* path) {
    std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Original PE input");
    const std::vector<unsigned char> b((std::istreambuf_iterator<char>(f)),{});
    const U pe=word(b,60),opt=pe+24,length=word(b,opt+56);
    if(word(b,opt+28)!=0x400000 || length>32*1024*1024)throw std::runtime_error("Unsupported image");
    auto* p=static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000),length,
        PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
    if(p==MAP_FAILED)throw std::runtime_error("Original private mapping");
    const U count=b.at(pe+6)|(U(b.at(pe+7))<<8),size=b.at(pe+20)|(U(b.at(pe+21))<<8);
    for(U n=0;n<count;++n){const U at=opt+size+n*40,rva=word(b,at+12),bytes=word(b,at+16),raw=word(b,at+20);
        if(std::uint64_t(raw)+bytes>b.size() || std::uint64_t(rva)+bytes>length)throw std::runtime_error("PE section extent");
        std::memcpy(p+rva,b.data()+raw,bytes);
    }
    for(U address:{0x58c4a0u,0x58c8a0u})
        if(*reinterpret_cast<unsigned char*>(address)!=0x83 || *reinterpret_cast<unsigned char*>(address+1)!=0xec)
            throw std::runtime_error("Original wrapper anchor");
    *reinterpret_cast<void**>(0x5c51fc)=reinterpret_cast<void*>(&clientToScreen);
    *reinterpret_cast<void**>(0x5c520c)=reinterpret_cast<void*>(&message);
    *reinterpret_cast<void**>(0x5c5208)=reinterpret_cast<void*>(&destroy);
    *reinterpret_cast<U*>(0x656614)=0;
}
#else
struct Adapter:mnm::reconstruction::SurfaceRetryAdapter {
    U draw(bool,bool,U,std::uint16_t) override {
        const auto p=geometry->draw({sourceRect.l,sourceRect.t,sourceRect.r,sourceRect.b},fastMode,noWait);
        return ::draw(p.fast?2:1,{p.destination.left,p.destination.top,p.destination.right,p.destination.bottom},sourceRect,p.flags);
    }
    U restore(mnm::reconstruction::RecoverySurface id) override {return ::restore(&surfaces[U(id)]);}
    U setSourceKey(mnm::reconstruction::RecoverySurface id,std::uint16_t value) override {
        U range[2]={value,value};return setKey(&surfaces[U(id)],8,range);
    }
    void reload(mnm::reconstruction::RecoverySurface) override {throw std::runtime_error("Unexpected reload");}
    void report(U result) override {if(result && result!=0x887601c2u){message(nullptr,nullptr,nullptr,0);destroy(nullptr);}}
};
#endif
int main(int argc,char** argv)try {
    if(argc!=3)throw std::runtime_error("Expected pinned PE and input path");
#ifndef MNM_NATIVE_MODEL
    mapImage(argv[1]);
#endif
    std::array<void*,33> table{};table[5]=reinterpret_cast<void*>(&blt);table[7]=reinterpret_cast<void*>(&fast);
    table[27]=reinterpret_cast<void*>(&restore);table[29]=reinterpret_cast<void*>(&setKey);
    std::array<void*,5> windowTable{};windowTable[4]=reinterpret_cast<void*>(&getWindow);
    surfaces[1]={table.data(),1};surfaces[2]={table.data(),2};windowInterface={windowTable.data(),0};
    std::ifstream inputs(argv[2]);if(!inputs)throw std::runtime_error("Inputs");
    U id,entry;I x,y;bool first=true;std::cout<<'[';
    while(inputs>>id>>entry>>fastMode>>noWait>>windowMode>>keyEnabled>>losses>>restoreFailure>>keyResult>>mutation
          >>sourceRect.l>>sourceRect.t>>sourceRect.r>>sourceRect.b>>x>>y) {
        if((entry!=0x58c4a0 && entry!=0x58c8a0) || losses>2 || mutation>7 || restoreFailure>3 || windowMode>2)
            throw std::runtime_error("Unsupported fixture");
        keyed=entry==0x58c8a0;events.clear();attempt=0;sourceObject={};targetObject={};
        sourceObject.words[11]=0x12345678;targetObject.words[11]=0xabcde123;
#ifndef MNM_NATIVE_MODEL
        sourceObject.words[2]=reinterpret_cast<U>(&surfaces[1]);targetObject.words[2]=reinterpret_cast<U>(&surfaces[2]);publish();
        using Copy=void(__attribute__((thiscall))*)(Object*,Object*,Rect*,I,I);
        reinterpret_cast<Copy>(entry)(&sourceObject,&targetObject,&sourceRect,x,y);
#else
        using namespace mnm::reconstruction;
        const bool translated=!fastMode && windowMode;
        if(translated){if(windowMode==1){U hwnd;getWindow(nullptr,&hwnd);}Point p{};clientToScreen(windowMode==1?0x1234:0,&p);}
        SurfaceCopyRetryGeometry state(keyed?SurfaceCopyWrapper::keyedRecovering:SurfaceCopyWrapper::opaqueRecovering,
            {sourceRect.l,sourceRect.t,sourceRect.r,sourceRect.b},x,y,
            {bool(fastMode),bool(noWait),bool(windowMode),windowMode==1?-19:0,windowMode==1?33:0});
        geometry=&state;SurfaceRetryInput config;config.wrapper=keyed?SurfaceWrapper::KeyedCopy:SurfaceWrapper::OpaqueCopy;
        config.keyEnabled=keyEnabled;config.sourceKey=0x5678;config.destinationKey=0xe123;
        Adapter adapter;const auto result=runSurfaceRetry(config,adapter,3);
        if(!result.returned || result.budgetExhausted)throw std::runtime_error("Model incomplete");
#endif
        if(!first)std::cout<<',';
        first=false;
        std::cout<<"{\"id\":"<<id<<",\"events\":[";bool firstEvent=true;
        for(const auto& event:events){if(!firstEvent)std::cout<<',';firstEvent=false;std::cout<<'[';
            for(std::size_t n=0;n<event.size();++n){if(n)std::cout<<',';std::cout<<event[n];}std::cout<<']';}
        std::cout<<"],\"source_after\":["<<sourceRect.l<<','<<sourceRect.t<<','<<sourceRect.r<<','<<sourceRect.b<<"]}";
    }
    if(!inputs.eof())throw std::runtime_error("Malformed fixture input");
    std::cout<<"]\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
