#include "catalog_preflight.hpp"
#include <algorithm>
namespace mnm::reconstruction::audio {
bool CatalogPreflight::playable(std::int32_t id) const{
    if(!valid())return false;
    for(const auto& g:groups)if(g.id==id)return g.playable;
    for(const auto& s:sources)if(s.id==id)return s.playable;
    return false; // Application rejects unknown requests rather than guessing.
}
CatalogPreflight preflightCatalog(mnm::assets::AssetStore assets,NativeSourcePathPolicy policy){
    CatalogPreflight report;report.policy=policy;report.inspected=true;
    AudioManager manager;
    NativeManagerBackend backend(std::move(assets),manager,[]{return 0u;},[]{return 0u;},48000,policy);
    try{
        if(!backend.openProfile("Sounds.ini"))throw std::runtime_error(backend.diagnostic());
        AdmissionCatalog catalog;
        if(loadManagerCatalog(backend,catalog))throw std::runtime_error("Cannot load audio catalog");
        report.simultaneousLimit=backend.profileInteger("Optimisation","MaxSimultaneousSounds",16);
        if(report.simultaneousLimit<2 || report.simultaneousLimit>65536)throw std::runtime_error("Schedule count outside supported 2..65536 domain");
        for(auto id:catalog.sourceIds){
            if(std::any_of(catalog.groups.begin(),catalog.groups.end(),[&](const auto& g){return g.id==id;}))continue;
            SourceProbe s;s.id=id;
            try{
                std::string value;backend.profileValue(id,value);
                s.path=sourceEntryName(value)+".wav";
                if(!backend.openWave(s.path))s.diagnostic=backend.diagnostic();
                else{
                    s.pcmBytes=backend.waveBytes();s.durationMs=backend.waveDuration();
                    s.playable=s.pcmBytes>0 && s.pcmBytes<=16u*1024*1024;
                    if(!s.playable)s.diagnostic="PCM size outside native source buffer domain (1..16 MiB)";
                }
            }catch(const std::exception& e){s.diagnostic=e.what();}
            backend.closeWave();report.sources.push_back(std::move(s));
        }
        for(const auto& group:catalog.groups){
            GroupProbe g;g.id=group.id;g.members=group.members;g.playable=!g.members.empty();
            if(g.members.empty())g.diagnostic="Empty randomized group";
            for(auto member:g.members){
                const auto resolved=std::binary_search(catalog.sourceIds.begin(),catalog.sourceIds.end(),member)?member:410;
                g.resolved.push_back(resolved);
                const auto s=std::find_if(report.sources.begin(),report.sources.end(),[&](const auto& v){return v.id==resolved;});
                if(s==report.sources.end() || !s->playable){
                    g.playable=false;
                    if(!g.diagnostic.empty())g.diagnostic+="; ";
                    g.diagnostic+="Member "+std::to_string(member)+" resolves to unavailable or nested source "+std::to_string(resolved);
                }
            }
            report.groups.push_back(std::move(g));
        }
    }catch(const std::exception& e){report.diagnostic=e.what();}
    return report;
}
}
