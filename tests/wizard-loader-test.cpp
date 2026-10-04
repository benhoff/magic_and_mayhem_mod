#include "wizard.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
Bytes text(const std::string& s) {return {s.begin(),s.end()};}
void check(bool v,const char* message) {if(!v) throw std::runtime_error(message);}
WizardDefinition good(WizardResult r) {
    if(const auto* e=std::get_if<PersistenceError>(&r)) throw std::runtime_error(e->detail);
    return std::get<WizardDefinition>(std::move(r));
}
void bad(const std::string& s,PersistenceErrorCode code=PersistenceErrorCode::malformedData,const WizardLimits& limits={}) {
    const auto result=decodeWizard(text(s),limits);const auto* e=std::get_if<PersistenceError>(&result);
    check(e && e->code==code,"wrong wizard failure");
}
class MemoryFile final:public AssetFile {
    Bytes bytes;std::size_t offset=0;
public:
    explicit MemoryFile(Bytes b):AssetFile("wizard",{}),bytes(std::move(b)) {}
    Result<std::int64_t> size() override {return static_cast<std::int64_t>(bytes.size());}
    Result<std::int64_t> position() override {return static_cast<std::int64_t>(offset);}
    Status seek(std::int64_t n) override {
        if(n<0 || std::uint64_t(n)>bytes.size()) return failure(ErrorCode::invalidArgument,"seek","out of range");
        offset=static_cast<std::size_t>(n);return std::monostate{};
    }
    ReadResult read(void* dest,std::int64_t count) override {
        if(count<0 || (!dest && count)) return {0,failure(ErrorCode::invalidArgument,"read","invalid buffer")};
        const auto n=std::min<std::size_t>(static_cast<std::size_t>(count),bytes.size()-offset);
        if(n) std::copy_n(bytes.data()+offset,n,static_cast<std::uint8_t*>(dest));
        offset+=n;return {static_cast<std::int64_t>(n),std::nullopt};
    }
};
const std::string minimal="[GENERAL]\nName=\"Default Wizard\"\nMaxMana=100\n";
}
int main() try {
    auto w=good(decodeWizard(text(minimal)));
    check(w.stats.name=="Default Wizard" && w.stats.maxMana==100 && !w.stats.maxHealth && !w.validWizardFile,"missing-field distinction");
    const std::string data="; comment with \"unclosed quote\r\n[header]\r\nvalidwizardfile=true\r\n[GENERAL]\r\nName=\"A;B\" ; comment\r\nMaxMana=0\r\nDifficultyModifier=-50\r\nWizardAnimFile=Greek Female\r\nStartTalismans_Law=3\r\n[ITEM_35]\r\nHasIt=1\r\nResearched=0\r\n[START_SPELLS]\r\nSPELL_00=FALSE\r\nSPELL_92=TRUE ; active\r\n;SPELL_11=TRUE\r\n[START_OBJECTS]\r\nOBJECT_71=0\r\n[START_MAGIC_ITEMS]\r\nMITEM_18=1\r\n[ACTION_03]\r\nValid=FALSE\r\nSpellType=-1\r\nDependancy=2\r\nDestination=Section\r\nXTargetPosition=11\r\n[future]\r\nKey=anything\r\n";
    auto bytes=text(data);w=good(decodeWizard(bytes));bytes.assign(1,0);
    check(w.stats.name=="A;B" && w.stats.maxMana==0 && w.stats.difficultyModifier==-50 && w.stats.animationFile=="Greek Female" && w.stats.startTalismansLaw==3,"typed general");
    check(w.items.at(35).hasIt==true && w.items.at(35).researched==false,"item flags");
    check(!w.startSpells.at(0) && w.startSpells.at(92) && !w.startSpells.count(11),"selection flags and comments");
    check(!w.startObjects.at(71) && w.startMagicItems.at(18),"other selections");
    check(w.actions.at(3).valid==false && w.actions.at(3).dependency==2 && w.actions.at(3).spellType==-1 && w.actions.at(3).xTargetPosition==11,"action fields");
    check(w.source==text(data) && *w.config.find("future","key")=="anything","source and unknown field ownership");
    {MemoryFile f(text(data));w=good(loadWizard(f));}
    check(w.source==text(data) && w.stats.name=="A;B","file lifetime ownership");
    bad("");bad("[GENERAL]\nName=X\n");bad("[GENERAL]\nName=\"\"\nMaxMana=1\n");
    bad(minimal+"MaxMana=2\n");bad(minimal+"MaxHealth=2147483648\n");bad(minimal+"Intelligence=5oops\n");
    bad(minimal+"[header]\nValidWizardFile=FALSE\n");bad(minimal+"[start_spells]\nSPELL_1=maybe\n");
    bad(minimal+"[start_objects]\nOBJECT_-1=TRUE\n");bad(minimal+"[item_abc]\nHasIt=1\n");
    bad(minimal+"[start_spells]\nSPELL_01=1\nSPELL_1=0\n");bad(minimal+"[item_01]\nHasIt=1\n[item_1]\nHasIt=0\n");
    bad(minimal+"[action_01]\nValid=TRUE\n[action_1]\nValid=TRUE\n");
    bad("[GENERAL]\nName=\"unterminated\nMaxMana=1\n");bad(minimal+std::string("\0x",2));bad(minimal+"unexpected line\n");
    WizardLimits limits;limits.inputBytes=4;bad(minimal,PersistenceErrorCode::limitExceeded,limits);
    {MemoryFile f(text(minimal));auto r=loadWizard(f,limits);const auto* e=std::get_if<PersistenceError>(&r);check(e && e->code==PersistenceErrorCode::assetInput && e->input && e->input->code==ErrorCode::limitExceeded,"underlying file limit");}
    limits={};limits.lines=2;bad(minimal,PersistenceErrorCode::limitExceeded,limits);
    limits={};limits.stringBytes=2;bad(minimal,PersistenceErrorCode::limitExceeded,limits);
    limits={};limits.entries=0;bad(minimal+"[START_SPELLS]\nSPELL_1=1\n",PersistenceErrorCode::limitExceeded,limits);
    limits={};limits.maximumIndex=91;bad(minimal+"[START_SPELLS]\nSPELL_92=1\n",PersistenceErrorCode::limitExceeded,limits);
    check(good(decodeWizard(text(minimal+"[START_SPELLS]\nSPELL_65535=0\n"))).startSpells.size()==1,"sparse index storage");
    std::cout<<"Wizard loader fixtures passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
