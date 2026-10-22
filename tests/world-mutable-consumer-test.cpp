// Compare complete queue-envelope state with unchanged pinned PE32 instructions.
// Mutable envelope: raster/finalizer callbacks are recording boundaries, not pixel or lifetime
// oracles. The clip setter's exact original body runs from a private copy.
#include "../reconstruction/rendering/world_mutable_consumer.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif

using namespace mnm::reconstruction;
using U = std::uint32_t;
struct Event { U tag; std::vector<U> values; ConsumerClip clip; };
static std::vector<Event> events;
static U resultValue;
static ConsumerState current;
static unsigned mutationMode,callbackNumber;
static U activeCount;
static bool rebound;
static std::vector<ConsumerRow>* ownedRows;
[[maybe_unused]] static MutableWorldQueue* mutableQueue;
static void mutate(U tag);
static const U tags[]{0x595677, 0x689b7c, 0x57de00, 0x6903dc, 0x57e540,
    0x57e8e0, 0x57f5f0, 0x57ec90, 0x57f0f0, 0x689b78, 0x57fe10,
    0x580a90, 0x580260, 0x581200, 0x5806f0, 0x8000000c, 0x4df840};
static void add(U tag, std::vector<U> values) {
    events.push_back({tag, std::move(values), current.active});
    mutate(tag);
}
#ifdef MNM_ORIGINAL_REFERENCE
static U object[20], vtable[4];
static U* originalRows;static U* originalOther;static U* originalQueue;static U* originalBase;
static void* clipBody;
static U pointer(const void* p) { return reinterpret_cast<std::uintptr_t>(p); }
extern "C" __attribute__((force_align_arg_pointer)) void capture_consumer(U tag, U* regs) {
    const U ecx = regs[6], edx = regs[5]; const U* args = regs + 10;
    std::vector<U> values;
    if (tag == 0x57dc60) {
        using Set = void (__attribute__((fastcall)) *)(U, U, U, U);
        reinterpret_cast<Set>(clipBody)(ecx, edx, args[0], args[1]);
        current.active = {std::int32_t(*reinterpret_cast<U*>(0x6e0008)),
            std::int32_t(*reinterpret_cast<U*>(0x6cbb6c)),
            std::int32_t(*reinterpret_cast<U*>(0x6a49b8)),
            std::int32_t(*reinterpret_cast<U*>(0x656618))};
        values = {edx, ecx, args[0], args[1]};
    } else if (tag == 0x4d8ab0) {
        if (ecx != 0x6ddf78) throw std::runtime_error("Wrong finalizer receiver");
        values = {99, args[0], args[1], args[2]};
    } else if (tag == 0x4df840) {
        if (ecx != object[0]) throw std::runtime_error("Wrong concrete helper receiver");
        values = {77, edx, args[0], args[1], args[2], args[3], args[4]};
    } else if (tag == 0x8000000c) {
        if (ecx != pointer(object)) throw std::runtime_error("Wrong virtual receiver");
        values = {42, args[0], args[1]};
    } else if (tag == 0x595677 || tag == 0x6c4e3c || tag == 0x689b7c ||
               tag == 0x57de00 || tag == 0x6903dc || tag == 0x689b78) {
        if (args[0] != pointer(object)) throw std::runtime_error("Wrong stack source");
        const unsigned count = tag == 0x6c4e3c || tag == 0x689b7c || tag == 0x57de00 ? 5 : 3;
        values = {42}; for (unsigned i=1; i<count; ++i) values.push_back(args[i]);
        if (tag == 0x6c4e3c) tag = 0x595677;
    } else {
        if (ecx != pointer(object)) throw std::runtime_error("Wrong register source");
        const unsigned count = tag == 0x57e540 || tag == 0x8000000c ? 1 :
            tag == 0x5806f0 ? 2 : tag == 0x581200 ? 5 : 3;
        values = {42, edx}; for (unsigned i=0; i<count; ++i) values.push_back(args[i]);
    }
    add(tag, std::move(values)); regs[7] = resultValue;
}
static void* stub(U tag, unsigned pop) {
    auto* code = static_cast<unsigned char*>(mmap(nullptr,4096,
        PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if (code == MAP_FAILED) throw std::runtime_error("Stub allocation failed");
    const unsigned char head[]{0x9c,0x60,0x89,0xe0,0x50,0x68};
    std::memcpy(code,head,sizeof(head)); std::memcpy(code+6,&tag,4); code[10]=0xe8;
    const U rel=pointer(reinterpret_cast<void*>(&capture_consumer))-pointer(code+15);
    std::memcpy(code+11,&rel,4);
    const unsigned char tail[]{0x83,0xc4,0x08,0x61,0x9d,0xc2};
    std::memcpy(code+15,tail,sizeof(tail)); const std::uint16_t n=pop;
    std::memcpy(code+21,&n,2); return code;
}
static void bind(U at, unsigned pop) {
    auto* dest=static_cast<unsigned char*>(stub(at,pop));
    auto* entry=reinterpret_cast<unsigned char*>(at); const U rel=pointer(dest)-at-5;
    entry[0]=0xe9; std::memcpy(entry+1,&rel,4);
}
static Bytes consumerBytes;
static void prepare(const char* executable) {
    map_image(read(executable));
    const auto* entry=reinterpret_cast<const unsigned char*>(0x5002a0);
    consumerBytes.assign(entry,entry+0xee1);
    clipBody=mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if (clipBody == MAP_FAILED) throw std::runtime_error("Clip allocation failed");
    std::memcpy(clipBody,reinterpret_cast<void*>(0x57dc60),34);
    if (static_cast<unsigned char*>(clipBody)[31]!=0xc2)
        throw std::runtime_error("Clip setter signature failed");
    for (auto item: std::vector<std::pair<U,unsigned>>{
        {0x4df840,20},{0x57dc60,8},{0x57de00,0},{0x57e540,4},{0x57e8e0,12},
        {0x57ec90,12},{0x57f0f0,12},{0x57f5f0,12},{0x57fe10,12},{0x580260,12},
        {0x5806f0,8},{0x580a90,12},{0x581200,20},{0x595677,0},{0x4d8ab0,12}})
        bind(item.first,item.second);
    for (U at: {0x689b78u,0x689b7cu,0x6c4e3cu,0x6903dcu}) global(at,pointer(stub(at,0)));
    vtable[3]=pointer(stub(0x8000000c,8));
}
static void original(std::vector<ConsumerRow>& rows) {
    U raw[4][9]{},other[4][9]{}; U base[]{U(current.base.left),U(current.base.top),
        U(current.base.right),U(current.base.bottom)};
    global(0x6c482c,pointer(base)); global(0x6e1f70,current.padding);
    global(0x5e1404,current.hardware);global(0x6de6d5,current.tallVideo);
    global(0x6e1f24,current.finalize);
    global(0x6e8db0,current.finalArguments[0]);global(0x6e8db4,current.finalArguments[1]);
    global(0x6e8db8,current.finalArguments[2]);
    global(0x6e0008,U(current.active.left));global(0x6cbb6c,U(current.active.top));
    global(0x6a49b8,U(current.active.right));global(0x656618,U(current.active.bottom));
    object[0]=pointer(vtable);object[1]=16;object[2]=rows.empty()?16:rows[0].sourceHeight;
    object[3]=3;object[4]=rows.empty()?4:rows[0].sourceOriginY;
    object[7]=rows.empty()||rows[0].draw.indexed?0:UINT32_MAX;
    for (unsigned i=0;i<rows.size();++i) {
        const auto& r=rows[i];raw[i][1]=pointer(object);raw[i][2]=r.draw.x;raw[i][3]=r.draw.y;
        const std::int16_t offsets[]{r.draw.offsetX,r.draw.offsetY};
        std::memcpy(raw[i]+4,offsets,4);raw[i][6]=U(r.draw.kind);
        const std::uint16_t markers[]{r.auxiliaryMarker,r.metadata};
        std::memcpy(raw[i]+7,markers,4);
    }
    std::memcpy(other,raw,sizeof(raw));for(unsigned i=0;i<4;++i)other[i][2]+=1000;
    U queue[]{pointer(raw),0,activeCount,4,0,0};
    originalRows=raw[0];originalOther=other[0];originalQueue=queue;originalBase=base;
    using Consume=void (__attribute__((thiscall)) *)(void*);
    reinterpret_cast<Consume>(0x5002a0)(queue);
    for (unsigned i=0;i<rows.size();++i) {
        rows[i].draw.y=raw[i][3];rows[i].metadata=std::uint16_t(raw[i][7]>>16);
        rows[i].auxiliaryMarker=std::uint16_t(raw[i][7]);
    }
    activeCount=queue[2];
    current.finalArguments={*reinterpret_cast<U*>(0x6e8db0),
        *reinterpret_cast<U*>(0x6e8db4),*reinterpret_cast<U*>(0x6e8db8)};
    if (!std::equal(consumerBytes.begin(),consumerBytes.end(),
            reinterpret_cast<unsigned char*>(0x5002a0)))
        throw std::runtime_error("Original consumer instructions changed");
}
#endif
static void mutate(U tag) {
    const bool raster=tag!=0x57dc60&&tag!=0x4d8ab0;
    const bool eligible=mutationMode==11?tag==0x57dc60:mutationMode==12?tag==0x4d8ab0:raster;
    if(!eligible||callbackNumber++)return;
#ifdef MNM_ORIGINAL_REFERENCE
    for(unsigned i=0;i<4;++i){
        auto& row=ownedRows->at(i);const auto* raw=originalRows+i*9;
        row.draw.x=raw[2];row.draw.y=raw[3];row.draw.kind=std::int32_t(raw[6]);
        row.metadata=std::uint16_t(raw[7]>>16);row.auxiliaryMarker=std::uint16_t(raw[7]);
    }
#endif
    auto& row=ownedRows->at(0);
    switch(mutationMode){
    case 0:break;
    case 1:activeCount=3;break;
    case 2:activeCount=1;break;
    case 3:activeCount=0;break;
    case 4:rebound=true;activeCount=3;break;
    case 5:current.hardware=!current.hardware;break;
    case 6:current.tallVideo=!current.tallVideo;current.base={1,4,250,590};current.padding=11;current.finalize=true;current.finalArguments={44,55,66};break;
    case 7:row.draw.kind=4;row.draw.x=13;row.draw.y=37;row.draw.offsetX=7;row.draw.offsetY=2;row.auxiliaryMarker=7;row.metadata=13;break;
    case 8:for(auto& source:*ownedRows){source.sourceHeight=23;source.sourceOriginY=5;source.draw.objectWords[2]=23;}break;
    case 9:ownedRows->at(1).draw.kind=35;ownedRows->at(1).metadata=0x5678;break;
    case 10:row.metadata=0xbeef;break;
    case 11:row.draw.kind=3;row.draw.x=17;row.draw.y=21;row.metadata=0x8ad0;row.auxiliaryMarker=0x8ad0;current.padding=3;break;
    case 12:current.finalArguments={77,88,99};break;
    default:throw std::runtime_error("Mutation mode");
    }
#ifdef MNM_ORIGINAL_REFERENCE
    for(unsigned i=0;i<4;++i){const auto& r=ownedRows->at(i);auto* raw=originalRows+i*9;
        raw[2]=r.draw.x;raw[3]=r.draw.y;raw[6]=U(r.draw.kind);
        const std::int16_t offsets[]{r.draw.offsetX,r.draw.offsetY};std::memcpy(raw+4,offsets,4);
        raw[7]=U(r.auxiliaryMarker)|(U(r.metadata)<<16);
    }
    object[2]=ownedRows->at(0).sourceHeight;object[4]=ownedRows->at(0).sourceOriginY;
    originalQueue[0]=pointer(rebound?originalOther:originalRows);originalQueue[2]=activeCount;
    originalBase[0]=U(current.base.left);originalBase[1]=U(current.base.top);originalBase[2]=U(current.base.right);originalBase[3]=U(current.base.bottom);
    global(0x6e1f70,current.padding);global(0x5e1404,current.hardware);global(0x6de6d5,current.tallVideo);global(0x6e1f24,current.finalize);
    global(0x6e8db0,current.finalArguments[0]);global(0x6e8db4,current.finalArguments[1]);global(0x6e8db8,current.finalArguments[2]);
#else
    mutableQueue->count=activeCount;
    if(rebound){mutableQueue->backing=std::make_shared<std::vector<ConsumerRow>>(*ownedRows);for(auto& r:*mutableQueue->backing)r.draw.x+=1000;}
#endif
}
static void printClip(const ConsumerClip& c) {
    std::cout<<'['<<c.left<<','<<c.top<<','<<c.right<<','<<c.bottom<<']';
}
int main(int argc, char** argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
    if (argc!=2) throw std::runtime_error("Expected pinned executable");
    prepare(argv[1]);
#else
    (void)argv; if (argc!=1) throw std::runtime_error("Unexpected native inputs");
#endif
    const std::array<std::array<std::uint16_t,2>,3> markers{{{{0x8ad0,0x8ad0}},{{5,0x8ad0}},{{5,7}}}};
    unsigned id=0;
    for (bool video:{false,true}) for (U padding:{0u,7u}) for(const auto& marker:markers)
    for(int kind=-2;kind<=35;++kind)for(U value:{0u,1u})for(unsigned count:{1u,3u})
    for(unsigned mutation=0;mutation<13;++mutation){
        const bool tall=false,finish=true;
        mutationMode=mutation;callbackNumber=0;activeCount=count;rebound=false;
        events.clear();resultValue=value;
        current={video,tall,finish,padding,{-3,2,320,tall?600:480},{-9,-8,27,26},{11,22,33}};
        std::vector<ConsumerRow> rows;
        for (unsigned i=0;i<4;++i) {
            const U height=16,origin=4;
            QueueRow draw{kind,42,120+19*i, i==0?100u:i==1?0xfffffff0u:479u,
                          std::int16_t(id%2?-127:3),-7,bool(id%3),{77,16,height,3}};
            rows.push_back({draw,height,origin,marker[0],marker[1]});
        }
#ifdef MNM_ORIGINAL_REFERENCE
        ownedRows=&rows;original(rows);
#else
        auto allocation=std::make_shared<std::vector<ConsumerRow>>(rows);
        MutableWorldQueue queue{allocation,count};mutableQueue=&queue;ownedRows=allocation.get();
        consumeMutableWorldRows(queue,current,[](const QueueCall& call) {
            add(tags[unsigned(call.backend)],{call.values.begin(),call.values.begin()+call.count});
            return resultValue;
        },[](const ConsumerClip& c) {
            add(0x57dc60,{U(c.left),U(c.top),U(c.right),U(c.bottom)});
        },[](const std::array<U,3>& args) { add(0x4d8ab0,{99,args[0],args[1],args[2]}); });
#endif
        #ifndef MNM_ORIGINAL_REFERENCE
        rows=*allocation;activeCount=queue.count;
        #endif
        std::cout<<"{\"id\":"<<id++<<",\"rows\":[";
        for (unsigned i=0;i<rows.size();++i) {
            if(i)std::cout<<',';
            std::cout<<'['<<rows[i].draw.y<<','<<rows[i].metadata<<']';
        }
        std::cout<<"],\"count\":"<<activeCount<<",\"rebound\":"<<rebound<<",\"clip\":";printClip(current.active);
        std::cout<<",\"final\":["<<current.finalArguments[0]<<','<<current.finalArguments[1]
                 <<','<<current.finalArguments[2]<<"],\"events\":[";
        for (unsigned i=0;i<events.size();++i) {
            if(i)std::cout<<',';
            const auto& e=events[i];std::cout<<'['<<e.tag<<",[";
            for (unsigned j=0;j<e.values.size();++j){if(j)std::cout<<',';std::cout<<e.values[j];}
            std::cout<<"],";printClip(e.clip);std::cout<<']';
        }
        std::cout<<"]}\n";
    }
    if(id!=23712)throw std::runtime_error("Incomplete consumer matrix");
} catch (const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
