// Execute unchanged 507190, including its real 464cb0 null-display fallback.
#define main existing_placement_reference_main
#include "animation-placement-reference.cpp"
#undef main
#include "body_placement.hpp"
#include <array>
using namespace mnm::reconstruction;
int main(int argc,char** argv)try{
    if(argc!=2)throw std::runtime_error("Expected pinned PE image");
    mapImage(read(argv[1]));
    const auto* body=reinterpret_cast<const unsigned char*>(0x507190);
    const auto* start=reinterpret_cast<const unsigned char*>(0x464cb0);
    const Bytes bodyBefore(body,body+0xc0),startBefore(start,start+0x170);
    using Start=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
    using Body=void(__attribute__((thiscall)) *)(void*,std::uint32_t*);
    std::uint64_t cases=0,fallbacks=0,retained=0;
    for(unsigned controls:{0u,1u})for(int sprite:{0,1,3,4,65535,0x7fffffff}){
        std::array<std::vector<mnm::assets::AnimationRecord>,8> sequences;
        std::vector<mnm::assets::AnimationRecord> records;
        std::vector<std::uint32_t> starts(297);
        const std::array<unsigned,5> words{0,1,0x7fffffff,0x80000000,0xffffffff};
        for(unsigned facing=0;facing<8;++facing){
            auto& sequence=sequences[facing];
            if(controls){sequence.push_back({1,2,{}});sequence.push_back({5,-7,{}});}
            mnm::assets::AnimationRecord frame{0,sprite,{}};
            frame.metadata[0]=words[facing%5];frame.metadata[1]=words[(facing+2)%5];
            sequence.push_back(frame);sequence.push_back({6,0,{}});
            starts[288+facing]=records.size();records.insert(records.end(),sequence.begin(),sequence.end());
        }
        starts[296]=records.size();
        std::array<std::uint32_t,11> header{0x00494e41,0,unsigned(records.size()),5,0,297};
        std::uint32_t spriteHeader[4]={0,0,0,3},markers[4][10]{},table[4]={0,40,80,120};
        std::uint32_t spr[4]={reinterpret_cast<std::uintptr_t>(spriteHeader),0,
            reinterpret_cast<std::uintptr_t>(table),reinterpret_cast<std::uintptr_t>(markers)};
        Bytes asset(0x114),type(12);
        pointer(asset,0,header.data());pointer(asset,4,starts.data());pointer(asset,8,records.data());pointer(asset,0x110,spr);
        for(unsigned facing=0;facing<8;++facing)for(bool visible:{false,true})
        for(unsigned footprint:{0u,1u,2u,3u})for(unsigned view:{0u,1u,2u,3u,4u,0xffffffffu}){
            Bytes creature(0x900);pointer(creature,0xac,type.data());pointer(creature,0xcc,asset.data());
            std::memcpy(type.data()+8,&footprint,4);*reinterpret_cast<unsigned*>(0x6e9850)=view;
            std::memcpy(creature.data()+0x60c,&facing,4);
            NoCdAnimationPlayer native({{6,0,{}}});
            if(visible){
                reinterpret_cast<Start>(0x464cb0)(creature.data()+0xb0,288+facing,0);
                native=NoCdAnimationPlayer(sequences[facing]);native.start();
                const unsigned ignored=0xffffffff;std::memcpy(creature.data()+0x60c,&ignored,4);
            }else{
                AnimationState dormant;dormant.delay=7;dormant.elapsed=3;dormant.repeats=5;dormant.breakFlag=1;native.restore(dormant);
                for(auto pair:std::array<std::pair<unsigned,unsigned>,4>{{{12,7},{16,3},{20,5},{32,1}}})
                    std::memcpy(creature.data()+0xb0+pair.first,&pair.second,4);
            }
            const auto previous=creature;std::uint32_t descriptor[3]{};
            reinterpret_cast<Body>(0x507190)(creature.data(),descriptor);
            unsigned calls=0;
            const auto actual=bodyPlacement(native,visible?0xffffffffu:facing,3,[&](unsigned sequence){
                require(sequence==288+facing);++calls;return sequences[facing];
            },footprint,view);
            const unsigned expected=sprite<=3?unsigned(sprite):0;
            require(actual.sprite==expected && descriptor[0]==reinterpret_cast<std::uintptr_t>(markers[expected]));
            require(descriptor[1]==std::uint32_t(actual.offset.x) && descriptor[2]==std::uint32_t(actual.offset.y));
            const auto& state=native.state();const auto origin=reinterpret_cast<std::uintptr_t>(records.data()+starts[288+facing]);
            require(u32(creature,0xb0)==288+facing && bool(u32(creature,0xb8))==state.active);
            require(u32(creature,0xbc)==state.delay && u32(creature,0xc0)==state.elapsed && u32(creature,0xc4)==state.repeats);
            require((u32(creature,0xc8)-origin)/44==state.pc && u32(creature,0xd0)==state.breakFlag);
            require(state.displayedRecord && (u32(creature,0xb4)-origin)/44==*state.displayedRecord);
            if(visible){require(!actual.startedSequence && !calls && previous==creature);++retained;}
            else{require(actual.startedSequence==288+facing && calls==1);++fallbacks;}
            require(Bytes(body,body+bodyBefore.size())==bodyBefore && Bytes(start,start+startBefore.size())==startBefore);
            ++cases;
        }
    }
    require(cases==4608 && fallbacks==2304 && retained==2304);
    std::cout<<"{\"cases\":"<<cases<<",\"fallback_starts\":"<<fallbacks
        <<",\"existing_displays\":"<<retained<<",\"controller_states_match\":true,\"original_targets_unchanged\":true}\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
