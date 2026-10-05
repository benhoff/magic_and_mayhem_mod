#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_projection.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Effect projection mismatch");}
#ifdef ORIGINAL_REFERENCE
static void put(unsigned char* p,unsigned at,std::uint32_t n){std::memcpy(p+at,&n,4);}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    if(argc!=3)throw std::runtime_error("Expected executable and output");map_image(read(argv[1]));
    const unsigned char entry[]={0x81,0xec,0x8c,0,0,0,0x53,0x55};
    require(!std::memcmp(reinterpret_cast<void*>(0x4883f0),entry,sizeof(entry)));
    using Move=unsigned(__attribute__((thiscall)) *)(void*,unsigned);
    const int outputArg=2;
#else
    if(argc!=2)throw std::runtime_error("Expected output");const int outputArg=1;
#endif
    std::ofstream output(argv[outputArg],std::ios::binary);unsigned calls=0,steps=0,wrapped=0;
    std::uint64_t cellBytes=0;
    const std::array<unsigned,12> offsets{0x1aa,0x1ae,0x1b2,0x1b6,0x1ba,0x1be,0x1c2,0x1c6,0x1ca,0x212,0x216,0x21a};
#ifndef ORIGINAL_REFERENCE
    (void)offsets;
#endif
    for(unsigned fixture=0;fixture<4096;++fixture){
        const unsigned w=fixture==4095?128:std::array<unsigned,3>{1,8,32}[fixture%3];
        const unsigned h=fixture==4095?128:std::array<unsigned,3>{1,8,32}[(fixture/3)%3];
        const unsigned layers=fixture==4095?32:std::array<unsigned,3>{1,3,8}[(fixture/9)%3];
        EffectPlacementPool pool(w,h,layers,1);std::array<std::uint32_t,63> p{};
        p[0]=(w/2)*32+16;p[1]=(h/2)*32+8;p[2]=(layers/2)*16+8;
        p[3]=p[0];p[4]=p[1];p[5]=p[2];p[6]=0xffffffffu;p[8]=fixture%2?68:0;p[9]=(fixture/27)%9;
        pool.place(0,std::array<unsigned,5>{3,13,22,24,36}[(fixture/243)%5],p);
        auto r=pool.records()[0];EffectProjectionState s;
        const unsigned mode=(fixture/5)%4;
        s.trajectory.words={1,0xffffffffu,fixture%2,0x12345678u,0xffffffffu,1,1,0xffffffffu,2,1,2,2,1,2};
        if(mode==1)s.trajectory.words[0]=s.trajectory.words[1]=0;
        if(mode==2)s.trajectory.words[4]=s.trajectory.words[5]=s.trajectory.words[6]=s.trajectory.words[7]=0;
        if(mode==3){
            s.trajectory.words[4]+=w*32;s.trajectory.words[6]+=w*32;
            s.trajectory.words[5]-=h*32;s.trajectory.words[7]-=h*32;
        }
        s.previousUnits={123,456,789};
#ifdef ORIGINAL_REFERENCE
        Bytes allocation(16 + 0x22e + 16,0xa5),terrain(16+356+16,0),cells(16+w*h*layers*12+16,0);
        auto* original=allocation.data()+16;auto* terrainData=terrain.data()+16;auto* cellBase=cells.data()+16;
        for(unsigned i=0;i<w*h*layers;++i){std::uint16_t empty=0xffff;std::memcpy(cellBase+i*12+2,&empty,2);std::memcpy(cellBase+i*12+4,&empty,2);}
        auto cellExpected=cells;
        put(original,0,0);put(original,4,1);put(original,0x28,r.type);
        std::memcpy(original+0x2c,r.parameters.data(),252);std::memcpy(original+0x128,s.trajectory.words.data(),56);
        put(original,0x190,reinterpret_cast<std::uintptr_t>(terrainData));put(original,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));put(original,0x198,0);
        put(original,0x1ce,s.changes);std::memcpy(original+0x1ea,s.previousUnits.data(),12);
        for(unsigned k=0;k<3;++k){put(original,8+4*k,r.position[k]);put(original,0x14+4*k,r.units[k]);put(original,0x1f6+4*k,r.initialPosition[k]);put(original,0x202+4*k,r.initialPosition[k]);}
        for(unsigned k=0;k<12;++k)put(original,offsets[k],r.sentinels[k]);
        const auto expectedInitial=allocation;
        global(0x6c5494,w);global(0x6c5498,h);global(0x6c549c,layers);global(0x6c54dc,reinterpret_cast<std::uintptr_t>(cellBase));
        global(0x6def5c,0);global(0x6b126c+p[8]*721,0);
        for(unsigned y=0;y<h;++y)global(0x6cb942+y*4,y*w);
        for(unsigned z=0;z<layers;++z)global(0x6cb8c2+z*4,z*w*h);
#endif
        const unsigned repetitions=p[9]<=4?2:1;
        for(unsigned repeat=0;repeat<repetitions;++repeat){
            require(projectEffectSameCell(r,s,w,h,layers)==3);
#ifdef ORIGINAL_REFERENCE
            const auto code=reinterpret_cast<Move>(0x4883f0)(original,fixture%2);require(code==3);
            auto expected=expectedInitial;auto* e=expected.data()+16;
            std::memcpy(e+0x2c,r.parameters.data(),252);std::memcpy(e+0x128,s.trajectory.words.data(),56);
            std::memcpy(e+0x14,r.units.data(),12);std::memcpy(e+0x1ea,s.previousUnits.data(),12);put(e,0x1ce,s.changes);
            for(unsigned k=0;k<12;++k)put(e,offsets[k],r.sentinels[k]);
            require(allocation==expected && cells==cellExpected);
            for(auto n:terrain)require(n==0);
#endif
            output.write(reinterpret_cast<const char*>(r.parameters.data()),252);
            output.write(reinterpret_cast<const char*>(r.units.data()),12);
            output.write(reinterpret_cast<const char*>(r.sentinels.data()),48);
            output.write(reinterpret_cast<const char*>(s.trajectory.words.data()),56);
            output.write(reinterpret_cast<const char*>(s.previousUnits.data()),12);
            output.write(reinterpret_cast<const char*>(&s.changes),4);
            steps+=p[9];++calls;cellBytes+=std::uint64_t(w)*h*layers*12+32;
            if(mode==3)wrapped+=p[9];
        }
    }
    if(!output)throw std::runtime_error("Output failed");
    std::cout<<"{\"fixtures\":4096,\"calls\":"<<calls<<",\"steps\":"<<steps<<",\"wrapped_steps\":"<<wrapped<<",\"cell_bytes\":"<<cellBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
