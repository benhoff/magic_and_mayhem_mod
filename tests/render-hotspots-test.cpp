#include "../renderer/blit.hpp"
#include "../renderer/surface_copy.hpp"
#include "../renderer/canvas_sequence.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
static Image image(int w,int h,unsigned seed=1){Image a{w,h,{}};for(int i=0;i<w*h;++i)a.pixels.push_back((unsigned(i)*997+seed*293)&65535);return a;}
static unsigned blend(unsigned s,unsigned d,CompositeMode mode,unsigned mask=0x7bef){
    if(mode==CompositeMode::half)return ((s>>1)&mask)+((d>>1)&mask);
    if(mode==CompositeMode::quarterDestination)std::swap(s,d);
    if(mode!=CompositeMode::additive){const auto h=(d>>1)&mask;return h+((h>>1)&mask)+((s>>2)&(mask>>1)&mask);}
    return (std::min(31u,(s>>11)+(d>>11))<<11)|(std::min(63u,((s>>5)&63)+((d>>5)&63))<<5)|std::min(31u,(s&31)+(d&31));
}
static void apply(Image& dst,const Image& source,const Image& mask,Rect r,int x,int y,CompositeMode mode,const std::array<std::uint16_t,256>* palette=nullptr,unsigned bits=0x7bef){
    for(int row=r.top;row<r.bottom;++row)for(int col=r.left;col<r.right;++col){const auto at=row*source.width+col;if(!mask.pixels[at])continue;auto& d=dst.pixels[(y+row-r.top)*dst.width+x+col-r.left];d=blend(palette?(*palette)[source.pixels[at]]:source.pixels[at],d,mode,bits);}
}
static unsigned colours(GlBlitter& renderer,bool rgb555,bool indexed){
    const auto format=rgb555?PixelFormat{16,{0x7c00,0x3e0,0x1f}}:PixelFormat{16,{0xf800,0x7e0,0x1f}};
    auto source=image(8,8);if(indexed)for(auto& p:source.pixels)p&=255;
    auto expected=image(128,128);Image coverage{8,8,{}};for(int i=0;i<64;++i)coverage.pixels.push_back(i%5?3:0);
    const auto src=renderer.create(source,format),mask=renderer.create(coverage,{8,{}}),dst=renderer.create(expected,format);
    renderer.batch([&]{for(unsigned i=0;i<384;++i){
        // Disjoint groups, deliberately followed by overlapping writes.
        const int x=int((i%256)%16)*8,y=int((i%256)/16)*8;
        const auto mode=i%4==0?CompositeMode::half:i%4==1?CompositeMode::quarterSource:i%4==2?CompositeMode::quarterDestination:rgb555?CompositeMode::half:CompositeMode::additive;
        std::array<std::uint16_t,256> palette{};for(unsigned j=0;j<256;++j)palette[j]=std::uint16_t(i*101+j*197);
        SpriteComposite c;c.mode=mode;renderer.composite(src,dst,{0,0,8,8},x,y,mask,c,indexed?&palette:nullptr);
        apply(expected,source,coverage,{0,0,8,8},x,y,mode,indexed?&palette:nullptr,rgb555?0x3def:0x7bef);
    }});
    require(renderer.read(dst).pixels==expected.pixels,"Disjoint/overlapping blend scalar mismatch");
    renderer.destroy(src);renderer.destroy(mask);renderer.destroy(dst);return 384;
}
static unsigned palettes(GlBlitter& renderer){
    const PixelFormat format{16,{0xf800,0x7e0,0x1f}};Image source{1,1,{17}},coverage{1,1,{1}};auto expected=image(128,16);
    const auto src=renderer.create(source,format),mask=renderer.create(coverage,{8,{}}),dst=renderer.create(expected,format);
    renderer.batch([&]{for(unsigned i=0;i<1050;++i){
        std::array<std::uint16_t,256> palette{};for(unsigned j=0;j<256;++j)palette[j]=std::uint16_t(i*13+j*71);
        SpriteComposite c;c.mode=CompositeMode::half;const int x=int(i%128),y=int(i/128);renderer.composite(src,dst,{0,0,1,1},x,y,mask,c,&palette);
        expected.pixels[y*128+x]=blend(palette[17],expected.pixels[y*128+x],c.mode);
    }});
    require(renderer.read(dst).pixels==expected.pixels,"Palette eviction changed deferred blend");
    renderer.destroy(src);renderer.destroy(mask);renderer.destroy(dst);return 1050;
}
static unsigned uploads(GlBlitter& renderer){
    const PixelFormat format{16,{0xf800,0x7e0,0x1f}};auto source=image(64,64),expected=image(64,64,2);Image coverage{64,64,std::vector<std::uint32_t>(4096,1)};
    const auto src=renderer.create(source,format),mask=renderer.create(coverage,{8,{}}),dst=renderer.create(expected,format);
    unsigned cases=0;
    renderer.batch([&]{
        const auto draw=[&](Rect r,int x,int y){SpriteComposite c;c.mode=CompositeMode::half;renderer.composite(src,dst,r,x,y,mask,c);apply(expected,source,coverage,r,x,y,c.mode);++cases;};
        const auto patch=[&](SurfaceId target,Image& model,int x,int y,const Image& plane){renderer.updateRegionRows(target,x,y,plane,0,plane.height);for(int row=0;row<plane.height;++row)for(int col=0;col<plane.width;++col)model.pixels[(y+row)*model.width+x+col]=plane.pixels[row*plane.width+col];++cases;};
        draw({0,0,8,8},0,0);
        patch(src,source,16,0,image(8,8,3)); // disjoint source upload can defer
        draw({16,0,24,8},8,0);
        patch(src,source,0,0,image(8,8,4)); // prior source dependency must flush
        draw({0,0,8,8},16,0);
        patch(mask,coverage,0,0,{8,8,std::vector<std::uint32_t>(64,0)});
        draw({0,0,8,8},24,0);
        patch(dst,expected,16,0,image(8,8,7)); // queued destination overwrite
        renderer.copy(src,dst,{16,0,24,8},32,0);for(int y=0;y<8;++y)for(int x=0;x<8;++x)expected.pixels[y*64+32+x]=source.pixels[y*64+16+x];
        patch(src,source,16,0,image(8,8,9));
        renderer.copy(src,dst,{16,0,24,8},40,0);for(int y=0;y<8;++y)for(int x=0;x<8;++x)expected.pixels[y*64+40+x]=source.pixels[y*64+16+x];
        Image bad{1,1,{65536}};bool refused=false;try{renderer.updateRegionRows(src,0,0,bad,0,1);}catch(const std::exception&){refused=true;}require(refused,"Invalid upload accepted");
        // Deleting a queued source must consume its last instance first.
        renderer.destroy(src);renderer.destroy(mask);
    });
    require(renderer.read(dst).pixels==expected.pixels,"Deferred upload/copy/lifetime order mismatch");renderer.destroy(dst);return cases+3;
}
static unsigned opaque(GlBlitter& renderer){
    const PixelFormat format{16,{0xf800,0x7e0,0x1f}};const auto source=image(8,8);auto expected=image(128,16);
    const auto src=renderer.create(source,format),dst=renderer.create(expected,format);
    renderer.batch([&]{for(unsigned i=0;i<1050;++i){const int x=int(i%128),y=int(i/128);SurfaceCopyRequest request;request.source={0,0,1,1};request.destination={x,y,x+1,y+1};require(renderer.surfaceCopy(src,dst,request).hresult==0,"Opaque surface copy failed");expected.pixels[y*128+x]=source.pixels[0];}});
    require(renderer.read(dst).pixels==expected.pixels,"Opaque copy batch/cap mismatch");renderer.destroy(src);renderer.destroy(dst);return 1050;
}
static unsigned producerCopies(){
    unsigned cases=0;
    for(bool keyed:{false,true})for(auto delta:{std::array<int,2>{0,0},{1,0},{-1,0},{0,1},{0,-1},{3,2},{-3,-2}}){
        CanvasSequence canvas;canvas.create(1,32,24);auto expected=image(32,24);for(unsigned i=0;i<expected.pixels.size();i+=7)expected.pixels[i]=0;
        canvas.update(1,0,0,expected);const Rect r{5,5,22,18};const auto frozen=expected;
        for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x){const auto word=frozen.pixels[y*32+x];if(!keyed||word)expected.pixels[(y+delta[1])*32+x+delta[0]]=word;}
        canvas.copy(1,1,r,r.left+delta[0],r.top+delta[1],keyed?std::optional<std::uint16_t>(0):std::nullopt);
        require(canvas.read(1).pixels==expected.pixels,"Regional producer snapshot changed frozen history");++cases;
        bool refused=false;try{canvas.copy(1,1,{-1,0,3,3},0,0);}catch(const std::exception&){refused=true;}require(refused&&canvas.read(1).pixels==expected.pixels,"Producer refusal mutated pixels");
    }
    // Crop a partly offscreen destination before choosing snapshot coordinates.
    CanvasSequence canvas;canvas.create(1,16,16);auto expected=image(16,16);canvas.update(1,0,0,expected);const auto frozen=expected;
    canvas.copy(1,1,{2,2,12,12},-3,-2);
    for(int y=0;y<8;++y)for(int x=0;x<7;++x)expected.pixels[y*16+x]=frozen.pixels[(y+4)*16+x+5];
    require(canvas.read(1).pixels==expected.pixels,"Cropped producer snapshot origin differs");return cases+1;
}
static unsigned tileLimit(GlBlitter& renderer){
    const PixelFormat format{16,{0xf800,0x7e0,0x1f}};auto expected=image(544,256);Image transparent{544,256,std::vector<std::uint32_t>(544*256,0)};
    const auto src=renderer.create(expected,format),mask=renderer.create(transparent,{8,{}}),dst=renderer.create(expected,format);const auto before=renderer.stats();
    renderer.batch([&]{SpriteComposite c;c.mode=CompositeMode::half;for(unsigned i=0;i<500;++i)renderer.composite(src,dst,{0,0,544,256},0,0,mask,c);});
    const auto draws=renderer.stats().drawCalls-before.drawCalls;
    require(draws==2||draws==500,"Tile-reference cap or serial fallback differs");
    require(renderer.read(dst).pixels==expected.pixels,"Transparent tile cap changed pixels");renderer.destroy(src);renderer.destroy(mask);renderer.destroy(dst);return 500;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);unsigned comparisons=0;
    for(bool baseline:{false,true}){
        for(const char* name:{"MNM_NATIVE_COMPOSITE_BATCH_BASELINE","MNM_UPLOAD_FLUSH_BASELINE","MNM_OPAQUE_COPY_BASELINE"})qputenv(name,baseline?"1":"0");
        GlBlitter renderer;const auto before=renderer.stats();comparisons+=colours(renderer,false,false);comparisons+=colours(renderer,false,true);comparisons+=colours(renderer,true,false);comparisons+=palettes(renderer);comparisons+=uploads(renderer);comparisons+=opaque(renderer);comparisons+=tileLimit(renderer);comparisons+=producerCopies();
        require(renderer.stats().surfaces==0,"Hotspot surface leak");require(renderer.stats().nativeReadbacks-before.nativeReadbacks==7,"Unexpected ordinary readback");
    }
    std::cout<<comparisons<<" hotspot scalar operations pass (optimized and forced serial)\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
