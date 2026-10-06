#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_animation_binding.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
namespace assets=mnm::assets;
static void require(bool ok){if(!ok)throw std::runtime_error("Effect animation binding/reference mismatch");}
static void put(unsigned char* p,unsigned at,std::uint32_t n){std::memcpy(p+at,&n,4);}
static assets::AnimationRecord record(unsigned op,int arg,unsigned seed){
    assets::AnimationRecord r;r.opcode=op;r.argument=arg;for(unsigned i=0;i<9;++i)r.metadata[i]=seed*100+i;return r;
}
static std::vector<assets::Animation> fixtureAssets(){
    std::vector<assets::Animation> result(8);
    for(unsigned asset=0;asset<result.size();++asset){auto& a=result[asset];a.starts={0};
        for(unsigned seq=0;seq<7;++seq){const int sprite=int(asset*100+seq*10);std::vector<assets::AnimationRecord> rows;
            auto add=[&](unsigned op,int arg){rows.push_back(record(op,arg,asset*7+seq));};
            switch(seq){
            case 0:add(0,sprite);add(0,sprite+1);break;
            case 1:add(1,2);add(5,-7);add(0,sprite);add(5,-123);add(0,sprite+1);break;
            case 2:add(2,2);add(0,sprite);add(3,-1);break;
            case 3:add(0,sprite);add(4,-1);break;
            case 4:add(99,47);add(1,0);add(0,sprite);break;
            case 5:break;
            case 6:add(1,3);add(2,1);add(0,sprite);add(3,-1);add(0,sprite+1);add(5,2);break;
            }
            add(6,0);a.records.insert(a.records.end(),rows.begin(),rows.end());a.starts.push_back(unsigned(a.records.size()));
        }
    }
    return result;
}
int main(int argc,char** argv)try{
    static_assert(sizeof(assets::AnimationRecord)==44,"Original normalized record extent");
#ifdef ORIGINAL_REFERENCE
    require(argc==3);map_image(read(argv[1]));const int outArg=2;
    const unsigned char bindEntry[]={0x8b,0x44,0x24,0x04,0x89,0x41,0x1c};
    const unsigned char startEntry[]={0x8b,0x44,0x24,0x04,0x53,0x56};
    require(!std::memcmp(reinterpret_cast<void*>(0x464ca0),bindEntry,sizeof(bindEntry)));
    require(!std::memcmp(reinterpret_cast<void*>(0x464cb0),startEntry,sizeof(startEntry)));
    using Bind=void(__attribute__((thiscall)) *)(void*,void*);
    using Start=void(__attribute__((thiscall)) *)(void*,std::uint32_t,std::uint32_t);
    using Tick=std::int32_t(__attribute__((thiscall)) *)(void*);
#else
    require(argc==2);const int outArg=1;
#endif
    auto animations=fixtureAssets();const auto beforeAnimations=animations;
    std::vector<EffectAnimationEntry> entries(88);
    std::ofstream out(argv[outArg],std::ios::binary);std::uint64_t fixtures=0,bindings=0,ticks=0,states=0;
#ifdef ORIGINAL_REFERENCE
    std::vector<unsigned char> catalog(8*0x114+32,0x5a);auto* assetBase=catalog.data()+16;
    std::array<unsigned char,8*44+32> headers{};headers.fill(0x3c);
    for(unsigned n=0;n<8;++n){auto* object=assetBase+n*0x114;
        put(object,0,reinterpret_cast<std::uintptr_t>(headers.data()+16+n*44));
        put(object,4,reinterpret_cast<std::uintptr_t>(animations[n].starts.data()));
        put(object,8,reinterpret_cast<std::uintptr_t>(animations[n].records.data()));
    }
    const auto initialCatalog=catalog;const auto initialHeaders=headers;
#endif
    for(unsigned asset=0;asset<8;++asset)for(unsigned seq=0;seq<7;++seq)for(unsigned fill:{0u,0xa5u,0xffu,0x3cu}){
        ++fixtures;const unsigned ordinal=asset*7+seq;
        entries[ordinal]={seq,asset,fill==0xff?0xffffffffu:asset*100+seq,fill==0xa5?0x80000000u:fill};
        const auto entry=entries[ordinal];
        std::array<unsigned char,80> raw{};raw.fill(static_cast<unsigned char>(fill));
#ifdef ORIGINAL_REFERENCE
        auto* controller=raw.data()+16;auto* object=assetBase+asset*0x114;
        const auto* base=reinterpret_cast<const unsigned char*>(animations[asset].records.data());
        const auto first=animations[asset].starts[seq];
#endif
        for(unsigned repeat=0;repeat<2;++repeat){
#ifdef ORIGINAL_REFERENCE
            const auto preBind=raw;auto expectedBind=preBind;put(expectedBind.data()+16,28,reinterpret_cast<std::uintptr_t>(object));
            reinterpret_cast<Bind>(0x464ca0)(controller,object);require(raw==expectedBind);
            reinterpret_cast<Start>(0x464cb0)(controller,seq,0);
#endif
            auto binding=bindEffectAnimation(ordinal,entries,animations);++bindings;
            require(binding.animationOrdinal==ordinal && binding.entry.sequence==seq && binding.entry.assetIndex==asset && binding.entry.property==entry.property && binding.entry.opaque==entry.opaque);
            for(unsigned tick=0;tick<=64;++tick){
                if(tick==8 && seq==3){binding.player.requestBreak();
#ifdef ORIGINAL_REFERENCE
                    put(controller,32,1);
#endif
                }
                std::int32_t event=0;
                if(tick){event=binding.player.tick();++ticks;
#ifdef ORIGINAL_REFERENCE
                    require(reinterpret_cast<Tick>(0x464ec0)(controller)==event);
#endif
                }
                const auto& state=binding.player.state();
#ifdef ORIGINAL_REFERENCE
                auto expected=raw;auto* q=expected.data()+16;
                put(q,0,seq);put(q,4,state.displayedRecord?reinterpret_cast<std::uintptr_t>(base+(first+*state.displayedRecord)*44):0);
                put(q,8,state.active);put(q,12,state.delay);put(q,16,state.elapsed);put(q,20,state.repeats);
                put(q,24,reinterpret_cast<std::uintptr_t>(base+(first+state.pc)*44));put(q,28,reinterpret_cast<std::uintptr_t>(object));put(q,32,state.breakFlag);put(q,36,0);
                require(raw==expected);
                // Unmodeled reverse counters and guards are preserved by these
                // forward calls, compared against the authored pre-bind fixture.
                require(!std::memcmp(raw.data(),preBind.data(),16) && !std::memcmp(raw.data()+64,preBind.data()+64,16));
                require(!std::memcmp(controller+40,preBind.data()+56,8));
                if(auto displayed=binding.player.displayedRecord())require(!std::memcmp(&*displayed,base+(first+*state.displayedRecord)*44,44));
#endif
                const std::array<std::uint32_t,14> row{ordinal,asset,seq,entry.property,entry.opaque,unsigned(repeat),tick,std::uint32_t(event),std::uint32_t(state.pc),state.displayedRecord?std::uint32_t(*state.displayedRecord):0xffffffffu,std::uint32_t(state.active),state.delay,state.elapsed,state.repeats};
                out.write(reinterpret_cast<const char*>(row.data()),sizeof(row));out.write(reinterpret_cast<const char*>(&state.breakFlag),4);
                const auto displayed=binding.player.displayedRecord();const auto bytes=displayed.value_or(assets::AnimationRecord{});
                out.write(reinterpret_cast<const char*>(&bytes),44);++states;
            }
        }
    }
    for(unsigned n=0;n<animations.size();++n){require(animations[n].starts==beforeAnimations[n].starts);
        require(animations[n].records.size()==beforeAnimations[n].records.size() && !std::memcmp(animations[n].records.data(),beforeAnimations[n].records.data(),animations[n].records.size()*44));}
#ifdef ORIGINAL_REFERENCE
    require(catalog==initialCatalog && headers==initialHeaders);
#endif
    require(bool(out));std::cout<<"{\"fixtures\":"<<fixtures<<",\"bindings\":"<<bindings<<",\"ticks\":"<<ticks<<",\"states\":"<<states<<",\"inputs_unchanged\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
