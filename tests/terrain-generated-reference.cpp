// Offline selected post-CFG attempt loop with unchanged instructions.
#include "terrain_generated.hpp"
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
#ifndef MNM_NATIVE_ONLY
// Enter unchanged 0052d269 with the original callee-save/local/argument stack.
// Original ret 12 returns to the local wrapper, which returns to its C caller.
extern "C" unsigned originalAttempts(void*);
asm(".text\n.globl originalAttempts\n.type originalAttempts,@function\n"
    "originalAttempts:\nmov 4(%esp),%ecx\npush $0\npush $0\npush $0\ncall 1f\nret\n"
    "1:\npush $0\npush %ebx\npush %ebp\npush %esi\npush %edi\n"
    "mov %ecx,%esi\nxor %ebx,%ebx\nmov $0x52d269,%eax\njmp *%eax\n"
    ".size originalAttempts,.-originalAttempts\n");
static void clearOriginalAttempt(Bytes& host){
    for(unsigned i=0;i<50;++i)put(host.data(),0x36+63*i,0);
    reinterpret_cast<Init>(0x52d440)(host.data());reinterpret_cast<Init>(0x52d390)(host.data());reinterpret_cast<Init>(0x52d400)(host.data());
    for(unsigned i=0;i<25;++i){put(host.data(),0x386c+12*i,0xffffffffU);put(host.data(),0x3870+12*i,0xffffffffU);put(host.data(),0x3874+12*i,0);}
}
#endif
static unsigned context=0;
int main(int argc,char** argv)try{
    if(argc<3)throw std::runtime_error("Expected pinned PE and installed catalog fixture");
#ifndef MNM_NATIVE_ONLY
    mapImage(read(argv[1]));installMessageImport();PrivateSeh seh;
#endif
    std::mt19937 fixtures(0x39b4);unsigned cases=0,plans=0,blocks=0,failures=0,refusals=0,installed=0,installedPlans=0;
    std::map<std::string,unsigned> refusalReasons;
    const auto compare=[&](const mnm::assets::RegionRecipe& recipe,const std::vector<mnm::assets::MapAsset>& maps,unsigned seed,bool shipped){
        std::vector<const mnm::assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);GeneratedTerrainRegionPlan result;
        try{result=generateTerrainRegionPlan(recipe,sources,seed);}catch(const std::invalid_argument& e){++refusals;++refusalReasons[e.what()];return;}catch(const std::out_of_range& e){++refusals;++refusalReasons[e.what()];return;}
        ++cases;plans+=bool(result.plan);failures+=!result.plan;installed+=shipped;installedPlans+=shipped && bool(result.plan);if(result.plan)blocks+=result.plan->blocks.size();
        require(bool(result.plan)==result.generation.complete);
#ifndef MNM_NATIVE_ONLY
        auto host=stateHost(makeRegionPlacementState(recipe.columns,recipe.rows,result.catalog.bank));putRequests(host,result.catalog.requests);put(host.data(),0,seed);dialogs.clear();
        const auto status=originalAttempts(host.data());require(bool(status)==result.generation.complete);
        checkState(host,result.generation.state,result.generation.requests);checkLocations(host,result.generation.locations);require(u32(host,0)==result.generation.nextSeed);seh.check();
        unsigned warnings=0;for(const auto& a:result.generation.attempts)for(const auto& n:a.diagnostics){require(warnings<dialogs.size() && dialogs[warnings]==unsigned(n.notice));++warnings;}require(warnings==dialogs.size());
        if(result.plan){const auto& p=*result.plan;require(p.columns==recipe.columns && p.rows==recipe.rows && p.blocks.size()==recipe.columns*recipe.rows);
            // Independent descriptor-owner spans from source header order, not
            // the catalog's sourceForDescriptor table under test.
            std::vector<unsigned> owners;for(unsigned i=0;i<maps.size();++i)for(unsigned n=0;n<maps[i].metadata[0]*maps[i].metadata[1];++n)owners.push_back(i);
            unsigned maxLayers=0;for(const auto& m:maps)maxLayers=std::max(maxLayers,m.layers);require(p.layers==maxLayers);
            for(unsigned i=0;i<p.blocks.size();++i){const unsigned x=i%recipe.columns,y=i/recipe.columns,at=0x39b4+37*(x*5+y);const auto& b=p.blocks[i];require(host[at]==1 && b.column==x && b.row==y);
                const unsigned descriptor=u32(host,at+5);require(descriptor<owners.size());const auto owner=owners[descriptor];const auto side=maps[owner].width/maps[owner].metadata[0];
                require(b.source==owner && p.side==side && b.sourceX==u32(host,at+13)*side && b.sourceY==u32(host,at+17)*side && b.rotation==u32(host,at+9));
            }
        }
#endif
    };
    for(unsigned n=0;n<2048;++n){context=n;mnm::assets::RegionRecipe recipe;recipe.columns=2+fixtures()%4;recipe.rows=2+fixtures()%4;
        std::vector<mnm::assets::MapAsset> maps;auto multi=synthetic(1+fixtures()%2,1+fixtures()%2,0);multi.width=2*multi.metadata[0];multi.height=2*multi.metadata[1];multi.layers=1+fixtures()%3;maps.push_back(multi);
        auto single=synthetic(1,1,0);single.width=single.height=2;single.layers=1+fixtures()%3;maps.push_back(single);
        // Same section number, different entry, shape, layers and occurrence cap.
        recipe.random={{4,n<1024?30U:1U},{4,n<1024?30U:1U}};compare(recipe,maps,fixtures(),false);
    }
    const auto input=read(argv[2]);const unsigned count=u32(input,0);unsigned at=4;
    for(unsigned n=0;n<count;++n){mnm::assets::RegionRecipe recipe;recipe.id=u32(input,at);recipe.columns=u32(input,at+4);recipe.rows=u32(input,at+8);const auto fixed=u32(input,at+12),random=u32(input,at+16);at+=20;std::vector<mnm::assets::MapAsset> maps;
        for(unsigned i=0;i<fixed+random;++i){if(i<fixed)recipe.specific.push_back({u32(input,at),int(u32(input,at+4)),int(u32(input,at+8)),int(u32(input,at+12))});else recipe.random.push_back({u32(input,at),u32(input,at+16)});maps.push_back(headerMap(input,at+20));at+=96;}
        for(unsigned i=0;i<4;++i){context=2048+4*n+i;compare(recipe,maps,std::array<unsigned,4>{{0,1,123,0xdeadbeefU}}[i],true);}
    }
    require(at==input.size() && plans>0 && failures>0 && installedPlans>0);
    std::cout<<"{\"cases\":"<<cases<<",\"plans\":"<<plans<<",\"plan_blocks\":"<<blocks<<",\"failed_generations\":"<<failures<<",\"installed_cases\":"<<installed<<",\"installed_plans\":"<<installedPlans<<",\"installed_recipes\":"<<count<<",\"native_refusals\":"<<refusals<<",\"refusal_reasons\":{";
    bool first=true;for(const auto& r:refusalReasons){if(!first)std::cout<<",";first=false;std::cout<<"\""<<r.first<<"\":"<<r.second;}
    std::cout<<"},\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<"Fixture "<<context<<": "<<e.what()<<'\n';return 1;}
