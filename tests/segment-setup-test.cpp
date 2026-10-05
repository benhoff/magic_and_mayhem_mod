#include "segment_setup.hpp"
#include <iostream>
#include <stdexcept>
#ifdef MNM_SEGMENT_REFERENCE
#include "movement-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
static void check(bool value) {if(!value) throw std::runtime_error("segment setup assertion");}
#ifdef MNM_SEGMENT_REFERENCE
namespace {
int animationCalls=0,selectedAnimation=-1,selectedAction=-1,acceptedCategory=0;
std::vector<unsigned char> terrain(2*0x164,0),cells(8*8*3*12,0);
extern "C" int __attribute__((thiscall)) inactive(void*) {return 0;}
extern "C" int __attribute__((thiscall)) accept(void*,int,int,int,int,int,int,int* category,int) {*category=acceptedCategory;return 1;}
extern "C" void __attribute__((thiscall)) occupancy(void*,int,int,int) {}
extern "C" void __attribute__((thiscall)) animation(void*,int index,int) {++animationCalls;selectedAnimation=index;}
extern "C" void __attribute__((thiscall)) action(void*,int index) {selectedAction=index;}
template<class T> void put(std::vector<unsigned char>& o,unsigned at,T v) {std::memcpy(o.data()+at,&v,sizeof(v));}
int get(const std::vector<unsigned char>& o,unsigned at) {int v;std::memcpy(&v,o.data()+at,4);return v;}
void compare(const SegmentPrevious& old,const SegmentRequest& p,const CreatureScalarState& scalar,const SegmentSetup& expected,unsigned count) {
    std::vector<unsigned char> object(0xe4b,0),type(0x198,0);
    acceptedCategory=p.category;
    const auto targetX=p.gridX+p.delta.x,targetY=p.gridY+p.delta.y,targetZ=p.gridZ+p.delta.z;
    put(cells,(targetX+8*(targetY+8*targetZ))*12,std::uint16_t(1));
    put(terrain,0x164+0x94,p.destinationTerrainHeight);
    put(object,4,1);put(object,8,p.gridX);put(object,12,p.gridY);put(object,16,p.gridZ);
    put(object,0x14,old.state.fineX);put(object,0x18,old.state.fineY);put(object,0x1c,p.heightOrigin);
    put(object,0xac,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(type.data())));
    put(object,0x5fc,old.action);put(object,0x608,old.direction);put(object,0x610,old.vertical);put(object,0x10c,old.category);
    put(object,0x977,0);put(object,0x97b,1);put(object,0x97f,targetX);put(object,0x983,targetY);put(object,0x987,targetZ);
    put(object,0x98b,p.direction);put(object,0x98f,p.vertical);put(object,0x993,0);put(object,0x997,p.category);
    put(object,0xb3f,old.state.progress);put(object,0xb47,old.state.travelX);put(object,0xb4b,old.state.travelY);
    constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
    put(object,0xb4f,old.vertical?0:dx[old.direction]*60);put(object,0xb53,old.vertical?0:dy[old.direction]*60);
    put(object,0xb63,old.rate);put(object,0xb6b,old.state.accumulator);put(object,0xb73,int(old.preserveCandidate));
    put(object,0xb7b,old.state.residualX);put(object,0xb7f,old.state.residualY);
    const auto samples=reinterpret_cast<std::uintptr_t>(type.data()+0xd8+old.state.frame*4);
    put(object,0xb8f,static_cast<std::uint32_t>(samples));
    put(object,0xb93,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(type.data()+0xd8+old.state.initialFrame*4)));
    put(object,0xb83,old.state.initialResidualX);put(object,0xb87,old.state.initialResidualY);
    put(type,0x10,scalar.type_10);put(type,0x3c,scalar.type_3c);
    std::memcpy(type.data()+0xd8,scalar.type_d8.data(),sizeof(scalar.type_d8));
    put(object,0x778,scalar.object_778);put(object,0x77c,scalar.object_77c);
    *reinterpret_cast<float*>(0x5e15f0)=scalar.slope_global;
    std::memcpy(reinterpret_cast<void*>(0x6c007d),&scalar.default_scalar,4);
    animationCalls=0;selectedAnimation=selectedAction=-1;
    using Fn=void (__attribute__((thiscall)) *)(void*);reinterpret_cast<Fn>(0x510e80)(object.data());
    if(get(object,0xb63)!=static_cast<int>(expected.rate) || get(object,0xb67)!=static_cast<int>(expected.duration)) {
        std::cerr<<"case "<<count<<" direction "<<old.direction<<"->"<<p.direction<<" action "<<old.action<<" rate "<<old.rate<<" observed "<<get(object,0xb63)<<" expected "<<expected.rate<<'\n';
        throw std::runtime_error("original scalar setup mismatch");
    }
    check(get(object,0xb6b)==expected.state.accumulator && get(object,0xb3f)==expected.state.progress);
    check(get(object,0xb47)==expected.state.travelX && get(object,0xb4b)==expected.state.travelY);
    check(get(object,0xb7b)==expected.state.residualX && get(object,0xb7f)==expected.state.residualY);
    check(get(object,0x608)==p.direction && get(object,0x614)==old.direction && get(object,0x610)==p.vertical);
    check(get(object,0xb57)==expected.heightDelta && get(object,0xb5f)==expected.heightOrigin && selectedAction==2);
    check(animationCalls==(expected.carried?0:1));if(!expected.carried) check(selectedAnimation==expected.animation);
    const auto pointer=static_cast<std::uint32_t>(get(object,0xb8f));
    const auto wanted=expected.carried?samples:reinterpret_cast<std::uintptr_t>(type.data()+0xd8+expected.sampleBank*48);
        check(pointer==wanted);
    check(get(object,0xb83)==expected.state.initialResidualX && get(object,0xb87)==expected.state.initialResidualY);
    check(static_cast<std::uint32_t>(get(object,0xb93))==reinterpret_cast<std::uintptr_t>(type.data()+0xd8+expected.state.initialFrame*4));
}
}
#endif
int main(int argc,char** argv) try {
#ifndef MNM_SEGMENT_REFERENCE
    (void)argc;(void)argv;
#else
    check(argc==2);native_reference::initialize(argv[1],true,true,true,true);
    for(auto pair:{std::pair<std::uint32_t,std::uintptr_t>{0x516ee0,reinterpret_cast<std::uintptr_t>(&inactive)},
         {0x514360,reinterpret_cast<std::uintptr_t>(&accept)},{0x5093e0,reinterpret_cast<std::uintptr_t>(&occupancy)},
         {0x509230,reinterpret_cast<std::uintptr_t>(&occupancy)},{0x464cb0,reinterpret_cast<std::uintptr_t>(&animation)},
         {0x50db60,reinterpret_cast<std::uintptr_t>(&action)}}) native_reference::redirect(pair.first,pair.second);
    *reinterpret_cast<int*>(0x6c5494)=8;*reinterpret_cast<int*>(0x6c5498)=8;*reinterpret_cast<int*>(0x6e9850)=0;
    *reinterpret_cast<std::uint32_t*>(0x6c54dc)=reinterpret_cast<std::uintptr_t>(cells.data());
    *reinterpret_cast<std::uint32_t*>(0x65660c)=reinterpret_cast<std::uintptr_t>(terrain.data());
    for(int i=0;i<8;++i) *reinterpret_cast<int*>(0x6cb942+i*4)=i*8;
    for(int i=0;i<3;++i) *reinterpret_cast<int*>(0x6cb8c2+i*4)=i*64;
    *reinterpret_cast<int*>(0x6a652d)=0;
#endif
    unsigned count=0;
    constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
    for(int direction=0;direction<8;++direction) for(int priorDirection=0;priorDirection<8;++priorDirection)
    for(int priorAction:{0,2,15}) for(int category:{0,4}) for(int vertical:{0,1})
    for(bool preserve:{false,true}) for(int acceleration:{0,64,500}) for(unsigned rate:{0U,30U,720U})
    for(int flags=0;flags<4;++flags) {
        CreatureScalarState scalar;scalar.type_3c=1;scalar.type_10=acceleration;
        scalar.object_778=flags&1;scalar.object_77c=(flags>>1)&1;scalar.default_scalar=720;
        for(unsigned i=0;i<48;++i) scalar.type_d8[i]=(i/12)%2?40:60;
        SegmentPrevious old;old.direction=priorDirection;old.action=priorAction;old.category=category;old.vertical=vertical;
        old.preserveCandidate=preserve;old.rate=rate;old.state.progress=240;
        old.state.travelX=dx[priorDirection]*240;old.state.travelY=dy[priorDirection]*240;
        old.state.accumulator=17;old.state.frame=(priorDirection&1)*12+4;old.state.initialFrame=(priorDirection&1)*12;old.state.animationFrame=4;old.state.residualX=-dx[priorDirection]*40;old.state.residualY=-dy[priorDirection]*40;
        old.state.fineX=old.state.fineY=96;old.state.fineZ=16;
        SegmentRequest p;p.gridX=p.gridY=3;p.heightOrigin=16;p.direction=direction;p.delta={dx[direction],dy[direction],0};
        auto result=initialize_creature_segment(old,p,scalar);
        check(result.duration>0 && result.sampleBank==(direction&1));
        check(result.carried==(priorAction==2 && priorDirection==direction && category==0 && vertical==0));
#ifdef MNM_SEGMENT_REFERENCE
        compare(old,p,scalar,result,count);
#endif
        ++count;
    }
    std::cout<<count<<" planar regression cases passed\n";
    unsigned terrainCount=0;
    for(int category:{0,4}) for(int direction=0;direction<8;++direction) for(int z:{-1,0,1})
    for(bool vertical:{false,true}) for(int originOffset:{0,8,16}) for(int targetHeight:{0,8,16})
    for(int priorAction:{0,2}) for(int acceleration:{64,500}) for(unsigned rate:{30U,720U}) {
        if(vertical && z==0) continue;
        SegmentRequest p;p.gridX=p.gridY=3;p.gridZ=1;p.heightOrigin=16+originOffset;p.destinationTerrainHeight=targetHeight;
        p.category=category;p.direction=direction;p.vertical=vertical?z:0;p.delta={vertical?0:dx[direction],vertical?0:dy[direction],z};
        CreatureScalarState scalar;scalar.type_3c=1;scalar.type_10=acceleration;scalar.slope_global=.969f;
        for(unsigned i=0;i<48;++i) scalar.type_d8[i]=(i/12)%2?40:60;
        SegmentPrevious old;old.direction=direction;old.action=priorAction;old.category=category;old.vertical=p.vertical;old.rate=rate;
        old.state.progress=240;old.state.travelX=vertical?0:dx[direction]*240;old.state.travelY=vertical?0:dy[direction]*240;
        old.state.fineX=old.state.fineY=96;old.state.fineZ=p.heightOrigin;
        old.state.accumulator=17;old.state.frame=(direction&1)*12+4;old.state.initialFrame=(direction&1)*12;old.state.animationFrame=4;
        const auto result=initialize_creature_segment(old,p,scalar);
        check(result.heightDelta==z*16+targetHeight-originOffset && result.carried==(priorAction==2));
#ifdef MNM_SEGMENT_REFERENCE
        compare(old,p,scalar,result,count+terrainCount);
#endif
        ++terrainCount;
    }
    std::cout<<terrainCount<<" terrain/slope/vertical/category-four setup cases passed\n";
    unsigned snaps=0;
    for(int x:{2,3,4,5}) for(int y:{1,3}) for(int z:{0,1,2}) for(int height:{0,4,8,16}) for(bool defined:{false,true}) {
        const auto expected=ordinary_creature_height(z,defined?height:0);
#ifdef MNM_SEGMENT_REFERENCE
        std::vector<unsigned char> object(0xe4b,0);
        put(cells,(x+8*(y+8*z))*12,std::uint16_t(defined?1:0));put(terrain,0x164+0x94,height);
        *reinterpret_cast<int*>(0x6a5f88)=0;
        using Fn=void (__attribute__((thiscall)) *)(void*,int,int,int,int);
        reinterpret_cast<Fn>(0x5070e0)(object.data(),x,y,z,0);
        check(get(object,8)==x && get(object,12)==y && get(object,16)==z && get(object,0x14)==x*32 && get(object,0x18)==y*32 && get(object,0x1c)==expected);
#else
        (void)x;(void)y;check(expected==z*16+(defined?height:0));
#endif
        ++snaps;
    }
    bool refused=false;try {(void)ordinary_creature_height(1,17);}catch(const std::invalid_argument&){refused=true;}check(refused);
    SegmentRequest invalid;invalid.delta={1,0,0};invalid.vertical=1;
    refused=false;try {(void)initialize_creature_segment({},invalid,{});}catch(const std::invalid_argument&){refused=true;}check(refused);
    std::cout<<snaps<<" ordinary terrain coordinate snaps passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
