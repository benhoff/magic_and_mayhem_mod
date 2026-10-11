// Original queue consumer, with explicit synthetic external/backend bindings.
// This oracle checks consumer state and call arguments, not downstream pixels.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <algorithm>
#include <array>

struct StateCall {std::uint32_t tag,ecx,edx,return_pc,args[6];};
static std::vector<StateCall> state_calls;
static std::uint32_t state_result;
extern "C" void capture_queue_state(std::uint32_t tag,std::uint32_t* regs){
    StateCall call{tag,regs[6],regs[5],regs[9],{}};
    std::memcpy(call.args,regs+10,24);state_calls.push_back(call);regs[7]=state_result;
}
static void* state_stub(std::uint32_t tag,unsigned pop){
    auto* code=static_cast<unsigned char*>(mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if(code==MAP_FAILED)throw std::runtime_error("Stub allocation");
    const unsigned char head[]={0x9c,0x60,0x89,0xe0,0x50,0x68};std::memcpy(code,head,sizeof(head));
    std::memcpy(code+6,&tag,4);code[10]=0xe8;
    const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(&capture_queue_state))-
                        std::uint32_t(reinterpret_cast<std::uintptr_t>(code+15));
    std::memcpy(code+11,&relative,4);
    const unsigned char tail[]={0x83,0xc4,0x08,0x61,0x9d,0xc2};std::memcpy(code+15,tail,sizeof(tail));
    const std::uint16_t cleanup=pop;std::memcpy(code+21,&cleanup,2);return code;
}
static void state_bind(std::uintptr_t address,unsigned pop){
    auto* destination=state_stub(address,pop);auto* entry=reinterpret_cast<unsigned char*>(address);
    const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(destination))-std::uint32_t(address+5);
    entry[0]=0xe9;std::memcpy(entry+1,&relative,4);
}
static std::uint32_t state_pointer(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
static std::uint32_t state_global(std::uintptr_t p){std::uint32_t v;std::memcpy(&v,reinterpret_cast<void*>(p),4);return v;}
static void state_check(bool value,const char* text){if(!value)throw std::runtime_error(text);}

int main(int argc,char** argv)try{
    state_check(argc==2,"Expected pinned original PE");map_image(read(argv[1]));
    const auto* consumer=reinterpret_cast<const unsigned char*>(0x5002a0);
    const Bytes before(consumer,consumer+(0x501181-0x5002a0));
    for(auto item:std::vector<std::pair<std::uintptr_t,unsigned>>{
        {0x4df840,20},{0x57dc60,8},{0x57de00,0},{0x57e540,4},{0x57e8e0,12},
        {0x57ec90,12},{0x57f0f0,12},{0x57f5f0,12},{0x57fe10,12},{0x580260,12},
        {0x5806f0,8},{0x580a90,12},{0x581200,20},{0x595677,0},{0x4d8ab0,12}})state_bind(item.first,item.second);
    for(auto address:{0x689b78u,0x689b7cu,0x6c4e3cu,0x6903dcu})global(address,state_pointer(state_stub(address,0)));
    std::uint32_t vtable[4]={0,0,0,state_pointer(state_stub(0x8000000c,8))};
    std::uint32_t object[20]={state_pointer(vtable),16,16,3,4};object[7]=UINT32_MAX;
    using Consumer=void (__attribute__((thiscall)) *)(void*);
    unsigned cases=0;
    for(unsigned video:{0u,1u})for(unsigned count:{0u,1u,2u})for(unsigned post:{0u,1u})
    for(unsigned kind:{9u,15u,35u})for(int y:{-20,-12,0,1,100,468,480,588,600})
    for(std::uint16_t first:{std::uint16_t(0x8ad0),std::uint16_t(-13),std::uint16_t(-12),std::uint16_t(-11),std::uint16_t(0),std::uint16_t(5)})
    for(std::uint16_t prior:{std::uint16_t(0x8ad0),std::uint16_t(0),std::uint16_t(1),std::uint16_t(0x1234),std::uint16_t(0xffff)})
    for(unsigned height_flag:{0u,1u})for(unsigned value:{0u,0x12345678u,0xffffffffu}){
        std::array<std::uint32_t,18> rows{};
        for(unsigned i=0;i<2;++i){auto* row=rows.data()+i*9;
            row[1]=state_pointer(object);row[2]=120+i;row[3]=std::uint32_t(y+int(i)*3);
            row[4]=std::uint32_t(3)|(std::uint32_t(7)<<16);row[6]=kind;
            row[7]=first|(std::uint32_t(prior)<<16);
        }
        const auto original_rows=rows;
        std::uint32_t clip[4]={3,0,799,600};const std::uint32_t pitch=19;
        global(0x6c482c,state_pointer(clip));global(0x6e1f70,pitch);
        global(0x5e1404,video);global(0x6de6d5,height_flag);global(0x6e1f24,post);
        const std::array<std::uint32_t,3> pending={0x10203040,0x11223344,0xaabbccdd};
        for(unsigned i=0;i<3;++i)global(0x6e8db0+i*4,pending[i]);
        std::array<std::uint32_t,6> queue={state_pointer(rows.data()),0x12345678,count,2,0x10203040,0xaabbccdd};
        const auto original_queue=queue;state_result=value;state_calls.clear();
        reinterpret_cast<Consumer>(0x5002a0)(queue.data());
        state_check(queue==original_queue,"Consumer changed queue container");
        for(unsigned i=0;i<2;++i){auto expected=original_rows;
            if(i<count){
                if(video&&prior!=0x8ad0)expected[i*9+3]=std::uint32_t(y+int(i)*3-int(std::int16_t(prior))+16);
                const auto metadata=kind==35?prior:kind==9?0:std::uint16_t(value);
                expected[i*9+7]=first|(std::uint32_t(metadata)<<16);
            }
            state_check(std::equal(rows.begin()+i*9,rows.begin()+(i+1)*9,expected.begin()+i*9),
                        "Consumer prior-metadata/y or untouched row policy");
        }
        const auto cleanup=std::count_if(state_calls.begin(),state_calls.end(),[](const auto& c){return c.tag==0x4d8ab0;});
        state_check(cleanup==int(post),"Postlude count, including empty queue");
        for(unsigned i=0;i<3;++i)state_check(state_global(0x6e8db0+i*4)==(post?0:pending[i]),"Postlude reset policy");
        if(post){const auto& call=state_calls.back();state_check(call.tag==0x4d8ab0&&call.ecx==0x6ddf78,
            "Postlude order/receiver");state_check(std::equal(pending.begin(),pending.end(),call.args),"Postlude original counter arguments");}
        unsigned opening_clips=0;std::vector<unsigned> opening_endpoints;
        if(video)for(unsigned i=0;i<count;++i){const int base=y+int(i)*3+12;
            if(first!=0x8ad0&&base+int(std::int16_t(first))>0&&base+int(std::int16_t(first))<600){++opening_clips;opening_endpoints.push_back(base+int(std::int16_t(first)));}
            if(prior!=0x8ad0&&base>0&&base<600){++opening_clips;opening_endpoints.push_back(base);}
        }
        // Each opening clip has a non-bottom endpoint; auxiliary/restoration
        // calls with endpoint600 are kept separately, never mistaken for it.
        unsigned actual_openings=0;
        for(const auto& call:state_calls)if(call.tag==0x57dc60&&call.args[1]!=600){
            state_check(actual_openings<opening_endpoints.size()&&call.args[1]==opening_endpoints[actual_openings],"Opening clip endpoint/order");
            state_check(call.return_pc==0x500337||call.return_pc==0x500388,"Opening clip exact original caller");
            ++actual_openings;state_check(call.edx==3+pitch&&call.args[0]==799+pitch*2,"Opening clip pitch/address arguments");}
        state_check(actual_openings==opening_clips,"Strict clip opening boundary policy");
        unsigned expected_waves=kind==15?count:0;
        unsigned expected_restores=0,expected_auxiliary_clips=0;std::vector<unsigned> auxiliary_tops;
        if(video)for(unsigned i=0;i<count;++i){const int endpoint=y+int(i)*3+12+int(std::int16_t(first));
            const bool auxiliary=first!=0x8ad0&&endpoint<int(height_flag?600:480)&&endpoint<600;
            if(kind==15&&auxiliary)++expected_waves;
            if(auxiliary){++expected_auxiliary_clips;auxiliary_tops.push_back(std::max(endpoint,0));}
            if(first!=0x8ad0||prior!=0x8ad0)++expected_restores;
        }
        const auto waves=std::count_if(state_calls.begin(),state_calls.end(),[](const auto& c){return c.tag==0x5806f0;});
        state_check(waves==int(expected_waves),"Auxiliary dispatch strict480/600 boundary policy");
        unsigned restores=0,auxiliary_clips=0;
        for(const auto& call:state_calls)if(call.tag==0x57dc60&&call.args[1]==600){
            state_check(call.edx==3&&call.args[0]==799+pitch*2,"Auxiliary/restored clip arguments");
            if(call.return_pc==0x500d25){++restores;state_check(call.ecx==0,"Restored clip top");}
            else {state_check(call.return_pc==0x5008b9,"Auxiliary clip exact caller");
                state_check(auxiliary_clips<auxiliary_tops.size()&&call.ecx==auxiliary_tops[auxiliary_clips],"Auxiliary clip signed/clamped top/order");++auxiliary_clips;}
        }
        state_check(restores==expected_restores&&auxiliary_clips==expected_auxiliary_clips,
                    "Auxiliary/full clip restoration count and exact caller");
        state_check(Bytes(consumer,consumer+before.size())==before,"Original consumer instructions changed");
        std::cout<<"{\"video\":"<<video<<",\"count\":"<<count<<",\"postlude\":"<<post<<",\"kind\":"<<kind
                 <<",\"y\":"<<y<<",\"first\":"<<first<<",\"prior\":"<<prior<<",\"height_flag\":"<<height_flag
                 <<",\"result\":"<<value<<",\"opening_clips\":"<<actual_openings<<",\"rows_after\":[";
        for(unsigned i=0;i<rows.size();++i){if(i)std::cout<<',';std::cout<<(i%9==1?0:rows[i]);}
        std::cout<<"],\"postlude_after\":[";
        for(unsigned i=0;i<3;++i){if(i)std::cout<<',';std::cout<<state_global(0x6e8db0+i*4);}
        std::cout<<"],\"calls\":[";
        for(unsigned i=0;i<state_calls.size();++i){if(i)std::cout<<',';const auto& c=state_calls[i];
            std::cout<<"{\"tag\":"<<c.tag<<",\"ecx\":"<<(c.ecx==state_pointer(object)?0:c.ecx)
                     <<",\"edx\":"<<c.edx<<",\"return_pc\":"<<c.return_pc<<",\"args\":[";
            for(unsigned k=0;k<6;++k){if(k)std::cout<<',';std::cout<<(c.args[k]==state_pointer(object)?0:c.args[k]);}
            std::cout<<"]}";
        }std::cout<<"]}\n";++cases;
    }
    state_check(cases==58320,"Case count");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
