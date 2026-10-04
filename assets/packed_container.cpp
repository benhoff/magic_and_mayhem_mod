#include "persistence_internal.hpp"

namespace mnm::assets {
namespace {
using namespace persistence_detail;
class Generator {
    std::array<std::uint32_t,250> state{};
    std::size_t left=0,right=103;
public:
    explicit Generator(std::uint32_t seed) {
        for(std::size_t i=250;i-- >0;) {
            const std::uint64_t product=std::uint64_t(seed)*0x41c64e6d;
            const auto old=static_cast<std::uint32_t>(product);
            seed=old+0x3039U;
            const auto high=static_cast<std::uint32_t>((product>>32)<<16)+0xffffU+std::uint32_t(seed<old);
            state[i]=(seed>>16)|(high&0xffff0000U);
        }
        std::uint32_t bit=0x80000000U,keep=0xffffffffU;
        for(std::size_t i=3;i<224;i+=7) {state[i]=(state[i]&keep)|bit;bit>>=1;keep>>=1;}
    }
    std::uint32_t next() {
        auto v=state[left]^state[right];state[left]=v;left=(left+1)%250;right=(right+1)%250;return v;
    }
};
std::uint32_t checksum(const Bytes& b) {
    std::uint32_t v=0;
    for(std::size_t p=0;p+4<=b.size();p+=4) {if((p/4)&1) v+=word(b,p);else v^=word(b,p);}
    return v;
}
struct Bits {
    const Bytes& bytes; std::size_t byte=0; unsigned bit=0;
    unsigned get(unsigned n) {
        unsigned v=0;
        while(n--) {
            if(byte>=bytes.size()) fail(PersistenceErrorCode::malformedData,20+byte,"truncated LZSS bitstream");
            v=(v<<1)|((bytes[byte]>>(7-bit))&1);if(++bit==8) {bit=0;++byte;}
        }
        return v;
    }
};
}
PersistenceResult<PackedContainer> decodePackedContainer(const Bytes& bytes,const PersistenceLimits& limits,ContainerTransform transform) {
    return guarded<PackedContainer>([&] {
        cap(bytes.size(),limits.inputBytes);
        if(bytes.size()<20) fail(PersistenceErrorCode::malformedData,0,"container needs 20-byte header");
        Bytes clear=bytes; PackedContainer result;result.seed=word(clear,0);Generator rng(result.seed);
        std::size_t p=4;
        for(;p+4<=clear.size();p+=4) {auto v=word(clear,p)^rng.next();for(unsigned i=0;i<4;++i) clear[p+i]=static_cast<std::uint8_t>(v>>(8*i));}
        const auto remainder=clear.size()-p;
        for(std::size_t i=0;i<remainder;++i) {
            const auto index=transform==ContainerTransform::noCdSave ? p : p+i;
            clear[index]^=static_cast<std::uint8_t>(rng.next());
        }
        auto size=word(clear,4);cap(size,limits.decodedBytes,4);
        result.packedChecksum=word(clear,8);result.decodedChecksum=word(clear,12);result.mode=word(clear,16);
        Bytes payload(clear.begin()+20,clear.end());
        if(checksum(payload)!=result.packedChecksum) fail(PersistenceErrorCode::checksumMismatch,8,"packed checksum mismatch");
        auto& out=result.decoded;out.reserve(size);
        if(result.mode==0) {
            if(payload.size()<size) fail(PersistenceErrorCode::malformedData,20,"truncated raw payload");
            out.assign(payload.begin(),payload.begin()+size);
        } else if(result.mode==1) {
            Cursor c{payload,0,20};
            while(out.size()<size) {
                auto control=c.u8();
                if(control==0) continue;
                const std::size_t count=control<128 ? control : 256-control;
                if(count>size-out.size()) fail(PersistenceErrorCode::malformedData,20+c.p-1,"RLE run exceeds decoded size");
                if(control<128) {auto v=c.u8();out.insert(out.end(),count,v);}
                else {auto literal=c.take(count);out.insert(out.end(),literal.begin(),literal.end());}
            }
        } else if(result.mode==2) {
            Bits bits{payload};std::array<std::uint8_t,4096> ring{};std::size_t write=1;
            auto append=[&](std::uint8_t v) {out.push_back(v);ring[write]=v;write=(write+1)&4095;};
            while(out.size()<size) {
                if(bits.get(1)) append(static_cast<std::uint8_t>(bits.get(8)));
                else {
                    auto read=bits.get(12);if(read==0) break;
                    auto count=bits.get(4)+2;
                    if(count>size-out.size()) fail(PersistenceErrorCode::malformedData,20+bits.byte,"LZSS run exceeds decoded size");
                    for(unsigned i=0;i<count;++i) append(ring[(read+i)&4095]);
                }
            }
        } else fail(PersistenceErrorCode::unsupportedMode,16,"unsupported packing mode");
        if(out.size()!=size) fail(PersistenceErrorCode::malformedData,4,"decoded length mismatch");
        if(checksum(out)!=result.decodedChecksum) fail(PersistenceErrorCode::checksumMismatch,12,"decoded checksum mismatch");
        return result;
    });
}
PersistenceResult<PackedContainer> loadPackedContainer(AssetFile& file,const PersistenceLimits& limits,ContainerTransform transform) {
    return persistence_detail::load<PackedContainer>(file,limits,[&](const Bytes& b){return decodePackedContainer(b,limits,transform);});
}
}
