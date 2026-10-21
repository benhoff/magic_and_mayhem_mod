#include "../reconstruction/rendering/world_cell_producer.hpp"
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "../runtime/world-cell-producer/original_bytes.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <csignal>
#include <ucontext.h>

using U=unsigned;
static constexpr U arena=0x20000000,scene=arena,map=arena+0x1000,mapMeta=arena+0x7000,
    actors=arena+0x20000,effects=arena+0x30000,artifacts=arena+0x34000,
    definitions=arena+0x38000,lights=arena+0x40000,queue=arena+0x48000,
    collection=arena+0x50000,header=arena+0x50040,offsets=arena+0x50100,
    sprites=arena+0x51000,reference=arena+0x55000,asset=arena+0x57000,palettes=arena+0x58000;
extern "C" int mnm_world_cell_install();
static U protectCalls,protectFailure,queryFailure,lastError=0x76543210,running;
extern "C" U __attribute__((stdcall)) GetLastError(){return lastError;}
extern "C" void __attribute__((stdcall)) SetLastError(U v){lastError=v;}
extern "C" U __attribute__((stdcall)) VirtualQuery(const void* p,void* out,U n){
    if(U(p)==queryFailure)return 0;
    U* v=static_cast<U*>(out);for(U k=0;k<7;++k)v[k]=0;
    v[0]=U(p)&~4095u;v[3]=4096;v[4]=0x1000;v[5]=4;return n;
}
extern "C" int __attribute__((stdcall)) VirtualProtect(void*,U,U,U* old){*old=0x20;return ++protectCalls!=protectFailure;}
extern "C" void* __attribute__((stdcall)) GetCurrentProcess(){return reinterpret_cast<void*>(~0u);}
extern "C" int __attribute__((stdcall)) FlushInstructionCache(void*,const void*,U){return 1;}
extern "C" [[noreturn]] void __attribute__((stdcall)) ExitProcess(U){_exit(83);}
static std::vector<unsigned char> captured;
static void emit(const void* p,U size){const auto* bytes=static_cast<const unsigned char*>(p);captured.insert(captured.end(),bytes,bytes+size);}
static void word(U v){emit(&v,4);}
static void failed(int signal,siginfo_t* info,void* context){
    auto* cpu=static_cast<ucontext_t*>(context);char text[240];
    const int n=std::snprintf(text,sizeof(text),"world cell signal=%d case=%u address=%p eip=%lx ecx=%lx edx=%lx\n",signal,running,info->si_addr,
        static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_EIP]),static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_ECX]),static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_EDX]));
    if(n>0)write(2,text,unsigned(n));
    _exit(128+signal);
}
struct Backend {
    U variant{},iterator{},calls{};bool tracing{true};
    U r(U p){U v;std::memcpy(&v,reinterpret_cast<void*>(p),4);return v;}
    U r16(U p){unsigned short v;std::memcpy(&v,reinterpret_cast<void*>(p),2);return v;}
    U r8(U p){return *reinterpret_cast<unsigned char*>(p);}
    void w(U p,U v){std::memcpy(reinterpret_cast<void*>(p),&v,4);}
    void w16(U p,U v){const auto n=static_cast<unsigned short>(v);std::memcpy(reinterpret_cast<void*>(p),&n,2);}
    void w8(U p,U v){*reinterpret_cast<unsigned char*>(p)=static_cast<unsigned char>(v);}
    U scratch(){return arena+0xf0000;}
    [[noreturn]] void fault(const char* text){throw std::runtime_error(text);}
    double real(U p){double d;std::memcpy(&d,reinterpret_cast<void*>(p),8);return d;}
    void globals(){
        for(U p:{0x6e8db0u,0x6c5488u,0x6c5484u,0x6cbb64u,0x6c3750u,0x6dbea4u,0x657d40u,0x656600u,0x6574c8u,0x6def60u})word(r(p));
    }
    void checkpoint(U receiver){
        globals();emit(reinterpret_cast<void*>(scene),0x244);
        if(receiver>=actors && receiver<actors+4*0xe4b)emit(reinterpret_cast<void*>(receiver),0xe4b);
    }
    U frame(U number){return sprites+(number%32)*64;}
    void visual(U out,U ref,int dx=0,int dy=0){w(out,frame(r(ref+4)));w(out+4,r(ref+8)+U(dx));w(out+8,r(ref+12)+U(dy));}
    U service(U entry,U receiver,U count,const U* args,U fast=0){
        if(tracing){
            word(entry);word(receiver);word(count);
            for(U k=0;k<count;++k){
                bool pointer=(entry==0x487c40 || entry==0x507190 || entry==0x507330 || entry==0x520540)
                    || (entry==0x534640 && k>=2) || (entry==0x51faa0 && k==1)
                    || entry==0x507250 || entry==0x5072c0 || (entry==0x505aa0 && k<2)
                    || entry==0x4eadf0;
                word(pointer?0:args[k]);
            }
            if(entry==0x57dcf0 || entry==0x57ddb0)word(fast);
            if(entry==0x4eadf0){word(r(fast));word(r(args[0]));word(r(args[1]));}
            checkpoint(receiver);
        }
        ++calls;
        switch(entry){
        case 0x49bd20:w(receiver+4,0);if(r16(args[0]+2)==(receiver-effects)/0x22e)w16(args[0]+2,r16(receiver+0x1a0));return 0;
        case 0x49a600:return (variant&2) && receiver==effects;
        case 0x49aa30:return variant&4;
        case 0x49a5f0:return r(receiver+0x28)&1;
        case 0x487c40:if(variant&8){w(args[0],0);w(args[0]+4,0);w(args[0]+8,0);}else visual(args[0],r(receiver+0x164));return 0;
        case 0x5344e0:return variant&16;
        case 0x534640:visual(args[2],reference+64);w(args[3],2);return 0;
        case 0x5437f0:return r16(receiver+0x62);
        case 0x52b0b0:iterator=0;w(0x6def60,0);return 0;
        case 0x52b0c0:w(0x6def60,++iterator);return iterator<=2?actors+iterator*0xe4b:0;
        case 0x506220:return r(receiver+0xe44)&1;
        case 0x466870:return variant&32?actors+3*0xe4b:0;
        case 0x508ca0:if(args[1]<8)w(receiver+0x24+args[1]*4,1);return 1;
        case 0x507190:visual(args[0],r(receiver+0xb4));return 0;
        case 0x507330:visual(args[0],r(receiver+0xb4),3,-2);return 0;
        case 0x51fef0:if(variant&8192)w(receiver+0xe4,0);return r(receiver+0xe40);
        case 0x54e200:{const U value=r(receiver)*1664525+1013904223;w(receiver,value);return value;}
        case 0x51faa0:{const U k=args[0],out=args[1];
            if(k==((variant&3)==3?1u:3u)){w(out+4,~0u);return 0;}
            w(out,frame(r(receiver+0x75c+k*12)));w(out+4,r(receiver+0x754+k*12));w(out+8,r(receiver+0x758+k*12));return 0;}
        case 0x507250:case 0x5072c0:{const U ref=r(receiver+0xb4),offset=entry==0x507250?28:36;w(args[0],r(ref+offset));w(args[1],r(ref+offset+4));return 0;}
        case 0x520540:visual(args[0],reference+2*64,8,-8);return 0;
        case 0x505aa0:{const int xs[]={-20,20,20,-20},ys[]={-12,-12,12,12};w(args[0],U(xs[args[3]&3])+args[2]);w(args[1],U(ys[args[3]&3])-args[2]);return 0;}
        case 0x58f7d0:w(receiver,args[0]);w(receiver+4,args[1]);return 0;
        case 0x57dcf0:case 0x57ddb0:return (variant&1) && ((entry==0x57ddb0)==bool(variant&64));
        case 0x4eadf0:w(args[0],r(args[0])+7);w(fast,r(fast)+9);w(args[1],r(args[1])+11);return 0;
        default:fault("Unknown producer service");
        }
    }
    template<class... A>U call(U entry,U receiver,A... args){const U values[]={U(args)...,0};return service(entry,receiver,sizeof...(args),values);}
    U hit(U entry,U sprite,U x,U y,U point){const U args[]={y,point};return service(entry,sprite,2,args,x);}
    void relocate(U actor,U x,U y,U z){const U args[]={y,z};service(0x4eadf0,actor,2,args,x);}
    double distance(int x,int y);
    void state(){
        globals();emit(reinterpret_cast<void*>(scene),0x300);emit(reinterpret_cast<void*>(map),8*10*5*12);
        emit(reinterpret_cast<void*>(actors),4*0xe4b);emit(reinterpret_cast<void*>(effects),3*0x22e);
        emit(reinterpret_cast<void*>(artifacts),3*0x7e);emit(reinterpret_cast<void*>(queue),256*36);
    }
};
static Backend* active;
double Backend::distance(int x,int y){if(tracing){word(0x59d60c);word(U(x));word(U(y));}return std::hypot(double(x),double(y));}
extern "C" __attribute__((force_align_arg_pointer)) double cell_distance(double x,double y){return active->distance(int(x),int(y));}
extern "C" __attribute__((force_align_arg_pointer)) U cell_service_dispatch(U* regs,U entry,U count){
    try{return active->service(entry,regs[6],count,regs+9,regs[5]);}catch(...){_exit(80);}
}
static void patch(U address,void* target){const U relative=U(target)-address-5;*reinterpret_cast<unsigned char*>(address)=0xe9;std::memcpy(reinterpret_cast<void*>(address+1),&relative,4);}
#define CELL_SERVICE(address,argc) extern "C" void cell_service_##address();
#include "world-cell-services.inc"
#undef CELL_SERVICE
extern "C" void cell_service_57dcf0(),cell_service_57ddb0(),cell_service_4eadf0();

static void seed(Backend& b,U view,U family,U kind,U variant,U position){
    b.variant=variant;b.iterator=b.calls=0;
    std::memset(reinterpret_cast<void*>(arena),0,0x100000);std::memset(reinterpret_cast<void*>(0x640000),0,0xc0000);
    std::memset(reinterpret_cast<void*>(queue),0xa5,256*36);
    const U col=position%3==0?0:position%3==1?7:3,row=position/3==0?0:position/3==1?9:4,level=2;
    const U tile=map+(level*80+row*8+col)*12;
    for(U n=0;n<32;++n){b.w(0x6cb942+n*4,n*8);b.w(0x6cb8c2+n*4,n*80);}
    for(U n=0;n<500;++n)b.w(0x6e818c+n*4,n*82/100);
    b.w(0x5e41ac,1);b.w(0x5e41b0,0);b.w(0x5e1404,1);b.w(0x5e18e8,U(-40));b.w(0x5e18f8,U(-70));b.w(0x5e18f4,U(-20));
    b.w(0x644520,(variant&128)?~0u:0);b.w(0x6574c8,42);b.w(0x6c5494,8);b.w(0x6c5498,10);b.w(0x6c54a0,80);
    b.w(0x6c54dc,map);b.w(0x65660c,definitions);b.w(0x6c5c5c,lights);b.w(0x6c5c80,3);
    std::memset(reinterpret_cast<void*>(lights),static_cast<unsigned char>((variant&8)?-100:-10),4096);
    b.w(0x689498,effects);b.w(0x68949c,3);b.w(0x6b1248,artifacts);b.w(0x6b124c,3);b.w(0x6def58,actors);b.w(0x6def5c,4);
    b.w(scene+0x21,mapMeta);b.w(mapMeta+0x10,80);b.w(mapMeta+0x4c,map);b.w(mapMeta+0x7f0,2);
    b.w(scene+0x2d,collection);b.w(scene+0x31,view);b.w(scene+0x61,1);b.w(scene+0x65,3);b.w(scene+0x6d,1);b.w(scene+0x75,1);
    b.w(scene+0x79,tile);b.w(scene+0x7d,col);b.w(scene+0x81,row);b.w(scene+0x85,level);b.w(scene+0x89,200);b.w(scene+0x8d,150);
    b.w(scene+0x91,213);b.w(scene+0x95,157);b.w(scene+0x9d,100);b.w(scene+0xb1,2);b.w(scene+0xb5,500);b.w(scene+0xb9,500);b.w(scene+0xbd,variant&4);
    b.w(scene+0x1ed,0x80000000);b.w(scene+0x229,queue);b.w(scene+0x22d,queue);b.w(scene+0x235,256);b.w(scene+0x23d,view);
    b.w16(tile+2,0xffff);b.w16(tile+4,0xffff);b.w16(tile+6,0xffff);
    b.w(collection,header);b.w(header+12,31);b.w(collection+8,offsets);b.w(collection+12,sprites);
    b.w(0x6593e0,header);b.w(0x6593e8,offsets);b.w(0x6593ec,sprites);
    for(U n=0;n<32;++n){b.w(offsets+n*4,n*64);b.w(sprites+n*64+4,64);b.w(sprites+n*64+8,40);b.w(sprites+n*64+12,4);b.w(sprites+n*64+16,2);}
    for(U n=0;n<16;++n){const U ref=reference+n*64;b.w(ref+4,n+1);b.w(ref+8,5+n);b.w(ref+12,U(-3)-n);b.w(ref+28,20);b.w(ref+32,U(-7));b.w(ref+36,11);b.w(ref+40,U(-15));}
    b.w(asset+0x110,collection);
    for(U n=0;n<4;++n){b.w(definitions+n*356+0x94,2);b.w8(definitions+n*356+0xb0,8);}
    for(U n=0;n<4;++n){b.w(definitions+0x84+n*4,1);b.w(definitions+0xd0+n*4,2);b.w(definitions+0xe0+n*4,3);}
    b.w(definitions+0x1e8,4);
    for(U n=0;n<3;++n){const U p=effects+n*0x22e;b.w(p+4,1);b.w(p+8,col);b.w(p+12,row);b.w(p+16,level);b.w(p+20,col*32+(variant&4?0:13));b.w(p+24,row*32+(variant&4?0:19));b.w(p+28,level*16+3);b.w(p+0x28,kind);b.w(p+0x164,reference+(n+1)*64);b.w(p+0x17c,asset);b.w16(p+0x1a0,n==2?0xffff:n+1);b.w(p+0x21e,n%2?9:2);b.w(p+0x22a,!(variant&256));}
    for(U n=0;n<3;++n){const U p=artifacts+n*0x7e;b.w(p+8,col);b.w(p+12,row);b.w(p+16,level);b.w(p+20,col*32+9);b.w(p+24,row*32+5);b.w(p+28,level*16+3);b.w(p+0x32,reference+(n+4)*64);b.w(p+0x4a,asset);b.w(p+0x5e,31);b.w16(p+0x62,n==2?0xffff:n+1);}
    for(U n=0;n<4;++n){const U a=actors+n*0xe4b,meta=palettes+n*1024;
        b.w(a+4,1);b.w(a+8,col);b.w(a+12,row);b.w(a+16,level);b.w(a+20,col*32+13);b.w(a+24,row*32+19);b.w(a+28,level*16+3);
        b.w(a+0x24,1);b.w(a+0xa8,(variant&16)?17:(variant&32)?13:1);b.w(a+0xac,definitions+3*356);b.w(a+0xb4,reference);b.w(a+0xcc,asset);
        b.w(a+0xe4,!(variant&512));b.w(a+0x174,0);b.w(a+0x5ec,1);b.w(a+0x5e8,0);b.w(a+0x570,52);b.w(a+0x5fc,(variant&1024)?10:0);
        b.w(a+0x608,kind&7);b.w(a+0x618,1);b.w(a+0x977,0);b.w(a+0x97f,col);b.w(a+0x983,row);b.w(a+0x987,level);
        b.w(a+0x85d,reference+3*64);b.w(a+0x875,asset);b.w(a+0x891,reference+4*64);b.w(a+0x8a9,asset);
        b.w(a+0x130,reference+2*64);b.w(a+0x148,asset);b.w(a+0x7d9,reference+64);b.w(a+0x7f1,asset);
        b.w(a+0xcf7,meta);b.w16(a+0xcfb,3);b.w(a+0xcff,(variant&4)?55:0);b.w(a+0xbf3,kind%30);b.w(a+0xddf,variant&8?U(-20):0);b.w(a+0xe40,kind%3==2?3:kind%3);
        for(U k=0;k<30;++k){b.w(meta+k*4,31);b.w(meta+120+k*4,(variant&2)?~0u:2);b.w(meta+240+k*4,(variant&4)?1:~0u);b.w(meta+360+k*4,(variant&8)?~0u:1);b.w(meta+600+k*4,k%5);}
        if(variant&2048)for(U off:{0x124u,0x744u,0x778u,0x780u,0x78cu,0x798u,0x821u,0x855u,0x889u,0x8bdu,0x8d1u,0x8d5u,0x963u})b.w(a+off,1);
        if(variant&2048){b.w8(a+0x962,1);b.w8(a+0x7cc,1);b.w(a+0x68,1);b.w(a+0x70,1);b.w(a+0x6e4,1);b.w(a+0x8e9,3);}
    }
    for(U n=0;n<8;++n){b.w(0x6df000+n*48,reference+(n+1)*64);b.w(0x6df018+n*48,asset);}
    b.w(0x6a49cc,family==3 || family==5?1:0);
    if(family==1 || family==5)b.w16(tile+2,0);
    if(family==2 || family==5)b.w16(tile+6,0);
    if(family>=4){b.w16(tile+4,0);b.w16(tile+10,1);if(variant&4096)b.w16(tile+10,0x2001);}
    if(family==0){b.w16(tile+8,(variant&2)?0x8004:0);b.w16(tile+10,(variant&4)?8:0);}
}

int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected pinned image and mode");
    const std::string mode=argv[2];
    struct sigaction action{};action.sa_sigaction=failed;action.sa_flags=SA_SIGINFO;sigaction(SIGSEGV,&action,nullptr);sigaction(SIGILL,&action,nullptr);sigaction(SIGTRAP,&action,nullptr);
    map_image(read(argv[1]));
    if(mmap(reinterpret_cast<void*>(arena),0x100000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Owned world arena");
    if(mode=="guards"){
        U mutations=0;
        for(U k=0;k<sizeof(mnm_world_cell_original);++k){auto* p=reinterpret_cast<unsigned char*>(0x4f8960+k);*p^=1;if(mnm_world_cell_install())throw std::runtime_error("Admitted changed producer guard");*p^=1;++mutations;}
        protectCalls=0;protectFailure=1;if(mnm_world_cell_install())throw std::runtime_error("Admitted protection refusal");protectFailure=0;
        if(std::memcmp(reinterpret_cast<void*>(0x4f8960),mnm_world_cell_original,sizeof(mnm_world_cell_original)))throw std::runtime_error("Partial producer install");
        for(U p:{0x4f8960u,0x4f9000u,0x4fa000u,0x4fb000u}){queryFailure=p;if(mnm_world_cell_install())throw std::runtime_error("Admitted guard ownership failure");}
        queryFailure=0;if(!mnm_world_cell_install())throw std::runtime_error("Producer install failed");
        std::cout<<"{\"success\":true,\"body_and_table_mutations\":"<<mutations<<",\"protection_refusals\":1,\"ownership_refusals\":4}\n";return 0;
    }
    if(mode=="runtime" && !mnm_world_cell_install())throw std::runtime_error("Producer install");
#define CELL_SERVICE(address,argc) patch(0x##address,reinterpret_cast<void*>(&cell_service_##address));
#include "world-cell-services.inc"
#undef CELL_SERVICE
    patch(0x57dcf0,reinterpret_cast<void*>(&cell_service_57dcf0));patch(0x57ddb0,reinterpret_cast<void*>(&cell_service_57ddb0));patch(0x4eadf0,reinterpret_cast<void*>(&cell_service_4eadf0));patch(0x59d60c,reinterpret_cast<void*>(&cell_distance));
    if(mode!="original"){
        const U first=mode=="runtime"?8:0;std::memset(reinterpret_cast<void*>(0x4f8960+first),0xcc,0x4fbe00-0x4f8960-first);
        std::memset(reinterpret_cast<void*>(0x4ffd30),0xcc,0x4fff49-0x4ffd30);
    }
    U cases=0;
    auto execute=[&]{
        running=cases;captured.clear();word(cases++);
        if(mode=="native")mnm::reconstruction::WorldCellProducer<Backend>(*active).produce(scene);
        else {using F=void(__attribute__((thiscall)) *)(U);reinterpret_cast<F>(0x4f8960)(scene);}
        active->state();if(lastError!=0x76543210)throw std::runtime_error("LastError changed");
        const U bytes=U(captured.size());std::cout.write(reinterpret_cast<const char*>(&running),4);std::cout.write(reinterpret_cast<const char*>(&bytes),4);
        std::cout.write(reinterpret_cast<const char*>(captured.data()),bytes);
    };
    for(U family=0;family<6;++family)for(U view=0;view<4;++view)for(U kind=0;kind<(family==1?34u:8u);++kind)for(U variant=0;variant<8;++variant){
        Backend b;active=&b;const U flags=variant|((variant&2)?2048:0)|((variant&4)?4096:0)|((variant&1)?64:0)|((kind&1)?16:0)|((kind&2)?32:0)|((kind&4)?128:0);
        seed(b,view,family,kind,flags,variant%9);execute();
    }
    // Cross each family/view with independent edge profiles, including branches
    // which normal drawing combines or bypasses. All inputs are authored here.
    const U masks[]={0,1,3,5,8,16,32,64,65,128,256,512,1024,2048,2049,2081,4096,6144,8192};
    for(U family=0;family<6;++family)for(U view=0;view<4;++view)for(U profile=0;profile<28;++profile){
        Backend b;active=&b;U flags=masks[profile%19];
        if(profile==22)flags=65;
        if(profile==23 || profile==24)flags=0;
        if(profile==26)flags=8192|2048;
        seed(b,view,family,8,flags,(profile+view)%9);const U tile=b.r(scene+0x79);
        switch(profile){
        case 0:b.w(actors+0x987,35);break;
        case 1:b.w(0x5e41ac,view==0?1:view==1?3:view==2?9:U(-3));break;
        case 2:b.w(scene+0x235,0);b.variant&=~1u;b.w(scene+0xbd,0);b.w(scene+0x91,100000);b.w(scene+0x95,100000);break;
        case 3:b.w(scene+0x235,1);break;
        case 4:b.w(scene+0x9d,0x7ffffffd);break;
        case 5:b.w(scene+0x61,20);b.w16(tile+8,0x8004|(8u<<view));b.w16(tile+10,b.r16(tile+10)|0xe40);b.w16(tile+6,0xffff);break;
        case 6:b.w(scene+0x65,1);b.w16(tile+8,3);break;
        case 7:b.w(scene+0x65,1);b.w16(tile+8,3);b.w(actors+0x108,1);break;
        case 8:b.w16(tile+10,0);b.w(actors+0xe44,1);b.w(actors+0x24,0);break;
        case 9:b.variant|=32;b.w(actors+0xe4,0);break;
        case 10:b.variant&=~32u;b.w(actors+0xe4,0);break;
        case 11:b.w(actors+0x174,1);b.w8(0x643bd4+24,1);b.w(actors+0x68,1);break;
        case 12:b.w(definitions+3*356+8,2);break;
        case 13:b.w(actors+0x108,1);b.w8(definitions+0xb0,0);b.w8(definitions+356+0xb0,0);b.w16(tile-80*12,1);b.w16(tile-160*12,2);b.w(definitions+2*356+0x94,6);break;
        case 14:b.w(actors+0x97f,(b.r(actors+8)+7)%8);b.w(actors+0x983,(b.r(actors+12)+9)%10);b.w(actors+0x987,3);break;
        case 15:b.w(effects+8,100);break;
        case 16:b.variant|=2;break;
        case 17:b.w8(definitions+0xa8,0x20);break;
        case 18:b.w(definitions+0xd0+view*4,0);b.w(definitions+0xe0+view*4,0);break;
        case 19:b.w16(tile+4,0xffff);b.w16(tile+10,b.r16(tile+10)|0x2000);break;
        case 20:b.w(0x644520,8);b.w(actors+0xe44,1);break;
        case 21:b.w(artifacts+0x32,0);break;
        case 22:b.w(0x6903e8,3);break;
        case 23:b.w(scene+0xbd,1);b.w(scene+0x91,230);b.w(scene+0x95,149);break;
        case 24:b.w(scene+0x91,237);b.w(scene+0x95,151);break;
        case 25:b.w(actors+0x6e4,1);b.w(actors+0x68,0);b.w(actors+0x70,0);break;
        case 26:b.w(actors+0x798,0);b.w(actors+0xe40,1);break;
        case 27:b.w16(scene+0xa9,0xffff);b.w(0x5e1404,0);b.w(0x6e1f0c,1);break;
        }
        execute();
    }
    std::cerr<<"world cell cases "<<cases<<"\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
