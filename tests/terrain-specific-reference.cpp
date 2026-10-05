// Offline Specific placement; private MessageBoxA import records notices only.
#include "terrain_specific.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <map>
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
static void checkState(const Bytes& host,const RegionPlacementState& state,const std::vector<RegionSpecificRequest>& requests){
    auto expectedHost=stateHost(state);
    for(unsigned i=0;i<requests.size();++i){put(expectedHost.data(),0x3a+63*i,unsigned(requests[i].column));put(expectedHost.data(),0x3e +63*i,unsigned(requests[i].row));put(expectedHost.data(),0x66+63*i,unsigned(requests[i].rotation));}
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
#ifndef MNM_NATIVE_ONLY
static std::vector<unsigned> dialogs;
static int __attribute__((stdcall)) message(void* window,const char* text,const char* title,unsigned flags){
    require(!window && reinterpret_cast<std::uintptr_t>(title)==0x5ea510 && flags==0x40030);
    const auto address=reinterpret_cast<std::uintptr_t>(text);
    const std::array<unsigned,4> addresses{{0x5ea49c,0x5ea430,0x5ea3c8,0x5ea378}};
    const auto found=std::find(addresses.begin(),addresses.end(),address);require(found!=addresses.end());
    dialogs.push_back(unsigned(found-addresses.begin()));return 1;
}
static void installMessageImport(){
    const auto name=*reinterpret_cast<const unsigned*>(0x5c520c);
    require(name<0x302000 && !std::strcmp(reinterpret_cast<const char*>(0x400000+name+2),"MessageBoxA"));
    // This private import binding changes no executable instruction or file.
    *reinterpret_cast<unsigned*>(0x5c520c)=reinterpret_cast<std::uintptr_t>(&message);
}
static void putRequests(Bytes& host,const std::vector<RegionSpecificRequest>& requests){
    for(unsigned i=0;i<requests.size();++i){put(host.data(),0x3a+63*i,unsigned(requests[i].column));put(host.data(),0x3e +63*i,unsigned(requests[i].row));put(host.data(),0x66+63*i,unsigned(requests[i].rotation));}
}
static void checkLocations(const Bytes& host,const std::vector<RegionLocation>& locations){
    for(unsigned i=0;i<25;++i){const auto at=0x386c+12*i;require(u32(host,at)==(i<locations.size()?locations[i].column:0xffffffffU));require(u32(host,at+4)==(i<locations.size()?locations[i].row:0xffffffffU));
        for(unsigned r=0;r<4;++r)require(host[at+8+r]==(i<locations.size()?bool(locations[i].rotations&(1u<<r)):false));}
}
#endif
static unsigned context=0;
int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected pinned PE and installed header fixture");
#ifndef MNM_NATIVE_ONLY
    mapImage(read(argv[1]));installMessageImport();PrivateSeh seh;
#endif
    std::mt19937 fixtures(0x52f3f0);unsigned cases=0,refusals=0,multiCases=0,placedBlocks=0,suppressed=0;
    std::array<unsigned,4> notices{};std::map<std::string,unsigned> refusalReasons;
    const auto compare=[&](const RegionPlacementState& initial,const std::vector<RegionSpecificRequest>& requests,unsigned seed){
        RegionSpecificResult result;
        try{result=placeRegionSpecifics(initial,requests,seed);}catch(const std::invalid_argument& e){++refusals;++refusalReasons[e.what()];return;}catch(const std::out_of_range& e){++refusals;++refusalReasons[e.what()];return;}
        ++cases;multiCases+=std::any_of(initial.bank.blocks.begin(),initial.bank.blocks.end(),[](const auto& b){return b.columns*b.rows>1;});
        for(unsigned i=0;i<initial.assignments.size();++i)placedBlocks+=bool(result.state.assignments[i]) && !initial.assignments[i];
        for(const auto& n:result.diagnostics){++notices[unsigned(n.notice)];suppressed+=n.notice==RegionSpecificNotice::AllLocationsEmpty;}
#ifndef MNM_NATIVE_ONLY
        auto host=stateHost(initial);putRequests(host,requests);put(host.data(),0,seed);
        for(unsigned i=0;i<25;++i){put(host.data(),0x386c+12*i,0xffffffffU);put(host.data(),0x3870+12*i,0xffffffffU);put(host.data(),0x3874+12*i,0);}
        dialogs.clear();reinterpret_cast<Init>(0x52f3f0)(host.data());
        checkState(host,result.state,result.requests);checkLocations(host,result.locations);require(u32(host,0)==seed);
        require(dialogs.size()==result.diagnostics.size());for(unsigned i=0;i<dialogs.size();++i)require(dialogs[i]==unsigned(result.diagnostics[i].notice));seh.check();
#endif
    };
    for(unsigned n=0;n<4096;++n){context=n;RegionDescriptorBank bank;RegionConnectorState next;std::vector<RegionSpecificRequest> requests;
        const unsigned columns=2+fixtures()%4,rows=2+fixtures()%4,groups=1+fixtures()%5;
        for(unsigned group=0;group<groups;++group){const unsigned w=n>=2048?1+fixtures()%2:1,h=n>=2048?1+fixtures()%2:1;
            auto map=synthetic(w,h,n<1024 || n>=2048?0:fixtures()%3);
            if(n>=1024 && n<2048)for(unsigned side=0;side<4;++side)map.metadata[2+2*side]=fixtures()%4;
            auto blocks=expandRegionDescriptors(map,group,true,1,next);
            const int mode=fixtures()%4;RegionSpecificRequest request;
            request.rotation=int(fixtures()%5)-1;
            if(mode==0 || mode==1){request.column=fixtures()%columns;request.row=fixtures()%rows;}
            if(mode==2)request.row=fixtures()%rows; // one-coordinate wildcard normalization
            if(mode==3)request.column=fixtures()%columns;
            for(const auto& block:blocks){bank.blocks.push_back(block);requests.push_back(request);}
        }
        auto initial=makeRegionPlacementState(columns,rows,bank);
        if(n%7==0){RegionBlockDescriptor obstacle;obstacle.selection.section=99;obstacle.selection.maximum=30;obstacle.selection.edges={{0,0,0,0}};
            initial.bank.blocks.push_back(obstacle);requests.push_back({});placeRegionSection(initial,initial.bank.blocks.size()-1,0,fixtures()%columns,fixtures()%rows);}
        if(n%19==0)for(auto& b:initial.bank.blocks)if(b.selection.specific)b.selection.placed=b.selection.maximum;
        compare(initial,requests,fixtures());
    }
    const auto input=read(argv[2]);const unsigned count=u32(input,0);require(input.size()==4+80*count);
    for(unsigned i=0;i<count;++i)for(unsigned mode=0;mode<10;++mode){context=4096+10*i+mode;auto map=headerMap(input,4+80*i+4);RegionConnectorState next;RegionDescriptorBank bank;bank.blocks=expandRegionDescriptors(map,i%100,true,1,next);
        auto initial=makeRegionPlacementState(4,4,bank);RegionSpecificRequest request;request.rotation=int(mode%5)-1;
        if(mode<5){request.column=2;request.row=2;}
        std::vector<RegionSpecificRequest> requests(bank.blocks.size(),request);compare(initial,requests,fixtures());
    }
    require(cases>0 && multiCases>0 && placedBlocks>0 && suppressed>0);
    for(const auto n:notices)require(n>0);
    std::cout<<"{\"cases\":"<<cases<<",\"multi_block_cases\":"<<multiCases<<",\"placed_blocks\":"<<placedBlocks<<",\"suppressed_descriptors\":"<<suppressed<<",\"notices\":["<<notices[0]<<","<<notices[1]<<","<<notices[2]<<","<<notices[3]<<"],\"native_refusals\":"<<refusals<<",\"installed_maps\":"<<count<<",\"refusal_reasons\":{";
    bool first=true;for(const auto& reason:refusalReasons){if(!first)std::cout<<",";first=false;std::cout<<"\""<<reason.first<<"\":"<<reason.second;}
    std::cout<<"},\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<"Fixture "<<context<<": "<<e.what()<<'\n';return 1;}
