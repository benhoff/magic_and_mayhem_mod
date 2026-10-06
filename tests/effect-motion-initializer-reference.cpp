#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_motion_initializer.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Motion initializer/reference mismatch");}
static std::uint32_t randomWord(){static std::uint32_t n=0x48a950;n^=n<<13;n^=n>>17;n^=n<<5;return n;}
static void put(unsigned char* p,unsigned at,std::uint32_t n){std::memcpy(p+at,&n,4);}
static void word(unsigned char* p,unsigned at,std::uint16_t n){std::memcpy(p+at,&n,2);}
static std::uint32_t get(const unsigned char* p,unsigned at){std::uint32_t n;std::memcpy(&n,p+at,4);return n;}
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    require(argc==3);map_image(read(argv[1]));const int outArg=2;
    const unsigned char entry[]={0x53,0x56,0x8b,0xf1,0x57,0x83,0xc8,0xff};
    require(!std::memcmp(reinterpret_cast<void*>(0x48a950),entry,sizeof(entry)));
    using Init=void(__attribute__((thiscall)) *)(void*,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t);
    using Step=void(__attribute__((thiscall)) *)(void*,std::uint32_t*,std::uint32_t*,std::uint32_t*,std::uint32_t*);
    std::vector<unsigned char> catalog(65536*356+32,0x5a);auto* terrainBase=catalog.data()+16;
    const auto initialCatalog=catalog;
#else
    require(argc==2);const int outArg=1;
#endif
    std::ofstream out(argv[outArg],std::ios::binary);std::uint64_t fixtures=0,steps=0,cellBytes=0;
    const std::array<unsigned,12> offsets{0x1aa,0x1ae,0x1b2,0x1b6,0x1ba,0x1be,0x1c2,0x1c6,0x1ca,0x212,0x216,0x21a};
    const std::array<std::array<unsigned,3>,5> grids{{{1,1,1},{2,3,2},{8,4,3},{128,1,32},{1,128,32}}};
    for(const auto& grid:grids)for(unsigned fixture=0;fixture<256;++fixture){
        const auto w=grid[0],h=grid[1],l=grid[2];
        std::array<std::uint32_t,3> from{},to{};
        for(unsigned axis=0;axis<3;++axis){
            const auto period=grid[axis]*(axis==2?16:32);
            from[axis]=fixture%4==0?0:fixture%4==1?period-1:randomWord()%period;
            to[axis]=fixture<64?from[axis]:fixture<128?(fixture%2?0:period-1):randomWord();
        }
        std::array<unsigned char,0x22e + 32> raw{};for(auto& n:raw)n=static_cast<unsigned char>(randomWord());
        auto* input=raw.data()+16;auto expected=raw;auto* q=expected.data()+16;
        EffectPlacementRecord r;std::memcpy(r.parameters.data(),input+0x2c,252);
        for(unsigned i=0;i<12;++i)r.sentinels[i]=get(input,offsets[i]);
        EffectTransitionState s;std::memcpy(s.motion.trajectory.words.data(),input+0x128,56);
        s.motion.changes=get(input,0x1ce);std::memcpy(s.motion.previousUnits.data(),input+0x1ea,12);
        std::memcpy(s.previousPosition.data(),input+0x202,12);
        std::array<std::uint32_t,3> startup{};std::memcpy(startup.data(),input+0x1de,12);
        std::vector<EffectCell> cells(w*h*l);std::vector<unsigned char> cellRaw(cells.size()*12+32,0xa5);
        auto* cellBase=cellRaw.data()+16;
        for(unsigned i=0;i<cells.size();++i){
            auto& c=cells[i];c.terrain=std::array<std::uint16_t,4>{0,1,3,65535}[(i+fixture)%4];c.head=static_cast<std::uint16_t>(randomWord());c.flags=randomWord();
            word(cellBase+i*12,0,c.terrain);word(cellBase+i*12,2,c.head);put(cellBase+i*12,8,c.flags);
        }
        const auto initialCells=cellRaw;
#ifdef ORIGINAL_REFERENCE
        auto global=[](unsigned at,std::uintptr_t n){put(reinterpret_cast<unsigned char*>(at),0,std::uint32_t(n));};
        global(0x6c5494,w);global(0x6c5498,h);global(0x6c54dc,reinterpret_cast<std::uintptr_t>(cellBase));global(0x65660c,reinterpret_cast<std::uintptr_t>(terrainBase));
        for(unsigned y=0;y<h;++y)global(0x6cb942+y*4,y*w);
        for(unsigned z=0;z<l;++z)global(0x6cb8c2+z*4,z*w*h);
        reinterpret_cast<Init>(0x48a950)(input,from[0],from[1],from[2],to[0],to[1],to[2]);
#endif
        initializeEffectMotionRecord(r,s,startup,from,to,w,h,l,cells);
        std::memcpy(q+8,r.position.data(),12);std::memcpy(q+0x14,r.units.data(),12);
        std::memcpy(q+0x128,s.motion.trajectory.words.data(),56);put(q,0x1ce,s.motion.changes);
        for(unsigned i=0;i<12;++i)put(q,offsets[i],r.sentinels[i]);
        std::memcpy(q+0x1de,startup.data(),12);
#ifdef ORIGINAL_REFERENCE
        put(q,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));
        put(q,0x190,reinterpret_cast<std::uintptr_t>(terrainBase+s.terrain*356));
        require(raw==expected && cellRaw==initialCells);
#endif
        // Host addresses are checked above, then normalized to owned ordinals.
        put(q,0x194,r.cell);put(q,0x190,s.terrain);
        out.write(reinterpret_cast<const char*>(expected.data()),expected.size());
        out.write(reinterpret_cast<const char*>(cellRaw.data()),cellRaw.size());cellBytes+=cellRaw.size();++fixtures;
        auto units=from;
#ifdef ORIGINAL_REFERENCE
        auto originalUnits=from;
#endif
        for(unsigned step=0;step<16;++step){
            s.motion.trajectory.step(units,s.motion.changes);
#ifdef ORIGINAL_REFERENCE
            auto count=get(input,0x1ce);reinterpret_cast<Step>(0x4df500)(input+0x128,&originalUnits[0],&originalUnits[1],&originalUnits[2],&count);put(input,0x1ce,count);
            std::memcpy(q+0x128,s.motion.trajectory.words.data(),56);put(q,0x1ce,s.motion.changes);
            put(q,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));put(q,0x190,reinterpret_cast<std::uintptr_t>(terrainBase+s.terrain*356));
            require(raw==expected && units==originalUnits && cellRaw==initialCells);
#endif
            out.write(reinterpret_cast<const char*>(s.motion.trajectory.words.data()),56);
            out.write(reinterpret_cast<const char*>(units.data()),12);out.write(reinterpret_cast<const char*>(&s.motion.changes),4);++steps;
        }
    }
#ifdef ORIGINAL_REFERENCE
    require(catalog==initialCatalog);
#endif
    require(bool(out));std::cout<<"{\"fixtures\":"<<fixtures<<",\"steps\":"<<steps<<",\"guarded_cell_bytes\":"<<cellBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
