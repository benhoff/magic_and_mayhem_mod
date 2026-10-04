#pragma once
#include "source_cache.hpp"
#include <memory>
namespace mnm::reconstruction::audio {
// Decoded profile API results, not a general-purpose INI parser.
struct ProfileSection {std::uint32_t returned=0;std::vector<std::string> entries;};
struct ConfigurationBackend:SourceCacheBackend {
    virtual ProfileSection section(const std::string& name,std::uint32_t capacity)=0;
    // GetPrivateProfileStringA("Randomised", decimal ID, default, capacity 256).
    virtual std::uint32_t groupValue(std::int32_t id,std::string& value)=0;
    virtual std::uint32_t profileInteger(const std::string& section,const std::string& key,
                                         std::uint32_t fallback)=0;
    virtual bool fileSize(const std::string& path,std::uint32_t& size)=0; // CRT stat size word
};
std::string managerProfilePath(const std::string& root);
std::vector<std::int32_t> soundTable(const ProfileSection&);
std::vector<std::int32_t> groupMembers(const std::string&);
// Selected table phase of 0x56df60; device/primary controls remain separate.
struct CatalogObserver {
    virtual ~CatalogObserver()=default;
    virtual void sourceTableCreated(std::size_t count)=0;
    virtual void groupTablesCreated(std::size_t count)=0;
};
Status loadManagerCatalog(ConfigurationBackend&,AdmissionCatalog&,CatalogObserver* observer=nullptr);
// Own stable host nodes, without pretending their pointers are x86 addresses.
struct SourcePool {
    std::vector<std::unique_ptr<VoiceWrapper>> nodes;
    std::vector<std::unique_ptr<VoiceWrapper>> ownedDuplicates; // Optional native host storage; ring roots may rotate here.
    VoiceWrapper* head=nullptr;
    VoiceWrapper disabled;
    std::uint32_t map=0,pinnedBytes=0;
};
// 0x56e910: release old roots, map classifications, size budget, ring, pinned preload.
Status initializeSourcePool(ConfigurationBackend&,SourceCacheState&,SourcePool&,std::uint32_t map);
void releaseSourcePool(LifetimeBackend&,SourcePool&);
struct SchedulePool {
    std::vector<std::unique_ptr<ScheduleNode>> nodes;
    ScheduleNode* head=nullptr;
};
// 0x5719b0 valid count >=2. Rebuild does not stop voices or clear caller slots.
void initializeSchedulePool(SchedulePool&,std::uint32_t count);
// Call after primary controls succeed: global default is caller-supplied.
void initializeConfiguredSchedules(ConfigurationBackend&,SchedulePool&,std::uint32_t& globalLimit);
// Exact 0x56ed55..74 unsigned wrapping arithmetic, before adding pinned count.
std::uint32_t dynamicSourceCapacity(std::uint32_t pinnedBytes);
}
