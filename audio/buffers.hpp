#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mnm::audio {
struct PcmFormat {
    std::uint16_t tag=1,channels=0;
    std::uint32_t rate=0,bytesPerSecond=0;
    std::uint16_t alignment=0,bits=0,extra=0;
};
bool validPcm(const PcmFormat& format);
struct Wave {PcmFormat format;std::vector<std::uint8_t> samples;};
// Strict native reader; not an emulation of the original unchecked mmio calls.
Wave readWave(const std::vector<std::uint8_t>& file);
using BufferId=std::uint64_t;
enum class Error {ok,invalid,unsupported,busy,badFormat,limit};
struct Region {std::uint8_t* data=nullptr;std::size_t size=0;};
struct WriteLock {BufferId owner=0;std::uint64_t ticket=0;Region first,second;};
struct BufferInfo {bool primary=false;std::uint32_t flags=0;std::optional<PcmFormat> format;std::size_t bytes=0;std::uint64_t revision=0;};
enum class Playback {stopped,playing,completed};
struct VoiceInfo {
    Playback playback=Playback::stopped;
    bool looping=false;
    std::uint64_t frame=0,frames=0;
    std::int32_t volume=0,pan=0; // Hundredths of a decibel, not linear gain.
    // DirectSound status bits: playing 0x1, buffer lost 0x2, looping 0x4.
    std::uint32_t status() const{return playback==Playback::playing?(looping?5u:1u):0u;}
};
struct PrimaryState {
    std::int32_t volume=0;bool explicitlyPlaying=false;std::optional<PcmFormat> format;
    std::uint32_t status() const{return explicitlyPlaying?5u:0u;}
};
struct AdvanceResult {std::uint64_t consumed=0;bool completed=false;};

// Thread-confined sample ownership, playback state and offline mixing. No device output.
// Duplicated secondary voices share committed samples, with distinct identities.
// New storage is PCM silence (128 for unsigned 8-bit, 0 for signed 16-bit).
// Errors preserve outputs/state. Allocation exceptions propagate before publication
// or playback mutation. Lock pointers expire on unlock, writer release/destruction.
class Device {
public:
    explicit Device(std::size_t maxBufferBytes=16*1024*1024,std::size_t maxBuffers=128,
                    std::uint32_t outputRate=48000);
    ~Device();
    Device(const Device&)=delete;
    Device& operator=(const Device&)=delete;
    Error createPrimary(std::uint32_t flags,BufferId& output);
    Error setPrimaryFormat(BufferId id,const PcmFormat& format);
    Error createStatic(std::uint32_t flags,const PcmFormat& format,std::size_t bytes,BufferId& output);
    Error duplicate(BufferId source,BufferId& output);
    Error release(BufferId id);
    Error lock(BufferId id,std::size_t offset,std::size_t bytes,std::uint32_t flags,WriteLock& output);
    Error unlock(const WriteLock& lock,std::size_t firstWritten,std::size_t secondWritten);
    Error play(BufferId id,std::uint32_t flags=0); // Only 0 or 1 (whole-buffer looping).
    Error stop(BufferId id); // Retain position; reset is a separate operation.
    Error resetPosition(BufferId id);
    Error setVolume(BufferId id,std::int32_t value); // [-10000,0].
    Error setPan(BufferId id,std::int32_t value); // [-10000,10000].
    Error advanceFrames(BufferId id,std::uint64_t frames,AdvanceResult& result);
    std::optional<VoiceInfo> voice(BufferId id) const;
    // Interleaved signed stereo PCM; advances voices in the fixed output clock.
    Error mixStereo(std::size_t frames,std::vector<std::int16_t>& output);
    PrimaryState primaryState() const{return primary_;}
    Error setPrimaryVolume(std::int32_t value);
    Error playPrimary(std::uint32_t flags=1);
    void stopPrimary(){primary_.explicitlyPlaying=false;}
    Error setPrimaryOutputFormat(const PcmFormat& format);
    std::uint32_t outputRate() const{return outputRate_;}
    static constexpr std::size_t maxMixFrames=65536;
    std::optional<BufferInfo> info(BufferId id) const;
    std::vector<std::uint8_t> samples(BufferId id) const;
    std::size_t count() const{return buffers_.size();}
    static constexpr std::uint32_t capabilities=0x0f; // Native PCM policy: mono/stereo, 8/16-bit.
private:
    struct Storage;
    struct Buffer {bool primary;std::uint32_t flags;std::optional<PcmFormat> format;std::shared_ptr<Storage> storage;VoiceInfo voice{};std::uint32_t phase=0;};
    static void advanceVoice(Buffer&,std::uint64_t frames,AdvanceResult&);
    static std::int32_t sample(const Buffer&,std::uint64_t frame,unsigned channel);
    std::unordered_map<BufferId,Buffer> buffers_;
    BufferId next_=1;std::uint64_t ticket_=1;std::size_t maxBytes_,maxBuffers_;
    std::uint32_t outputRate_;PrimaryState primary_;
};
}
