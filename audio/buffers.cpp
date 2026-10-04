#include "buffers.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace mnm::audio {
namespace {
std::uint16_t u16(const std::uint8_t* p){return std::uint16_t(p[0])|(std::uint16_t(p[1])<<8);}
std::uint32_t u32(const std::uint8_t* p){return std::uint32_t(p[0])|(std::uint32_t(p[1])<<8)|(std::uint32_t(p[2])<<16)|(std::uint32_t(p[3])<<24);}
bool equal(const std::uint8_t* p,const char* value){return std::memcmp(p,value,4)==0;}
}
bool validPcm(const PcmFormat& f){
    if(f.tag!=1 || (f.channels!=1 && f.channels!=2) || (f.bits!=8 && f.bits!=16) || !f.rate || f.extra)return false;
    const std::uint64_t alignment=f.channels*(f.bits/8),average=alignment*f.rate;
    return alignment==f.alignment && average<=std::numeric_limits<std::uint32_t>::max() && average==f.bytesPerSecond;
}
Wave readWave(const std::vector<std::uint8_t>& file){
    if(file.size()<12 || !equal(file.data(),"RIFF") || !equal(file.data()+8,"WAVE"))throw std::runtime_error("Not RIFF/WAVE");
    const std::uint64_t end=std::uint64_t(u32(file.data()+4))+8;
    if(end<12 || end>file.size())throw std::runtime_error("Truncated RIFF container");
    Wave result;bool format=false,data=false;
    for(std::uint64_t at=12;at<end;){
        if(end-at<8)throw std::runtime_error("Truncated RIFF chunk header");
        const auto* header=file.data()+at;const std::uint64_t length=u32(header+4),payload=at+8;
        if(length>end-payload || (length&1)>end-payload-length)throw std::runtime_error("Truncated RIFF chunk/padding");
        if(equal(header,"fmt ")){
            if(format || length<16 || (length>16 && length<18))throw std::runtime_error("Invalid/duplicate PCM format");
            const auto* p=file.data()+payload;
            result.format={u16(p),u16(p+2),u32(p+4),u32(p+8),u16(p+12),u16(p+14),length>=18?u16(p+16):std::uint16_t(0)};
            if(!validPcm(result.format))throw std::runtime_error("Unsupported/inconsistent PCM format");
            format=true;
        }else if(equal(header,"data")){
            if(data || !length)throw std::runtime_error("Invalid/duplicate PCM data");
            result.samples.assign(file.begin()+std::size_t(payload),file.begin()+std::size_t(payload+length));data=true;
        }
        at=payload+length+(length&1);
    }
    if(!format || !data || result.samples.size()%result.format.alignment)throw std::runtime_error("Missing or partial PCM frames");
    return result;
}
struct Device::Storage {
    explicit Storage(std::size_t size):committed(size,0){}
    std::vector<std::uint8_t> committed,staging;
    BufferId owner=0;std::uint64_t ticket=0,revision=0;
    std::size_t offset=0,first=0,second=0;
};
Device::Device(std::size_t bytes,std::size_t buffers,std::uint32_t rate):maxBytes_(bytes),maxBuffers_(buffers),outputRate_(rate){
    if(!rate)throw std::invalid_argument("Output sample rate must be nonzero");
}
Device::~Device()=default;
Error Device::createPrimary(std::uint32_t flags,BufferId& output){
    if(flags!=0x81)return Error::unsupported;
    if(buffers_.size()>=maxBuffers_)return Error::limit;
    for(const auto& pair:buffers_)if(pair.second.primary)return Error::busy;
    output=next_++;buffers_.emplace(output,Buffer{true,flags,{},nullptr});return Error::ok;
}
Error Device::setPrimaryFormat(BufferId id,const PcmFormat& format){
    auto it=buffers_.find(id);if(it==buffers_.end() || !it->second.primary)return Error::invalid;
    if(!validPcm(format))return Error::badFormat;
    it->second.format=format;return Error::ok;
}
Error Device::createStatic(std::uint32_t flags,const PcmFormat& format,std::size_t bytes,BufferId& output){
    if(flags!=0xea)return Error::unsupported;
    if(!validPcm(format))return Error::badFormat;
    if(!bytes || bytes%format.alignment)return Error::invalid;
    if(bytes>maxBytes_ || buffers_.size()>=maxBuffers_)return Error::limit;
    auto storage=std::make_shared<Storage>(bytes);output=next_++;
    Buffer buffer{false,flags,format,std::move(storage)};buffer.voice.frames=bytes/format.alignment;
    buffers_.emplace(output,std::move(buffer));return Error::ok;
}
Error Device::duplicate(BufferId source,BufferId& output){
    auto it=buffers_.find(source);if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    if(buffers_.size()>=maxBuffers_)return Error::limit;
    auto buffer=it->second;
    // Duplicate controls, not playback activity or cursor. Samples stay shared.
    buffer.voice.playback=Playback::stopped;buffer.voice.looping=false;buffer.voice.frame=0;
    buffer.phase=0;
    output=next_++;buffers_.emplace(output,std::move(buffer));return Error::ok;
}
Error Device::release(BufferId id){
    auto it=buffers_.find(id);if(it==buffers_.end())return Error::invalid;
    auto storage=it->second.storage;
    if(storage && storage->owner==id){storage->owner=0;storage->ticket=0;storage->staging.clear();}
    buffers_.erase(it);return Error::ok;
}
Error Device::lock(BufferId id,std::size_t offset,std::size_t bytes,std::uint32_t flags,WriteLock& output){
    auto it=buffers_.find(id);if(it==buffers_.end() || it->second.primary)return Error::invalid;
    auto& s=*it->second.storage;
    if(flags&~2u)return Error::unsupported; // No write cursor until playback is reconstructed.
    if(flags&2)bytes=s.committed.size();
    if(offset>=s.committed.size() || !bytes || bytes>s.committed.size())return Error::invalid;
    if(s.owner)return Error::busy;
    s.first=std::min(bytes,s.committed.size()-offset);s.second=bytes-s.first;s.offset=offset;
    s.staging.assign(bytes,0);s.owner=id;s.ticket=ticket_++;
    output={id,s.ticket,{s.staging.data(),s.first},{s.second?s.staging.data()+s.first:nullptr,s.second}};return Error::ok;
}
Error Device::unlock(const WriteLock& lock,std::size_t firstWritten,std::size_t secondWritten){
    auto it=buffers_.find(lock.owner);if(it==buffers_.end() || it->second.primary)return Error::invalid;
    auto& s=*it->second.storage;
    if(!s.owner || s.owner!=lock.owner || s.ticket!=lock.ticket || firstWritten>s.first || secondWritten>s.second ||
       lock.first.data!=s.staging.data() || lock.first.size!=s.first || lock.second.size!=s.second ||
       lock.second.data!=(s.second?s.staging.data()+s.first:nullptr))return Error::invalid;
    std::copy_n(s.staging.begin(),firstWritten,s.committed.begin()+s.offset);
    std::copy_n(s.staging.begin()+s.first,secondWritten,s.committed.begin());
    if(firstWritten || secondWritten)++s.revision;
    s.owner=0;s.ticket=0;s.staging.clear();return Error::ok;
}
std::optional<BufferInfo> Device::info(BufferId id) const{
    const auto it=buffers_.find(id);if(it==buffers_.end())return {};
    const auto& b=it->second;return BufferInfo{b.primary,b.flags,b.format,b.storage?b.storage->committed.size():0,b.storage?b.storage->revision:0};
}
std::vector<std::uint8_t> Device::samples(BufferId id) const{
    const auto it=buffers_.find(id);if(it==buffers_.end() || !it->second.storage)throw std::runtime_error("Not a secondary buffer");
    return it->second.storage->committed;
}
std::int32_t Device::sample(const Buffer& buffer,std::uint64_t frame,unsigned channel){
    const auto& f=*buffer.format;
    const auto offset=std::size_t(frame)*f.alignment+(f.channels==1?0:channel)*(f.bits/8);
    const auto* p=buffer.storage->committed.data()+offset;
    if(f.bits==8)return (std::int32_t(p[0])-128)*256;
    const auto value=u16(p);return value<32768?std::int32_t(value):std::int32_t(value)-65536;
}
}
