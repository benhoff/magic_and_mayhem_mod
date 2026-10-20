// Execute the pinned consumer unchanged with synthetic backend bindings.
// Raster/backend bodies are deliberately outside this argument/state oracle.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <algorithm>

struct Call {std::uint32_t tag,ecx,edx,result,args[6];};
static std::vector<Call> calls;
static std::uint32_t result_value;
extern "C" void capture_dispatch(std::uint32_t tag,std::uint32_t* regs){
    Call call{tag,regs[6],regs[5],result_value,{}};
    std::memcpy(call.args,regs+10,24);calls.push_back(call);regs[7]=result_value;
}
static void* stub(std::uint32_t tag,unsigned pop){
    auto* code=static_cast<unsigned char*>(mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if(code==MAP_FAILED)throw std::runtime_error("Stub allocation");
    // pushfd/pushad; capture the actual ECX/EDX and caller stack before clobber.
    const unsigned char head[]={0x9c,0x60,0x89,0xe0,0x50,0x68};std::memcpy(code,head,sizeof(head));
    std::memcpy(code+6,&tag,4);code[10]=0xe8;
    const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(&capture_dispatch))-
                        std::uint32_t(reinterpret_cast<std::uintptr_t>(code+15));
    std::memcpy(code+11,&relative,4);
    const unsigned char tail[]={0x83,0xc4,0x08,0x61,0x9d,0xc2};std::memcpy(code+15,tail,sizeof(tail));
    const std::uint16_t cleanup=pop;std::memcpy(code+21,&cleanup,2);return code;
}
static void bind(std::uintptr_t address,unsigned pop){
    auto* destination=stub(address,pop);auto* entry=reinterpret_cast<unsigned char*>(address);
    const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(destination))-std::uint32_t(address+5);
    entry[0]=0xe9;std::memcpy(entry+1,&relative,4);
}
static std::uint32_t pointer(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
static void check(bool value,const char* text){if(!value)throw std::runtime_error(text);}
int main(int argc,char** argv)try{
    check(argc==2,"Expected pinned original PE");const auto original=read(argv[1]);map_image(original);
    const auto* consumer=reinterpret_cast<const unsigned char*>(0x5002a0);
    const Bytes before(consumer,consumer+(0x501181-0x5002a0));
    for(auto item:std::vector<std::pair<std::uintptr_t,unsigned>>{
        {0x4df840,20},{0x57dc60,8},{0x57de00,0},{0x57e540,4},{0x57e8e0,12},
        {0x57ec90,12},{0x57f0f0,12},{0x57f5f0,12},{0x57fe10,12},{0x580260,12},
        {0x5806f0,8},{0x580a90,12},{0x581200,20},{0x595677,0},{0x4d8ab0,12}})bind(item.first,item.second);
    for(auto address:{0x689b78u,0x689b7cu,0x6c4e3cu,0x6903dcu})global(address,pointer(stub(address,0)));
    std::uint32_t vtable[4]={0,0,0,pointer(stub(0x8000000c,8))};
    std::uint32_t object[20]={pointer(vtable),16,16,3,4};
    std::uint32_t clip[4]={0,0,800,600};global(0x6c482c,pointer(clip));global(0x6e1f70,0);
    using Consumer=void (__attribute__((thiscall)) *)(void*);
    unsigned cases=0;
    for(unsigned video:{0u,1u})for(unsigned auxiliary:{0u,1u})for(unsigned indexed:{0u,1u})
    for(int offset:{-127,3})for(unsigned value:{0u,1u,0x1234u,0xffffu,
        0x10000u,0x10001u,0x12340001u,0xffff0000u,0xffffffffu}){
      for(int kind=-2;kind<=35;++kind){
        std::uint32_t row[9]={0,pointer(object),120,100,0,0,std::uint32_t(kind),0,0};
        const std::int16_t x=offset,y=7;std::memcpy(reinterpret_cast<unsigned char*>(row)+16,&x,2);
        std::memcpy(reinterpret_cast<unsigned char*>(row)+18,&y,2);
        const std::uint16_t first=auxiliary?5:0x8ad0,second=0x8ad0;
        std::memcpy(reinterpret_cast<unsigned char*>(row)+28,&first,2);
        std::memcpy(reinterpret_cast<unsigned char*>(row)+30,&second,2);
        std::uint32_t queue[6]={pointer(row),0,1,1,0,0};object[7]=indexed?0:UINT32_MAX;
        global(0x5e1404,video);global(0x6e1f24,0);global(0x6de6d5,0);result_value=value;calls.clear();
        reinterpret_cast<Consumer>(0x5002a0)(queue);
        std::uint16_t actual;std::memcpy(&actual,reinterpret_cast<unsigned char*>(row)+30,2);
        const bool defaulted=kind<0||kind>34||(!video&&(kind==7||kind==32));
        const bool normalized=video&&indexed&&offset==-127&&(kind==0||kind==31);
        const std::uint16_t expected=kind==9?0:defaulted?second:normalized?std::uint16_t(std::uint16_t(value)==1):std::uint16_t(value);
        check(actual==expected,"Consumer AX/zero/default metadata policy");
        if(kind>=15&&kind<=30){
          const auto waves=std::count_if(calls.begin(),calls.end(),[](const auto& c){return c.tag==0x5806f0;});
          check(waves==int(video&&auxiliary?2:1),"Wave primary/secondary dispatch count");
          for(const auto& c:calls)if(c.tag==0x5806f0)check(c.args[1]==unsigned(kind-14),"Wave literal amplitude");
        }
        check(Bytes(consumer,consumer+before.size())==before,"Original consumer instructions changed");
        std::cout<<"{\"video\":"<<video<<",\"auxiliary\":"<<auxiliary<<",\"indexed\":"<<indexed
                 <<",\"offset\":"<<offset<<",\"result\":"<<value<<",\"kind\":"<<kind
                 <<",\"metadata\":"<<actual<<",\"calls\":[";
        for(unsigned i=0;i<calls.size();++i){if(i)std::cout<<',';const auto& c=calls[i];
          // Host heap pointers are normalized to the explicit fixture object.
          std::cout<<"{\"tag\":"<<c.tag<<",\"ecx_object\":"<<(c.ecx==pointer(object)?"true":"false")
                   <<",\"edx\":"<<c.edx<<",\"args\":[";
          for(unsigned j=0;j<6;++j){if(j)std::cout<<',';std::cout<<c.args[j];}std::cout<<"]}";
        }std::cout<<"]}\n";++cases;
      }
    }
    check(cases==5472,"Case count");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
