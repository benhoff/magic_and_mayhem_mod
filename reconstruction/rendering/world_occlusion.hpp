#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace mnm::reconstruction {
inline std::uint32_t occlusionWord(const std::vector<unsigned char>& bytes,std::size_t at){
    if(at>bytes.size()||bytes.size()-at<4)throw std::invalid_argument("Occlusion source extent");
    return bytes[at]|std::uint32_t(bytes[at+1])<<8|std::uint32_t(bytes[at+2])<<16|std::uint32_t(bytes[at+3])<<24;
}
inline std::uint32_t occlusionSwap(std::uint32_t value){
    return value>>24|(value&0xff0000u)>>8|(value&0xff00u)<<8|value<<24;
}
inline std::int32_t occlusionSigned(std::uint32_t value){
    return value<=0x7fffffffu?std::int32_t(value):-1-std::int32_t(0xffffffffu-value);
}
// Original 5013c0 queries before stamping. These are separate masks within the
// sprite record; query can stop early, while an admitted stamp visits every row.
inline bool originalWorldOcclusion(std::vector<unsigned char>* mask,std::size_t base,
    std::uint32_t pitch,const std::vector<unsigned char>& sprite,std::int32_t x,
    std::int32_t y,std::int32_t kind,bool tall){
    if(kind==8||kind==9||!mask)return false;
    const auto coverage=occlusionWord(sprite,36);
    if(kind==1||!coverage||coverage>=occlusionWord(sprite,0))return false;
    const auto stamp=occlusionWord(sprite,32);
    if(stamp>sprite.size()||sprite.size()-stamp<4)throw std::invalid_argument("Occlusion header extent");
    const auto height=sprite[stamp+1];
    x=occlusionSigned(std::uint32_t(x)-occlusionWord(sprite,12)-sprite[stamp+2]);
    y=occlusionSigned(std::uint32_t(y)-occlusionWord(sprite,16)-sprite[stamp+3]);
    if(x < -32||x >= (tall?832:672)||y < -32||y >= (tall?632:512))return false;
    const auto bit=(2*y+x)/8+24;
    const auto row=(2*y-x+(tall?160:0)+647)/8+24;
    const auto column=bit/8;
    const unsigned shift=unsigned(bit%8)&31;
    const auto first=std::int64_t(base)+std::int64_t(pitch)*row+column;
    // Native owned-input preflight is intentional policy. Malformed original
    // pointers/strides are excluded; no partial write is invented for them.
    if(first<0||std::uint64_t(first)>mask->size()||
       (height && (std::uint64_t(first)+std::uint64_t(height-1)*pitch+4>mask->size()))||
       std::uint64_t(coverage)+std::uint64_t(height)*4>sprite.size()||
       std::uint64_t(stamp)+4+std::uint64_t(height)*4>sprite.size())
        throw std::invalid_argument("Occlusion owned extent");
    unsigned covered=0;
    for(unsigned i=0;i<height;++i){
        const auto bits=occlusionSwap(occlusionWord(sprite,coverage+i*4)>>shift);
        if((occlusionWord(*mask,std::size_t(first)+i*pitch)&bits)!=bits)break;
        ++covered;
    }
    if(kind==0||kind==33)for(unsigned i=0;i<height;++i){
        const auto at=std::size_t(first)+i*pitch;
        const auto value=occlusionWord(*mask,at)|occlusionSwap(occlusionWord(sprite,stamp+4+i*4)>>shift);
        for(unsigned j=0;j<4;++j)(*mask)[at+j]=static_cast<unsigned char>(value>>(8*j));
    }
    return covered==height;
}
} // namespace mnm::reconstruction
