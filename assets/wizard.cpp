#include "wizard.hpp"
#include "persistence_internal.hpp"
#include <charconv>

namespace mnm::assets {
namespace {
using namespace persistence_detail;
std::string lower(std::string s) {
    for(auto& c:s) if(c>='A' && c<='Z') c=static_cast<char>(c-'A'+'a');
    return s;
}
std::string trim(std::string s) {
    const auto begin=s.find_first_not_of(" \t\r");
    return begin==std::string::npos ? std::string{} : s.substr(begin,s.find_last_not_of(" \t\r")-begin+1);
}
Bytes uncomment(const Bytes& bytes) {
    Bytes clean=bytes;
    bool quoted=false, comment=false;
    for(std::size_t i=0;i<clean.size();++i) {
        const auto c=bytes[i];
        if(c==0) fail(PersistenceErrorCode::malformedData,i,"NUL in WZD text");
        if(c=='\n' || c=='\r') {
            if(quoted) fail(PersistenceErrorCode::malformedData,i,"unterminated WZD quote");
            comment=false;
        } else if(comment) clean[i]=' ';
        else if(c=='"') quoted=!quoted;
        else if(c==';' && !quoted) {comment=true;clean[i]=' ';}
    }
    if(quoted) fail(PersistenceErrorCode::malformedData,bytes.size(),"unterminated WZD quote");
    return clean;
}
std::int32_t integer(const std::string& s,const std::string& field) {
    std::int32_t value=0;
    const auto parsed=std::from_chars(s.data(),s.data()+s.size(),value);
    if(parsed.ec!=std::errc{} || parsed.ptr!=s.data()+s.size())
        fail(PersistenceErrorCode::malformedData,0,"invalid WZD integer: "+field);
    return value;
}
bool boolean(const std::string& s,const std::string& field) {
    const auto v=lower(s);
    if(v=="true" || v=="1") return true;
    if(v=="false" || v=="0") return false;
    fail(PersistenceErrorCode::malformedData,0,"invalid WZD boolean: "+field);
}
std::uint32_t index(const std::string& s,const WizardLimits& limits) {
    std::uint32_t value=0;
    const auto parsed=std::from_chars(s.data(),s.data()+s.size(),value);
    if(s.empty() || parsed.ec!=std::errc{} || parsed.ptr!=s.data()+s.size())
        fail(PersistenceErrorCode::malformedData,0,"invalid WZD indexed name: "+s);
    cap(value,limits.maximumIndex);
    return value;
}
std::optional<std::int32_t> number(const Config& c,const std::string& section,const char* key) {
    const auto* value=c.find(section,key);
    return value ? std::optional<std::int32_t>(integer(*value,section+"/"+key)) : std::nullopt;
}
std::optional<bool> flag(const Config& c,const std::string& section,const char* key) {
    const auto* value=c.find(section,key);
    return value ? std::optional<bool>(boolean(*value,section+"/"+key)) : std::nullopt;
}
std::optional<std::string> string(const Config& c,const std::string& section,const char* key,const WizardLimits& limits) {
    const auto* found=c.find(section,key);
    if(!found) return std::nullopt;
    auto value=trim(*found);
    if(!value.empty() && value.front()=='"') {
        if(value.size()<2 || value.back()!='"') fail(PersistenceErrorCode::malformedData,0,"invalid quoted WZD string");
        value=value.substr(1,value.size()-2);
    }
    cap(value.size(),limits.stringBytes);
    return value;
}
WizardAction action(const Config& c,const std::string& section,const WizardLimits& limits) {
    WizardAction a;
    a.valid=flag(c,section,"valid");
    const std::pair<const char*,std::optional<std::int32_t> WizardAction::*> fields[]={
        {"begintime",&WizardAction::beginTime},{"endtime",&WizardAction::endTime},
        {"dependancy",&WizardAction::dependency},{"waitingtime",&WizardAction::waitingTime},
        {"rating",&WizardAction::rating},{"numberofactions",&WizardAction::numberOfActions},
        {"health",&WizardAction::health},{"mana",&WizardAction::mana},
        {"sectionid",&WizardAction::sectionId},{"spelltype",&WizardAction::spellType},
        {"spelltargetid",&WizardAction::spellTargetId},{"xposition",&WizardAction::xPosition},
        {"yposition",&WizardAction::yPosition},{"zposition",&WizardAction::zPosition},
        {"xtargetposition",&WizardAction::xTargetPosition},{"ytargetposition",&WizardAction::yTargetPosition},
        {"ztargetposition",&WizardAction::zTargetPosition}};
    for(const auto& f:fields) a.*f.second=number(c,section,f.first);
    a.nodeType=string(c,section,"nodetype",limits);a.targetNodeType=string(c,section,"targetnodetype",limits);
    a.destination=string(c,section,"destination",limits);a.spellTarget=string(c,section,"spelltarget",limits);
    return a;
}
}
WizardResult decodeWizard(const Bytes& bytes,const WizardLimits& limits) {
    return guarded<WizardDefinition>([&] {
        cap(bytes.size(),limits.inputBytes);
        PersistenceLimits textLimits;textLimits.decodedBytes=limits.inputBytes;textLimits.lines=limits.lines;
        auto result=decodeConfig(uncomment(bytes),textLimits);
        if(const auto* error=std::get_if<PersistenceError>(&result)) throw *error;
        WizardDefinition w;w.config=std::get<Config>(std::move(result));w.source=bytes;
        const auto& c=w.config;
        if(!c.annotations.empty()) fail(PersistenceErrorCode::malformedData,c.annotations.front().first,"unexpected WZD non-property line");
        w.validWizardFile=flag(c,"header","validwizardfile");
        if(w.validWizardFile && !*w.validWizardFile) fail(PersistenceErrorCode::malformedData,0,"ValidWizardFile is false");
        auto name=string(c,"general","name",limits);
        if(!name || name->empty()) fail(PersistenceErrorCode::malformedData,0,"WZD requires a nonempty GENERAL/Name");
        w.stats.name=*name;w.stats.animationFile=string(c,"general","wizardanimfile",limits);
        const std::pair<const char*,std::optional<std::int32_t> WizardStats::*> fields[]={
            {"maxhealth",&WizardStats::maxHealth},{"maxmana",&WizardStats::maxMana},
            {"intelligence",&WizardStats::intelligence},{"magicresistance",&WizardStats::magicResistance},
            {"controllimit",&WizardStats::controlLimit},{"combatmodifier",&WizardStats::combatModifier},
            {"difficultymodifier",&WizardStats::difficultyModifier},{"aggressiveattack",&WizardStats::aggressiveAttack},
            {"cautiousattack",&WizardStats::cautiousAttack},{"occupypowerpoint",&WizardStats::occupyPowerPoint},
            {"collectmana",&WizardStats::collectMana},{"evadedetection",&WizardStats::evadeDetection},
            {"scout",&WizardStats::scout},{"retreat",&WizardStats::retreat},
            {"collectfood",&WizardStats::collectFood},{"protectwizard",&WizardStats::protectWizard},
            {"starttalismans_law",&WizardStats::startTalismansLaw},{"starttalismans_neutral",&WizardStats::startTalismansNeutral},
            {"starttalismans_chaos",&WizardStats::startTalismansChaos}};
        for(const auto& f:fields) w.stats.*f.second=number(c,"general",f.first);
        if(!w.stats.maxMana) fail(PersistenceErrorCode::malformedData,0,"WZD requires GENERAL/MaxMana");
        std::size_t entries=0;
        for(const auto& s:c.sections) {
            if(s.first.rfind("item_",0)==0) {
                cap(++entries,limits.entries);
                const auto id=index(s.first.substr(5),limits);
                if(!w.items.emplace(id,WizardItem{flag(c,s.first,"hasit"),flag(c,s.first,"researched")}).second)
                    fail(PersistenceErrorCode::malformedData,0,"duplicate numeric item index");
            } else if(s.first.rfind("action_",0)==0) {
                cap(++entries,limits.entries);
                const auto id=index(s.first.substr(7),limits);
                if(!w.actions.emplace(id,action(c,s.first,limits)).second)
                    fail(PersistenceErrorCode::malformedData,0,"duplicate numeric action index");
            }
        }
        auto selections=[&](const char* section,const std::string& prefix,std::map<std::uint32_t,bool>& destination) {
            const auto found=c.sections.find(section);if(found==c.sections.end()) return;
            for(const auto& entry:found->second) {
                if(entry.first.rfind(prefix,0)!=0) continue; // Unknown fields stay in Config.
                cap(++entries,limits.entries);
                const auto id=index(entry.first.substr(prefix.size()),limits);
                if(!destination.emplace(id,boolean(entry.second,entry.first)).second)
                    fail(PersistenceErrorCode::malformedData,0,"duplicate numeric selection index");
            }
        };
        selections("start_spells","spell_",w.startSpells);selections("start_objects","object_",w.startObjects);
        selections("start_magic_items","mitem_",w.startMagicItems);
        return w;
    });
}
WizardResult loadWizard(AssetFile& file,const WizardLimits& limits) {
    PersistenceLimits inputLimits;inputLimits.inputBytes=limits.inputBytes;
    return persistence_detail::load<WizardDefinition>(file,inputLimits,[&](const Bytes& bytes){return decodeWizard(bytes,limits);});
}
}
