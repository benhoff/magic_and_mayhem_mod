// Offline, unchanged No-CD RNG and single-block region selection helpers.
#include "terrain_selection.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <sys/mman.h>
#ifndef MNM_NATIVE_ONLY
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
using Bytes=std::vector<unsigned char>;
static Bytes read(const char* path){
 std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("Cannot open input");
 const auto n=f.tellg();if(n<0 || n>32*1024*1024)throw std::runtime_error("Input extent");
 Bytes b(static_cast<std::size_t>(n));f.seekg(0);if(n && !f.read(reinterpret_cast<char*>(b.data()),n))throw std::runtime_error("Input read");return b;
}
static std::uint32_t u32(const Bytes& b,std::size_t at){if(at>b.size() || b.size()-at<4)throw std::runtime_error("Read extent");std::uint32_t v;std::memcpy(&v,b.data()+at,4);return v;}
static void mapImage(const Bytes& b){
    const auto pe=u32(b,60),opt=pe+24,length=u32(b,opt+56);
    if(u32(b,opt+28)!=0x400000 || length>32*1024*1024)throw std::runtime_error("Unexpected PE image");
    auto* p=static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000),length,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
    if(p==MAP_FAILED)throw std::runtime_error("Private PE mapping failed");
    const unsigned count=std::uint16_t(b.at(pe+6))|(std::uint16_t(b.at(pe+7))<<8);
    const auto size=std::uint16_t(b.at(pe+20))|(std::uint16_t(b.at(pe+21))<<8);
    for(unsigned i=0;i<count;++i){const auto at=opt+size+i*40,rva=u32(b,at+12),n=u32(b,at+16),raw=u32(b,at+20);
        if(std::uint64_t(raw)+n>b.size() || std::uint64_t(rva)+n>length)throw std::runtime_error("PE extent");
        std::memcpy(p+rva,b.data()+raw,n);}
    if(p[0xffd30]!=0x56 || p[0xfff60]!=0x55)throw std::runtime_error("Controller entry bytes differ");
}


static void put(void* p,unsigned off,std::uint32_t v){std::memcpy(static_cast<unsigned char*>(p)+off,&v,4);}
#define require(ok) do { if(!(ok)) throw std::runtime_error("Selection mismatch at line "+std::to_string(__LINE__)); } while(false)



using namespace mnm::reconstruction;
#ifndef MNM_NATIVE_ONLY
class PrivateSeh {
    unsigned short saved_=0; std::uint32_t head_=0xffffffffU;
public:
    PrivateSeh() {
        asm volatile("mov %%fs,%0":"=r"(saved_));
        user_desc segment{}; segment.entry_number=unsigned(-1);
        segment.base_addr=reinterpret_cast<std::uintptr_t>(&head_); segment.limit=3;
        segment.seg_32bit=1; segment.useable=1;
        if (syscall(SYS_set_thread_area,&segment)) throw std::runtime_error("Cannot install private FS exception head");
        const unsigned short selector=(segment.entry_number<<3)|3;
        asm volatile("mov %0,%%fs"::"r"(selector):"memory");
    }
    void check() const { require(head_==0xffffffffU); }
    ~PrivateSeh() { asm volatile("mov %0,%%fs"::"r"(saved_):"memory"); }
};
using Init=void(__attribute__((thiscall)) *)(void*);
using Load=void(__attribute__((thiscall)) *)(void*,unsigned);
using Pair=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
static Bytes hostFor(const RegionSelectionGrid& g,const std::vector<SingleRegionDescriptor>& descriptors) {
    Bytes host(0x4000,0);
    put(host.data(),4,g.columns);put(host.data(),8,g.rows);put(host.data(),12,1000);
    reinterpret_cast<Init>(0x52d320)(host.data());reinterpret_cast<Init>(0x52d440)(host.data());
    reinterpret_cast<Init>(0x52d390)(host.data());
    for(unsigned i=0;i<descriptors.size();++i){const auto& d=descriptors[i];const unsigned at=0x2c+63*i;
        put(host.data(),at,d.section);host[at+4]=d.specific;host[at+5]=0;
        put(host.data(),at+6,d.maximum);put(host.data(),at+10,d.placed);
        put(host.data(),at+30,1);put(host.data(),at+34,1);host[at+62]=d.special;
        for(unsigned j=0;j<4;++j)put(host.data(),at+42+4*j,d.edges[j]);
    }
    for(unsigned x=0;x<g.columns;++x)for(unsigned y=0;y<g.rows;++y){
        const auto& c=g.cells[x*g.rows+y];const unsigned at=0x39b4+37*(x*5+y);
        host[at]=c.occupied;
        for(unsigned j=0;j<4;++j)put(host.data(),at+21+4*j,c.edges[j]);
    }
    return host;
}
static unsigned originalMask(Bytes& host,unsigned descriptor,unsigned x,unsigned y){
    unsigned mask=0;
    using Admit=unsigned(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
    for(unsigned r=0;r<4;++r){reinterpret_cast<Init>(0x52d400)(host.data());reinterpret_cast<Load>(0x52f3a0)(host.data(),descriptor);reinterpret_cast<Load>(0x531880)(host.data(),r);
        if(reinterpret_cast<Admit>(0x5300a0)(host.data(),x,y)&255)mask|=1u<<r;
    }
    return mask;
}
#endif
static RegionSelectionGrid grid(unsigned columns,unsigned rows) {
    RegionSelectionGrid g;g.columns=columns;g.rows=rows;g.cells.resize(columns*rows);return g;
}
int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected pinned PE and installed single-block header fixture");
#ifndef MNM_NATIVE_ONLY
    mapImage(read(argv[1]));PrivateSeh seh;
    // The unchanged next helper uses this immutable cyclic index table.
    const auto* cycle=reinterpret_cast<const unsigned*>(0x5eb940);
    for(unsigned i=0;i<250;++i)require(cycle[i]==(i+1)%250);
#endif
    std::mt19937 fixtures(0x39b4);unsigned rngWords=0,admissions=0,candidateTables=0,locationTables=0,choices=0,rotationMaskChecks=0,headers=0;
    for(unsigned n=0;n<256;++n){const unsigned seed=n<4?std::array<unsigned,4>{{0,1,0x80000000U,0xffffffffU}}[n]:fixtures();TerrainRegionRandom rng(seed);
#ifndef MNM_NATIVE_ONLY
        std::array<unsigned,252> original{};
        using Seed=void(__attribute__((thiscall)) *)(void*,unsigned);
        using Next=unsigned(__attribute__((thiscall)) *)(void*);
        reinterpret_cast<Seed>(0x54e170)(original.data(),seed);
        require(original[0]==rng.left() && original[1]==rng.right());require(std::equal(rng.words().begin(),rng.words().end(),original.begin()+2));
#endif
        for(unsigned j=0;j<1024;++j){const auto value=rng.next();++rngWords;
#ifndef MNM_NATIVE_ONLY
            require(value==reinterpret_cast<Next>(0x54e200)(original.data()));
            require(original[0]==rng.left() && original[1]==rng.right());require(std::equal(rng.words().begin(),rng.words().end(),original.begin()+2));
#else
            (void)value;
#endif
        }
    }
    for(unsigned n=0;n<2048;++n){auto g=grid(1+fixtures()%5,1+fixtures()%5);
        for(auto& c:g.cells) {c.occupied=fixtures()%9==0;for(auto& e:c.edges)e=n%3?int(fixtures()%5)-1:-1;}
        std::vector<SingleRegionDescriptor> descriptors(1+fixtures()%50);
        for(auto& d:descriptors){d.section=fixtures()%100;d.specific=fixtures()%7==0;d.special=fixtures()%2;d.maximum=fixtures()%5;d.placed=fixtures()%6;for(auto& e:d.edges)e=int(fixtures()%5)-1;}
        const unsigned x=fixtures()%g.columns,y=fixtures()%g.rows;
#ifndef MNM_NATIVE_ONLY
        auto host=hostFor(g,descriptors);
#endif
        for(unsigned i=0;i<descriptors.size();++i){unsigned mask=0;
            for(unsigned r=0;r<4;++r){if(admitSingleRegionSection(g,descriptors[i],x,y,r))mask|=1u<<r;++admissions;}
#ifndef MNM_NATIVE_ONLY
            require(mask==originalMask(host,i,x,y));
#endif
            for(unsigned m=0;m<16;++m){const unsigned seed=fixtures();const auto choice=chooseRegionRotation(seed,m);++rotationMaskChecks;if(m==mask)++choices;
#ifndef MNM_NATIVE_ONLY
                // Exercise rotation chooser with the same actual admission mask.
                if(m==mask){reinterpret_cast<Init>(0x52d400)(host.data());reinterpret_cast<Load>(0x52f3a0)(host.data(),i);put(host.data(),0,seed);
                    using ChooseRotation=int(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
                    const int actual=reinterpret_cast<ChooseRotation>(0x530c60)(host.data(),x,y);
                    require(actual==(choice?int(*choice):-1));seh.check();
                }
#else
                (void)choice;
#endif
            }
        }
        for(unsigned special=0;special<2;++special){auto table=singleRegionCandidates(g,descriptors,x,y,special);++candidateTables;
#ifndef MNM_NATIVE_ONLY
            reinterpret_cast<Pair>(0x52d4e0)(host.data(),x,y);
            using Candidates=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned);
            reinterpret_cast<Candidates>(0x52f790)(host.data(),x,y,special);
            const unsigned at=0xc7a+450*(x*5+y);
            for(unsigned i=0;i<50;++i){require(host[at+9*i]==(i<table.size()));require(u32(host,at+9*i+1)==(i<table.size()?table[i].descriptor:0xffffffffU));
                for(unsigned r=0;r<4;++r)require(host[at+9*i+5+r]==(i<table.size()?bool(table[i].rotations&(1u<<r)):false));}
#endif
            for(unsigned j=0;j<4;++j){unsigned seed=fixtures();auto choice=chooseRegionCandidate(seed,table);++choices;
#ifndef MNM_NATIVE_ONLY
                put(host.data(),0,seed);unsigned rotation=99;
                using Choose=int(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned*);
                const int actual=reinterpret_cast<Choose>(0x52ff10)(host.data(),x,y,&rotation);
                require(actual==(choice?int(choice->index):-1));require(rotation==(choice?choice->rotation:99));require(u32(host,0)==seed);seh.check();
#else
                (void)choice;
#endif
            }
        }
        for(int rotation=-1;rotation<4;++rotation){auto table=singleRegionLocations(g,descriptors[0],rotation);++locationTables;
#ifndef MNM_NATIVE_ONLY
            for(unsigned i=0;i<25;++i){put(host.data(),0x386c+12*i,0xffffffffU);put(host.data(),0x3870+12*i,0xffffffffU);put(host.data(),0x3874+12*i,0);}
            using Locations=void(__attribute__((thiscall)) *)(void*,unsigned,int);
            reinterpret_cast<Locations>(0x5310f0)(host.data(),0,rotation);
            for(unsigned i=0;i<25;++i){const unsigned at=0x386c+12*i;require(u32(host,at)==(i<table.size()?table[i].column:0xffffffffU));require(u32(host,at+4)==(i<table.size()?table[i].row:0xffffffffU));for(unsigned r=0;r<4;++r)require(host[at+8+r]==(i<table.size()?bool(table[i].rotations&(1u<<r)):false));}
#endif
            const unsigned seed=fixtures();auto choice=chooseRegionLocation(seed,table);++choices;
#ifndef MNM_NATIVE_ONLY
            put(host.data(),0,seed);unsigned actualRotation=99;using Choose=int(__attribute__((thiscall)) *)(void*,unsigned*);
            const int actual=reinterpret_cast<Choose>(0x531710)(host.data(),&actualRotation);
            require(actual==(choice?int(choice->index):-1));require(actualRotation==(choice?choice->rotation:99));seh.check();
#else
            (void)choice;
#endif
        }
    }
    const auto input=read(argv[2]);const unsigned count=u32(input,0);require(input.size()==4+80*count);
    for(unsigned i=0;i<count;++i){const unsigned at=4+80*i;mnm::assets::MapAsset map;map.width=u32(input,at+8);map.height=u32(input,at+12);map.layers=u32(input,at+16);for(unsigned j=0;j<13;++j)map.metadata[j]=u32(input,at+28+4*j);
        for(unsigned specific=0;specific<2;++specific){const auto d=describeSingleRegionSection(map,u32(input,at),specific,7);++headers;
#ifndef MNM_NATIVE_ONLY
            auto g=grid(5,5);auto host=hostFor(g,{});unsigned entry=0;const std::array<int,5> request{{int(d.section),-1,-1,-1,7}};
            using Describe=void(__attribute__((thiscall)) *)(void*,unsigned*,const void*,const void*,unsigned,unsigned,unsigned);
            reinterpret_cast<Describe>(0x52edf0)(host.data(),&entry,input.data()+at+4,request.data(),specific,0,0);
            require(entry==1 && host[0x30]==specific && host[0x31]==0 && host[0x6a]==d.special && u32(host,0x32)==unsigned(d.maximum));
            for(unsigned j=0;j<4;++j)require(u32(host,0x56+4*j)==unsigned(d.edges[j]));
#endif
        }
    }
    std::cout<<"{\"rng_seeds\":256,\"rng_words\":"<<rngWords<<",\"admission_checks\":"<<admissions<<",\"candidate_tables\":"<<candidateTables<<",\"location_tables\":"<<locationTables<<",\"choices\":"<<choices<<",\"native_rotation_mask_checks\":"<<rotationMaskChecks<<",\"installed_descriptors\":"<<headers<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
