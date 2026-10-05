#include "creature_motion.hpp"
#include "no_cd.hpp"
#include <iostream>
#include <stdexcept>
#ifdef MNM_ANI_MOTION_REFERENCE
#include "movement-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
static unsigned currentCase=0,currentTick=0;
static void require(bool v,int line){if(!v) throw std::runtime_error("ANI/motion comparison mismatch at line "+std::to_string(line)+" case "+std::to_string(currentCase)+" tick "+std::to_string(currentTick));}
#define check(value) require((value),__LINE__)
#ifdef MNM_ANI_MOTION_REFERENCE
namespace {
extern "C" void __attribute__((thiscall)) finish(void* object){std::uint32_t zero=0;std::memcpy(static_cast<char*>(object)+4,&zero,4);}
void put(std::vector<unsigned char>& o,unsigned at,int v){std::memcpy(o.data()+at,&v,4);}
int get(const std::vector<unsigned char>& o,unsigned at){int v;std::memcpy(&v,o.data()+at,4);return v;}
}
#endif
int main(int argc,char** argv) try {
#ifndef MNM_ANI_MOTION_REFERENCE
    (void)argc;(void)argv;
#else
    check(argc==2);native_reference::initialize(argv[1],true,true,true,true);
    native_reference::redirect(0x512460,reinterpret_cast<std::uintptr_t>(&finish));
#endif
    unsigned transitions=0;
    for(int direction=0;direction<8;++direction) for(int delay:{0,1,2}) for(int rate:{0,60,720})
    for(int duration:{120,720}) for(bool repeat:{false,true}) for(bool vertical:{false,true})
    for(int category:{0,4}) for(int height:{-32,-16,0,16,32}) {
        (void)category;
        ++currentCase;
        std::vector<mnm::assets::AnimationRecord> records{{1,delay,{}},{0,7,{}},{0,8,{}}};
        if(repeat){records.push_back({2,2,{}});records.push_back({0,9,{}});records.push_back({3,-1,{}});}
        records.push_back({5,2,{}});records.push_back({6,-1,{}});
        NoCdAnimationPlayer player(records);player.start();
        MotionInputs p;p.separateCursor=true;p.rate=rate;p.duration=duration;p.gridX=3;p.gridY=2;
        p.heightOrigin=24;p.heightDelta=height;p.vertical=vertical;p.direction=direction;p.samples.fill(12);
        MotionState state;state.fineX=96;state.fineY=64;state.fineZ=p.heightOrigin;
#ifdef MNM_ANI_MOTION_REFERENCE
        std::array<std::uint32_t,11> header{};header[5]=9;
        std::array<std::uint32_t,9> offsets{};offsets[8]=records.size();
        std::uint32_t asset[3]={reinterpret_cast<std::uintptr_t>(header.data()),reinterpret_cast<std::uintptr_t>(offsets.data()),reinterpret_cast<std::uintptr_t>(records.data())};
        std::vector<unsigned char> object(0xe4b,0);put(object,0xb0+28,reinterpret_cast<std::uintptr_t>(asset));
        using Start=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
        reinterpret_cast<Start>(0x464cb0)(object.data()+0xb0,direction,0);
#endif
        for(unsigned tick=0;tick<100;++tick){
            currentTick=tick;
            auto staged=player;
            MotionAnimation events{[&]{return staged.tick();},[&]{staged.start();}};
            const auto done=advance_creature_motion(state,p,&events);player=std::move(staged);
#ifdef MNM_ANI_MOTION_REFERENCE
            // Motion is seeded from the previous native state only at invocation zero;
            // subsequent original states remain independent across calls/segments.
            if(!tick){put(object,0x14,96);put(object,0x18,64);put(object,0x1c,p.heightOrigin);
                put(object,0xb8f,reinterpret_cast<std::uintptr_t>(p.samples.data()));put(object,0xb93,reinterpret_cast<std::uintptr_t>(p.samples.data()));}
            put(object,4,1);put(object,8,p.gridX);put(object,12,p.gridY);put(object,0x608,direction);
            put(object,0x98f,vertical?1:0);put(object,0x10c,category);
            put(object,0xb5f,p.heightOrigin);put(object,0xb57,height);put(object,0xb63,rate);put(object,0xb67,duration);
            using Tick=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Tick>(0x5104b0)(object.data());
            check((get(object,4)==0)==done);
            check(state.accumulator==get(object,0xb6b) && state.progress==get(object,0xb3f) && state.travelX==get(object,0xb47) && state.travelY==get(object,0xb4b));
            check(state.fineX==get(object,0x14) && state.fineY==get(object,0x18) && state.fineZ==get(object,0x1c));
            check(state.residualX==get(object,0xb7b) && state.residualY==get(object,0xb7f) && state.animationFrame==static_cast<unsigned>(get(object,0xb77)));
            check(state.frame==(static_cast<std::uint32_t>(get(object,0xb8f))-reinterpret_cast<std::uintptr_t>(p.samples.data()))/4);
            const auto& a=player.state();const auto base=reinterpret_cast<std::uintptr_t>(records.data());
            check(a.pc==(static_cast<std::uint32_t>(get(object,0xb0+24))-base)/44 && a.active==bool(get(object,0xb0+8)));
            check(a.delay==static_cast<unsigned>(get(object,0xbc)) && a.elapsed==static_cast<unsigned>(get(object,0xc0)) && a.repeats==static_cast<unsigned>(get(object,0xc4)) && a.breakFlag==static_cast<unsigned>(get(object,0xd0)));
            check(a.displayedRecord && *a.displayedRecord==(static_cast<std::uint32_t>(get(object,0xb4))-base)/44);
#endif
            ++transitions;
            if(done){
                constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
                state.progress-=192;state.travelX-=vertical?0:dx[direction]*192;state.travelY-=vertical?0:dy[direction]*192;
                p.gridX+=vertical?0:dx[direction];p.gridY+=vertical?0:dy[direction];state.fineX=p.gridX*32;state.fineY=p.gridY*32;state.fineZ=p.heightOrigin;
#ifdef MNM_ANI_MOTION_REFERENCE
                put(object,0xb3f,state.progress);put(object,0xb47,state.travelX);put(object,0xb4b,state.travelY);put(object,0x14,state.fineX);put(object,0x18,state.fineY);put(object,0x1c,state.fineZ);
#endif
            }
        }
    }
    // Caller-staged controller and arithmetic state survive event refusal.
    NoCdAnimationPlayer unsupported({{0,1,{}},{5,7,{}},{6,-1,{}}});unsupported.start();unsupported.tick();
    auto staged=unsupported;MotionInputs p;p.separateCursor=true;p.rate=120;p.duration=120;p.samples.fill(12);
    MotionState state;MotionAnimation events{[&]{return staged.tick();},[&]{staged.start();}};
    bool refused=false;try{advance_creature_motion(state,p,&events);}catch(const std::invalid_argument&){refused=true;}
    check(refused && state.progress==0 && state.frame==0 && unsupported.state().pc==1);
    std::cout<<transitions<<" ANI-driven motion/controller transitions passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
