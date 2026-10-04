#include "cursor.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
using Bytes=std::vector<std::uint8_t>;
void check(bool v,const char* detail) {if(!v) throw std::runtime_error(detail);}
void put(Bytes& b,std::size_t p,std::uint32_t value,unsigned n=4) {for(unsigned i=0;i<n;++i) b[p+i]=static_cast<std::uint8_t>(value>>(8*i));}
CursorAsset good(CursorResult r) {if(const auto* e=std::get_if<CursorError>(&r)) throw std::runtime_error(e->detail);return std::get<CursorAsset>(std::move(r));}
void bad(const Bytes& b,CursorErrorCode code,const CursorLimits& limits={}) {
    auto r=decodeCursor(b,limits);const auto* e=std::get_if<CursorError>(&r);check(e && e->code==code,"wrong cursor error");
}
Bytes fixture(unsigned width=9,unsigned height=2,unsigned depth=1,unsigned count=1) {
    const unsigned colors=depth==1?2:3;
    const std::size_t xs=((width*depth+31)/32)*4,as=((width+31)/32)*4;
    const std::size_t imageBytes=40+colors*4+(xs+as)*height,offset=6+count*16;
    Bytes b(offset+imageBytes);
    put(b,2,2,2);put(b,4,count,2);
    for(unsigned i=0;i<count;++i) {
        const auto p=6+i*16;b[p]=static_cast<std::uint8_t>(width);b[p+1]=static_cast<std::uint8_t>(height);
        put(b,p+4,i%width,2);put(b,p+6,i%height,2);put(b,p+8,imageBytes);put(b,p+12,offset);
    }
    put(b,offset,40);put(b,offset+4,width);put(b,offset+8,height*2);put(b,offset+12,1,2);put(b,offset+14,depth,2);put(b,offset+20,xs*height);put(b,offset+32,colors);
    for(unsigned c=0;c<colors;++c) {
        auto p=offset+40+c*4;
        b[p]=c==1?255:static_cast<std::uint8_t>(c*10);b[p+1]=c==1?255:static_cast<std::uint8_t>(c*20);b[p+2]=c==1?255:static_cast<std::uint8_t>(c*30);b[p+3]=static_cast<std::uint8_t>(c*40);
    }
    const auto pixels=offset+40+colors*4;
    for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x) {
        const unsigned id=(x+y)%colors,a=((x/2+y)%2);
        const auto row=height-1-y;
        if(depth==8) b[pixels+row*xs+x]=static_cast<std::uint8_t>(id);
        else b[pixels+row*xs+x/8]|=static_cast<std::uint8_t>(id<<(7-x%8));
        b[pixels+xs*height+row*as+x/8]|=static_cast<std::uint8_t>(a<<(7-x%8));
    }
    return b;
}
class MemoryFile final:public AssetFile {
    Bytes b;std::size_t p=0;
public:
    explicit MemoryFile(Bytes bytes):AssetFile("cursor",{}),b(std::move(bytes)) {}
    Result<std::int64_t> size() override {return static_cast<std::int64_t>(b.size());}
    Result<std::int64_t> position() override {return static_cast<std::int64_t>(p);}
    Status seek(std::int64_t n) override {
        if(n<0 || std::uint64_t(n)>b.size()) return failure(ErrorCode::invalidArgument,"seek","range");
        p=static_cast<std::size_t>(n);return std::monostate{};
    }
    ReadResult read(void* dest,std::int64_t count) override {
        if(count<0 || (!dest && count)) return {0,failure(ErrorCode::invalidArgument,"read","buffer")};
        const auto n=std::min<std::size_t>(static_cast<std::size_t>(count),b.size()-p);
        if(n) std::copy_n(b.data()+p,n,static_cast<std::uint8_t*>(dest));
        p+=n;
        return {static_cast<std::int64_t>(n),std::nullopt};
    }
};
}
int main() try {
    for(unsigned depth:{1U,8U}) {
        auto b=fixture(9,2,depth);auto c=good(decodeCursor(b));b.clear();const auto& image=c.images[0];
        check(image.width==9 && image.height==2 && image.bitDepth==depth && image.andMask.size()==18,"shape");
        for(unsigned y=0;y<2;++y) for(unsigned x=0;x<9;++x) {
            const auto id=(x+y)%(depth==1?2:3),a=(x/2+y)%2;
            check(image.xorIndices[y*9+x]==id && image.andMask[y*9+x]==a,"row orientation, bit order and padding");
        }
        check(image.palette[1]==std::array<std::uint8_t,4>({255,255,255,40}),"palette reserved byte preservation");
        if(depth==8) check(image.palette[2]==std::array<std::uint8_t,4>({60,40,20,80}),"BGR to RGB");
        {MemoryFile f(fixture(9,2,depth));c=good(loadCursor(f));}
        check(c.images[0].xorIndices[17]==(8+1)%(depth==1?2:3),"file lifetime ownership");
    }
    auto c=good(decodeCursor(fixture(9,2,1,3)));
    check(c.images.size()==3 && c.images[2].hotspotX==2 && c.images[2].hotspotY==0,"aliased variants retain hotspots and order");
    good(decodeCursor(fixture(256,2,8)));
    auto b=fixture();for(std::size_t n=0;n<b.size();++n) {
        Bytes shortFile(b.begin(),b.begin()+n);check(std::holds_alternative<CursorError>(decodeCursor(shortFile)),"truncation accepted");
    }
    auto mutation=[&](std::size_t p,std::uint32_t v,unsigned n,CursorErrorCode code) {auto f=fixture();put(f,p,v,n);bad(f,code);};
    mutation(0,1,2,CursorErrorCode::invalidFormat);mutation(2,1,2,CursorErrorCode::invalidFormat);
    mutation(4,0,2,CursorErrorCode::malformedData);mutation(9,1,1,CursorErrorCode::malformedData);
    mutation(10,9,2,CursorErrorCode::malformedData);mutation(12,2,2,CursorErrorCode::malformedData);
    mutation(18,0,4,CursorErrorCode::malformedData);mutation(18,0xffffffff,4,CursorErrorCode::malformedData);
    mutation(14,1,4,CursorErrorCode::malformedData);mutation(22,12,4,CursorErrorCode::unsupportedEncoding);
    mutation(26,8,4,CursorErrorCode::malformedData);mutation(30,0xfffffffc,4,CursorErrorCode::malformedData);
    mutation(34,2,2,CursorErrorCode::malformedData);mutation(36,4,2,CursorErrorCode::unsupportedEncoding);
    mutation(38,1,4,CursorErrorCode::unsupportedEncoding);mutation(42,17,4,CursorErrorCode::malformedData);
    mutation(54,3,4,CursorErrorCode::malformedData);
    b=fixture(9,2,8);b[22+40+12]=3;bad(b,CursorErrorCode::malformedData);
    b=fixture();put(b,54,0);good(decodeCursor(b)); // Zero colorsUsed means full bit-depth palette.
    CursorLimits limits;limits.images=1;bad(fixture(9,2,1,2),CursorErrorCode::limitExceeded,limits);
    limits={};limits.width=8;bad(fixture(),CursorErrorCode::limitExceeded,limits);
    limits={};limits.pixels=35;bad(fixture(9,2,1,2),CursorErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=43;bad(fixture(),CursorErrorCode::limitExceeded,limits);
    limits={};limits.inputBytes=10;bad(fixture(),CursorErrorCode::limitExceeded,limits);
    {MemoryFile f(fixture());auto r=loadCursor(f,limits);const auto* e=std::get_if<CursorError>(&r);check(e && e->code==CursorErrorCode::assetInput && e->input && e->input->code==ErrorCode::limitExceeded,"underlying read limit");}
    limits={};limits.inputBytes=0xffffffffffffffffULL;
    {MemoryFile f(fixture());auto r=loadCursor(f,limits);check(std::get<CursorError>(r).code==CursorErrorCode::invalidArgument,"signed file limit");}
    // The decoded planes can express black, white, unchanged and inversion.
    c=good(decodeCursor(fixture()));const auto& i=c.images[0];std::vector<unsigned> results;
    for(unsigned x=0;x<4;++x) results.push_back((0x5aU&(i.andMask[x]?255:0))^i.palette[i.xorIndices[x]][0]);
    check(results==std::vector<unsigned>({0,255,0x5a,0xa5}),"classic cursor operations retained");
    std::cout<<"Cursor loader fixtures passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
