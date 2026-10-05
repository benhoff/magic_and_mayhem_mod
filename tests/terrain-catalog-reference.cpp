// Offline catalog/threshold reconstruction; selected valid paths make no Win32 calls.
#include "terrain_catalog.hpp"
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
#ifndef MNM_NATIVE_ONLY
extern "C" void thresholdWindow(void*,const void*,unsigned,void*);
asm(".text\n.globl thresholdWindow\n.type thresholdWindow,@function\n"
    "thresholdWindow:\npush %ebx\npush %esi\npush %edi\npush %ebp\nsub $0x2500,%esp\n"
    "mov 0x2514(%esp),%eax\nmov %eax,0x18(%esp)\nmov 0x2518(%esp),%esi\n"
    "mov 0x251c(%esp),%ebx\nlea 0x13a4(%esp),%edi\nimul $19,%ebx,%ecx\nrep movsl\n"
    "mov 0x2520(%esp),%eax\ncall *%eax\nadd $0x2500,%esp\npop %ebp\npop %edi\npop %esi\npop %ebx\nret\n"
    ".size thresholdWindow,.-thresholdWindow\n");
static void* thresholdCode(){
    constexpr unsigned length=0x52e44d-0x52e3ca;
    auto* code=static_cast<unsigned char*>(mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));require(code!=MAP_FAILED);
    const unsigned char begin[]{0x8b,0x74,0x24,0x1c,0x33,0xc0};const unsigned char end[]{0x89,0x56,0x18};
    require(!std::memcmp(reinterpret_cast<void*>(0x52e3ca),begin,sizeof begin) && !std::memcmp(reinterpret_cast<void*>(0x52e44a),end,sizeof end));
    // All relative branches stay in this unchanged bounded window. Append only
    // an adapter return; never write to the mapped original image.
    std::memcpy(code,reinterpret_cast<void*>(0x52e3ca),length);code[length]=0xc3;return code;
}
static Bytes mapHeader(const mnm::assets::MapAsset& map){
    Bytes b(76);put(b.data(),0,0);put(b.data(),4,map.width);put(b.data(),8,map.height);put(b.data(),12,map.layers);
    for(unsigned i=0;i<13;++i)put(b.data(),24+4*i,map.metadata[i]);return b;
}
#endif
static unsigned context=0;
int main(int argc,char** argv)try{
    if(argc<3)throw std::runtime_error("Expected PE, catalog fixture and optional decoded CFGs");
#ifndef MNM_NATIVE_ONLY
    mapImage(read(argv[1]));PrivateSeh seh;void* code=thresholdCode();
#endif
    unsigned cases=0,descriptors=0,quirks=0,refusals=0,installed=0,installedAccepted=0;std::map<std::string,unsigned> refusalReasons;std::mt19937 fixtures(0x52e3ca);
    const auto compare=[&](const mnm::assets::RegionRecipe& recipe,const std::vector<mnm::assets::MapAsset>& maps){
        std::vector<const mnm::assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);TerrainRegionCatalog catalog;
        try{catalog=buildTerrainRegionCatalog(recipe,sources);}catch(const std::exception& e){++refusals;++refusalReasons[e.what()];return;}
        ++cases;descriptors+=catalog.bank.blocks.size();int simple=0;for(const auto& m:maps)for(unsigned i=2;i<10;++i)simple=std::max(simple,int(std::int32_t(m.metadata[i])));quirks+=catalog.bank.internalThreshold!=simple+1;
#ifndef MNM_NATIVE_ONLY
        auto expected=makeRegionPlacementState(recipe.columns,recipe.rows,catalog.bank);auto host=stateHost(makeRegionPlacementState(recipe.columns,recipe.rows,{}));
        Bytes headers;for(const auto& m:maps){auto header=mapHeader(m);headers.insert(headers.end(),header.begin(),header.end());}
        thresholdWindow(host.data(),headers.data(),maps.size(),code);require(u32(host,12)==unsigned(catalog.bank.internalThreshold));
        unsigned entries=0;
        using Describe=void(__attribute__((thiscall)) *)(void*,unsigned*,const void*,const void*,unsigned,unsigned,unsigned);
        for(unsigned i=0;i<maps.size();++i){std::array<unsigned,5> request{};const bool specific=i<recipe.specific.size();
            if(specific){const auto& q=recipe.specific[i];request={{q.section,unsigned(q.rotation),unsigned(q.column),unsigned(q.row),1}};}
            else{const auto& q=recipe.random[i-recipe.specific.size()];request={{q.section,0xffffffffU,0xffffffffU,0xffffffffU,q.occurrences}};}
            require(entries==catalog.firstDescriptors[i]);
            for(unsigned y=0;y<maps[i].metadata[1];++y)for(unsigned x=0;x<maps[i].metadata[0];++x)reinterpret_cast<Describe>(0x52edf0)(host.data(),&entries,headers.data()+76*i,request.data(),specific,x,y);
        }
        require(entries==catalog.bank.blocks.size() && u32(host,32)==catalog.layers);
        for(unsigned i=0;i<4;++i)require(u32(host,16+4*i)==unsigned(catalog.connectors.next[i]));
        checkState(host,expected,catalog.requests);seh.check();
#endif
    };
    for(unsigned n=0;n<2048;++n){context=n;mnm::assets::RegionRecipe recipe;recipe.columns=recipe.rows=4;std::vector<mnm::assets::MapAsset> maps;
        for(unsigned i=0,count=1+fixtures()%10;i<count;++i){mnm::assets::MapAsset map;map.layers=1+fixtures()%4;map.metadata[0]=1+fixtures()%2;map.metadata[1]=1+fixtures()%2;map.width=20*map.metadata[0];map.height=20*map.metadata[1];
            for(unsigned e=2;e<10;++e)map.metadata[e]=n<1024?fixtures()%30:std::uint32_t(int(fixtures()%40)-10);
            maps.push_back(map);if(i%2==0 && recipe.random.empty())recipe.specific.push_back({i,int(fixtures()%5)-1,int(fixtures()%5)-1,int(fixtures()%5)-1});else recipe.random.push_back({i,1+fixtures()%9});}
        compare(recipe,maps);
    }
    const auto input=read(argv[2]);unsigned at=4;const unsigned count=u32(input,0);std::vector<mnm::assets::RegionRecipe> expectedRecipes;
    for(unsigned n=0;n<count;++n){context=2048+n;mnm::assets::RegionRecipe recipe;recipe.id=u32(input,at);recipe.columns=u32(input,at+4);recipe.rows=u32(input,at+8);const unsigned fixed=u32(input,at+12),random=u32(input,at+16);at+=20;
        std::vector<mnm::assets::MapAsset> maps;
        for(unsigned i=0;i<fixed+random;++i){if(i<fixed)recipe.specific.push_back({u32(input,at),int(u32(input,at+4)),int(u32(input,at+8)),int(u32(input,at+12))});else recipe.random.push_back({u32(input,at),u32(input,at+16)});
            maps.push_back(headerMap(input,at+20));at+=96;}
        const auto previous=cases;compare(recipe,maps);installedAccepted+=cases>previous;expectedRecipes.push_back(recipe);++installed;
    }
    require(at==input.size());unsigned decodedRecipes=0;
    // Independently decoded CFGs enter through the existing native reader; their
    // list fields are compared to the independent fixture recipes used above.
    for(int arg=3;arg<argc;++arg){const auto decoded=mnm::assets::decodeRegionRecipes(read(argv[arg]));require(std::holds_alternative<std::vector<mnm::assets::RegionRecipe>>(decoded));
        for(const auto& actual:std::get<std::vector<mnm::assets::RegionRecipe>>(decoded)){require(decodedRecipes<expectedRecipes.size());const auto& expected=expectedRecipes[decodedRecipes++];
            require(actual.id==expected.id && actual.columns==expected.columns && actual.rows==expected.rows && actual.specific.size()==expected.specific.size() && actual.random.size()==expected.random.size());
            for(unsigned i=0;i<actual.specific.size();++i){const auto& a=actual.specific[i];const auto& e=expected.specific[i];require(a.section==e.section && a.rotation==e.rotation && a.column==e.column && a.row==e.row);}
            for(unsigned i=0;i<actual.random.size();++i)require(actual.random[i].section==expected.random[i].section && actual.random[i].occurrences==expected.random[i].occurrences);
        }
    }
    require(decodedRecipes==count);
    require(quirks>0 && cases>0 && descriptors>0 && installed>0);
    std::cout<<"{\"cases\":"<<cases<<",\"descriptors\":"<<descriptors<<",\"threshold_quirk_cases\":"<<quirks<<",\"native_refusals\":"<<refusals<<",\"installed_recipes\":"<<installed<<",\"decoded_recipes\":"<<decodedRecipes<<",\"installed_catalogs_matched\":"<<installedAccepted<<",\"refusal_reasons\":{";
    bool first=true;for(const auto& r:refusalReasons){if(!first)std::cout<<",";first=false;std::cout<<"\""<<r.first<<"\":"<<r.second;}
    std::cout<<"},\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<"Fixture "<<context<<": "<<e.what()<<'\n';return 1;}
