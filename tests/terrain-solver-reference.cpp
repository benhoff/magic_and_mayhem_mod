// Offline, unchanged No-CD placement and one-attempt backtracking driver.
#include "terrain_solver.hpp"
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

#ifndef MNM_NATIVE_ONLY
static Bytes stateHost(const RegionPlacementState& state){
    auto host=multiHost(state.grid,state.bank);reinterpret_cast<Init>(0x52d400)(host.data());
    if(state.carry){const auto& c=*state.carry;put(host.data(),0x3998,c.section);put(host.data(),0x399c,c.descriptor);put(host.data(),0x39a0,c.rotation);for(unsigned d=0;d<4;++d)put(host.data(),0x39a4+4*d,c.edges[d]);}
    for(unsigned x=0;x<state.grid.columns;++x)for(unsigned y=0;y<state.grid.rows;++y){const auto i=x*state.grid.rows+y;const unsigned at=0x39b4+37*(x*5+y);
        if(state.assignments[i]){const auto b=*state.assignments[i];const auto& d=state.bank.blocks[b.descriptor];put(host.data(),at+1,d.selection.section);put(host.data(),at+5,b.descriptor);put(host.data(),at+9,b.rotation);put(host.data(),at+13,d.sourceX);put(host.data(),at+17,d.sourceY);}
        put(host.data(),at+17,state.storedSourceRows[i]);
        const auto& table=state.candidates[i];const auto base=0xc7a+450*(x*5+y);
        for(unsigned j=0;j<table.size();++j){host[base+9*j]=1;put(host.data(),base+9*j+1,table[j].descriptor);for(unsigned r=0;r<4;++r)host[base+9*j+5+r]=bool(table[j].rotations&(1u<<r));}
    }
    return host;
}
static void checkState(const Bytes& host,const RegionPlacementState& state){
    const auto expectedHost=stateHost(state);
    require(std::equal(host.begin()+0x2c,host.begin()+0xc7a,expectedHost.begin()+0x2c));
    require(std::equal(host.begin()+0xc7a,host.begin()+0x386c,expectedHost.begin()+0xc7a));
    require(std::equal(host.begin()+0x39b4,host.begin()+0x3d51,expectedHost.begin()+0x39b4));
    for(unsigned i=0;i<state.bank.blocks.size();++i)require(u32(host,0x36+63*i)==unsigned(state.bank.blocks[i].selection.placed));
    for(unsigned x=0;x<state.grid.columns;++x)for(unsigned y=0;y<state.grid.rows;++y){const auto i=x*state.grid.rows+y;const unsigned at=0x39b4+37*(x*5+y);require(host[at]==state.grid.cells[i].occupied);
        std::array<unsigned,5> expected{{0xffffffffU,0xffffffffU,0xffffffffU,0xffffffffU,state.storedSourceRows[i]}};
        if(state.assignments[i]){const auto b=*state.assignments[i];const auto& d=state.bank.blocks[b.descriptor];expected={{d.selection.section,b.descriptor,b.rotation,d.sourceX,d.sourceY}};}
        for(unsigned j=0;j<5;++j)require(u32(host,at+1+4*j)==expected[j]);for(unsigned j=0;j<4;++j)require(u32(host,at+21+4*j)==unsigned(state.grid.cells[i].edges[j]));
        checkCandidates(host,x,y,state.candidates[i]);
    }
    if(state.carry)checkCarry(host,*state.carry);
    else{require(u32(host,0x3998)==0xffffffffU && u32(host,0x399c)==0xffffffffU && u32(host,0x39a0)==0);for(unsigned d=0;d<4;++d)require(u32(host,0x39a4+4*d)==0xffffffffU);}
}
#endif
static mnm::assets::MapAsset synthetic(unsigned w,unsigned h,unsigned edge){
    mnm::assets::MapAsset m;m.width=20*w;m.height=20*h;m.layers=1;m.metadata[0]=w;m.metadata[1]=h;
    for(unsigned i=2;i<10;++i)m.metadata[i]=edge;return m;
}
static unsigned context=0;
int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected pinned PE and installed header fixture");
#ifndef MNM_NATIVE_ONLY
    mapImage(read(argv[1]));PrivateSeh seh;
#endif
    std::mt19937 fixtures(0x532030);unsigned placements=0,removals=0,solves=0,successes=0,backtracked=0,multiBacktracked=0,refusals=0,pruningCarries=0;
    for(unsigned w=1;w<=2;++w)for(unsigned h=1;h<=2;++h)for(unsigned c=2;c<=5;++c)for(unsigned r=2;r<=5;++r)for(unsigned x=0;x<c;++x)for(unsigned y=0;y<r;++y)for(unsigned rotation=0;rotation<4;++rotation){
        RegionConnectorState next;RegionDescriptorBank bank;bank.blocks=expandRegionDescriptors(synthetic(w,h,0),37,false,4,next);
        for(auto& b:bank.blocks)b.selection.placed=1;
        auto state=makeRegionPlacementState(c,r,bank);
#ifndef MNM_NATIVE_ONLY
        auto host=stateHost(state);using Place=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);
        reinterpret_cast<Place>(0x531b20)(host.data(),0,rotation,x,y);
#endif
        placeRegionSection(state,0,rotation,x,y);++placements;
#ifndef MNM_NATIVE_ONLY
        checkState(host,state);
#endif
        for(unsigned sx=0;sx<c;++sx)for(unsigned sy=0;sy<r;++sy)if(state.assignments[sx*r+sy]){
#ifndef MNM_NATIVE_ONLY
            const auto descriptor=state.assignments[sx*r+sy]->descriptor;put(host.data(),0x36+63*descriptor,unsigned(state.bank.blocks[descriptor].selection.placed-1));reinterpret_cast<Pair>(0x52d480)(host.data(),sx,sy);
#endif
            removeRegionBlock(state,sx,sy);++removals;
#ifndef MNM_NATIVE_ONLY
            checkState(host,state);
#endif
        }
    }
    // Scratch carry left by pruning, including empty tables, has driver effects.
    for(unsigned n=0;n<2048;++n){RegionDescriptorBank bank;bank.blocks.resize(1+fixtures()%12);for(auto& b:bank.blocks){b.selection.maximum=5;b.selection.placed=fixtures()%7;for(auto& e:b.selection.edges)e=fixtures()%3;}
        std::vector<RegionCandidate> table;for(unsigned i=0;i<fixtures()%20;++i)table.push_back({unsigned(fixtures()%bank.blocks.size()),std::uint8_t(1+fixtures()%15)});
        const unsigned tried=fixtures()%bank.blocks.size();const int rotation=int(fixtures()%5)-1;auto state=makeRegionPlacementState(3,3,bank);state.candidates[4]=table;
#ifndef MNM_NATIVE_ONLY
        auto host=stateHost(state);using Prune=void(__attribute__((thiscall)) *)(void*,unsigned,int,unsigned,unsigned);reinterpret_cast<Prune>(0x52fb00)(host.data(),tried,rotation,1,1);
#endif
        auto detail=pruneRegionCandidateDetails(bank,table,tried,rotation);state.candidates[4]=detail.candidates;if(detail.carry)state.carry=detail.carry;++pruningCarries;
#ifndef MNM_NATIVE_ONLY
        checkState(host,state);
#endif
    }
    for(unsigned n=0;n<3072;++n){context=n;RegionDescriptorBank bank;RegionConnectorState next;
        const unsigned columns=1+fixtures()%5,rows=1+fixtures()%5;
        if(n>=2048 && columns>=2 && rows>=2){
            for(unsigned group=0;group<2;++group){auto blocks=expandRegionDescriptors(synthetic(2,2,0),group,false,1+fixtures()%3,next);bank.blocks.insert(bank.blocks.end(),blocks.begin(),blocks.end());}
        }
        const unsigned count=1+fixtures()%8;
        for(unsigned i=0;i<count;++i){auto m=synthetic(1,1,n<1024?0:fixtures()%3);if(n>=1024 && n<2048)for(unsigned d=0;d<4;++d)m.metadata[2+2*d]=fixtures()%3;
            auto blocks=expandRegionDescriptors(m,50+i,false,fixtures()%9,next);bank.blocks.insert(bank.blocks.end(),blocks.begin(),blocks.end());}
        auto initial=makeRegionPlacementState(columns,rows,bank);
        // A fixed non-origin obstacle tests the original specific rollback exit.
        if(n%17==0 && columns>=3){RegionBlockDescriptor fixed;fixed.selection.section=99;fixed.selection.specific=true;initial.bank.blocks.push_back(fixed);placeRegionSection(initial,initial.bank.blocks.size()-1,0,1,0);}
        const unsigned seed=fixtures();RegionSolveResult result;
        try{result=solveRegionPlacement(initial,seed);}catch(const std::invalid_argument&){++refusals;continue;}catch(const std::out_of_range&){++refusals;continue;}
        ++solves;successes+=result.complete;backtracked+=result.backtracks!=0;multiBacktracked+=result.multiBlockBacktracks!=0;
#ifndef MNM_NATIVE_ONLY
        auto host=stateHost(initial);put(host.data(),0,seed);using Solve=unsigned(__attribute__((thiscall)) *)(void*);
        const auto actual=reinterpret_cast<Solve>(0x532030)(host.data());
        if(bool(actual&255)!=result.complete){std::cerr<<"solve "<<n<<" grid "<<columns<<","<<rows<<" seed "<<seed<<" actual "<<actual<<" native "<<result.complete<<" retreats "<<result.backtracks<<"\n";for(const auto& b:bank.blocks)std::cerr<<b.selection.maximum<<" ";std::cerr<<"\n";}
        require(bool(actual&255)==result.complete);checkState(host,result.state);require(u32(host,0)==seed);seh.check();
#endif
    }
    const auto input=read(argv[2]);const unsigned count=u32(input,0);require(input.size()==4+80*count);
    for(unsigned i=0;i<count;++i){auto map=headerMap(input,4+80*i+4);RegionConnectorState next;RegionDescriptorBank bank;bank.blocks=expandRegionDescriptors(map,i%100,false,9,next);
        auto initial=makeRegionPlacementState(4,4,bank);const unsigned seed=fixtures();auto result=solveRegionPlacement(initial,seed);++solves;successes+=result.complete;backtracked+=result.backtracks!=0;multiBacktracked+=result.multiBlockBacktracks!=0;
#ifndef MNM_NATIVE_ONLY
        auto host=stateHost(initial);put(host.data(),0,seed);using Solve=unsigned(__attribute__((thiscall)) *)(void*);require(bool(reinterpret_cast<Solve>(0x532030)(host.data())&255)==result.complete);checkState(host,result.state);seh.check();
#endif
    }
    require(backtracked>0 && multiBacktracked>0);
    std::cout<<"{\"placements\":"<<placements<<",\"removals\":"<<removals<<",\"pruning_carries\":"<<pruningCarries<<",\"solves\":"<<solves<<",\"successes\":"<<successes<<",\"backtracked_cases\":"<<backtracked<<",\"multi_backtracked_cases\":"<<multiBacktracked<<",\"native_refusals\":"<<refusals<<",\"installed_maps\":"<<count<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<"Fixture "<<context<<": "<<e.what()<<'\n';return 1;}
