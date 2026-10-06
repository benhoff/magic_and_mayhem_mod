#include "dib.hpp"
#include "blit.hpp"
#include <stdexcept>

namespace mnm::render {
Image decodeDibRgb(const DibInput& in){
    const auto u16=[&](unsigned at){return unsigned(in.header[at])|(unsigned(in.header[at+1])<<8);};
    const auto u32=[&](unsigned at){return std::uint32_t(in.header[at])|(std::uint32_t(in.header[at+1])<<8)|
        (std::uint32_t(in.header[at+2])<<16)|(std::uint32_t(in.header[at+3])<<24);};
    const auto w=u32(4),h=u32(8),bits=u16(14);
    if(u32(0)!=40 || u16(12)!=1 || u32(16)!=0 || w<1 || h<1 || w>2048 || h>2048 ||
       (bits!=1 && bits!=4 && bits!=8 && bits!=24))throw std::runtime_error("Unsupported DIB layout");
    const unsigned colors=bits==24?0:1u<<bits;
    if(in.usage!=(bits==24?1u:0u) || in.palette.size()!=colors*4u || u32(32)!=0 || u32(36)!=0)
        throw std::runtime_error("Unsupported DIB palette/usage");
    const std::size_t stride=((std::size_t(w)*bits+31)/32)*4,required=stride*h;
    if(in.pixels.size()<required || in.pixels.size()>2*1024*1024 ||
       (u32(20)!=0 && u32(20)!=required))throw std::runtime_error("Unsupported DIB pixel extent");
    Image out{int(w),int(h),std::vector<std::uint32_t>(std::size_t(w)*h)};
    for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){
        const auto row=(h-1-y)*stride;
        std::size_t at;const std::vector<std::uint8_t>* bytes;
        if(bits==24){at=row+x*3;bytes=&in.pixels;}
        else{
            const auto index=bits==8?in.pixels[row+x]:bits==4?
                (in.pixels[row+x/2]>>(x%2?0:4))&15u:(in.pixels[row+x/8]>>(7-x%8))&1u;
            at=index*4;bytes=&in.palette;
        }
        out.pixels[std::size_t(y)*w+x]=std::uint32_t((*bytes)[at])|
            (std::uint32_t((*bytes)[at+1])<<8)|(std::uint32_t((*bytes)[at+2])<<16);
    }
    return out;
}
}
