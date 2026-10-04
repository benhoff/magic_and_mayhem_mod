#include "persistence_internal.hpp"
#include <charconv>
#include <cstdio>

namespace mnm::assets {
namespace {
using namespace persistence_detail;
std::string lower(std::string s) {for(auto& c:s) if(c>='A'&&c<='Z') c=static_cast<char>(c-'A'+'a');return s;}
std::string trim(std::string s) {
    auto first=s.find_first_not_of(" \t\r");if(first==std::string::npos) return {};
    return s.substr(first,s.find_last_not_of(" \t\r")-first+1);
}
std::vector<std::string> lines(const Bytes& bytes,const PersistenceLimits& limits) {
    cap(bytes.size(),limits.decodedBytes);
    std::vector<std::string> result;
    for(std::size_t p=0;p<bytes.size();) {
        auto end=std::find(bytes.begin()+p,bytes.end(),'\n');std::size_t n=static_cast<std::size_t>(end-bytes.begin());
        auto line=std::string(bytes.begin()+p,end);
        if(!line.empty()&&line.back()=='\r') line.pop_back();
        if(line.find('\0')!=std::string::npos) fail(PersistenceErrorCode::malformedData,p,"NUL in text file");
        cap(result.size()+1,limits.lines,p);result.push_back(std::move(line));p=n+(end!=bytes.end());
    }
    return result;
}
std::string required(const Config& c,const std::string& section,const std::string& key) {
    auto* v=c.find(section,key);if(!v) fail(PersistenceErrorCode::malformedData,0,"missing "+section+"/"+key);return *v;
}
std::int32_t integer(const std::string& value,const std::string& key) {
    std::int32_t v=0;auto r=std::from_chars(value.data(),value.data()+value.size(),v);
    if(r.ec!=std::errc{}||r.ptr!=value.data()+value.size()) fail(PersistenceErrorCode::malformedData,0,"invalid decimal integer: "+key);
    return v;
}
std::int32_t number(const Config& c,const std::string& section,const std::string& key) {return integer(required(c,section,key),key);}
std::string indexed(const char* prefix,std::uint32_t index) {char out[64];std::snprintf(out,sizeof(out),"%s%02u",prefix,index);return out;}
}
const std::string* Config::find(std::string section,std::string key) const {
    auto s=sections.find(lower(std::move(section)));if(s==sections.end()) return nullptr;
    auto k=s->second.find(lower(std::move(key)));return k==s->second.end()?nullptr:&k->second;
}
PersistenceResult<Config> decodeConfig(const Bytes& bytes,const PersistenceLimits& limits) {
    return guarded<Config>([&] {
        Config c;std::string section;std::size_t offset=0;
        for(const auto& raw:lines(bytes,limits)) {
            auto line=trim(raw);
            if(!line.empty()&&line[0]!=';'&&line[0]!='#') {
                if(line[0]=='[') {
                    if(line.back()!=']') fail(PersistenceErrorCode::malformedData,offset,"unterminated section");
                    section=lower(trim(line.substr(1,line.size()-2)));
                    if(section.empty()) fail(PersistenceErrorCode::malformedData,offset,"empty section name");
                    c.sections.try_emplace(section);
                } else {
                    auto split=line.find('=');
                    auto key=split==std::string::npos ? std::string{} : lower(trim(line.substr(0,split)));
                    if(key.empty()) c.annotations.emplace_back(offset,raw);
                    else if(!c.sections[section].emplace(key,trim(line.substr(split+1))).second)
                        fail(PersistenceErrorCode::malformedData,offset,"duplicate key: "+key);
                }
            }
            auto end=std::find(bytes.begin()+offset,bytes.end(),'\n');
            offset=static_cast<std::size_t>(end-bytes.begin())+(end!=bytes.end());
        }
        return c;
    });
}
PersistenceResult<Config> loadConfig(AssetFile& file,bool packed,const PersistenceLimits& limits) {
    return persistence_detail::load<Config>(file,limits,[&](const Bytes& b)->PersistenceResult<Config> {
        if(!packed) return decodeConfig(b,limits);
        auto result=decodePackedContainer(b,limits);if(auto* e=std::get_if<PersistenceError>(&result)) return *e;
        return decodeConfig(std::get<PackedContainer>(result).decoded,limits);
    });
}
PersistenceResult<RealmConfig> decodeRealmConfig(const Bytes& bytes,const PersistenceLimits& limits) {
    return guarded<RealmConfig>([&] {
        auto parsed=decodeConfig(bytes,limits);if(auto* e=std::get_if<PersistenceError>(&parsed)) throw *e;
        const auto& c=std::get<Config>(parsed);RealmConfig realm;
        realm.name=required(c,"GENERAL","currentRealmName");realm.nextRealm=required(c,"GENERAL","nextRealm");
        realm.playerWizard=number(c,"GENERAL","playerWizard1");realm.lastRegion=number(c,"GENERAL","lastRegion");
        auto w=number(c,"GENERAL","wizardCount"),r=number(c,"GENERAL","regionCount");
        if(w<1||w>80||r<1||r>20||realm.playerWizard<0||realm.playerWizard>=w||realm.lastRegion< -1||realm.lastRegion>=r)
            fail(PersistenceErrorCode::malformedData,0,"realm counts or indices outside supported capacities");
        realm.regionCount=static_cast<std::uint32_t>(r);
        for(int i=0;i<w;++i) {
            auto section=indexed("WIZARD_",i);
            RealmWizardConfig wizard{number(c,section,"wizardIcon"),number(c,section,"wizardFlag"),number(c,section,"wizardLocation"),number(c,section,"wizardMovement_oldRegion")};
            realm.wizards.push_back(wizard);
        }
        realm.regionOwners.resize(r);
        for(int i=0;i<r;++i) if(auto* v=c.find("REGION_INFO",indexed("regionOwner_",i))) {
            auto owner=integer(*v,"region owner");
            if(owner< -1||owner>=w) fail(PersistenceErrorCode::malformedData,0,"owner outside wizard range");
            realm.regionOwners[i]=owner;
        }
        return realm;
    });
}
PersistenceResult<RealmConfig> loadRealmConfig(AssetFile& f,const PersistenceLimits& l) {return persistence_detail::load<RealmConfig>(f,l,[&](const Bytes& b){return decodeRealmConfig(b,l);});}
PersistenceResult<std::vector<std::string>> decodeRegionNames(const Bytes& b,const PersistenceLimits& l) {
    return guarded<std::vector<std::string>>([&]{return lines(b,l);});
}
PersistenceResult<std::vector<std::string>> loadRegionNames(AssetFile& f,const PersistenceLimits& l) {return persistence_detail::load<std::vector<std::string>>(f,l,[&](const Bytes& b){return decodeRegionNames(b,l);});}
}
