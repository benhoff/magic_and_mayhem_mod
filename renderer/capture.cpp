#include "capture.hpp"
#include <QtEndian>
#include <array>
#include <limits>
#include <stdexcept>

namespace mnm::render {
Blit decodeCapture(const QByteArray& data){
    const auto fail=[](const char* message){throw std::runtime_error(message);};
    if(data.size()<128 || data.size()>maxCaptureBytes || data.first(8)!="MNMBLT01")
        fail("Invalid or truncated MNMBLT01 capture");
    std::array<std::uint32_t,32> h{};
    for(std::size_t i=2;i<h.size();++i)h[i]=qFromLittleEndian<quint32>(data.constData()+i*4);
    if(h[2]!=1 || (h[3]!=1 && h[3]!=2) || h[31] || (h[9]&0x80000000u))
        fail("Unsupported version, operation, reserved field, or failed HRESULT");
    const bool fast=h[3]==2;
    if((h[5]&~(fast?0x31u:0x09008000u)) || h[6]!=((h[5]&(fast?1u:0x8000u))!=0))
        fail("Unsupported flags or inconsistent key mode");
    if(!h[18] || h[18]>256 || !h[19] || h[19]>256 || !h[20] || h[20]>2048 || !h[21] || h[21]>2048)
        fail("Dimensions exceed the capture bounds");
    const auto bits=h[22];
    if(bits!=8 && bits!=16 && bits!=24 && bits!=32)fail("Unsupported native pixel format");
    const auto maxPixel=bits==32?UINT32_MAX:((std::uint32_t{1}<<bits)-1);
    if(h[6] && (h[7]!=h[8] || h[7]>maxPixel))fail("Only a single exact source key is supported");
    if(bits!=8){
        for(int i=23;i<26;++i){
            const auto mask=h[i],low=mask&(~mask+1),normalized=low?mask/low:0;
            if(!mask || mask>maxPixel || normalized>255 || (normalized&(normalized+1)))fail("Unsupported RGB mask");
        }
        if((h[23]&h[24]) || (h[23]&h[25]) || (h[24]&h[25]))fail("Overlapping RGB masks");
    }
    for(int i=10;i<18;++i)if(h[i]>std::uint32_t(std::numeric_limits<int>::max()))fail("Negative rectangle coordinate");
    Blit c;c.bits=bits;
    c.source.width=int(h[18]);c.source.height=int(h[19]);c.destination.width=int(h[20]);c.destination.height=int(h[21]);
    c.sourceRect={int(h[10]),int(h[11]),int(h[12]),int(h[13])};
    c.destinationX=int(h[14]);c.destinationY=int(h[15]);
    if(h[16]>h[20] || h[17]>h[21] || h[16]<=h[14] || h[17]<=h[15] ||
       h[16]-h[14]!=h[12]-h[10] || h[17]-h[15]!=h[13]-h[11])fail("Invalid or stretched destination rectangle");
    if(h[6])c.sourceKey=h[7];
    const auto size=bits/8;
    if(h[26]!=h[18]*h[19]*size || h[27]!=h[20]*h[21]*size || h[28]!=(bits==8?1024u:0u))
        fail("Invalid payload lengths");
    if(data.size()!=qint64(128)+h[26]+2*h[27]+2*h[28])fail("Truncated capture or trailing bytes");
    const auto decode=[&data,size](Image& image,qsizetype offset){
        image.pixels.resize(std::size_t(image.width)*std::size_t(image.height));
        for(auto& pixel:image.pixels){
            pixel=0;
            for(unsigned byte=0;byte<size;++byte)pixel|=std::uint32_t(static_cast<unsigned char>(data[offset++]))<<(byte*8);
        }
    };
    decode(c.source,128);decode(c.destination,128+h[26]);
    // Captured-after and palettes are comparison/preview evidence, never GPU inputs.
    validate(c);return c;
}
QByteArray encodeNative(const Image& image,unsigned bits){
    if(bits!=8 && bits!=16 && bits!=24 && bits!=32)throw std::runtime_error("Unsupported output pixel size");
    QByteArray out;out.resize(qsizetype(image.pixels.size())*(bits/8));qsizetype offset=0;
    for(auto pixel:image.pixels)for(unsigned byte=0;byte<bits/8;++byte)out[offset++]=char((pixel>>(byte*8))&255);
    return out;
}
}
