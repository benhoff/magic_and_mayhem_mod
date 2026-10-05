#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_transition.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Membership-disabled mismatch");}
#ifdef ORIGINAL_REFERENCE
static void put(unsigned char* p,unsigned at,std::uint32_t n){std::memcpy(p+at,&n,4);}
static void word(unsigned char* p,unsigned at,std::uint16_t n){std::memcpy(p+at,&n,2);}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    if(argc!=3)throw std::runtime_error("Expected executable and output");map_image(read(argv[1]));
    const unsigned char entry[]={0x81,0xec,0x8c,0,0,0,0x53,0x55};
    require(!std::memcmp(reinterpret_cast<void*>(0x4883f0),entry,sizeof(entry)));
    using Move=unsigned(__attribute__((thiscall)) *)(void*,unsigned);
    const int outArg=2;
#else
    if(argc!=2)throw std::runtime_error("Expected output");const int outArg=1;
#endif
    std::ofstream output(argv[outArg],std::ios::binary);unsigned calls=0,steps=0,transitions=0,cacheChanges=0,blocked=0;
    std::uint64_t cellBytes=0;
    const std::array<unsigned,12> offsets{0x1aa,0x1ae,0x1b2,0x1b6,0x1ba,0x1be,0x1c2,0x1c6,0x1ca,0x212,0x216,0x21a};
#ifndef ORIGINAL_REFERENCE
    (void)offsets;
#endif
    for(unsigned fixture=0;fixture<2048;++fixture){
        const unsigned w=fixture==2047?128:std::array<unsigned,3>{8,16,32}[fixture%3];
        const unsigned h=fixture==2047?128:std::array<unsigned,3>{8,16,32}[(fixture/3)%3];
        const unsigned layers=fixture==2047?32:fixture%2?3:8;
        const unsigned slot=(fixture/9)%3,mode=(fixture/27)%8;
        EffectPlacementPool pool(w,h,layers,4);for(unsigned i=0;i<pool.cells().size();++i){pool.cells()[i].terrain=std::uint16_t(i%4);pool.cells()[i].flags=0x1280;}
        std::array<std::uint32_t,63> p{};p[0]=(w/2)*32+1;p[1]=(h/2)*32+1;p[2]=(layers/2)*16+1;
        p[3]=p[0];p[4]=p[1];p[5]=p[2];p[6]=0xffffffffu;p[8]=fixture%2?68:0;p[9]=(fixture/24)%9;
        for(unsigned n=0;n<3;++n)pool.place(n,std::array<unsigned,5>{3,13,22,24,36}[(fixture+n)%5],p);
        EffectTransitionState s;s.previousPosition=pool.records()[slot].position;s.terrain=pool.cells()[pool.records()[slot].cell].terrain;
        s.motion.previousUnits={123,456,789};s.motion.trajectory.words={0,0,1,99,32,0,32,0,2,1,2,2,1,2};
        if(mode==1){s.motion.trajectory.words[4]=s.motion.trajectory.words[6]=0xffffffe0u;s.motion.trajectory.words[5]=s.motion.trajectory.words[7]=32;}
        if(mode==2)s.motion.trajectory.words[4]=s.motion.trajectory.words[6]=64;
        if(mode==3){s.motion.trajectory.words[4]=s.motion.trajectory.words[6]=0;s.motion.trajectory.words[0]=16;s.motion.trajectory.words[1]=0xfffffff0u;}
        if(mode==4)s.motion.trajectory.words[5]=s.motion.trajectory.words[7]=0xffffffe0u;
        if(mode==5)s.motion.trajectory.words[4]=s.motion.trajectory.words[6]=0;
        if(mode==6){s.motion.trajectory.words[4]=s.motion.trajectory.words[6]=w*32;s.motion.trajectory.words[5]=s.motion.trajectory.words[7]=0xffffffe0u;}
        if(mode==7){s.motion.trajectory.words[4]=s.motion.trajectory.words[6]=0;s.motion.trajectory.words[5]=s.motion.trajectory.words[7]=64;}
        std::vector<EffectCleanupColumn> columns;
        if(fixture%3){
            columns.resize(w*h);
            for(unsigned i=0;i<columns.size();++i)columns[i]={std::uint16_t((i+fixture)%3),std::int8_t((i+fixture)%layers),std::int8_t(fixture%2?-1:(i+fixture+1)%layers)};
        }
        auto firstUnits=pool.records()[slot].units;auto firstTrajectory=s.motion.trajectory;unsigned count=0;firstTrajectory.step(firstUnits,count);
        auto normalize=[](std::uint32_t v,unsigned period){auto n=v<0x80000000u?std::int64_t(v):std::int64_t(v)-0x100000000ll;if(n<0)n+=period;if(n>=period)n-=period;return unsigned(n);};
        auto destination=p;destination[0]=normalize(firstUnits[0],w*32);destination[1]=normalize(firstUnits[1],h*32);destination[2]=firstUnits[2];pool.place(3,24,destination);
        // Set the first distinct cell on one of the first four authored steps.
        auto probe=pool.records()[slot].units;auto trajectory=s.motion.trajectory;unsigned probeChanges=0;
        for(unsigned tick=0;tick<1+(fixture%4);++tick){
            trajectory.step(probe,probeChanges);probe[0]=normalize(probe[0],w*32);probe[1]=normalize(probe[1],h*32);
            const auto target=((probe[2]>>4)*h+(probe[1]>>5))*w+(probe[0]>>5);
            if(target!=pool.records()[slot].cell && tick==fixture%4)pool.cells()[target].flags|=0x40000000u;
        }
        // Include initial blocked same-cell/zero-iteration cases: this flag is
        // tested by the original only after a cell change.
        if(fixture%7==0)pool.cells()[pool.records()[slot].cell].flags|=0x40000000u;

#ifdef ORIGINAL_REFERENCE
        Bytes records(16 + 4*0x22e + 16,0xa5),terrain(16+4*356+16,0),cells(16+pool.cells().size()*12+16,0);
        auto* original=records.data()+16;auto* catalog=terrain.data()+16;auto* cellBase=cells.data()+16;
        std::array<std::uint32_t,2> manager{reinterpret_cast<std::uintptr_t>(original),4};
        for(unsigned i=0;i<4;++i){
            auto* q=original+i*0x22e;const auto& r=pool.records()[i];put(q,0,i);put(q,4,1);put(q,0x24,reinterpret_cast<std::uintptr_t>(manager.data()));put(q,0x28,r.type);
            std::memcpy(q+0x2c,r.parameters.data(),252);word(q,0x1a0,r.next);put(q,0x198,0);
            put(q,0x190,reinterpret_cast<std::uintptr_t>(catalog+pool.cells()[r.cell].terrain*356));put(q,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));
            for(unsigned k=0;k<3;++k){put(q,8+k*4,r.position[k]);put(q,0x14+k*4,r.units[k]);put(q,0x1f6+k*4,r.initialPosition[k]);put(q,0x202+k*4,r.initialPosition[k]);}
            for(unsigned k=0;k<12;++k)put(q,offsets[k],r.sentinels[k]);
        }
        auto* moving=original+slot*0x22e;std::memcpy(moving+0x128,s.motion.trajectory.words.data(),56);std::memcpy(moving+0x1ea,s.motion.previousUnits.data(),12);put(moving,0x1ce,s.motion.changes);
        for(unsigned i=0;i<pool.cells().size();++i){const auto& c=pool.cells()[i];word(cellBase+i*12,0,c.terrain);word(cellBase+i*12,2,c.head);word(cellBase+i*12,4,0xffff);word(cellBase+i*12,6,0xffff);put(cellBase+i*12,8,c.flags);}
        Bytes columnBytes(16+w*h*6+16,0xa5);auto* columnBase=columnBytes.data()+16;
        for(unsigned i=0;i<columns.size();++i){word(columnBase+i*6,0,columns[i].marker);columnBase[i*6+2]=std::uint8_t(columns[i].lower);columnBase[i*6+3]=std::uint8_t(columns[i].upper);}
        const auto initialColumns=columnBytes;
        global(0x6a49c0,columns.empty()?0:reinterpret_cast<std::uintptr_t>(columnBase));global(0x6c54a0,w*h);
        // The callback receiver may be an invalid pointer: arg=0 must never use it.
        if(fixture%5==0)global(0x6a49c0,1);
        const auto initialRecords=records,initialCells=cells;
        global(0x6c5494,w);global(0x6c5498,h);global(0x6c549c,layers);global(0x6c54dc,reinterpret_cast<std::uintptr_t>(cellBase));global(0x65660c,reinterpret_cast<std::uintptr_t>(catalog));
        global(0x6def5c,0);global(0x6b126c+p[8]*721,0);
        for(unsigned y=0;y<h;++y)global(0x6cb942+y*4,y*w);
        for(unsigned z=0;z<layers;++z)global(0x6cb8c2+z*4,z*w*h);
#endif
        const unsigned repeats=p[9]<=4?2:1;
        for(unsigned repeat=0;repeat<repeats;++repeat){
            // Count actual intermediate transitions/cache updates with one-step owned continuations.
            auto trace=pool;auto traceState=s;trace.records()[slot].parameters[9]=1;
            unsigned executed=0,expectedResult=3;
            for(unsigned tick=0;tick<p[9];++tick){const auto before=trace.records()[slot];expectedResult=transitionEffectEmptyWorld(trace,slot,traceState,w,h,layers,columns,false);++executed;transitions+=before.cell!=trace.records()[slot].cell;cacheChanges+=before.initialPosition!=trace.records()[slot].initialPosition;if(expectedResult==1)break;}

            const auto result=transitionEffectEmptyWorld(pool,slot,s,w,h,layers,columns,false);require(result==expectedResult);blocked+=result==1;
#ifdef ORIGINAL_REFERENCE
            require(reinterpret_cast<Move>(0x4883f0)(moving,0)==result);
            auto expected=initialRecords;auto* expectedBase=expected.data()+16;
            for(unsigned i=0;i<4;++i){auto* q=expectedBase+i*0x22e;const auto& r=pool.records()[i];std::memcpy(q+0x2c,r.parameters.data(),252);word(q,0x1a0,r.next);
                put(q,0x190,reinterpret_cast<std::uintptr_t>(catalog+pool.cells()[r.cell].terrain*356));put(q,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));
                for(unsigned k=0;k<3;++k){put(q,8+k*4,r.position[k]);put(q,0x14+k*4,r.units[k]);put(q,0x1f6+k*4,r.initialPosition[k]);}
                for(unsigned k=0;k<12;++k)put(q,offsets[k],r.sentinels[k]);
            }
            auto* q=expectedBase+slot*0x22e;std::memcpy(q+0x128,s.motion.trajectory.words.data(),56);std::memcpy(q+0x1ea,s.motion.previousUnits.data(),12);put(q,0x1ce,s.motion.changes);std::memcpy(q+0x202,s.previousPosition.data(),12);
            require(records==expected);
            auto expectedCells=initialCells;for(unsigned i=0;i<pool.cells().size();++i){word(expectedCells.data()+16+i*12,2,pool.cells()[i].head);put(expectedCells.data()+16+i*12,8,pool.cells()[i].flags);}
            require(cells==expectedCells);require(cells==initialCells);require(columnBytes==initialColumns);for(auto n:terrain)require(n==0);
#endif
            for(const auto& r:pool.records()){
                output.write(reinterpret_cast<const char*>(r.parameters.data()),252);
                for(const auto* a:{&r.position,&r.units,&r.initialPosition})output.write(reinterpret_cast<const char*>(a->data()),12);
                output.write(reinterpret_cast<const char*>(r.sentinels.data()),48);output.write(reinterpret_cast<const char*>(&r.next),2);
            }
            output.write(reinterpret_cast<const char*>(s.motion.trajectory.words.data()),56);output.write(reinterpret_cast<const char*>(s.motion.previousUnits.data()),12);output.write(reinterpret_cast<const char*>(&s.motion.changes),4);output.write(reinterpret_cast<const char*>(s.previousPosition.data()),12);output.write(reinterpret_cast<const char*>(&s.terrain),4);
            for(const auto& c:pool.cells()){output.write(reinterpret_cast<const char*>(&c.terrain),2);output.write(reinterpret_cast<const char*>(&c.head),2);output.write(reinterpret_cast<const char*>(&c.flags),4);}
            output.write(reinterpret_cast<const char*>(&result),4);++calls;steps+=executed;cellBytes+=pool.cells().size()*12+32;
        }
    }
    if(!output)throw std::runtime_error("Output failed");
    std::cout<<"{\"fixtures\":2048,\"calls\":"<<calls<<",\"steps\":"<<steps<<",\"transitions\":"<<transitions<<",\"cache_updates\":"<<cacheChanges<<",\"blocked\":"<<blocked<<",\"cell_bytes\":"<<cellBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
