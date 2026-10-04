#include "persistence.hpp"
#include "persistence-container-fixtures.hpp"
#include <iostream>
#include <algorithm>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void check(bool ok,const char* msg) {if(!ok) throw std::runtime_error(msg);}
template<class T> T good(PersistenceResult<T> r) {
    if(auto* e=std::get_if<PersistenceError>(&r)) throw std::runtime_error(e->detail);
    return std::get<T>(std::move(r));
}
template<class T> void bad(const PersistenceResult<T>& r,PersistenceErrorCode code) {
    const auto* e=std::get_if<PersistenceError>(&r);check(e&&e->code==code,"wrong error");
}
Bytes text(const std::string& s) {return Bytes(s.begin(),s.end());}
void word(Bytes& b,std::uint32_t n) {for(unsigned i=0;i<4;++i) b.push_back(static_cast<std::uint8_t>(n>>(8*i)));}
void at(Bytes& b,std::size_t p,std::uint32_t n) {for(unsigned i=0;i<4;++i) b[p+i]=static_cast<std::uint8_t>(n>>(8*i));}
Bytes fixture(bool world) {
    Bytes b;word(b,0x564153);word(b,20);word(b,0x12345678);b.resize(0x20c);
    for(unsigned i=0;i<80;++i) {
        b.resize(b.size()+0x14a,static_cast<std::uint8_t>(i));
        word(b,i==0?2:0);if(i==0) for(unsigned k=0;k<10;++k) word(b,100+k);
        b.resize(b.size()+1000,0x55);word(b,0x11);word(b,0x22);word(b,i==79?3:0);
        if(i==79) {word(b,4);word(b,5);word(b,6);}
        b.push_back(i==0?1:0);if(i==0) b.resize(b.size()+0x57c,0xcc);
    }
    Bytes realm(0x13dc);realm[0]='C';at(realm,0x104,9);at(realm,0xe8c,0x11223344);at(realm,0x124c+79*4,0xaabbccdd);at(realm,0x138c+19*4,0xffffffff);
    b.insert(b.end(),realm.begin(),realm.end());b.resize(b.size()+0x16c+0x18+0x3268);
    word(b,8);word(b,9);word(b,world?0x17:0);
    if(world) {b.resize(b.size()+24,0x88);b.insert(b.end(),{1,2,3,4,5});}
    return b;
}
class MemoryFile final:public AssetFile {
    Bytes bytes;std::size_t p=0;
public:
    explicit MemoryFile(Bytes b):AssetFile("fixture",{}),bytes(std::move(b)) {}
    Result<std::int64_t> size() override {return static_cast<std::int64_t>(bytes.size());}
    Result<std::int64_t> position() override {return static_cast<std::int64_t>(p);}
    Status seek(std::int64_t n) override {if(n<0||std::uint64_t(n)>bytes.size()) return failure(ErrorCode::invalidArgument,"seek","range");p=n;return std::monostate{};}
    ReadResult read(void* dest,std::int64_t count) override {
        auto n=std::min<std::size_t>(count,bytes.size()-p);std::copy_n(bytes.data()+p,n,static_cast<std::uint8_t*>(dest));p+=n;return {static_cast<std::int64_t>(n),std::nullopt};
    }
};
}
int main() try {
    for(const auto& f:{packedMode0,packedMode1,packedMode2}) {
        auto c=good(decodePackedContainer(f));check(c.mode<3,"mode");
        auto expected=c.mode==0?text("raw-words"):c.mode==1?text("AAAABCDE"):text("AAAAAAAA");check(c.decoded==expected,"container bytes");
        for(std::size_t i=0;i<f.size();++i) {Bytes shortFile(f.begin(),f.begin()+i);check(std::holds_alternative<PersistenceError>(decodePackedContainer(shortFile)),"container truncation accepted");}
        PersistenceLimits l;l.decodedBytes=1;bad(decodePackedContainer(f,l),PersistenceErrorCode::limitExceeded);
    }
    auto corrupt=packedMode0;corrupt[20]^=1;bad(decodePackedContainer(corrupt),PersistenceErrorCode::checksumMismatch);
    auto cfg=good(decodeConfig(text(";comment\r\n[General]\r\nName = a=b\r\nEmpty=\r\n")));check(*cfg.find("GENERAL","NAME")=="a=b","config owned values");
    auto annotated=good(decodeConfig(text("Title\r\n[Object]\r\n = ANI_102\r\nKey=2\r\n")));
    check(annotated.annotations.size()==2&&annotated.annotations[1].first==17&&*annotated.find("object","key")=="2","CFG annotations");
    bad(decodeConfig(text("[g]\nx=1\nX=2\n")),PersistenceErrorCode::malformedData);
    bad(decodeConfig(text("[broken\n")),PersistenceErrorCode::malformedData);
    auto names=good(decodeRegionNames(text("One\r\n\r\nTwo\n")));check(names==std::vector<std::string>({"One","","Two"}),"region line indices");
    PersistenceLimits lineLimit;lineLimit.lines=1;bad(decodeRegionNames(text("A\nB"),lineLimit),PersistenceErrorCode::limitExceeded);
    const auto realmText=text("[GENERAL]\ncurrentRealmName=X\nnextRealm=\nplayerWizard1=0\nlastRegion=-1\nwizardCount=1\nregionCount=2\n[WIZARD_00]\nwizardIcon=2\nwizardFlag=3\nwizardLocation=1\nwizardMovement_oldRegion=1\n[REGION_INFO]\nregionOwner_00=0\n");
    auto realm=good(decodeRealmConfig(realmText));check(realm.wizards[0].flag==3&&realm.regionOwners[0]==0&&!realm.regionOwners[1],"typed config");
    bad(decodeRealmConfig(text("[GENERAL]\nwizardCount=81\n")),PersistenceErrorCode::malformedData);
    for(bool world:{false,true}) {
        auto bytes=fixture(world);auto save=good(decodeSavedGame(bytes));
        check(save.wizards[0].entries[1][4]==109&&save.wizards[0].extension->size()==0x57c,"nested wizard layout");
        check(save.wizards[79].list==std::vector<std::uint32_t>({4,5,6}),"last wizard alignment");
        check(save.realm.icons[0]==0x11223344&&static_cast<std::uint32_t>(save.realm.flags[79])==0xaabbccdd&&save.realm.regionOwners[19]==-1,"realm offsets");
        check(save.accountingValue==0x12345678&&save.counters[1]==9,"accounting is not file length");
        check(save.worldTail==(world?Bytes{1,2,3,4,5}:Bytes{})&&save.worldGlobals.has_value()==world,"world ownership");
        for(auto n:{std::size_t(0),std::size_t(11),std::size_t(0x20c),std::size_t(0x20c+0x14a+3),bytes.size()-(world?6:1)}) {
            Bytes truncated(bytes.begin(),bytes.begin()+n);check(std::holds_alternative<PersistenceError>(decodeSavedGame(truncated)),"save truncation accepted");
        }
        PersistenceResult<SavedGame> loaded;{MemoryFile f(bytes);loaded=loadSavedGame(f,false);}bytes.clear();
        check(good(std::move(loaded)).wizards[0].extension->at(0)==0xcc,"handle lifetime ownership");
    }
    auto bytes=fixture(false);at(bytes,4,19);bad(decodeSavedGame(bytes),PersistenceErrorCode::unsupportedVersion);
    bytes=fixture(false);at(bytes,0x20c+0x14a,43);bad(decodeSavedGame(bytes),PersistenceErrorCode::limitExceeded);
    bytes=fixture(false);bytes.push_back(0);bad(decodeSavedGame(bytes),PersistenceErrorCode::malformedData);
    bytes=fixture(false);at(bytes,bytes.size()-4,22);bad(decodeSavedGame(bytes),PersistenceErrorCode::malformedData);
    bad(decodeRealmState(Bytes(4)),PersistenceErrorCode::malformedData);
    std::cout<<"Persistence loader fixtures passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
