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

// Thread-confined sample ownership foundation. No device output or mixer yet.
// Duplicated secondary voices share committed samples, with distinct identities.
class Device {
public:
    explicit Device(std::size_t maxBufferBytes=16*1024*1024,std::size_t maxBuffers=128);
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
    std::optional<BufferInfo> info(BufferId id) const;
    std::vector<std::uint8_t> samples(BufferId id) const;
    std::size_t count() const{return buffers_.size();}
    static constexpr std::uint32_t capabilities=0x0f; // Native PCM policy: mono/stereo, 8/16-bit.
private:
    struct Storage;
    struct Buffer {bool primary;std::uint32_t flags;std::optional<PcmFormat> format;std::shared_ptr<Storage> storage;};
    std::unordered_map<BufferId,Buffer> buffers_;
    BufferId next_=1;std::uint64_t ticket_=1;std::size_t maxBytes_,maxBuffers_;
};
}
