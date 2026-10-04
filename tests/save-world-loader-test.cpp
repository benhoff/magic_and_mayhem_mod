#include "save_world.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
static void check(bool value) { if(!value) throw std::runtime_error("world loader assertion"); }
static void word(Bytes& b, std::uint32_t n=0) {for(unsigned i=0;i<4;++i) b.push_back((n>>(i*8))&255);}
static void zero(Bytes& b,std::size_t n) {b.resize(b.size()+n);}
static void replaceWord(Bytes& b,std::size_t at,std::uint32_t n) {
    for(unsigned i=0;i<4;++i) b.at(at+i)=(n>>(i*8))&255;
}
static Bytes emptyWorld() {
    Bytes b(256); b[0]='M'; b[1]='P';
    for(unsigned i=0;i<4;++i) word(b,i+1);
    word(b); word(b,123);
    zero(b,76);word(b);zero(b,20);zero(b,20);zero(b,1864);word(b);word(b);zero(b,20);zero(b,20);
    zero(b,8);word(b);word(b);zero(b,36);
    zero(b,16+112);word(b);zero(b,64+12);
    zero(b,8+12+16);zero(b,12);zero(b,104*256);word(b);word(b);zero(b,100*4);
    zero(b,8+412+20);word(b);zero(b,8); return b;
}
int main() try {
    auto bytes=emptyWorld(); const auto source=bytes;
    auto decoded=decodeWorldState(bytes);check(std::holds_alternative<SavedWorld>(decoded));
    auto world=std::get<SavedWorld>(std::move(decoded));bytes.clear();
    check(world.raw==source && world.mapPath=="MP" && world.counter==123 && world.globals[3]==4);
    check(world.blocks[0].offset==0 && world.blocks[0].size==source.size());
    for(const auto& block:world.blocks) {
        check(block.offset<=source.size() && block.size<=source.size()-block.offset);
        if(block.parent) {const auto& p=world.blocks.at(*block.parent);check(block.offset>=p.offset && block.offset+block.size<=p.offset+p.size);}
        if(block.offset) {auto cut=source;cut.resize(block.offset-1);check(std::holds_alternative<PersistenceError>(decodeWorldState(cut)));}
    }
    auto bad=source;bad.push_back(0);check(std::holds_alternative<PersistenceError>(decodeWorldState(bad)));
    WorldLimits limits;limits.inputBytes=source.size()-1;check(std::holds_alternative<PersistenceError>(decodeWorldState(source,limits)));
    limits={};limits.decodedBytes=source.size();check(std::holds_alternative<PersistenceError>(decodeWorldState(source,limits)));
    limits={};limits.blocks=1;check(std::holds_alternative<PersistenceError>(decodeWorldState(source,limits)));
    bad=source;bad[272]=255;bad[273]=255;bad[274]=255;bad[275]=127;
    check(std::holds_alternative<PersistenceError>(decodeWorldState(bad)));
    for(const auto& block:world.blocks) {
        if(block.name=="map" || block.name=="three-resource-slots") {
            bad=source;replaceWord(bad,block.offset+(block.name=="map"?20:0),0xffffffffU);
            const auto failed=decodeWorldState(bad);
            check(std::holds_alternative<PersistenceError>(failed));
            check(std::get<PersistenceError>(failed).code==PersistenceErrorCode::limitExceeded);
        }
    }
    limits={};limits.records=0;bad=source;replaceWord(bad,272,1);
    const auto limited=decodeWorldState(bad,limits);
    check(std::holds_alternative<PersistenceError>(limited));
    check(std::get<PersistenceError>(limited).code==PersistenceErrorCode::limitExceeded);
    SavedGame save;save.worldMarker=0x17;save.worldTail=source;
    check(std::holds_alternative<SavedWorld>(decodeSavedWorld(save)));save.worldMarker=0;
    check(std::holds_alternative<PersistenceError>(decodeSavedWorld(save)));
    std::cout<<"world ownership, ranges, truncation, budgets and envelope checks passed\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
