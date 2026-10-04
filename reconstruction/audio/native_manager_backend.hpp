#pragma once
#include "manager_lifecycle.hpp"
#include "profile.hpp"
#include "wave_loader.hpp"
#include <functional>
#include <unordered_map>
#include <unordered_set>
namespace mnm::reconstruction::audio {
// Explicit native adaptation; literal keeps the recovered filename boundary.
enum class NativeSourcePathPolicy {literal, dequoteMissingLeaf};
// Thread-confined native adapter. It opens no audio output device and invokes no
// Windows API. Supplied manager and caller slots must outlive this backend.
class NativeManagerBackend final:public ConfigurationBackend,public PrimaryBackend,
    public ManagerBackend,public AdmissionBackend,public LifecycleBackend {
public:
    NativeManagerBackend(mnm::assets::AssetStore,AudioManager&,
                         std::function<std::uint32_t()> ticks,std::function<std::uint32_t()> random,
                         std::uint32_t outputRate=48000,NativeSourcePathPolicy sourcePaths=NativeSourcePathPolicy::literal);
    ~NativeManagerBackend() override;
    ManagerServices services(){return {*this,*this,*this,*this};}
    mnm::audio::Device& device(){return device_;}
    std::vector<std::uint8_t> bufferSamples(std::uint32_t token) const{return device_.samples(native(token));}
    const std::string& diagnostic() const{return diagnostic_;}
    const std::optional<mnm::assets::Error>& assetError() const{return assetError_;}
    // Remove logically disposed duplicate annotations at an idle boundary.
    void collectDisposed();
    bool openProfile(const std::string&) override;
    ProfileSection section(const std::string&,std::uint32_t) override;
    std::uint32_t profileValue(std::int32_t,std::string&) override;
    std::uint32_t groupValue(std::int32_t,std::string&) override;
    std::uint32_t profileInteger(const std::string&,const std::string&,std::uint32_t) override;
    bool fileSize(const std::string&,std::uint32_t&) override;
    Status createDevice(const DeviceRequest&,std::uint32_t&) override;
    Status cooperativeLevel(std::uint32_t,std::uint32_t,std::uint32_t) override;
    std::uint32_t primaryIdentity() override{return primary_;}
    void freeAllocation(ManagerAllocation,std::size_t) override;
    Status create(const Descriptor32&) override;
    Status deviceCaps(std::uint32_t&) override;
    Status setFormat(const mnm::audio::PcmFormat&) override;
    Status bufferBytes(std::uint32_t&) override;
    Status compact() override;
    Status getVolume(std::uint32_t,std::int32_t&) override;
    Status getStatus(std::uint32_t,std::uint32_t&) override;
    Status stop(std::uint32_t) override;
    Status position(std::uint32_t,std::uint32_t) override;
    Status volume(std::uint32_t,std::int32_t) override;
    Status pan(std::uint32_t,std::int32_t) override;
    Status play(std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t) override;
    void volumeRecord(VoiceContract&,std::int32_t) override;
    std::uint32_t tickCount() override;
    std::uint32_t randomWord() override;
    std::uint32_t bufferForVoice(std::uint32_t) override;
    void retireVoice(std::uint32_t) override;
    void releaseVoice(std::uint32_t) override;
    void releaseDevice(std::uint32_t) override;
    void clearVoiceSchedule(std::uint32_t) override;
    void releaseBuffer(std::uint32_t) override;
    void freeWrapper(std::uint32_t) override;
    Status loadSource(std::int32_t,VoiceWrapper*&) override;
    Status duplicateBuffer(std::uint32_t,std::uint32_t&) override;
    VoiceWrapper& allocateWrapper() override;
    bool openWave(const std::string&) override;
    void closeWave() override;
    std::uint32_t waveBytes() override;
    mnm::audio::PcmFormat waveFormat() override;
    Status createSource(const Descriptor32&,const mnm::audio::PcmFormat&,std::uint32_t&) override;
    Status lockSource(std::uint32_t,std::uint32_t,std::uint8_t*&) override;
    std::uint32_t readSource(std::uint8_t*&,std::uint32_t) override;
    Status unlockSource(std::uint32_t,std::uint8_t*,std::uint32_t) override;
    std::uint32_t waveDuration() override;
private:
    mnm::assets::Result<std::unique_ptr<mnm::assets::AssetFile>> openSourceFile(const std::string&);
    NativeSourcePathPolicy sourcePaths_;
    static Status status(mnm::audio::Error);
    Status bind(mnm::audio::Error,mnm::audio::BufferId,std::uint32_t&);
    mnm::audio::BufferId native(std::uint32_t) const;
    bool primary(std::uint32_t) const;
    VoiceWrapper* wrapper(std::uint32_t) const;
    const mnm::assets::ProfileSnapshot& profile() const;
    const mnm::audio::Wave& wave() const;
    void failure(const mnm::assets::Error&);
    mnm::assets::AssetStore assets_;AudioManager& manager_;mnm::audio::Device device_;
    std::function<std::uint32_t()> ticks_,random_;
    std::optional<mnm::assets::ProfileSnapshot> profile_;
    std::optional<mnm::audio::Wave> wave_;
    std::optional<mnm::assets::Error> assetError_;std::string diagnostic_;
    std::unordered_map<std::uint32_t,mnm::audio::BufferId> buffers_;
    std::unordered_map<std::uint32_t,mnm::audio::WriteLock> locks_;
    std::unordered_set<std::uint32_t> disposed_;
    std::uint32_t nextBuffer_=1,nextWrapper_=65537,primary_=0,activeLock_=0;
    bool activeDevice_=false;
};
}
