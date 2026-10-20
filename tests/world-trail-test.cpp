#ifdef MNM_ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#else
#include <iostream>
#include <stdexcept>
#include <vector>
#endif
#include "../reconstruction/rendering/world_trail.hpp"
using namespace mnm::reconstruction;
static std::vector<WorldTrailStamp> stamps;
static std::uint32_t capture(WorldTrailStamp request) {
    stamps.push_back(request);
    return 0x87650000u + unsigned(stamps.size());
}
#ifdef MNM_ORIGINAL_REFERENCE
static std::uint32_t __attribute__((fastcall)) raster(void* source, int x, int y, int shade, int ordinal) {
    return capture({std::uint32_t(reinterpret_cast<std::uintptr_t>(source)),x,y,shade,ordinal});
}
#endif
int main(int argc,char** argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::invalid_argument("Expected pinned original PE");
    map_image(read(argv[1]));
    auto* entry=reinterpret_cast<unsigned char*>(0x580a90);entry[0]=0xe9;
    auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(&raster)-0x580a95);
    std::memcpy(entry+1,&relative,4);
    auto* helper=reinterpret_cast<const unsigned char*>(0x4df840);
    const Bytes before(helper,helper+0x15f);
    auto* consumer=reinterpret_cast<const unsigned char*>(0x5002a0);
    const Bytes consumerBefore(consumer,consumer+(0x501181-0x5002a0));
#else
    (void)argv;if(argc!=1)throw std::invalid_argument("Expected no native inputs");
#endif
    unsigned cases=0;
    for(int x1=-3;x1<=3;++x1)for(int y1=-3;y1<=3;++y1)
    for(int x2=-8;x2<=8;++x2)for(int y2=-8;y2<=8;++y2)
    for(int shade:{-127,0,3})for(unsigned phase:{0u,1u,2u}) {
        stamps.clear();unsigned result;
#ifdef MNM_ORIGINAL_REFERENCE
        using Method=std::uint32_t (__attribute__((fastcall)) *)(int,int,int,int,void*,int,int);
        if(!phase)result=reinterpret_cast<Method>(0x4df840)(x1,y1,x2,y2,reinterpret_cast<void*>(42),shade,-32768);
        else {
            std::uint32_t points[5]={std::uint32_t(x1),std::uint32_t(y1),std::uint32_t(x2),std::uint32_t(y2),0};
            std::uint32_t row[9]={0,std::uint32_t(reinterpret_cast<std::uintptr_t>(points)),42,0x12345678,
                std::uint16_t(shade)|(std::uint32_t(0x8000)<<16),0,9,0x8ad08ad0,0};
            const std::vector<std::uint32_t> rowBefore(row,row+9);
            std::uint32_t queue[6]={std::uint32_t(reinterpret_cast<std::uintptr_t>(row)),0,1,1,0,0};
            const std::vector<std::uint32_t> queueBefore(queue,queue+6);
            global(0x5e1404,phase==1);global(0x6e1f24,0);
            using Consumer=void (__attribute__((thiscall)) *)(void*);
            reinterpret_cast<Consumer>(0x5002a0)(queue);
            result=row[7]>>16;
            auto expected=rowBefore;expected[7]=0x8ad0;
            if(std::vector<std::uint32_t>(row,row+9)!=expected ||
               std::vector<std::uint32_t>(queue,queue+6)!=queueBefore)
                throw std::runtime_error("Kind9 queue row/container effects differ");
            if(Bytes(consumer,consumer+consumerBefore.size())!=consumerBefore)
                throw std::runtime_error("Original queue instructions changed");
        }
        if(Bytes(helper,helper+before.size())!=before)throw std::runtime_error("Original trail instructions changed");
#else
        if(!phase)result=drawWorldTrail({x1,y1,x2,y2,42,shade,-32768},32,capture);
        else {
            const QueueRow row{9,99,42,0x12345678,std::int16_t(shade),-32768,false,
                {std::uint32_t(x1),std::uint32_t(y1),std::uint32_t(x2),std::uint32_t(y2)}};
            auto metadata=dispatchQueueRow(phase==1?QueueTable::primary:QueueTable::software,row,
                [](const QueueCall& call) {return drawWorldTrail(worldTrailForQueueCall(call),32,capture);});
            if(!metadata)throw std::runtime_error("Kind9 queue omitted metadata");
            result=*metadata;
        }
#endif
        std::cout<<"["<<phase<<','<<x1<<','<<y1<<','<<x2<<','<<y2<<','<<shade<<','<<result<<",[";
        for(unsigned i=0;i<stamps.size();++i) {
            if(i)std::cout<<',';
            const auto& s=stamps[i];std::cout<<'['<<s.source<<','<<s.x<<','<<s.y<<','<<s.shade<<','<<s.ordinal<<']';
        }
        std::cout<<"]]\n";++cases;
    }
    if(cases!=127449)throw std::runtime_error("Incomplete trail matrix");
#ifndef MNM_ORIGINAL_REFERENCE
    stamps.clear();bool refused=false;
    try {drawWorldTrail({0,0,10,0,42,0,0},10,capture);}
    catch(const std::invalid_argument&) {refused=true;}
    if(!refused||!stamps.empty())throw std::runtime_error("Oversized trail did not refuse atomically");
    refused=false;
    try {drawWorldTrail({INT32_MIN,0,INT32_MAX,0,42,0,0},UINT32_MAX,capture);}
    catch(const std::invalid_argument&) {refused=true;}
    if(!refused||!stamps.empty())throw std::runtime_error("Overflow-domain trail admitted");
    for(const auto& call:{QueueCall{QueueBackend::copy,{},7},QueueCall{QueueBackend::objectHelper,{},6}}) {
        refused=false;
        try {drawWorldTrail(worldTrailForQueueCall(call),32,capture);}
        catch(const std::invalid_argument&) {refused=true;}
        if(!refused||!stamps.empty())throw std::runtime_error("Malformed trail call admitted");
    }
#endif
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
