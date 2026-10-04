#include "manager_configuration.hpp"
#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
std::int32_t integerPrefix(const std::string& s){
    std::size_t i=0;while(i<s.size() && std::isspace(static_cast<unsigned char>(s[i])))++i;
    bool negative=false;if(i<s.size() && (s[i]=='+' || s[i]=='-'))negative=s[i++]=='-';
    std::uint64_t value=0;
    while(i<s.size() && s[i]>='0' && s[i]<='9'){
        value=value*10+std::uint32_t(s[i++]-'0');
        if(value>std::uint64_t(std::numeric_limits<std::int32_t>::max())+negative)
            throw std::invalid_argument("Outside recovered atoi integer domain");
    }
    return std::int32_t(negative?-std::int64_t(value):std::int64_t(value));
}
void validate(const ProfileSection& s){
    if(s.returned>=0x3ffe)throw std::invalid_argument("Truncated profile section");
    std::size_t bytes=0;for(const auto& e:s.entries){
        if(e.find('\0')!=std::string::npos)throw std::invalid_argument("Embedded section terminator");
        bytes+=e.size()+1;
    }
    if(bytes!=s.returned)throw std::invalid_argument("Inconsistent decoded MULTI_SZ count");
}
}
void releaseSourcePool(LifetimeBackend& b,SourcePool& pool){
    // Original traverses backward, starting at head->previous, ending at head.
    if(pool.head){auto* start=pool.head->previous;auto* v=start;
        do{auto* previous=v->previous;destroyWrapper(b,*v,1);v=previous;}while(v!=start);
    }
    pool.head=nullptr;pool.nodes.clear();
}
std::string managerProfilePath(const std::string& root){
    const auto path=root+"\\Sounds.ini";
    if(path.size()>=260 || root.find('\0')!=std::string::npos)throw std::invalid_argument("Original manager path buffer overflow");
    return path;
}
std::vector<std::int32_t> soundTable(const ProfileSection& s){
    validate(s);std::vector<std::int32_t> positive;
    for(const auto& entry:s.entries){
        if(!entry.empty() && entry[0]==';')continue;
        const auto equal=entry.find('=');
        if(equal==std::string::npos || equal<1 || equal>6)throw std::invalid_argument("Invalid profile ID key");
        const auto id=integerPrefix(entry.substr(0,equal));if(id>0)positive.push_back(id);
    }
    std::vector<std::int32_t> ids(positive.size(),0);std::size_t used=0;
    for(auto id:positive)if(std::find(ids.begin(),ids.end(),id)==ids.end())ids[used++]=id;
    std::sort(ids.begin(),ids.end());return ids; // Preserve original zero-filled duplicate slack.
}
std::vector<std::int32_t> groupMembers(const std::string& text){
    if(text.size()>=256 || text.find('\0')!=std::string::npos)throw std::invalid_argument("Group scratch overflow");
    std::vector<std::int32_t> result;
    // Delimiter bytes verified by the guarded string export.
    std::size_t begin=0;
    while(begin<text.size()){
        begin=text.find_first_not_of(",",begin);if(begin==std::string::npos)break;
        const auto end=text.find_first_of(",",begin);
        const auto id=integerPrefix(text.substr(begin,end-begin));if(id>0)result.push_back(id);
        if(end==std::string::npos)break;
        begin=end+1;
    }
    return result;
}
Status loadManagerCatalog(ConfigurationBackend& b,AdmissionCatalog& catalog,CatalogObserver* observer){
    try{
        auto ids=soundTable(b.section("Sounds",0x4000));if(ids.empty())return SourceLoadFailure;
        catalog.sourceIds=std::move(ids);
        if(observer)observer->sourceTableCreated(catalog.sourceIds.size());
        auto groups=soundTable(b.section("Randomised",0x4000));
        catalog.groups.clear();
        for(auto id:groups)catalog.groups.push_back({id,{}});
        if(observer)observer->groupTablesCreated(groups.size());
        for(auto& group:catalog.groups){std::string value;const auto n=b.groupValue(group.id,value);
            if(n)group.members=groupMembers(value);
        }
        return 0;
    }catch(const std::invalid_argument&){return SourceLoadFailure;}
}
std::uint32_t dynamicSourceCapacity(std::uint32_t bytes){
    const auto available=0x100000u-bytes;
    const auto high=std::uint32_t((std::uint64_t(available)*0x6c16c16du)>>32);
    return (((available-high)>>1)+high)>>15;
}
Status initializeSourcePool(ConfigurationBackend& b,SourceCacheState& state,SourcePool& pool,std::uint32_t map){
    if(!state.manager.initialized)return 0;
    releaseSourcePool(b,pool);pool.map=map;state.classes.assign(state.catalog.sourceIds.size(),1);state.field22c=0;
    for(auto sourceClass:{0u,2u}){
        auto section=b.section(std::to_string(map)+(sourceClass?" Load Temporary":" Load Permanent"),0x4000);
        try{validate(section);}catch(const std::invalid_argument&){return SourceLoadFailure;}
        for(const auto& entry:section.entries){
            if(!entry.empty() && entry[0]==';')continue;
            if(entry.empty() || entry.size()>6)return SourceLoadFailure;
            const auto id=integerPrefix(entry);
            const auto found=std::lower_bound(state.catalog.sourceIds.begin(),state.catalog.sourceIds.end(),id);
            if(found!=state.catalog.sourceIds.end() && *found==id){
                state.classes[std::size_t(found-state.catalog.sourceIds.begin())]=sourceClass;
                if(!sourceClass)++state.field22c; // Duplicate entries counted, even if later overridden.
            }
        }
    }
    pool.pinnedBytes=0;
    for(std::size_t i=0;i<state.classes.size();++i)if(!state.classes[i]){
        std::string value;if(!b.profileValue(state.catalog.sourceIds[i],value))continue;
        const auto path=state.assetRoot+state.pathInfix+sourceEntryName(value)+state.pathSuffix;
        if(path.size()>=260)throw std::invalid_argument("Original source path scratch overflow");
        std::uint32_t bytes=0;if(b.fileSize(path,bytes))pool.pinnedBytes+=bytes;
    }
    state.field234=state.field22c+dynamicSourceCapacity(pool.pinnedBytes);
    // Avoid reproducing original unsafe unbounded allocation/null-pointer writes.
    if(!state.field234 || state.field234>65536)throw std::domain_error("Outside bounded source-pool allocation domain");
    for(std::uint32_t i=0;i<state.field234;++i){auto v=std::make_unique<VoiceWrapper>();v->identity=i+1;pool.nodes.push_back(std::move(v));}
    for(std::size_t i=0;i<pool.nodes.size();++i){auto& v=*pool.nodes[i];
        v.previous=pool.nodes[(i+pool.nodes.size()-1)%pool.nodes.size()].get();
        v.next=pool.nodes[(i+1)%pool.nodes.size()].get();
    }
    pool.head=pool.nodes.front().get();
    for(std::size_t i=0;i<state.classes.size();++i)if(!state.classes[i]){
        const auto status=loadSourceCache(b,state,pool.head,&pool.disabled,state.catalog.sourceIds[i]);
        if(status)return status; // Retain allocated ring and any earlier preloads.
    }
    return 0;
}
void initializeSchedulePool(SchedulePool& pool,std::uint32_t count){
    if(count<2 || count>65536)throw std::invalid_argument("Outside bounded original schedule-ring domain");
    pool.head=nullptr;pool.nodes.clear();
    for(std::uint32_t i=0;i<count;++i)pool.nodes.push_back(std::make_unique<ScheduleNode>());
    for(std::size_t i=0;i<count;++i){auto& n=*pool.nodes[i];n.next=pool.nodes[(i+1)%count].get();n.previous=pool.nodes[(i+count-1)%count].get();}
    pool.head=pool.nodes.front().get();
}
void initializeConfiguredSchedules(ConfigurationBackend& b,SchedulePool& pool,std::uint32_t& globalLimit){
    globalLimit=b.profileInteger("Optimisation","MaxSimultaneousSounds",globalLimit);
    initializeSchedulePool(pool,globalLimit);
}
}
