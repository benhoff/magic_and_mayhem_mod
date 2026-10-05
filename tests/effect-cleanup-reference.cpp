#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_cleanup.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Cleanup mismatch");}
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    require(argc==3);map_image(read(argv[1]));const int outArg=2;
    const unsigned char entry[]={0x83,0xec,0x08,0x53,0x8b,0xd9,0x57};
    require(!std::memcmp(reinterpret_cast<void*>(0x534520),entry,sizeof(entry)));
    require(*reinterpret_cast<unsigned char*>(0x488330)==0x53);
    using Check=unsigned(__attribute__((thiscall)) *)(void*,void*);
    using Unlink=void(__attribute__((thiscall)) *)(void*,void*);
#else
    require(argc==2);const int outArg=1;
#endif
    std::ofstream out(argv[outArg],std::ios::binary);unsigned marked=0;
    for(unsigned fixture=0;fixture<3072;++fixture){
        const unsigned w=8,h=16,layers=8,cell=fixture%(w*h*layers),mode=fixture%8;
        std::vector<EffectCleanupColumn> columns;
        if(fixture%3){columns.resize(w*h);columns[cell%(w*h)]={std::uint16_t(fixture%5),std::int8_t(fixture%2?cell/(w*h):-1),std::int8_t(fixture%4?7:-128)};}
        const auto eligible=effectCleanupEligible(cell,w,h,layers,columns);
        EffectCell value;value.terrain=mode==1?1:0;value.flags=mode==4?0x20000000u:mode==5?0x80:0x1200;
        // Moving slot is head. Remaining head blocks cleanup; head absence still invokes it.
        const auto next=std::uint16_t(mode==0?1:NoEffect);
        value.head=mode==6?NoEffect:next;
        const auto creature=std::uint16_t(mode==2?0:NoEffect),other=std::uint16_t(mode==3?0:NoEffect);
        cleanupEmptyEffectCell(value,cell,w,h,layers,columns,creature,other);
#ifdef ORIGINAL_REFERENCE
        auto put=[](unsigned char* p,unsigned n,std::uint32_t v){std::memcpy(p+n,&v,4);};
        auto word=[](unsigned char* p,unsigned n,std::uint16_t v){std::memcpy(p+n,&v,2);};
        Bytes cells(16+w*h*layers*12+16,0xa5),table(16+w*h*6+16,0xa5),records(16+2*0x22e + 16,0xa5);
        auto* base=cells.data()+16;auto* q=base+cell*12;auto* t=table.data()+16;auto* r=records.data()+16;
        word(q,0,mode==1?1:0);word(q,2,mode==6?NoEffect:0);word(q,4,creature);word(q,6,other);put(q,8,mode==4?0x20000000u:mode==5?0x80:0x1200);
        for(unsigned i=0;i<columns.size();++i){word(t+i*6,0,columns[i].marker);t[i*6+2]=std::uint8_t(columns[i].lower);t[i*6+3]=std::uint8_t(columns[i].upper);}
        std::array<std::uint32_t,2> manager{reinterpret_cast<std::uintptr_t>(r),2};put(r,0,0);put(r,0x24,reinterpret_cast<std::uintptr_t>(manager.data()));word(r,0x1a0,next);word(r+0x22e,0x1a0,NoEffect);
        global(0x6c54dc,reinterpret_cast<std::uintptr_t>(base));global(0x6c5494,w);global(0x6c5498,h);global(0x6c54a0,w*h);
        for(unsigned y=0;y<h;++y)global(0x6cb942+y*4,y*w);
        for(unsigned z=0;z<layers;++z)global(0x6cb8c2+z*4,z*w*h);
        global(0x6a49c0,columns.empty()?0:reinterpret_cast<std::uintptr_t>(t));
        const auto beforeTable=table,beforeCells=cells,beforeRecords=records;
        require(reinterpret_cast<Check>(0x534520)(reinterpret_cast<void*>(0x6a49c0),q)==unsigned(eligible));
        require(cells==beforeCells && records==beforeRecords && table==beforeTable);
        reinterpret_cast<Unlink>(0x488330)(r,q);
        auto expectedCells=beforeCells,expectedRecords=beforeRecords;
        word(expectedCells.data()+16+cell*12,2,value.head);put(expectedCells.data()+16+cell*12,8,value.flags);
        if(mode!=6)word(expectedRecords.data()+16,0x1a0,NoEffect);
        require(cells==expectedCells && records==expectedRecords && table==beforeTable);
#endif
        out.write(reinterpret_cast<const char*>(&value.flags),4);out.put(eligible);marked+=(value.flags&0x80)!=0;
    }
    // Explicit caller validation is native policy, without equivalence claims.
    bool rejected=false;try{effectCleanupEligible(0,8,16,8,std::vector<EffectCleanupColumn>(1));}catch(const std::invalid_argument&){rejected=true;}require(rejected);
    std::cout<<"{\"cleanup_fixtures\":3072,\"cleanup_calls\":6144,\"marked\":"<<marked<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
