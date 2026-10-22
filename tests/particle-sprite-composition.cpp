#include "../reconstruction/rendering/particle_sprite.hpp"
#include "../reconstruction/rendering/palette_shading.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U=std::uint32_t;
static std::vector<U> events;
static void output(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
static std::uint16_t colour(unsigned palette,unsigned table,unsigned index){return std::uint16_t(palette*17137+table*1357+index*7919);}
struct Frame {
    std::vector<unsigned char> bytes;
    std::array<unsigned char,20> mask{},indices{};
    bool words;
    Frame(unsigned identity,bool split,bool word):bytes(72),words(word){
        put(4,5);put(8,4);put(12,1);put(16,2);put(28,word?UINT32_MAX:0);
        for(unsigned y=0;y<4;++y){
            for(unsigned x=0;x<5;++x){mask[y*5+x]=!split||(x+2*y)%3;indices[y*5+x]=identity*57+x+y*17;}
            put(40+y*8,U(bytes.size()));unsigned x=0;
            while(x<5){unsigned start=x;while(x<5&&!mask[y*5+x])++x;bytes.push_back(x-start);if(x==5)break;
                start=x;while(x<5&&mask[y*5+x])++x;bytes.push_back(x-start);}
        }
        // Optimized original rasters continue sequentially across rows after
        // consulting the first row offsets. Preserve the original layout:
        // one contiguous RLE mask plane, then one contiguous pixel plane.
        for(unsigned y=0;y<4;++y){
            put(44+y*8,U(bytes.size()));
            for(unsigned i=0;i<5;++i)if(mask[y*5+i]){
                const auto value=word?colour(0,15,indices[y*5+i]):indices[y*5+i];
                bytes.push_back(value);if(word)bytes.push_back(value>>8);
            }
        }put(0,U(bytes.size()));
    }
    void put(unsigned at,U value){std::memcpy(bytes.data()+at,&value,4);}
};
[[maybe_unused]] static U address(ParticleSpriteEntry entry){
    switch(entry){case ParticleSpriteEntry::word:return 0x597086;case ParticleSpriteEntry::indexed:return 0x5947b2;
    case ParticleSpriteEntry::black:return 0x595677;case ParticleSpriteEntry::fallback:return 0x57de00;
    case ParticleSpriteEntry::blend50:return 0x57ec90;case ParticleSpriteEntry::blend75:return 0x57f0f0;
    case ParticleSpriteEntry::blend25:return 0x57f5f0;}throw std::runtime_error("Entry");
}
#ifdef MNM_ORIGINAL_REFERENCE
static Frame* currentFrames;
static U ptr(const void* p){return U(reinterpret_cast<std::uintptr_t>(p));}
static void* stub(U tag,unsigned pop);
extern "C" __attribute__((force_align_arg_pointer)) void capture_particle_sprite(U tag,U* regs){
    const auto* args=regs+10;
    const bool stack=tag==0x597086||tag==0x5947b2||tag==0x595677||tag==0x57de00;
    const U source=stack?args[0]:regs[6],x=stack?args[1]:regs[5],y=stack?args[2]:args[0];
    U token=0;for(unsigned i=0;i<3;++i)if(source==ptr(currentFrames[i].bytes.data()))token=i+1;
    if(!token)throw std::runtime_error("Unowned frame");
    const auto shade=tag==0x597086||tag==0x595677?0u:stack?args[3]:args[1];
    const auto palette=tag==0x597086||tag==0x595677?0u:stack?args[4]:args[2];
    events.insert(events.end(),{tag,token,x,y,shade,palette});
    regs[7]=0x12340001; // Every indexed/black caller must take AX==1 fallback.
}
static void* stub(U tag,unsigned pop){
    auto* code=static_cast<unsigned char*>(mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if(code==MAP_FAILED)throw std::runtime_error("Stub allocation");
    const unsigned char head[]{0x9c,0x60,0x89,0xe0,0x50,0x68};std::memcpy(code,head,6);std::memcpy(code+6,&tag,4);code[10]=0xe8;
    U rel=ptr(reinterpret_cast<void*>(&capture_particle_sprite))-ptr(code+15);std::memcpy(code+11,&rel,4);
    const unsigned char tail[]{0x83,0xc4,8,0x61,0x9d,0xc2};std::memcpy(code+15,tail,6);const std::uint16_t bytes=pop;std::memcpy(code+21,&bytes,2);return code;
}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=3)throw std::runtime_error("Expected executable and trace/pixels");
    map_image(read(argv[1]));const std::string mode=argv[2];
    std::vector<std::pair<U,Bytes>> original;
    for(auto range:std::vector<std::pair<U,U>>{{0x5492b0,0x549380},{0x549730,0x549820},{0x549ba0,0x549c90},{0x54a4a0,0x54a650},{0x54e200,0x54e24e}}){
        const auto* begin=reinterpret_cast<const unsigned char*>(range.first);original.push_back({range.first,Bytes(begin,begin+range.second-range.first)});}
    if(mode=="trace")for(auto item:std::vector<std::pair<U,unsigned>>{{0x597086,0},{0x5947b2,0},{0x595677,0},{0x57de00,0},{0x57ec90,12},{0x57f0f0,12},{0x57f5f0,12}}){
        auto* entry=reinterpret_cast<unsigned char*>(item.first);const U rel=ptr(stub(item.first,item.second))-item.first-5;entry[0]=0xe9;std::memcpy(entry+1,&rel,4);}
    std::array<std::vector<U>,2> palettes;
    for(unsigned n=0;n<2;++n){palettes[n].resize(3+31*128);palettes[n][1]=3;palettes[n][2]=15;
        auto* words=reinterpret_cast<std::uint16_t*>(palettes[n].data()+3);for(unsigned t=0;t<31;++t)for(unsigned i=0;i<256;++i)words[t*256+i]=colour(n,t,i);}
    palettes[0][0]=ptr(palettes[1].data());
    global(0x6c4e3c,0x5947b2);global(0x6903dc,0x597086);
#else
    if(argc!=2)throw std::runtime_error("Expected trace/pixels");
    const std::string mode=argv[1];
#endif
    if(mode!="trace"&&mode!="pixels")throw std::runtime_error("Mode");
    const bool trace=mode=="trace";
    const U seeds[]{0,1,0x12345678,0xffffffff,0x80000000,0xdeadbeef,50,51,99};
    const std::array<int,2> positions[]{{-1,-1},{2,3},{12,11},{28,22}};
    const int phases[]{-1,0,25,26,50,51,75,76,100,151,INT32_MAX,INT32_MIN};
    U id=0;
    for(unsigned type=0;type<4;++type)for(unsigned gate=0;gate<6;++gate)for(U seed:seeds)
    for(unsigned phase=0;phase<(type==3?12u:1u);++phase)for(auto xy:positions)
    for(unsigned palette:{0u,1u})for(int shade:{-127,-8,127})for(bool format:{false,true})for(bool split:{false,true}){
        // Real fading sprites must be indexed while a palette blend is used.
        const bool words=type<3&&seed%2;
        Frame frames[]{Frame(0,split,words),Frame(1,split,words),Frame(2,split,words)};
        std::vector<std::uint16_t> pixels(40*30);for(U i=0;i<pixels.size();++i)pixels[i]=std::uint16_t(i*2017+0x1234);
        Kind8Random random(seed);if(seed==50||seed==51||seed==99){random.words[0]=seed;random.words[103]=0;}
        const int player=gate==1?-1:gate==2?8:3;const bool resource=gate!=0,visible=gate!=3;const U count=gate==4?0:3;
        events.clear();
        const auto category=seed%2?13:4;const int remaining=phases[phase];std::uint16_t returned;
#ifdef MNM_ORIGINAL_REFERENCE
        std::array<unsigned char,160> object;for(U i=0;i<object.size();++i)object[i]=unsigned(i*17+13);
        auto put=[&](unsigned at,U value){std::memcpy(object.data()+at,&value,4);};
        currentFrames=frames;for(auto& frame:frames)if(!frame.words)frame.put(28,ptr(palettes[0].data()));
        U header[4]{};header[3]=count;U offsets[]{0,ptr(frames[1].bytes.data())-ptr(frames[0].bytes.data()),ptr(frames[2].bytes.data())-ptr(frames[0].bytes.data())};
        // Separately allocated frame byte vectors may have negative relative offsets; the original adds DWORDs modulo 32 bits.
        U owner[80]{};if(player>=0&&player<8)owner[9+player]=visible;
        put(type==0?0x1c:0x10,ptr(owner));put(0x76,category);put(0x5a,gate==5?ptr(frames[0].bytes.data()):0);
        put(0x46,U(shade));put(0x72,palette);put(0x56,U(remaining));
        const U bank=type==0?0x6f2150:type==1?0x6f2268:0x6f21e8;
        global(bank,resource?ptr(header):0);global(bank+8,ptr(offsets));global(bank+12,ptr(frames[0].bytes.data()));
        global(0x6f1d58+category*36,resource?ptr(header):0);global(0x5e1428,100);global(0x644520,U(player));
        global(0x658174,ptr(pixels.data()));global(0x6a2dc8,40);global(0x6def50,30);
        global(0x6e0008,2);global(0x6cbb6c,3);global(0x6a49b8,29);global(0x656618,23);global(0x6e1f68,0);global(0x6e1f88,format);
        global(0x6574c8,random.first);global(0x6574cc,random.second);global(0x6f2a9c,random.lcg);
        std::memcpy(reinterpret_cast<void*>(0x6574d0),random.words.data(),1000);const auto before=object;
        using Draw=U(__attribute__((thiscall)) *)(void*,int,int);
        const U entries[]{0x5492b0,0x549730,0x549ba0,0x54a4a0};returned=std::uint16_t(reinterpret_cast<Draw>(entries[type])(object.data(),xy[0],xy[1]));
        if(object!=before)throw std::runtime_error("Original sprite draw mutated object");
        random.first=*reinterpret_cast<U*>(0x6574c8);random.second=*reinterpret_cast<U*>(0x6574cc);random.lcg=*reinterpret_cast<U*>(0x6f2a9c);
        std::memcpy(random.words.data(),reinterpret_cast<void*>(0x6574d0),1000);
#else
        auto draw=[&](ParticleSpriteEntry entry,U token,int x,int y,int sh,U pal){
            const auto target=address(entry);const bool noShade=entry==ParticleSpriteEntry::word||entry==ParticleSpriteEntry::black;
            if(trace){events.insert(events.end(),{target,token,U(x),U(y),noShade?0u:U(sh),noShade?0u:pal});return 0x12340001u;}
            const Frame& frame=frames[token-1];const int left=x-1,top=y-2;
            if(entry==ParticleSpriteEntry::black&&(left<2||left+5>=29))return 1u;
            ShadedPalette shading;shading.shift=3;shading.neutral=15;shading.tables.resize(31);
            const auto table=shading.tableIndex(sh);
            for(int row=0;row<4;++row)for(int col=0;col<5;++col){const auto at=row*5+col;const int dx=left+col,dy=top+row;
                if(!frame.mask[at]||dx<2||dx>=29||dy<3||dy>=23)continue;
                auto& dest=pixels.at(dy*40+dx);U value=frame.words?colour(0,15,frame.indices[at]):colour(pal,table,frame.indices[at]);
                // The original555 path also retains the incoming unused
                // high bit through its shift; owned words need not clear it.
                const U mask=format?0x7def:0x7bef;
                if(entry==ParticleSpriteEntry::black)value=0;
                else if(entry==ParticleSpriteEntry::blend50)value=((value>>1)&mask)+((dest>>1)&mask);
                else if(entry==ParticleSpriteEntry::blend75){const auto half=(value>>1)&mask;value=half+((half>>1)&mask)+((dest>>2)&(mask>>1)&mask);}
                else if(entry==ParticleSpriteEntry::blend25){const auto half=(dest>>1)&mask;value=half+((half>>1)&mask)+((value>>2)&(mask>>1)&mask);}
                dest=std::uint16_t(value);
            }return 0u;
        };
        returned=type<3?drawRandomParticleSprite(type!=0,{resource,player,visible,count},random,xy[0],xy[1],
            [&](U frame){return ParticleSpriteSource{frame+1,words};},draw):
            drawFadingParticleSprite(resource,{gate==5?1u:0u,false},category,remaining,100,shade,palette,xy[0],xy[1],draw);
#endif
        if(returned)throw std::runtime_error("Particle draw AX must be zero");
        const U record[]{id++,type,gate,seed,phase,U(xy[0]),U(xy[1]),palette,U(shade),U(format),U(split),returned,U(events.size())};
        output(record,sizeof(record));output(&random,sizeof(random));if(trace)output(events.data(),events.size()*4);else output(pixels.data(),pixels.size()*2);
    }
    if(id!=77760)throw std::runtime_error("Particle draw denominator "+std::to_string(id));
#ifdef MNM_ORIGINAL_REFERENCE
    for(const auto& item:original)if(std::memcmp(reinterpret_cast<const void*>(item.first),item.second.data(),item.second.size()))throw std::runtime_error("Original particle/RNG bytes changed");
#endif
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
