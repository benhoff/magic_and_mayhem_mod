#include "creature_motion.hpp"
#include <iostream>
#include <stdexcept>
#ifdef MNM_MOTION_REFERENCE
#include "movement-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
static void check(bool v) {if(!v) throw std::runtime_error("motion comparison mismatch");}
#ifdef MNM_MOTION_REFERENCE
namespace {
unsigned events=0;bool completed=false;
extern "C" int __attribute__((thiscall)) animation(void*) {return ++events==12?2:0;}
extern "C" void __attribute__((thiscall)) resetAnimation(void*) {events=0;}
extern "C" void __attribute__((thiscall)) finish(void* object) {completed=true;std::uint32_t zero=0;std::memcpy(static_cast<char*>(object)+4,&zero,4);}
extern "C" void __attribute__((thiscall)) noop(void*) {}
template<class T> void put(std::vector<unsigned char>& o,unsigned at,T v) {std::memcpy(o.data()+at,&v,sizeof(v));}
int get(const std::vector<unsigned char>& o,unsigned at) {int v;std::memcpy(&v,o.data()+at,4);return v;}
void original(MotionState& s,const MotionInputs& p,bool expected) {
    std::vector<unsigned char> object(0xe4b,0);
    std::array<int,48> samples=p.samples;
    put(object,4,1);put(object,8,p.gridX);put(object,12,p.gridY);
    put(object,0x14,s.fineX);put(object,0x18,s.fineY);put(object,0x1c,s.fineZ);
    put(object,0xa8,p.force32?12:17);put(object,0x10c,p.force32?2:0);
    put(object,0x608,p.direction);put(object,0x98f,p.vertical?1:0);
    put(object,0xb3f,s.progress);put(object,0xb47,s.travelX);put(object,0xb4b,s.travelY);
    put(object,0xb57,p.heightDelta);put(object,0xb5f,p.heightOrigin);
    put(object,0xb63,p.rate);put(object,0xb67,p.duration);put(object,0xb6b,s.accumulator);
    put(object,0xb7b,s.residualX);put(object,0xb7f,s.residualY);
    put(object,0xb8f,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(samples.data()+s.frame)));
    put(object,0xb93,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(samples.data()+s.initialFrame)));
    put(object,0xb83,s.initialResidualX);put(object,0xb87,s.initialResidualY);
    events=p.separateCursor?s.animationFrame:s.frame;completed=false;
    using Fn=void (__attribute__((thiscall)) *)(void*);
    reinterpret_cast<Fn>(0x5104b0)(object.data());
    check(completed==expected);
    MotionState output{s};output.accumulator=get(object,0xb6b);output.progress=get(object,0xb3f);
    output.travelX=get(object,0xb47);output.travelY=get(object,0xb4b);
    output.fineX=get(object,0x14);output.fineY=get(object,0x18);output.fineZ=get(object,0x1c);
    output.residualX=get(object,0xb7b);output.residualY=get(object,0xb7f);
    output.frame=(get(object,0xb8f)-reinterpret_cast<std::uintptr_t>(samples.data()))/4;
    if(p.separateCursor) output.animationFrame=events;
    s=output;
}
void consumption() {
    // Run actual 512460 and 5070e0; only environment/animation callbacks redirected.
    native_reference::redirect(0x51a8f0,reinterpret_cast<std::uintptr_t>(&noop));
    native_reference::redirect(0x50f850,reinterpret_cast<std::uintptr_t>(&noop));
    native_reference::redirect(0x51aa30,reinterpret_cast<std::uintptr_t>(&noop));
    std::vector<unsigned char> cells(8*8*4*12,0);
    *reinterpret_cast<std::uint32_t*>(0x6c54dc)=reinterpret_cast<std::uintptr_t>(cells.data());
    for(int i=0;i<8;++i) *reinterpret_cast<int*>(0x6cb942+i*4)=i*8;
    for(int i=0;i<4;++i) *reinterpret_cast<int*>(0x6cb8c2+i*4)=i*64;
    *reinterpret_cast<int*>(0x6a5f88)=0;
    unsigned cases=0;
    for(int count=1;count<=16;++count) for(int cursor=0;cursor<count;++cursor) {
        std::vector<unsigned char> object(0xe4b,0);put(object,4,1);
        put(object,0x977,cursor);put(object,0x97b,count);put(object,0xb8b,1);
        put(object,0xb57,16);put(object,0xb5b,8);put(object,0x618,1);
        put(object,0x963,2);
        const auto offset=0x97f+cursor*28;put(object,offset,3);put(object,offset+4,2);put(object,offset+8,1);
        using Fn=void (__attribute__((thiscall)) *)(void*);
        reinterpret_cast<Fn>(0x512460)(object.data());
        check(get(object,8)==3 && get(object,12)==2 && get(object,16)==1);
        check(get(object,0x14)==96 && get(object,0x18)==64 && get(object,0x1c)==16);
        check(get(object,0x977)==cursor+1 && get(object,0x963)==1);
        check(get(object,0xb8b)==(cursor+1<count?1:0));
        check(get(object,0xb57)==0 && get(object,0xb5b)==0 && get(object,0x618)==0);
        ++cases;
    }
    std::cout<<cases<<" original route-consumption and coordinate-snap cases passed\n";
}
}
#endif
int main(int argc,char** argv) try {
#ifndef MNM_MOTION_REFERENCE
    (void)argc;(void)argv;
#else
    check(argc==2);native_reference::initialize(argv[1],true,true,true,true);
    native_reference::redirect(0x464ec0,reinterpret_cast<std::uintptr_t>(&animation));
    native_reference::redirect(0x464d70,reinterpret_cast<std::uintptr_t>(&resetAnimation));
    native_reference::redirect(0x512460,reinterpret_cast<std::uintptr_t>(&finish));
#endif
    MotionInputs simple;simple.rate=50;simple.duration=100;simple.direction=2;simple.samples.fill(12);
    MotionState known;check(!advance_creature_motion(known,simple));check(known.accumulator==50 && known.progress==0);
    check(!advance_creature_motion(known,simple));check(known.progress==12 && known.fineX==2 && known.frame==1 && known.accumulator==0);
    for(int i=0;i<29;++i) check(!advance_creature_motion(known,simple));
    check(advance_creature_motion(known,simple));check(known.progress==192 && known.fineX==32);
    MotionState unchanged;auto invalid=simple;invalid.duration=0;
    bool caught=false;try {advance_creature_motion(unchanged,invalid);} catch(const std::invalid_argument&) {caught=true;}
    check(caught && unchanged.progress==0 && unchanged.accumulator==0);
    auto bounded=simple;bounded.separateCursor=true;bounded.rate=720;bounded.duration=120;
    MotionState end;end.frame=47;
    caught=false;try {advance_creature_motion(end,bounded);} catch(const std::invalid_argument&) {caught=true;}
    check(caught && end.frame==47 && end.progress==0 && end.accumulator==0);
    unsigned cases=0;
    for(int direction=0;direction<8;++direction) for(int vertical=0;vertical<2;++vertical)
    for(int force=0;force<2;++force) for(int rate:{0,1,30,119,120,121,300,999})
    for(int duration:{60,120,700}) for(int height:{-32,-16,0,16,32}) {
        MotionInputs p;p.rate=rate;p.duration=duration;p.gridX=3;p.gridY=2;
        p.heightOrigin=16;p.heightDelta=height;p.direction=direction;p.vertical=vertical;p.force32=force;
        for(unsigned i=0;i<12;++i) p.samples[i]=1+(i*13)%31;
        MotionState model;model.fineX=96;model.fineY=64;model.fineZ=16;
        for(unsigned tick=0;tick<1000;++tick) {
#ifdef MNM_MOTION_REFERENCE
            auto reference=model;
#endif
            auto done=advance_creature_motion(model,p);
#ifdef MNM_MOTION_REFERENCE
            original(reference,p,done);
            check(model.accumulator==reference.accumulator && model.progress==reference.progress &&
                  model.travelX==reference.travelX && model.travelY==reference.travelY &&
                  model.fineX==reference.fineX && model.fineY==reference.fineY && model.fineZ==reference.fineZ &&
                  model.residualX==reference.residualX && model.residualY==reference.residualY && model.frame==reference.frame);
#endif
            check(done==(model.progress>=192));++cases;if(done) break;
        }
    }
    for(int direction=0;direction<8;++direction) for(unsigned initial:{0u,4u,12u,16u})
    for(unsigned clock:{0u,4u,10u}) for(int rate:{0,60,720}) for(int duration:{120,720}) {
        MotionInputs p;p.separateCursor=true;p.direction=direction;p.rate=rate;p.duration=duration;
        p.gridX=3;p.gridY=2;p.heightOrigin=16;
        for(unsigned i=0;i<48;++i) p.samples[i]=i%2?40:60;
        MotionState model;model.frame=initial+4;model.initialFrame=initial;model.animationFrame=clock;
        model.fineX=96;model.fineY=64;model.fineZ=16;model.initialResidualX=-40;
        for(unsigned tick=0;tick<100;++tick) {
#ifdef MNM_MOTION_REFERENCE
            auto reference=model;
#endif
            auto done=advance_creature_motion(model,p);
#ifdef MNM_MOTION_REFERENCE
            original(reference,p,done);
            check(model.accumulator==reference.accumulator && model.progress==reference.progress &&
                  model.travelX==reference.travelX && model.travelY==reference.travelY &&
                  model.fineX==reference.fineX && model.fineY==reference.fineY && model.fineZ==reference.fineZ &&
                  model.residualX==reference.residualX && model.residualY==reference.residualY &&
                  model.frame==reference.frame && model.animationFrame==reference.animationFrame);
#endif
            ++cases;if(done) break;
        }
    }
#ifdef MNM_MOTION_REFERENCE
    // Restore actual completion bytes overwritten by finish before its separate test.
    std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char> data((std::istreambuf_iterator<char>(f)),{});
    unsigned pe;std::memcpy(&pe,data.data()+0x3c,4);unsigned short sections,optSize;
    std::memcpy(&sections,data.data()+pe+6,2);std::memcpy(&optSize,data.data()+pe+20,2);
    bool restored=false;
    for(unsigned i=0;i<sections;++i) {auto offset=pe+24+optSize+i*40;unsigned rva,size,raw;
        std::memcpy(&rva,data.data()+offset+12,4);std::memcpy(&size,data.data()+offset+16,4);std::memcpy(&raw,data.data()+offset+20,4);
        if(0x112460>=rva && 0x112460+5<=rva+size) {std::memcpy(reinterpret_cast<void*>(0x512460),data.data()+raw+0x112460-rva,5);restored=true;}
    }
    check(restored);consumption();
#endif
    std::cout<<cases<<" bounded motion transitions passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
