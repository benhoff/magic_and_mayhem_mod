#include "dsound_setup.hpp"
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <string>

using namespace mnm::audio;
namespace rec=mnm::reconstruction::audio;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void put32(std::vector<std::uint8_t>& b,std::uint32_t v){for(int i=0;i<4;++i)b.push_back(std::uint8_t(v>>(8*i)));}
static void chunk(std::vector<std::uint8_t>& b,const char* id,const std::vector<std::uint8_t>& payload){b.insert(b.end(),id,id+4);put32(b,std::uint32_t(payload.size()));b.insert(b.end(),payload.begin(),payload.end());if(payload.size()&1)b.push_back(0);}
static std::vector<std::uint8_t> riff(bool reversed=false){
    std::vector<std::uint8_t> b={'R','I','F','F',0,0,0,0,'W','A','V','E'};
    const std::vector<std::uint8_t> format={1,0,1,0,0x22,0x56,0,0,0x44,0xac,0,0,2,0,16,0},samples={0,0,0xff,0x7f,0,0x80,1,0};
    chunk(b,"JUNK",{1,2,3});if(reversed)chunk(b,"data",samples);chunk(b,"fmt ",format);if(!reversed)chunk(b,"data",samples);
    auto size=std::uint32_t(b.size()-8);for(int i=0;i<4;++i)b[4+i]=std::uint8_t(size>>(8*i));return b;
}
static void rejects(const std::vector<std::uint8_t>& b){bool failed=false;try{readWave(b);}catch(const std::runtime_error&){failed=true;}require(failed,"Malformed WAV accepted");}
struct PrimaryFixture final:rec::PrimaryBackend {
    std::string calls;int fault=0;PcmFormat format;
    rec::Status create(const rec::Descriptor32& d) override{calls+='C';require(d.size==20 && d.flags==0x81 && !d.bytes && !d.formatAddress,"Primary descriptor");return fault==1?17:0;}
    rec::Status deviceCaps(std::uint32_t& flags) override{calls+='D';flags=0x0a;return fault==2?-19:0;}
    rec::Status setFormat(const PcmFormat& f) override{calls+='F';format=f;return fault==3?23:0;}
    rec::Status bufferBytes(std::uint32_t& bytes) override{calls+='B';bytes=4096;return fault==4?-27:0;}
    rec::Status compact() override{calls+='M';return fault==5?31:0;}
};
int main(){
    try {
        const auto request=rec::deviceRequest();require(request.defaultDevice && request.noAggregation && request.cooperativeLevel==2,"Device request");
        const std::array<std::uint8_t,20> primaryWire={20,0,0,0,0x81,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        require(rec::encodeDescriptor(rec::primaryDescriptor())==primaryWire,"PE32 primary layout");
        const std::array<std::uint8_t,18> primaryFormatWire={1,0,2,0,0x22,0x56,0,0,0x88,0x58,1,0,4,0,16,0,0,0};
        require(rec::encodeFormat(rec::primaryFormat(10))==primaryFormatWire,"Packed 18-byte WAVEFORMATEX");
        const auto wire=rec::encodeDescriptor(rec::secondaryDescriptor(0x12345678,0x11223344));
        require(wire[4]==0xea && wire[8]==0x78 && wire[11]==0x12 && wire[16]==0x44 && wire[19]==0x11,"PE32 pointer/byte layout");
        for(unsigned caps:{0u,2u,8u,10u}){
            const auto f=rec::primaryFormat(caps);require(f.channels==(caps&2?2:1) && f.bits==(caps&8?16:8) && f.rate==22050 && f.alignment==4 && f.bytesPerSecond==88200,"Original capability fallback");
            require(validPcm(f)==(caps==10),"Do not normalize original fallback fields");
        }
        const std::string calls[]={"CDFBM","C","CD","CDF","CDFBM","CDFBM"};
        const int statuses[]={0,17,-19,23,0,31};
        for(int fault=0;fault<6;++fault){PrimaryFixture fixture;fixture.fault=fault;const auto result=rec::setupPrimary(fixture);
            require(fixture.calls==calls[fault] && result.status==statuses[fault] && result.created==(fault!=1),"Primary call/failure order");
            require(result.bytes.has_value()==(fault==0 || fault==5),"Ignored GetCaps failure must not invent bytes");}
        auto bytes=riff();auto wave=readWave(bytes);require(readWave(riff(true)).samples==wave.samples,"RIFF chunk order");
        for(std::size_t i=0;i<bytes.size();++i)rejects({bytes.begin(),bytes.begin()+i});
        auto bad=bytes;bad[32]=3;rejects(bad); // non-PCM format tag
        bad=bytes;std::fill(bad.begin()+36,bad.begin()+40,0);rejects(bad); // zero sample rate
        bad=bytes;bad[40]=0;rejects(bad); // inconsistent byte rate
        bad=bytes;bad[52]=9;rejects(bad); // data extends beyond RIFF container
        bad=bytes;bad[44]=4;rejects(bad); // inconsistent frame alignment
        bad=bytes;bad[46]=24;rejects(bad); // unsupported sample width
        for(unsigned channels:{1u,2u})for(unsigned bits:{8u,16u}){
            auto file=riff();file[34]=std::uint8_t(channels);file[46]=std::uint8_t(bits);file[44]=std::uint8_t(channels*(bits/8));
            const auto average=22050u*file[44];for(int i=0;i<4;++i)file[40+i]=std::uint8_t(average>>(8*i));
            const auto sample=readWave(file);Device model;const auto upload=rec::uploadStatic(model,sample);
            require(upload.error==Error::ok && model.samples(upload.buffer)==wave.samples,"Mono/stereo 8/16-bit PCM exactness");
        }
        auto extended=bytes;extended.insert(extended.begin()+48,2,0);extended[28]=18;extended[4]=58;
        require(readWave(extended).samples==wave.samples,"18-byte PCM fmt chunk");extended[48]=1;rejects(extended);
        Device device;BufferId primary=0;require(device.createPrimary(0x81,primary)==Error::ok,"Native primary");
        require(device.setPrimaryFormat(primary,rec::primaryFormat(Device::capabilities))==Error::ok,"Native output format");
        require(device.setPrimaryFormat(primary,rec::primaryFormat(0))==Error::badFormat,"Reject inconsistent native PCM");
        BufferId untouched=777;require(device.createPrimary(0x81,untouched)==Error::busy && untouched==777,"Single native primary policy");
        const auto uploaded=rec::uploadStatic(device,wave);require(uploaded.error==Error::ok && uploaded.copied==8 && device.samples(uploaded.buffer)==wave.samples,"Exact uploaded PCM");
        BufferId duplicate=0;require(device.duplicate(uploaded.buffer,duplicate)==Error::ok && duplicate!=uploaded.buffer,"Distinct duplicate voice");
        WriteLock lock;require(device.lock(duplicate,6,4,0,lock)==Error::ok && lock.first.size==2 && lock.second.size==2,"Circular write split");
        const auto snapshot=device.samples(uploaded.buffer);std::fill_n(lock.first.data,2,9);std::fill_n(lock.second.data,2,7);
        require(device.samples(uploaded.buffer)==snapshot,"Uncommitted writes exposed");
        WriteLock other;require(device.lock(uploaded.buffer,0,8,0,other)==Error::busy,"Duplicate storage lock isolation");
        require(device.unlock(lock,3,2)==Error::invalid && device.samples(uploaded.buffer)==snapshot,"Invalid unlock mutated samples");
        auto forged=lock;++forged.ticket;require(device.unlock(forged,2,2)==Error::invalid,"Forged ticket");
        require(device.unlock(lock,2,2)==Error::ok && device.info(duplicate)->revision==2,"Commit sample revision");
        require(device.samples(uploaded.buffer)==std::vector<std::uint8_t>({7,7,0xff,0x7f,0,0x80,9,9}),"Shared duplicate sample mutation");
        require(device.unlock(lock,2,2)==Error::invalid,"Repeated unlock");
        require(device.release(uploaded.buffer)==Error::ok && device.samples(duplicate).size()==8,"Duplicate lifetime after source release");
        require(device.lock(duplicate,0,1,2,lock)==Error::ok && lock.first.size==8,"Entire buffer flag");
        std::fill_n(lock.first.data,8,3);require(device.unlock(lock,2,0)==Error::ok && device.samples(duplicate)[2]==0xff,"Partial commit preserves untouched samples");
        require(device.lock(duplicate,0,8,1,lock)==Error::unsupported,"Unreconstructed write cursor");
        require(device.lock(duplicate,8,1,0,lock)==Error::invalid && device.lock(duplicate,0,9,0,lock)==Error::invalid,"Bounds checks");
        require(device.lock(duplicate,0,8,0,lock)==Error::ok,"Lock before release");
        require(device.release(duplicate)==Error::ok && device.unlock(lock,8,0)==Error::invalid,"Release invalidates pending lock");
        require(device.duplicate(primary,untouched)==Error::unsupported,"Primary duplication rejected");
        require(device.release(primary)==Error::ok && !device.count(),"All buffers released");
        Device limited(4,1);require(rec::uploadStatic(limited,wave).error==Error::limit && !limited.count(),"Upload limit has no leaked buffer");
        Device voiceLimit(16,1);const auto one=rec::uploadStatic(voiceLimit,wave);untouched=777;
        require(one.error==Error::ok && voiceLimit.duplicate(one.buffer,untouched)==Error::limit && untouched==777 && voiceLimit.count()==1,"Duplicate limit preserves source");
        BufferId heldCopy=0;Device shared;const auto original=rec::uploadStatic(shared,wave);shared.duplicate(original.buffer,heldCopy);
        require(shared.lock(original.buffer,0,8,0,lock)==Error::ok,"Shared pending lock");std::fill_n(lock.first.data,8,4);
        shared.release(original.buffer);require(shared.samples(heldCopy)==wave.samples && shared.unlock(lock,8,0)==Error::invalid,"Released owner cannot publish to surviving duplicate");
        require(shared.lock(heldCopy,0,8,0,lock)==Error::ok,"Survivor can lock after owner release");shared.release(heldCopy);
        std::puts("Audio descriptor, primary setup, RIFF validation, exact upload, shared ownership and lock lifecycle passed");return 0;
    }catch(const std::exception& error){std::fprintf(stderr,"Audio test failed: %s\n",error.what());return 1;}
}
