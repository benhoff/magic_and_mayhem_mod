// Offline, unchanged No-CD multi-block admission, descriptor and pruning helpers.
#include "terrain_constraints.hpp"
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

#ifndef MNM_NATIVE_ONLY
static Bytes multiHost(const RegionSelectionGrid& g,const RegionDescriptorBank& bank) {
    std::vector<SingleRegionDescriptor> singles;for(const auto& b:bank.blocks)singles.push_back(b.selection);
    auto host=hostFor(g,singles);put(host.data(),12,bank.internalThreshold);
    for(unsigned i=0;i<bank.blocks.size();++i){const auto& b=bank.blocks[i];const unsigned at=0x2c+63*i;
        host[at+5]=b.columns*b.rows>1;put(host.data(),at+22,b.sourceX);put(host.data(),at+26,b.sourceY);
        put(host.data(),at+30,b.columns);put(host.data(),at+34,b.rows);put(host.data(),at+38,b.layers);
    }
    return host;
}
static void checkCarry(const Bytes& host,const RegionAdmissionCarry& c){
    require(u32(host,0x3998)==c.section && u32(host,0x399c)==c.descriptor && u32(host,0x39a0)==c.rotation);
    for(unsigned d=0;d<4;++d)require(u32(host,0x39a4+4*d)==unsigned(c.edges[d]));
}
static void checkCandidates(const Bytes& host,unsigned x,unsigned y,const std::vector<RegionCandidate>& table){
    const unsigned at=0xc7a+450*(x*5+y);
    for(unsigned i=0;i<50;++i){require(host[at+9*i]==(i<table.size()));require(u32(host,at+9*i+1)==(i<table.size()?table[i].descriptor:0xffffffffU));
        for(unsigned r=0;r<4;++r)require(host[at+9*i+5+r]==(i<table.size()?bool(table[i].rotations&(1u<<r)):false));}
}
#endif
static mnm::assets::MapAsset headerMap(const Bytes& raw,unsigned at){
    mnm::assets::MapAsset map;map.width=u32(raw,at+4);map.height=u32(raw,at+8);map.layers=u32(raw,at+12);
    for(unsigned i=0;i<13;++i)map.metadata[i]=u32(raw,at+24+4*i);return map;
}
int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected pinned PE and installed header fixture");
#ifndef MNM_NATIVE_ONLY
    mapImage(read(argv[1]));PrivateSeh seh;
#endif
    std::mt19937 fixtures(0xc7a);unsigned descriptors=0,admissions=0,accepted=0,connectorSearches=0,prunes=0,candidates=0,locations=0,choices=0;
    // All original connector lookup priorities, including duplicate north matches.
    for(unsigned n=0;n<2048;++n){RegionDescriptorBank bank;bank.blocks.resize(1+fixtures()%50);
        for(auto& b:bank.blocks){b.selection.section=fixtures()%100;b.selection.placed=fixtures()%3;for(auto& e:b.selection.edges)e=1000+fixtures()%5;}
        const unsigned current=fixtures()%bank.blocks.size(),placed=fixtures()%3,label=1000+fixtures()%5;
        const auto found=findRegionConnector(bank,label,placed,current);++connectorSearches;
#ifndef MNM_NATIVE_ONLY
        auto host=multiHost(grid(5,5),bank);put(host.data(),0x399c,current);
        using Find=int(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
        require(reinterpret_cast<Find>(0x531a50)(host.data(),label,placed)==(found?int(*found):-1));
#else
        (void)found;
#endif
    }
    // Real section shapes, every toroidal anchor and all four rotations.
    for(unsigned w=1;w<=2;++w)for(unsigned h=1;h<=2;++h)for(unsigned columns=1;columns<=5;++columns)for(unsigned rows=1;rows<=5;++rows){
        mnm::assets::MapAsset map;map.width=20*w;map.height=20*h;map.layers=7;map.metadata[0]=w;map.metadata[1]=h;
        for(unsigned i=2;i<10;++i)map.metadata[i]=fixtures()%4;
        RegionConnectorState connectors;RegionDescriptorBank bank;bank.blocks=expandRegionDescriptors(map,37,false,4,connectors);
        for(unsigned variation=0;variation<4;++variation)for(unsigned x=0;x<columns;++x)for(unsigned y=0;y<rows;++y){auto g=grid(columns,rows);
            if(variation==1)for(auto& c:g.cells)c.occupied=fixtures()%5==0;
            if(variation==2)for(auto& c:g.cells)for(auto& e:c.edges)e=fixtures()%2?int(fixtures()%4):-1;
            if(variation==3)for(auto& c:g.cells)c.occupied=true;
            for(unsigned index=0;index<bank.blocks.size();++index)for(unsigned r=0;r<4;++r){const auto result=admitRegionSection(g,bank,index,x,y,r);++admissions;accepted+=result.admitted;
#ifndef MNM_NATIVE_ONLY
                auto host=multiHost(g,bank);const auto before=host;
                reinterpret_cast<Init>(0x52d400)(host.data());reinterpret_cast<Load>(0x52f3a0)(host.data(),index);reinterpret_cast<Load>(0x531880)(host.data(),r);
                using Admit=unsigned(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
                const auto actual=reinterpret_cast<Admit>(0x5300a0)(host.data(),x,y)&255;
                require(bool(actual)==result.admitted);checkCarry(host,result.carry);
                require(std::equal(host.begin()+0x2c,host.begin()+0xc7a,before.begin()+0x2c));
                require(std::equal(host.begin()+0x39b4,host.begin()+0x3d51,before.begin()+0x39b4));
#endif
            }
            for(unsigned special=0;special<2;++special){const auto table=regionCandidates(g,bank,x,y,special);++candidates;
#ifndef MNM_NATIVE_ONLY
                auto host=multiHost(g,bank);using Candidates=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned);
                reinterpret_cast<Candidates>(0x52f790)(host.data(),x,y,special);checkCandidates(host,x,y,table);
#endif
                const unsigned seed=fixtures();auto choice=chooseRegionCandidate(seed,table);++choices;
#ifndef MNM_NATIVE_ONLY
                put(host.data(),0,seed);unsigned rotation=99;using Choose=int(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned*);
                require(reinterpret_cast<Choose>(0x52ff10)(host.data(),x,y,&rotation)==(choice?int(choice->index):-1));require(rotation==(choice?choice->rotation:99));seh.check();
#else
                (void)choice;
#endif
            }
        }
        auto g=grid(columns,rows);
        for(int rotation=-1;rotation<4;++rotation){auto table=regionLocations(g,bank,0,rotation);++locations;
#ifndef MNM_NATIVE_ONLY
            auto host=multiHost(g,bank);for(unsigned i=0;i<25;++i){put(host.data(),0x386c+12*i,0xffffffffU);put(host.data(),0x3870+12*i,0xffffffffU);put(host.data(),0x3874+12*i,0);}
            using Locations=void(__attribute__((thiscall)) *)(void*,unsigned,int);reinterpret_cast<Locations>(0x5310f0)(host.data(),0,rotation);
            for(unsigned i=0;i<25;++i){const unsigned at=0x386c+12*i;require(u32(host,at)==(i<table.size()?table[i].column:0xffffffffU));require(u32(host,at+4)==(i<table.size()?table[i].row:0xffffffffU));for(unsigned r=0;r<4;++r)require(host[at+8+r]==(i<table.size()?bool(table[i].rotations&(1u<<r)):false));}
#endif
        }
    }
    // Three-pass pruning, rotation-equivalent edges, duplicate entries and counts.
    for(unsigned n=0;n<8192;++n){RegionDescriptorBank bank;bank.blocks.resize(1+fixtures()%50);
        for(auto& b:bank.blocks){b.selection.section=fixtures()%100;b.selection.maximum=5;b.selection.placed=n%7?fixtures()%7:0;for(auto& e:b.selection.edges)e=fixtures()%3;}
        std::vector<RegionCandidate> input;const unsigned size=n%7?fixtures()%50:49;
        for(unsigned i=0;i<size;++i)input.push_back({unsigned(fixtures()%bank.blocks.size()),std::uint8_t(1+fixtures()%15)});
        const unsigned tried=fixtures()%bank.blocks.size();const int rotation=int(fixtures()%5)-1;
        const auto output=pruneRegionCandidates(bank,input,tried,rotation);++prunes;
#ifndef MNM_NATIVE_ONLY
        auto host=multiHost(grid(5,5),bank);const unsigned at=0xc7a+450*12;
        for(unsigned i=0;i<input.size();++i){host[at+9*i]=1;put(host.data(),at+9*i+1,input[i].descriptor);for(unsigned r=0;r<4;++r)host[at+9*i+5+r]=bool(input[i].rotations&(1u<<r));}
        const auto before=host;
        using Prune=void(__attribute__((thiscall)) *)(void*,unsigned,int,unsigned,unsigned);
        reinterpret_cast<Prune>(0x52fb00)(host.data(),tried,rotation,2,2);checkCandidates(host,2,2,output);
        require(std::equal(host.begin()+0x2c,host.begin()+0xc7a,before.begin()+0x2c));
#endif
    }
    const auto input=read(argv[2]);const unsigned count=u32(input,0);require(input.size()==4+80*count);
    for(unsigned i=0;i<count;++i){const unsigned at=4+80*i;auto map=headerMap(input,at+4);
        for(unsigned specific=0;specific<2;++specific){RegionConnectorState next;next.next={{2000,2001,2002,2003}};const auto expanded=expandRegionDescriptors(map,u32(input,at),specific,7,next);descriptors+=expanded.size();
#ifndef MNM_NATIVE_ONLY
            auto host=hostFor(grid(5,5),{});for(unsigned d=0;d<4;++d)put(host.data(),16+4*d,2000+d);unsigned entry=0;const std::array<int,5> request{{int(u32(input,at)),-1,-1,-1,7}};
            using Describe=void(__attribute__((thiscall)) *)(void*,unsigned*,const void*,const void*,unsigned,unsigned,unsigned);
            for(unsigned y=0;y<map.metadata[1];++y)for(unsigned x=0;x<map.metadata[0];++x)reinterpret_cast<Describe>(0x52edf0)(host.data(),&entry,input.data()+at+4,request.data(),specific,x,y);
            require(entry==expanded.size() && u32(host,32)==map.layers);
            for(unsigned j=0;j<expanded.size();++j){const auto& b=expanded[j];const unsigned base=0x2c+63*j;
                require(u32(host,base)==b.selection.section && host[base+4]==b.selection.specific && host[base+5]==(b.columns*b.rows>1) && host[base+62]==b.selection.special);
                require(u32(host,base+6)==unsigned(b.selection.maximum) && u32(host,base+10)==0);
                require(u32(host,base+22)==b.sourceX && u32(host,base+26)==b.sourceY && u32(host,base+30)==b.columns && u32(host,base+34)==b.rows && u32(host,base+38)==b.layers);
                for(unsigned d=0;d<4;++d)require(u32(host,base+42+4*d)==unsigned(b.selection.edges[d]));
            }
            for(unsigned d=0;d<4;++d)require(u32(host,16+4*d)==unsigned(next.next[d]));
#endif
            RegionDescriptorBank bank;bank.internalThreshold=2000;bank.blocks=expanded;auto g=grid(5,5);
            for(unsigned j=0;j<expanded.size();++j)for(unsigned r=0;r<4;++r){auto result=admitRegionSection(g,bank,j,0,0,r);++admissions;accepted+=result.admitted;
#ifndef MNM_NATIVE_ONLY
                auto host=multiHost(g,bank);reinterpret_cast<Load>(0x52f3a0)(host.data(),j);reinterpret_cast<Load>(0x531880)(host.data(),r);
                using Admit=unsigned(__attribute__((thiscall)) *)(void*,unsigned,unsigned);require(bool(reinterpret_cast<Admit>(0x5300a0)(host.data(),0,0)&255)==result.admitted);checkCarry(host,result.carry);
#endif
            }
        }
    }
    std::cout<<"{\"installed_maps\":"<<count<<",\"installed_descriptors\":"<<descriptors<<",\"admissions\":"<<admissions<<",\"accepted\":"<<accepted<<",\"connector_searches\":"<<connectorSearches<<",\"candidate_tables\":"<<candidates<<",\"location_tables\":"<<locations<<",\"choices\":"<<choices<<",\"prunes\":"<<prunes<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
