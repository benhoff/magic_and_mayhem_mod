#pragma once
#include "buffers.hpp"
#include <array>

namespace mnm::reconstruction::audio {
// DWORD addresses remain 32-bit even when reconstruction runs on an x86-64 host.
struct Descriptor32 {std::uint32_t size,flags,bytes,reserved,formatAddress;};
static_assert(sizeof(Descriptor32)==20);
struct DeviceRequest {bool defaultDevice=true,noAggregation=true;std::uint32_t cooperativeLevel=2;};
constexpr DeviceRequest deviceRequest(){return {};}
constexpr Descriptor32 primaryDescriptor(){return {20,0x81,0,0,0};}
mnm::audio::PcmFormat primaryFormat(std::uint32_t deviceCaps);
Descriptor32 secondaryDescriptor(std::uint32_t bytes,std::uint32_t formatAddress);
std::array<std::uint8_t,20> encodeDescriptor(const Descriptor32& descriptor);
std::array<std::uint8_t,18> encodeFormat(const mnm::audio::PcmFormat& format);
using Status=std::int32_t;
// Narrow 0x0056fe80 contract. Ownership on failure belongs to the outer manager.
struct PrimaryBackend {
    virtual ~PrimaryBackend()=default;
    virtual Status create(const Descriptor32& descriptor)=0;
    virtual Status deviceCaps(std::uint32_t& flags)=0;
    virtual Status setFormat(const mnm::audio::PcmFormat& format)=0;
    virtual Status bufferBytes(std::uint32_t& bytes)=0;
    virtual Status compact()=0;
};
struct PrimaryResult {Status status;bool created;std::optional<std::uint32_t> bytes;};
PrimaryResult setupPrimary(PrimaryBackend& backend);

struct UploadResult {mnm::audio::Error error;mnm::audio::BufferId buffer;std::size_t copied;};
// 0x00570140's successful static upload path, expressed through native storage.
// Strict WAV validation occurs before this call; no claim to emulate mmio failures.
UploadResult uploadStatic(mnm::audio::Device& device,const mnm::audio::Wave& wave);
}
