#pragma once
#include "voice_admission.hpp"
#include <string>
namespace mnm::reconstruction::audio {
inline constexpr Status SourceLoadFailure=-2147467259; // E_FAIL, 0x80004005
struct SourceCacheState {
    ManagerState manager;
    AdmissionCatalog catalog;
    std::vector<std::uint32_t> classes; // Parallel source-ID table, manager +0x24c.
    std::uint32_t field22c=0,field234=0; // Rank divisor uses their wrapping difference.
    std::string assetRoot,pathInfix="\\",pathSuffix=".wav"; // Decoded +0x20 / 0x5d9058 / 0x5f0448.
};
struct SourceCacheBackend:LifetimeBackend {
    // Decoded GetPrivateProfileStringA result and output; profile semantics delegated.
    virtual std::uint32_t profileValue(std::int32_t sound,std::string& output)=0;
    virtual bool openWave(const std::string& path)=0;
    virtual void closeWave()=0;
    virtual std::uint32_t waveBytes()=0;
    virtual mnm::audio::PcmFormat waveFormat()=0;
    virtual Status createSource(const Descriptor32&,const mnm::audio::PcmFormat&,std::uint32_t& buffer)=0;
    virtual Status lockSource(std::uint32_t buffer,std::uint32_t bytes,std::uint8_t*& first)=0;
    virtual std::uint32_t readSource(std::uint8_t*& first,std::uint32_t bytes)=0;
    virtual Status unlockSource(std::uint32_t buffer,std::uint8_t* first,std::uint32_t bytes)=0;
    virtual std::uint32_t waveDuration()=0;
};
// Exact wrapping score arithmetic followed by signed comparison in the selector.
std::int32_t sourceCacheScore(const VoiceWrapper&,std::uint32_t rank,std::uint32_t step);
VoiceWrapper* selectSourceCache(SourceCacheBackend&,VoiceWrapper* head,std::uint32_t field22c,std::uint32_t field234);
// Valid quoted-comment domain: first semicolon truncates after nearest preceding apostrophe.
// Malformed strings/oversized fixed-buffer domains reject rather than emulate OOB writes.
std::string sourceEntryName(std::string value);
// Selected 0x570140 ownership and upload sequence, not WinMM parser/SEH internals.
Status recycleSource(SourceCacheBackend&,VoiceWrapper&,const std::string& path,
                     std::int32_t source,std::uint32_t sourceClass);
// 0x56f400: group preload, cached hit, victim/profile/upload and success publication.
// Cached hit deliberately leaves output untouched. Valid rings/acyclic group graph required.
Status loadSourceCache(SourceCacheBackend&,const SourceCacheState&,VoiceWrapper*& head,
                       VoiceWrapper* disabled,std::int32_t sound,VoiceWrapper** output=nullptr);
}
