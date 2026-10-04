#pragma once
#include "manager_configuration.hpp"
namespace mnm::reconstruction::audio {
enum class ManagerAllocation {classes,sourceIds,groupIds,groupMembers,groupDescriptors,schedules};
struct LifecycleBackend {
    virtual ~LifecycleBackend()=default;
    // Bind decoded profile reads to this path; result is CRT access success/failure.
    virtual bool openProfile(const std::string& path)=0;
    virtual Status createDevice(const DeviceRequest&,std::uint32_t& device)=0;
    virtual Status cooperativeLevel(std::uint32_t device,std::uint32_t window,std::uint32_t level)=0;
    // Called after PrimaryBackend::create, even on failure (COM out-parameter).
    virtual std::uint32_t primaryIdentity()=0;
    // Logical original heap events; host vectors are owned separately.
    virtual void freeAllocation(ManagerAllocation,std::size_t index=0)=0;
};
struct ManagerGlobals {
    std::uint32_t device=0,manager=0,simultaneousLimit=16;
    Status lastPrimaryStop=0; // 0x6f4370; mirrored in ManagerState host annotation.
};
struct AudioManager {
    SourceCacheState cache;
    SourcePool sources;
    SchedulePool schedules;
    std::uint32_t identity=1,application=0,window=0;
    std::string profilePath;
    std::optional<std::uint32_t> primaryBytes=0;
    std::size_t sourceCount=0,groupCount=0; // Counts survive pointer/table frees.
    bool ownsClasses=false,ownsSourceIds=false,ownsGroupIds=false,ownsGroupDescriptors=false;
    bool ownsDisabled=true,destroyed=false; // Host constructor sentinel annotation.
};
// Supply separate interfaces backed by the same session. PrimaryBackend::create
// must publish its out-parameter through LifecycleBackend::primaryIdentity.
struct ManagerServices {ConfigurationBackend& files;PrimaryBackend& primary;ManagerBackend& controls;LifecycleBackend& lifecycle;};
// Fresh or shut-down manager only; constructor global registration supplied by caller.
// Invalid original dereference/allocation domains throw rather than pretend safe unwind.
Status initializeManager(ManagerServices,AudioManager&,ManagerGlobals&,
                         std::uint32_t application,std::uint32_t window,const std::string& root);
// 0x56e640: restore, retire/clear schedules, Stop, source destruction, globals,
// device, tables. Primary buffer and scheduler allocation are retained.
Status shutdownManager(ManagerServices,AudioManager&,ManagerGlobals&);
// 0x56de50: disabled wrapper before shutdown, scheduler allocation afterwards.
void destroyManager(ManagerServices,AudioManager&,ManagerGlobals&);
}
