#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_creature_collision.hpp"
#include "creature_footprint.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
#define require(v) do { if(!(v)) throw std::runtime_error("Creature comparison mismatch at line " + std::to_string(__LINE__)); } while(false)
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
    std::ofstream output(argv[outArg],std::ios::binary);unsigned calls=0,hits=0;
    std::uint64_t cellBytes=0;
    const std::array<unsigned,12> offsets{0x1aa,0x1ae,0x1b2,0x1b6,0x1ba,0x1be,0x1c2,0x1c6,0x1ca,0x212,0x216,0x21a};
#ifndef ORIGINAL_REFERENCE
    (void)offsets;
#endif
    constexpr unsigned orderedFixtures=27*6*5*5*2;
    constexpr unsigned exitFixtures=5*2*2;
    constexpr unsigned mutationFixtures=6*2;
    const unsigned fixtureCount=orderedFixtures+exitFixtures+mutationFixtures;
    unsigned orderedCalls=0,exitCalls=0,mutationCalls=0;
    const std::array<std::array<int,2>,9> compass{{{0,0},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}}};
    for(unsigned fixture=0;fixture<fixtureCount;++fixture){
        const bool ordered=fixture<orderedFixtures;
        const bool mixed=!ordered && fixture<orderedFixtures+exitFixtures;
        unsigned code=fixture;
        const bool membership=code%2;code/=2;
        const unsigned typeIndex=code%5;code/=5;
        const unsigned filter=ordered?code%5:0;code/=5;
        const unsigned geometry=ordered?code%6:0;code/=6;
        const unsigned target=ordered?code%27:0;
        const unsigned w=geometry==3 || geometry==5?1:geometry==4?2:8;
        const unsigned h=geometry==3?1:geometry==4 || geometry==5?2:8;
        const unsigned layers=geometry==3?1:geometry==5?2:3;
        const unsigned slot=0;
        EffectPlacementPool pool(w,h,layers,1);
        for(auto& cell:pool.cells()){cell.terrain=0;cell.flags=0x1280;}
        std::array<std::uint32_t,63> p{};
        p[0]=p[3]=(geometry==1?0:geometry==2?w-1:w/2)*32+8;
        p[1]=p[4]=(geometry==1?0:geometry==2?h-1:h/2)*32+8;
        p[2]=p[5]=(geometry==1?0:geometry==2?layers-1:layers/2)*16+8;
        p[6]=NoCreature;p[9]=1;
        unsigned type=std::array<unsigned,5>{3,13,22,24,36}[typeIndex];
        if(!ordered){p[0]=p[3]=64;p[1]=p[4]=64;p[2]=p[5]=16;type=3;}
        pool.place(0,type,p);
        EffectCreatureTransitionState state;auto& s=state.movement;
        s.previousPosition=pool.records()[0].position;s.terrain=0;
        s.motion.previousUnits={123,456,789};
        s.motion.trajectory.words={0,0,1,99,0,0,0,0,2,1,2,2,1,2};
        const auto pos=pool.records()[0].position;
        auto cellAt=[&](unsigned rank)->unsigned {
            int plane=rank/9==0?0:rank/9==1?1:-1;
            auto xy=compass[rank%9];int z=int(pos[2])+plane;
            if(z<0 || z>=int(layers))return NoCreature;
            unsigned x=(int(pos[0])+xy[0]+int(w))%int(w);
            unsigned y=(int(pos[1])+xy[1]+int(h))%int(h);
            return (unsigned(z)*h+y)*w+x;
        };
        EffectCreatureWorld world;world.cells.assign(pool.cells().size(),NoEffect);
        for(unsigned i=0;i<3;++i){
            EffectCollisionCreature c;c.id=100+i;c.units=pool.records()[0].units;
            for(auto& n:c.units)n-=8;
            c.footprint.rows.fill(0xffff);c.footprint.height=4;world.creatures.push_back(c);
        }
        EffectTerrainOccupancy occupancy{};
        unsigned scenario=NoCreature;
        if(ordered){
            auto cell=cellAt(target);
            if(cell!=NoCreature)world.cells[cell]=filter==3?4:0;
            if(filter==1)world.creatures[0].status=27;
            if(filter==2)pool.records()[0].parameters[6]=0;
            if(filter==4){
                for(unsigned rank=0;rank<27;++rank){auto at=cellAt(rank);if(at!=NoCreature)world.cells[at]=std::uint16_t(rank%3);}
                world.creatures[0].status=27;
            }
        }else if(mixed){
            scenario=(fixture-orderedFixtures)/4;
            // First hit is guaranteed at X=68; second step tests each subsequent outcome.
            pool.records()[0].parameters[9]=2;
            s.motion.trajectory.words[6]=4;s.motion.trajectory.words[4]=scenario==1?32:4;
            world.cells[pool.records()[0].cell]=0;
            if(scenario==0)occupancy[0][0]=0x20; // X-local 2 at the second step.
            if(scenario==1)pool.cells()[(1*h+2)*w+3].flags|=0x40000000u;
            if(scenario==2)s.motion.trajectory.words[1]=layers*16; // Raw height exit.
            if(scenario>=3){
                world.creatures[0].units={64,64,16};world.creatures[0].footprint.rows.fill(0);world.creatures[0].footprint.rows[0]=0x4000;
                if(scenario==4){world.creatures[1].units={64,64,16};world.creatures[1].footprint.rows.fill(0);world.creatures[1].footprint.rows[0]=0x2000;world.cells[(1*h+1)*w+2]=1;}
            }
        }else{
            scenario=(fixture-orderedFixtures-exitFixtures)/2;
            world.cells[pool.records()[0].cell]=0;
        }
        state.candidate=2;

#ifdef ORIGINAL_REFERENCE
        Bytes creatures(16+3*0xe4b+16,0xa5);auto* creatureBase=creatures.data()+16;
        for(unsigned i=0;i<3;++i){auto* c=creatureBase+i*0xe4b;put(c,0,world.creatures[i].id);put(c,0xa8,world.creatures[i].status);std::memcpy(c+0x14,world.creatures[i].units.data(),12);std::memcpy(c+0xb97,world.creatures[i].footprint.rows.data(),32);put(c,0xbb7,world.creatures[i].footprint.height);}

        Bytes records(16 + 0x22e + 16,0xa5),terrain(16+4*356+16,0),cells(16+pool.cells().size()*12+16,0);
        auto* original=records.data()+16;auto* catalog=terrain.data()+16;auto* cellBase=cells.data()+16;
        std::array<std::uint32_t,2> manager{reinterpret_cast<std::uintptr_t>(original),1};
        for(unsigned i=0;i<1;++i){
            auto* q=original+i*0x22e;const auto& r=pool.records()[i];put(q,0,i);put(q,4,1);put(q,0x24,reinterpret_cast<std::uintptr_t>(manager.data()));put(q,0x28,r.type);
            std::memcpy(q+0x2c,r.parameters.data(),252);word(q,0x1a0,r.next);put(q,0x198,i==slot && state.candidate!=NoCreature?reinterpret_cast<std::uintptr_t>(creatureBase+state.candidate*0xe4b):0);
            put(q,0x190,reinterpret_cast<std::uintptr_t>(catalog+pool.cells()[r.cell].terrain*356));put(q,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));
            for(unsigned k=0;k<3;++k){put(q,8+k*4,r.position[k]);put(q,0x14+k*4,r.units[k]);put(q,0x1f6+k*4,r.initialPosition[k]);put(q,0x202+k*4,r.initialPosition[k]);}
            for(unsigned k=0;k<12;++k)put(q,offsets[k],r.sentinels[k]);
        }
        auto* moving=original+slot*0x22e;std::memcpy(moving+0x128,s.motion.trajectory.words.data(),56);std::memcpy(moving+0x1ea,s.motion.previousUnits.data(),12);put(moving,0x1ce,s.motion.changes);
        for(unsigned i=0;i<pool.cells().size();++i){const auto& c=pool.cells()[i];word(cellBase+i*12,0,c.terrain);word(cellBase+i*12,2,c.head);word(cellBase+i*12,4,world.cells[i]);word(cellBase+i*12,6,0xffff);put(cellBase+i*12,8,c.flags);}

        for(unsigned entry=0;entry<4;++entry)std::memcpy(catalog+entry*356,occupancy[entry].data(),32);
        const auto initialTerrain=terrain;
        global(0x6a49c0,0);global(0x6c54a0,w*h);

        const auto initialRecords=records;
        global(0x6c5494,w);global(0x6c5498,h);global(0x6c549c,layers);global(0x6c54dc,reinterpret_cast<std::uintptr_t>(cellBase));global(0x65660c,reinterpret_cast<std::uintptr_t>(catalog));
        global(0x6def5c,3);global(0x6def58,reinterpret_cast<std::uintptr_t>(creatureBase));global(0x6b126c+p[8]*721,0);
        for(unsigned y=0;y<h;++y)global(0x6cb942+y*4,y*w);
        for(unsigned z=0;z<layers;++z)global(0x6cb8c2+z*4,z*w*h);
#endif
        const unsigned repeats=!ordered && !mixed?2:1;
        for(unsigned repeat=0;repeat<repeats;++repeat){
            if(repeat==1){
                switch(scenario){
                case 0:world.creatures[0].status=27;break;
                case 1:world.cells[pool.records()[0].cell]=1;break;
                case 2:world.creatures[0].units[0]+=64;break;
                case 3:world.creatures[0].footprint.rows.fill(0);break;
                case 4:world.creatures[0].id=777;break;
                case 5:world.cells[pool.records()[0].cell]=NoEffect;break;
                }
#ifdef ORIGINAL_REFERENCE
                for(unsigned i=0;i<3;++i){auto* c=creatureBase+i*0xe4b;put(c,0,world.creatures[i].id);put(c,0xa8,world.creatures[i].status);std::memcpy(c+0x14,world.creatures[i].units.data(),12);std::memcpy(c+0xb97,world.creatures[i].footprint.rows.data(),32);put(c,0xbb7,world.creatures[i].footprint.height);}
                for(unsigned i=0;i<world.cells.size();++i)word(cellBase+i*12,4,world.cells[i]);
#endif
            }
#ifdef ORIGINAL_REFERENCE
            const auto initialCreatures=creatures,initialCells=cells;
#endif
            const auto result=transitionEffectCreatureWorld(pool,slot,state,w,h,layers,world,{},membership,occupancy);
            if(ordered){
                unsigned first=NoCreature;
                for(unsigned rank=0;rank<27;++rank){auto at=cellAt(rank);if(at==NoCreature)continue;auto n=world.cells[at];if(n<3 && n!=pool.records()[0].parameters[6] && world.creatures[n].status!=27){first=n;break;}}
                require(result==(first==NoCreature?3u:0u));
                if(first!=NoCreature){require(state.candidate==first);require(pool.records()[0].parameters[7]==100+first);}
                ++orderedCalls;
            }else if(mixed){
                unsigned expected=scenario==2?2:scenario<2?1:0;
                require(result==expected);require(pool.records()[0].parameters[7]==(scenario<2?NoCreature:scenario==4?101u:100u));
                if(scenario==3)require(state.candidate==NoCreature);
                ++exitCalls;
            }else{require(result==(repeat==0 || scenario==1 || scenario==4?0u:3u));++mutationCalls;}

            hits+=result==0;

#ifdef ORIGINAL_REFERENCE
            auto originalResult=reinterpret_cast<Move>(0x4883f0)(moving,membership?1:0);if(originalResult!=result)std::cerr<<"fixture "<<fixture<<" repeat "<<repeat<<" result original "<<originalResult<<" native "<<result<<"\n";if(originalResult!=result){std::uint32_t ptr,id;std::memcpy(&ptr,moving+0x198,4);std::memcpy(&id,moving+0x48,4);std::cerr<<"candidate original "<<(ptr?((ptr-reinterpret_cast<std::uintptr_t>(creatureBase))/0xe4b):NoCreature)<<" native "<<state.candidate<<" id "<<id<<" nativeid "<<pool.records()[slot].parameters[7]<<"\n";}require(originalResult==result);
            auto expected=initialRecords;auto* expectedBase=expected.data()+16;
            for(unsigned i=0;i<1;++i){auto* q=expectedBase+i*0x22e;const auto& r=pool.records()[i];std::memcpy(q+0x2c,r.parameters.data(),252);word(q,0x1a0,r.next);
                put(q,0x190,reinterpret_cast<std::uintptr_t>(catalog+pool.cells()[r.cell].terrain*356));put(q,0x194,reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));
                for(unsigned k=0;k<3;++k){put(q,8+k*4,r.position[k]);put(q,0x14+k*4,r.units[k]);put(q,0x1f6+k*4,r.initialPosition[k]);}
                for(unsigned k=0;k<12;++k)put(q,offsets[k],r.sentinels[k]);
            }
            auto* q=expectedBase+slot*0x22e;std::memcpy(q+0x128,s.motion.trajectory.words.data(),56);std::memcpy(q+0x1ea,s.motion.previousUnits.data(),12);put(q,0x1ce,s.motion.changes);std::memcpy(q+0x202,s.previousPosition.data(),12);
            put(q,0x198,state.candidate==NoCreature?0:reinterpret_cast<std::uintptr_t>(creatureBase+state.candidate*0xe4b));
            if(records!=expected){for(unsigned k=0;k<records.size();++k)if(records[k]!=expected[k]){std::cerr<<"fixture "<<fixture<<" repeat "<<repeat<<" offset "<<std::hex<<k-16<<" original "<<unsigned(records[k])<<" native "<<unsigned(expected[k])<<std::dec<<"\n";break;}} require(records==expected);require(creatures==initialCreatures);
            auto expectedCells=initialCells;for(unsigned i=0;i<pool.cells().size();++i){word(expectedCells.data()+16+i*12,2,pool.cells()[i].head);put(expectedCells.data()+16+i*12,8,pool.cells()[i].flags);}
            require(cells==expectedCells);if(!membership)require(cells==initialCells);require(terrain==initialTerrain);
#endif
            for(const auto& r:pool.records()){
                output.write(reinterpret_cast<const char*>(r.parameters.data()),252);
                for(const auto* a:{&r.position,&r.units,&r.initialPosition})output.write(reinterpret_cast<const char*>(a->data()),12);
                output.write(reinterpret_cast<const char*>(r.sentinels.data()),48);output.write(reinterpret_cast<const char*>(&r.next),2);
            }
            output.write(reinterpret_cast<const char*>(s.motion.trajectory.words.data()),56);output.write(reinterpret_cast<const char*>(s.motion.previousUnits.data()),12);output.write(reinterpret_cast<const char*>(&s.motion.changes),4);output.write(reinterpret_cast<const char*>(s.previousPosition.data()),12);output.write(reinterpret_cast<const char*>(&s.terrain),4);
            for(const auto& c:pool.cells()){output.write(reinterpret_cast<const char*>(&c.terrain),2);output.write(reinterpret_cast<const char*>(&c.head),2);output.write(reinterpret_cast<const char*>(&c.flags),4);}
            output.write(reinterpret_cast<const char*>(&state.candidate),4);output.write(reinterpret_cast<const char*>(&result),4);++calls;cellBytes+=pool.cells().size()*12+32;
        }
    }
    if(!output)throw std::runtime_error("Output failed");
    std::cout<<"{\"fixtures\":"<<fixtureCount<<",\"calls\":"<<calls<<",\"ordered_calls\":"<<orderedCalls<<",\"mixed_exit_calls\":"<<exitCalls<<",\"mutation_calls\":"<<mutationCalls<<",\"creature_hits\":"<<hits<<",\"cell_bytes\":"<<cellBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
