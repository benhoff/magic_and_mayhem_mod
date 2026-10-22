// Real 4df840 + 580a90 pixels, and unchanged 5002a0 kind9 composition.
// Trace-only mode replaces 580a90 in a separate process; pixel mode patches no
// instructions. Every destination, palette and encoded source is independently
// generated in each process. No original output is a native input.
#include "../reconstruction/rendering/world_line.hpp"
#include "../reconstruction/rendering/world_consumer.hpp"
#include "../reconstruction/rendering/palette_shading.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <string>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U=std::uint32_t;
static std::vector<U> calls;
static void output(const void* data,std::size_t n){std::cout.write(static_cast<const char*>(data),n);}
static std::uint16_t colour(unsigned ordinal,unsigned table,unsigned index) {
    return std::uint16_t(ordinal*17137+table*1357+index*7919);
}
struct Sprite {
    std::vector<unsigned char> encoded;
    std::array<unsigned char,20> mask{},indices{};
    Sprite(bool split) : encoded(40+4*8) {
        auto put=[&](unsigned at,U word){std::memcpy(encoded.data()+at,&word,4);};
        put(4,5);put(8,4);put(12,1);put(16,2);
        for(unsigned y=0;y<4;++y) {
            for(unsigned x=0;x<5;++x){mask[y*5+x]=!split||(x+2*y)%3;indices[y*5+x]=x+y*17;}
            put(40+y*8,U(encoded.size()));unsigned x=0;
            while(x<5) {
                unsigned begin=x;while(x<5&&!mask[y*5+x])++x;
                encoded.push_back(x-begin);if(x==5)break;
                begin=x;while(x<5&&mask[y*5+x])++x;
                encoded.push_back(x-begin);
            }
            put(44+y*8,U(encoded.size()));
            for(unsigned i=0;i<5;++i)if(mask[y*5+i])encoded.push_back(indices[y*5+i]);
        }
        put(0,U(encoded.size()));
    }
};
[[maybe_unused]] static void nativeLine(std::vector<std::uint16_t>& pixels,const Sprite& sprite,
        std::array<int,4> clip,int x,int y,int endX,int endY,int shade,unsigned ordinal,
        bool format,bool trace) {
    ShadedPalette palette;palette.shift=3;palette.neutral=15;palette.tables.resize(31);
    const auto table=palette.tableIndex(shade);
    for(auto p:worldLine(x,y,endX,endY)) {
        if(trace){calls.insert(calls.end(),{U(p.x),U(p.y),42,U(shade),ordinal});continue;}
        for(int row=0;row<4;++row)for(int col=0;col<5;++col) {
            const int dx=p.x-1+col,dy=p.y-2+row;
            if(dx<clip[0]||dy<clip[1]||dx>=clip[2]||dy>=clip[3]||!sprite.mask[row*5+col])continue;
            auto& destination=pixels.at(std::size_t(dy)*40+dx);
            destination=addWorldPaletteWord(destination,colour(ordinal,unsigned(table),sprite.indices[row*5+col]),format);
        }
    }
}
#ifdef MNM_ORIGINAL_REFERENCE
static std::array<std::vector<U>,2> palettes;
static void* expectedLineSource;
static U __attribute__((fastcall,force_align_arg_pointer)) capture_line(void* source,int x,int y,int shade,unsigned ordinal) {
    if(source!=expectedLineSource)throw std::runtime_error("Wrong line source identity");
    calls.insert(calls.end(),{U(x),U(y),42,U(shade),ordinal});return 0x1234;
}
static void originalLine(void* source,int x,int y,int endX,int endY,int shade,unsigned ordinal) {
    using Draw=void(__attribute__((fastcall)) *)(int,int,int,int,void*,int,unsigned);
    reinterpret_cast<Draw>(0x4df840)(x,y,endX,endY,source,shade,ordinal);
}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=3)throw std::runtime_error("Expected PE and trace/pixels/queue");
    map_image(read(argv[1]));const std::string mode=argv[2];
    std::vector<std::pair<U,Bytes>> original;
    for(auto range:std::vector<std::pair<U,U>>{{0x4df840,0x4df99f},{0x5002a0,0x501181},{0x580a90,0x5811f6}}) {
        const auto* start=reinterpret_cast<const unsigned char*>(range.first);
        original.push_back({range.first,Bytes(start,start+range.second-range.first)});
    }
    if(mode=="trace") {
        auto* entry=reinterpret_cast<unsigned char*>(0x580a90);
        if(std::memcmp(entry,"\x83\xec\x30\x53\x55",5))throw std::runtime_error("Raster entry bytes");
        const U rel=reinterpret_cast<U>(&capture_line)-0x580a95;entry[0]=0xe9;std::memcpy(entry+1,&rel,4);
    }
    for(unsigned n=0;n<2;++n) {
        palettes[n].resize(3+31*128);palettes[n][1]=3;palettes[n][2]=15;
        auto* words=reinterpret_cast<std::uint16_t*>(palettes[n].data()+3);
        for(unsigned t=0;t<31;++t)for(unsigned i=0;i<256;++i)words[t*256+i]=colour(n,t,i);
    }
    palettes[0][0]=reinterpret_cast<U>(palettes[1].data());
#else
    if(argc!=2)throw std::runtime_error("Expected trace/pixels/queue");
    const std::string mode=argv[1];
#endif
    if(mode!="trace"&&mode!="pixels"&&mode!="queue")throw std::runtime_error("Unknown line mode");
    const bool trace=mode=="trace",queue=mode=="queue";
    const std::array<std::array<int,2>,21> deltas{{{{0,0}},{{1,0}},{{-1,0}},{{0,1}},{{0,-1}},
        {{7,0}},{{-7,0}},{{0,7}},{{0,-7}},{{7,3}},{{7,-3}},{{-7,3}},{{-7,-3}},
        {{3,7}},{{3,-7}},{{-3,7}},{{-3,-7}},{{7,7}},{{7,-7}},{{-7,7}},{{-7,-7}}}};
    U id=0;
    for(bool format:{false,true})for(int shade:{-127,-8,0,127})for(unsigned ordinal:{0u,1u})
    for(bool split:{false,true})for(std::array<int,4> clip:{std::array<int,4>{0,0,32,24},{2,3,27,21},{5,7,6,8}})
    for(int x:{-4,0,7,15,27,32})for(int y:{-4,0,7,15,23,28})for(auto delta:deltas) {
        Sprite sprite(split);std::vector<std::uint16_t> pixels(40*30);
        for(U i=0;i<pixels.size();++i)pixels[i]=std::uint16_t(i*2017+0x1234);
        calls.clear();
#ifdef MNM_ORIGINAL_REFERENCE
        expectedLineSource=sprite.encoded.data();
        const U pointer=reinterpret_cast<U>(palettes[0].data());std::memcpy(sprite.encoded.data()+28,&pointer,4);
        global(0x658174,reinterpret_cast<U>(pixels.data()));global(0x6a2dc8,40);global(0x6def50,30);
        global(0x6e0008,clip[0]);global(0x6cbb6c,clip[1]);global(0x6a49b8,clip[2]);global(0x656618,clip[3]);
        global(0x6e1f68,0);global(0x6e1f88,format);
        *reinterpret_cast<std::uint16_t*>(0x6e1f7c)=format?0x7c00:0xf800;
        *reinterpret_cast<std::uint16_t*>(0x6e1f7e)=format?0x3e0:0x7e0;
        *reinterpret_cast<std::uint16_t*>(0x6e1f80)=31;
        if(queue) {
            U facts[3][4],raw[3][9]{};
            const auto count=id%2?3u:1u;
            for(U i=0;i<count;++i){
                facts[i][0]=x+i;facts[i][1]=y-i;facts[i][2]=x+delta[0]-i;facts[i][3]=y+delta[1]+i;
                raw[i][1]=reinterpret_cast<U>(facts[i]);raw[i][2]=reinterpret_cast<U>(sprite.encoded.data());
                const std::int16_t offsets[]{std::int16_t(shade),std::int16_t(ordinal)};
                std::memcpy(raw[i]+4,offsets,4);raw[i][6]=9;raw[i][7]=0x8ad08ad0;
            }
            global(0x6c482c,reinterpret_cast<U>(clip.data()));global(0x6e1f70,0);
            global(0x5e1404,id%2);global(0x6de6d5,0);global(0x6e1f24,0);
            U state[]{reinterpret_cast<U>(raw),0,count,count,0,0};
            using Consume=void(__attribute__((thiscall)) *)(void*);
            reinterpret_cast<Consume>(0x5002a0)(state);
            for(U i=0;i<count;++i)if(raw[i][7]!=0x00008ad0)throw std::runtime_error("Kind9 metadata");
        } else originalLine(sprite.encoded.data(),x,y,x+delta[0],y+delta[1],shade,ordinal);
#else
        if(queue) {
            ConsumerState state{bool(id%2),false,false,0,{clip[0],clip[1],clip[2],clip[3]},
                {clip[0],clip[1],clip[2],clip[3]}, {}};
            std::vector<ConsumerRow> rows;
            for(U i=0;i<(id%2?3u:1u);++i) {
                QueueRow draw{9,99,42,0,std::int16_t(shade),std::int16_t(ordinal),true,
                    {U(x+i),U(y-i),U(x+delta[0]-i),U(y+delta[1]+i)}};
                rows.push_back({draw,0,0,0x8ad0,0x8ad0});
            }
            consumeWorldRows(rows,state,[&](const QueueCall& c){
                if(c.backend!=QueueBackend::objectHelper||c.count!=7||c.values[4]!=42)throw std::runtime_error("Kind9 owned call");
                nativeLine(pixels,sprite,clip,std::int32_t(c.values[0]),std::int32_t(c.values[1]),
                    std::int32_t(c.values[2]),std::int32_t(c.values[3]),std::int32_t(c.values[5]),c.values[6],format,false);return 0;
            },[](const auto&){throw std::runtime_error("Unexpected auxiliary kind9 clip");},[](const auto&){});
            for(auto& row:rows)if(row.metadata)throw std::runtime_error("Kind9 native metadata");
        } else nativeLine(pixels,sprite,clip,x,y,x+delta[0],y+delta[1],shade,ordinal,format,trace);
#endif
        const U header[]{id++,U(calls.size())};output(header,sizeof(header));
        if(trace)output(calls.data(),calls.size()*4);else output(pixels.data(),pixels.size()*2);
    }
    if(id!=72576)throw std::runtime_error("Line case denominator");
#ifdef MNM_ORIGINAL_REFERENCE
    for(auto& item:original) {
        if(trace&&item.first==0x580a90)continue;
        if(std::memcmp(reinterpret_cast<const void*>(item.first),item.second.data(),item.second.size()))
            throw std::runtime_error("Original helper/consumer/raster bytes changed");
    }
#else
    bool refused=false;try{worldLine(INT32_MIN,0,INT32_MAX,0);}catch(const std::invalid_argument&){refused=true;}
    if(!refused)throw std::runtime_error("Unbounded line must refuse");
#endif
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
