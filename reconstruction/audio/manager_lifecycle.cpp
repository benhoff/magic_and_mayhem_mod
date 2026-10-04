#include "manager_lifecycle.hpp"
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
void freeSources(ManagerServices b,AudioManager& m,bool unconditional=false){
    if(m.ownsClasses || unconditional){b.lifecycle.freeAllocation(ManagerAllocation::classes);m.ownsClasses=false;m.cache.classes.clear();}
    if(m.ownsSourceIds || unconditional){b.lifecycle.freeAllocation(ManagerAllocation::sourceIds);m.ownsSourceIds=false;m.cache.catalog.sourceIds.clear();}
}
void freeGroups(ManagerServices b,AudioManager& m){
    if(m.ownsGroupIds){b.lifecycle.freeAllocation(ManagerAllocation::groupIds);m.ownsGroupIds=false;}
    if(m.ownsGroupDescriptors){
        for(std::size_t i=m.cache.catalog.groups.size();i>0;--i)if(!m.cache.catalog.groups[i-1].members.empty())
            b.lifecycle.freeAllocation(ManagerAllocation::groupMembers,i-1);
        b.lifecycle.freeAllocation(ManagerAllocation::groupDescriptors);m.ownsGroupDescriptors=false;
    }
    m.cache.catalog.groups.clear();
}
struct Tables:CatalogObserver {
    AudioManager& manager;
    explicit Tables(AudioManager& m):manager(m){}
    void sourceTableCreated(std::size_t count)override{manager.sourceCount=count;manager.ownsSourceIds=manager.ownsClasses=true;}
    void groupTablesCreated(std::size_t count)override{manager.groupCount=count;manager.ownsGroupIds=manager.ownsGroupDescriptors=count!=0;}
};
struct Primary:PrimaryBackend {
    ManagerServices backend;AudioManager& manager;
    Primary(ManagerServices b,AudioManager& m):backend(b),manager(m){}
    Status create(const Descriptor32& d)override{
        const auto status=backend.primary.create(d);manager.cache.manager.primary=backend.lifecycle.primaryIdentity();
        if(!status && !manager.cache.manager.primary)throw std::domain_error("Original successful primary create would dereference a null buffer");
        return status;
    }
    Status deviceCaps(std::uint32_t& f)override{return backend.primary.deviceCaps(f);}
    Status setFormat(const mnm::audio::PcmFormat& f)override{return backend.primary.setFormat(f);}
    Status bufferBytes(std::uint32_t& n)override{const auto status=backend.primary.bufferBytes(n);
        manager.primaryBytes=status?std::nullopt:std::optional<std::uint32_t>(n);return status;}
    Status compact()override{return backend.primary.compact();}
};
}
Status shutdownManager(ManagerServices b,AudioManager& m,ManagerGlobals& globals){
    auto& state=m.cache.manager;if(!state.initialized)return 0;
    if(!state.primary)throw std::domain_error("Original manager shutdown would dereference an absent primary buffer");
    b.controls.volume(state.primary,state.savedVolume); // Ignored HRESULT.
    if(state.active){
        if(!m.schedules.head)throw std::domain_error("Original active shutdown would dereference an absent schedule ring");
        auto* start=m.schedules.head;auto* node=start;
        do{
            auto& r=node->record;
            if(r.voiceAddress){
                if(r.deadline==0xffffffffu || !retirementDue(b.controls.tickCount(),r.deadline))b.controls.retireVoice(r.voiceAddress);
                if(r.outputAddress && node->output)*node->output=0;
            }
            clearSchedule(r);node=node->next;
        }while(node!=start);
        globals.lastPrimaryStop=state.lastPrimaryStop=0;state.active=false;
        globals.lastPrimaryStop=state.lastPrimaryStop=b.controls.stop(state.primary);
    }
    releaseSourcePool(b.files,m.sources);
    globals.device=globals.manager=0;
    if(state.device){b.controls.releaseDevice(state.device);state.device=0;}
    freeSources(b,m);freeGroups(b,m);state.initialized=false;return 0;
}
Status initializeManager(ManagerServices b,AudioManager& m,ManagerGlobals& globals,
                         std::uint32_t application,std::uint32_t window,const std::string& root){
    if(m.destroyed || m.cache.manager.initialized || m.ownsSourceIds || m.ownsClasses || m.ownsGroupDescriptors || m.ownsGroupIds)
        throw std::invalid_argument("Manager startup requires constructor or fully shut-down ownership state");
    m.application=application;m.window=window;m.cache.assetRoot=root;m.profilePath=managerProfilePath(root);
    if(!b.lifecycle.openProfile(m.profilePath))return SourceLoadFailure;
    Tables tables(m);auto status=loadManagerCatalog(b.files,m.cache.catalog,&tables);if(status)return status;
    auto& state=m.cache.manager;status=b.lifecycle.createDevice(deviceRequest(),state.device);
    if(status){freeSources(b,m);return status;} // Groups deliberately survive; initialized remains false.
    if(!state.device)throw std::domain_error("Original successful device create would dereference a null device");
    state.initialized=true;
    status=b.lifecycle.cooperativeLevel(state.device,window,2);
    if(!status){
        state.primary=0;m.primaryBytes=0;Primary primary(b,m);const auto result=setupPrimary(primary);status=result.status;
    }
    if(!status)status=initializePrimaryControls(b.controls,state);
    if(status){
        shutdownManager(b,m,globals);state.initialized=false;
        freeSources(b,m,true); // Original outer block calls free(NULL) after shutdown.
        return status;
    }
    globals.device=state.device;
    // Globals publish before profile integer/schedule allocation; invalid original
    // allocation domains preserve this partial state rather than rolling back.
    initializeConfiguredSchedules(b.files,m.schedules,globals.simultaneousLimit);
    return 0;
}
void destroyManager(ManagerServices b,AudioManager& m,ManagerGlobals& globals){
    if(m.destroyed)return;
    if(m.ownsDisabled){destroyWrapper(b.files,m.sources.disabled,1);m.ownsDisabled=false;}
    shutdownManager(b,m,globals);
    if(!m.schedules.nodes.empty())b.lifecycle.freeAllocation(ManagerAllocation::schedules);
    m.schedules.head=nullptr;m.schedules.nodes.clear();m.destroyed=true;
    // Uninitialized managers skip original table cleanup, including failed-create
    // retained groups. Host RAII eventually reclaims annotations, not engine heaps.
}
}
