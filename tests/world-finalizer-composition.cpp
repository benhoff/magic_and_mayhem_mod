// Full original consumer/finalizer/canvas-wrapper/font/glyph instruction bodies
// with an owned software COM boundary. This is not a physical driver oracle.
#include "../reconstruction/rendering/world_finalizer.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#else
#include "../compat/legacy/font_layout.hpp"
#include "../reconstruction/rendering/glyph_backend_state.h"
#endif
namespace {
using U=std::uint32_t;using H=std::uint16_t;using B=std::vector<unsigned char>;
constexpr U width=404,stride=408,height=96,metadataSlots=6;
U get(const B& b,unsigned at){if(at+4>b.size())throw std::runtime_error("Font word extent");U n;std::memcpy(&n,b.data()+at,4);return n;}
B load(const char* path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Missing font");return B{std::istreambuf_iterator<char>(f),{}};}
void output(U n){std::cout.write(reinterpret_cast<const char*>(&n),4);}
struct FontResult {U x=77,y=29,count=0;std::vector<U> trailing,lines;};
struct Metrics {U fills{},maps{},unmaps{},closedUnmaps{};};
#ifdef MNM_ORIGINAL_REFERENCE
U address(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
struct Surface {U* table;H* pixels;bool mapped;Metrics stats;U textBase;};
U __attribute__((stdcall)) fillSurface(Surface* s,const U* rect,void* source,void* sourceRect,U flags,const U* fx){
    if(source||sourceRect||flags!=0x1000400||!rect||s->mapped||rect[2]>width||rect[3]>height)
        throw std::runtime_error("Original fill escaped owned surface");
    for(U y=rect[1];y<rect[3];++y)std::fill(s->pixels+y*stride+rect[0],s->pixels+y*stride+rect[2],H(fx[20]));
    // The original caller's rectangle is entrySP-0x214 and its text array is
    // entrySP-0x200. Retain the identity before return; no expired bytes read.
    s->textBase=U(reinterpret_cast<std::uintptr_t>(rect))+0x14;
    ++s->stats.fills;return 0;
}
U __attribute__((stdcall)) mapSurface(Surface* s,const void* rect,U* descriptor,U flags,void* event){
    if(rect||event||flags!=1||s->mapped||descriptor[0]!=108)throw std::runtime_error("Original Lock contract");
    descriptor[2]=height;descriptor[3]=width;descriptor[4]=stride*2;descriptor[9]=address(s->pixels);
    s->mapped=true;++s->stats.maps;return 0;
}
U __attribute__((stdcall)) unmapSurface(Surface* s,void* p){
    if(p!=s->pixels)throw std::runtime_error("Original Unlock pointer");
    ++s->stats.unmaps;if(!s->mapped){++s->stats.closedUnmaps;return 0x80004005;}
    s->mapped=false;return 0;
}
FontResult executeOriginal(const B& font,std::vector<H>& pixels,mnm::reconstruction::WorldFinalizerState& state,
        std::int32_t value,bool format,U configuration,Metrics& stats,U& observedHeight,U& observedStride,std::array<U,4>& clip){
    U vtable[33]{};vtable[5]=address(reinterpret_cast<void*>(&fillSurface));vtable[25]=address(reinterpret_cast<void*>(&mapSurface));vtable[32]=address(reinterpret_cast<void*>(&unmapSurface));
    Surface surface{vtable,pixels.data(),false,{},0};
    std::array<U,0xe3c/4+32> fontStorage;fontStorage.fill(0xa55aa55a);U* object=fontStorage.data()+16;std::fill_n(object,0xe3c/4,0);
    const U rows=get(font,16),glyphs=get(font,12),metrics=40+(get(font,32)?768:0),table=metrics+rows*get(font,36)*8,base=table+glyphs*4;
    object[0]=address(font.data());object[2]=address(font.data()+metrics);object[3]=address(font.data()+table);object[4]=address(font.data()+base);
    object[0x218/4]=77;object[0x21c/4]=29;object[0x220/4]=get(font,20)+get(font,24);object[0x230/4]=0;object[0x224/4]=11;
    for(U r=0;r<rows;++r)object[6+r]=7+r%5;
    std::array<unsigned char,512> owner{};
    auto put=[&](unsigned at,U v){std::memcpy(owner.data()+at,&v,4);};
    put(4,state.ready);put(8,address(object));put(12,address(state.widths.data()));put(16,address(state.rectangles.data()));put(20,address(state.locations.data()));
    put(0x128,address(&surface));put(0x12c,address(&surface));put(0x134,width);put(0x138,height);put(0x13c,stride*2);
    put(0x156,state.lineHeight);put(0x15a,state.rowCount);put(0x15e,state.enabled);
    global(0x6de6dd,state.blueBackground);global(0x6de6d5,state.largeVideo);global(0x6e1ed4,0);global(0x6e1f88,format);
    global(0x6e1f20,0);global(0x6e1f1c,bool(configuration&2));global(0x6f8150,0);global(0x6f5eac,0);
    for(U i=0;i<64;++i)global(0x6f5dac+i*4,0xdeadbeef);
    *reinterpret_cast<H*>(0x6e1f7c)=format?0x7c00:0xf800;*reinterpret_cast<H*>(0x6e1f7e)=format?0x3e0:0x7e0;*reinterpret_cast<H*>(0x6e1f80)=31;
    for(U a:{0x6e0008u,0x6cbb6cu,0x6a49b8u,0x656618u})global(a,0xdeadface);
    global(0x6a2dc8,0xdeadface);global(0x6def50,0xdeadface);
    using Final=void(__attribute__((thiscall)) *)(void*,std::int32_t,U,U);
    if(configuration&1){
        std::memcpy(reinterpret_cast<void*>(0x6ddf78),owner.data(),owner.size());U queue[6]{};
        global(0x6e1f24,1);global(0x6e8db0,U(value));global(0x6e8db4,0x76543210);global(0x6e8db8,0x89abcdef);
        using Consume=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Consume>(0x5002a0)(queue);
        if(*reinterpret_cast<U*>(0x6e8db0)||*reinterpret_cast<U*>(0x6e8db4)||*reinterpret_cast<U*>(0x6e8db8))throw std::runtime_error("Consumer finalizer argument reset");
    }else reinterpret_cast<Final>(0x4d8ab0)(owner.data(),value,0x76543210,0x89abcdef);
    for(U i=0;i<16;++i)if(fontStorage[i]!=0xa55aa55a||fontStorage[16+0xe3c/4+i]!=0xa55aa55a)throw std::runtime_error("Font owner guard");
    stats=surface.stats;observedHeight=*reinterpret_cast<U*>(0x6def50);observedStride=*reinterpret_cast<U*>(0x6a2dc8);
    for(U i=0;i<4;++i)clip[i]=*reinterpret_cast<U*>(std::array<U,4>{0x6e0008,0x6cbb6c,0x6a49b8,0x656618}[i]);
    FontResult result;result.x=object[0x218/4];result.y=object[0x21c/4];result.count=object[0xe38/4];
    if(result.count>256)throw std::runtime_error("Original line table bound");
    for(U i=0;i<rows;++i)result.trailing.push_back(object[6+i]);
    for(U i=0;i<result.count;++i){const auto at=0x238/4+i*3;result.lines.insert(result.lines.end(),{object[at],object[at+1],object[at+2]});}
    for(U i=0;i<result.count;++i)result.lines[i*3+2]-=surface.textBase;
    return result;
}
#else
FontResult executeNative(const B& raw,std::vector<H>& pixels,mnm::reconstruction::WorldFinalizerState& state,
        std::int32_t value,bool,U configuration,Metrics& stats,U& observedHeight,U& observedStride,std::array<U,4>& clip){
    auto decoded=mnm::assets::decodeSft(raw);if(auto* e=std::get_if<mnm::assets::SftError>(&decoded))throw std::runtime_error(e->detail);
    mnm::compat::FontLayout font(std::get<mnm::assets::SftFont>(std::move(decoded)));
    mnm::render::CanvasSequence canvas;canvas.create(1,stride,height);mnm::render::Image background{int(stride),int(height),{}};
    background.pixels.assign(pixels.begin(),pixels.end());canvas.update(1,0,0,background);
    FontResult result;for(U r=0;r<get(raw,16);++r)result.trailing.push_back(7+r%5);
    struct Backend {
        mnm::render::CanvasSequence& canvas;mnm::compat::FontLayout& layout;const B& raw;FontResult& result;
        Metrics& stats;U& frameHeight;U& frameStride;std::array<U,4>& clipping;bool closed,mapped=false;
        void fill(mnm::reconstruction::FinalizerRectangle r,U colour){canvas.fill(1,{int(r.left),int(r.top),int(r.right),int(r.bottom)},H(colour));++stats.fills;}
        bool map(){++stats.maps;mapped=true;if(closed)unmap();return true;}
        void unmap(){++stats.unmaps;if(!mapped)++stats.closedUnmaps;mapped=false;}
        void bind(){frameHeight=height;frameStride=stride;}
        void clip(mnm::reconstruction::FinalizerRectangle r){clipping={r.left,r.top,r.right,r.bottom};}
        U drawFont(const std::string& text,mnm::reconstruction::FinalizerRectangle r){
            mnm::reconstruction::rendering::FontLayoutInput input;input.text.assign(text.begin(),text.end());input.text.push_back(0);
            input.bounds={int(r.left),int(r.top),int(r.right),int(r.bottom)};input.cursor.x=77;input.cursor.y=29;input.cursor.tracking=0;input.cursor.tabAdvance=11;
            input.cursor.trailing.assign(result.trailing.begin(),result.trailing.end());input.lineHeight=get(raw,20)+get(raw,24);
            std::array<U,64> bits{};if(mnm_glyph_coverage_bits(0,bits.data()))throw std::runtime_error("Coverage generation");std::array<float,64> table{};std::memcpy(table.data(),bits.data(),sizeof(table));
            const auto out=layout.draw(canvas,1,input,{int(r.left),int(r.top),int(r.right),int(r.bottom)},{0,0,0},table);
            result.x=out.cursor.x;result.y=out.cursor.y;result.count=out.lines.size();result.trailing.assign(out.cursor.trailing.begin(),out.cursor.trailing.end());
            for(auto line:out.lines)result.lines.insert(result.lines.end(),{U(line.x),line.count,line.source});
            return out.width;
        }
        U font(const std::string& t,mnm::reconstruction::FinalizerRectangle r){return drawFont(t,r);}
    } backend{canvas,font,raw,result,stats,observedHeight,observedStride,clip,bool(configuration&2)};
    mnm::reconstruction::finalizeWorld(state,value,backend);
    const auto image=canvas.read(1);pixels.assign(image.pixels.begin(),image.pixels.end());return result;
}
#endif
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=8)throw std::runtime_error("Expected PE and six font paths");
    map_image(read(argv[1]));const unsigned first=2;
    std::vector<std::pair<U,B>> bodies;
    for(auto r:std::vector<std::pair<U,U>>{{0x5002a0,0xee1},{0x4d8ab0,0x1fd},
        {0x58bac0,0xc5},{0x58b660,0x200},{0x57dc20,0xa0},{0x4a5ce0,0x4b0},
        {0x4a58e0,0x100},{0x4a59e0,0x300},{0x581ec0,0x600}}){
        const auto* start=reinterpret_cast<unsigned char*>(r.first);bodies.push_back({r.first,B(start,start+r.second)});
    }
#else
    if(argc!=7)throw std::runtime_error("Expected six font paths");
    const unsigned first=1;
#endif
    U id=0;const std::int32_t values[]{0,1,-1,1234,std::numeric_limits<std::int32_t>::min(),std::numeric_limits<std::int32_t>::max()};
    for(unsigned f=0;f<6;++f){const auto font=load(argv[first+f]);
    for(bool enabled:{false,true})for(bool ready:{false,true})for(bool blue:{false,true})for(bool large:{false,true})for(bool format:{false,true})
    for(U rows:{3u,5u})for(U lineHeight:{8u,20u})for(auto value:values)for(U composed:{0u,1u,2u,3u}){
        mnm::reconstruction::WorldFinalizerState state;state.enabled=enabled;state.ready=ready;state.blueBackground=blue;state.largeVideo=large;state.lineHeight=lineHeight;state.rowCount=rows;
        state.widths.assign(metadataSlots,0xc0ffee42);state.locations.assign(metadataSlots,0xc0ffee42);state.rectangles.assign(metadataSlots,{0xc0ffee42,0xc0ffee42,0xc0ffee42,0xc0ffee42});
        std::vector<H> pixels(stride*height);for(U i=0;i<pixels.size();++i)pixels[i]=H(i*1273+113);
        Metrics stats;U frameHeight=0xdeadface,frameStride=0xdeadface;std::array<U,4> clip;clip.fill(0xdeadface);
        FontResult result;
#ifdef MNM_ORIGINAL_REFERENCE
        result=executeOriginal(font,pixels,state,value,format,composed,stats,frameHeight,frameStride,clip);
        for(const auto& body:bodies)if(!std::equal(body.second.begin(),body.second.end(),reinterpret_cast<unsigned char*>(body.first)))throw std::runtime_error("Original composition instructions changed");
#else
        result=executeNative(font,pixels,state,value,format,composed,stats,frameHeight,frameStride,clip);
#endif
        for(U n:{id++,f,U(enabled),U(ready),U(blue),U(large),U(format),rows,lineHeight,U(value),U(composed),U(result.trailing.size()),result.count,U(pixels.size())})output(n);
        for(U n:state.widths)output(n);
        for(U n:state.locations)output(n);
        for(auto r:state.rectangles)for(U n:{r.left,r.top,r.right,r.bottom})output(n);
        for(U n:clip)output(n);
        for(U n:{frameHeight,frameStride,stats.fills,stats.maps,stats.unmaps,stats.closedUnmaps,result.x,result.y})output(n);
        for(U n:result.trailing)output(n);
        for(U n:result.lines)output(n);
        std::cout.write(reinterpret_cast<const char*>(pixels.data()),pixels.size()*2);
    }}
    if(id!=18432)throw std::runtime_error("Finalizer domain incomplete");
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
