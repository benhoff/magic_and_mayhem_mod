#include "dsound_setup.hpp"
#include <algorithm>
#include <limits>

namespace mnm::reconstruction::audio {
mnm::audio::PcmFormat primaryFormat(std::uint32_t caps){
    // Preserve the original fixed alignment/average even in capability fallbacks.
    return {1,std::uint16_t(caps&2?2:1),22050,88200,4,std::uint16_t(caps&8?16:8),0};
}
Descriptor32 secondaryDescriptor(std::uint32_t bytes,std::uint32_t address){return {20,0xea,bytes,0,address};}
std::array<std::uint8_t,20> encodeDescriptor(const Descriptor32& d){
    std::array<std::uint8_t,20> result{};const std::uint32_t fields[]={d.size,d.flags,d.bytes,d.reserved,d.formatAddress};
    for(std::size_t i=0;i<5;++i)for(std::size_t b=0;b<4;++b)result[i*4+b]=std::uint8_t(fields[i]>>(8*b));
    return result;
}
PrimaryResult setupPrimary(PrimaryBackend& backend){
    auto status=backend.create(primaryDescriptor());if(status)return {status,false,{}};
    std::uint32_t caps=0,bytes=0;status=backend.deviceCaps(caps);if(status)return {status,true,{}};
    status=backend.setFormat(primaryFormat(caps));if(status)return {status,true,{}};
    const auto queried=backend.bufferBytes(bytes); // Original ignores this HRESULT.
    return {backend.compact(),true,queried==0?std::optional<std::uint32_t>(bytes):std::nullopt};
}
std::array<std::uint8_t,18> encodeFormat(const mnm::audio::PcmFormat& f){
    std::array<std::uint8_t,18> result{};
    const std::uint32_t values[]={f.tag,f.channels,f.rate,f.bytesPerSecond,f.alignment,f.bits,f.extra};
    const std::size_t sizes[]={2,2,4,4,2,2,2};std::size_t at=0;
    for(std::size_t i=0;i<7;++i)for(std::size_t b=0;b<sizes[i];++b)result[at++]=std::uint8_t(values[i]>>(8*b));
    return result;
}
UploadResult uploadStatic(mnm::audio::Device& device,const mnm::audio::Wave& wave){
    using mnm::audio::Error;mnm::audio::BufferId id=0;
    if(wave.samples.size()>std::numeric_limits<std::uint32_t>::max())return {Error::limit,0,0};
    auto error=device.createStatic(secondaryDescriptor(std::uint32_t(wave.samples.size()),0).flags,wave.format,wave.samples.size(),id);
    if(error!=Error::ok)return {error,0,0};
    mnm::audio::WriteLock lock;error=device.lock(id,0,wave.samples.size(),0,lock);
    if(error!=Error::ok){device.release(id);return {error,0,0};}
    // Original supplies no second region: offset zero + whole buffer is contiguous.
    if(lock.second.size || lock.first.size!=wave.samples.size()){device.release(id);return {Error::invalid,0,0};}
    std::copy(wave.samples.begin(),wave.samples.end(),lock.first.data);
    error=device.unlock(lock,wave.samples.size(),0);
    if(error!=Error::ok){device.release(id);return {error,0,0};}
    return {Error::ok,id,wave.samples.size()};
}
}
