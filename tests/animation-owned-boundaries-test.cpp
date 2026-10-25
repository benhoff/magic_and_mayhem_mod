// Native safety and ownership controls; malformed inputs are never sent to the PE.
#include "attachment.hpp"
#include "body_placement.hpp"
#include <array>
#include <climits>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
using Record=mnm::assets::AnimationRecord;
static void require(bool ok){if(!ok)throw std::runtime_error("Owned animation boundary differs");}
static bool same(const AnimationState& a,const AnimationState& b){
    return a.pc==b.pc && a.displayedRecord==b.displayedRecord && a.active==b.active &&
        a.delay==b.delay && a.elapsed==b.elapsed && a.repeats==b.repeats && a.breakFlag==b.breakFlag;
}
template<class F>void refused(F f){bool fail=false;try{f();}catch(const std::runtime_error&){fail=true;}require(fail);}
int main()try{
    const std::vector<Record> sequence{{0,7,{}},{0,8,{}},{6,0,{}}};
    NoCdAnimationPlayer forward(sequence);
    AnimationState edge{1,0,true,UINT_MAX,UINT_MAX-1,UINT_MAX,1};forward.restore(edge);
    require(forward.tick()==0 && forward.state().elapsed==UINT_MAX && forward.sprite()==7);
    require(forward.tick()==0 && forward.state().elapsed==0 && forward.sprite()==8 && forward.state().repeats==UINT_MAX);
    const auto saved=forward.state();
    std::array<AnimationState,7> invalid{saved,saved,saved,saved,saved,saved,saved};
    invalid[0].pc=4;invalid[1].pc=3;invalid[2].elapsed=UINT_MAX;invalid[2].delay=UINT_MAX-1;
    invalid[3].breakFlag=2;invalid[4].active=false;invalid[5].displayedRecord=2;invalid[6].displayedRecord=99;
    for(const auto& state:invalid){refused([&]{forward.restore(state);});require(same(forward.state(),saved));}
    // A dormant body can retain control words; a failed candidate must not erase them.
    NoCdAnimationPlayer body(sequence);AnimationState dormant{1,{},false,9,4,17,1};body.restore(dormant);
    const std::vector<std::vector<Record>> bad{{},{{0,1,{}}},{{6,0,{}}},{{1,3,{}},{0,-1,{}},{6,0,{}}},{{4,0,{}},{6,0,{}}}};
    for(const auto& records:bad){
        refused([&]{bodyPlacement(body,3,7,[&](unsigned n){require(n==291);return records;});});
        require(same(body.state(),dormant));
    }
    body.start();require(body.sprite()==7); // Original owned source survived every refusal.
    // Rebinding is explicit removal followed by entry; each candidate is independently owned.
    ModeOneAttachment layer;
    unsigned calls=0;
    for(unsigned facing=0;facing<8;++facing){
        const unsigned base=UINT_MAX-facing,asset=UINT_MAX-facing;
        auto records=sequence;records[0].argument=int(100+facing);
        require(layer.change(1,base,asset,facing,[&](unsigned a,unsigned n){++calls;require(a==asset && n==UINT_MAX);return records;}));
        records[0].argument=999;
        require(layer.controller()->sprite()==100+facing && layer.selection()->assetIndex==asset && layer.selection()->sequence==UINT_MAX);
        for(int health:{INT_MIN,-1,0,1,INT_MAX})require(layer.visible(health)==(health!=0));
        const auto phase=layer.controller()->state();const auto snapshot=layer.controller()->displayedRecord();
        require(!layer.change(1,UINT_MAX,UINT_MAX,99,{}) && same(layer.controller()->state(),phase));
        refused([&]{layer.change(UINT_MAX,0,0,0,{});});require(same(layer.controller()->state(),phase));
        require(layer.change(0,0,0,99,{}) && layer.tick()==0 && !layer.visible(1));
        const auto stopped=layer.controller()->state();
        for(const auto& candidate:bad){
            if(candidate.size()==1 && candidate[0].opcode==6)continue; // A stopped child is valid in mode one.
            refused([&]{layer.change(1,0,0,0,[&](unsigned,unsigned){return candidate;});});
            require(layer.mode()==0 && same(layer.controller()->state(),stopped) && layer.selection()->assetIndex==asset);
        }
        require(snapshot && snapshot->argument==int(100+facing));
        refused([&]{layer.change(1,UINT_MAX,0,1,{});});
        require(same(layer.controller()->state(),stopped));
    }
    require(calls==8);
    std::cout<<"DWORD delay/restore, dormant body refusal and eight owned attachment rebindings pass\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
