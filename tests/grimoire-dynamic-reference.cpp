// Private pinned original descriptor parser. No game process or input is patched.
#define main sprite_reference_unused_main
#define u32 sprite_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "grimoire_dynamic_bytes.h"
#include "../reconstruction/grimoire/dynamic_field.hpp"
#include <asm/ldt.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <cstdlib>
#include <set>
using u32=unsigned;using u8=unsigned char;
namespace g=mnm::reconstruction::grimoire;
extern "C" {u32 dynamic_saved_esp=0,dynamic_result=0,dynamic_abi_failure=0;u32 invoke_dynamic(u32,void*,void*);}
static void require(bool ok,const std::string& why){if(!ok)throw std::runtime_error(why);}
static u32 get(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(u32 p,u32 n){std::memcpy(reinterpret_cast<void*>(p),&n,4);}
static std::set<void*> allocations;
static void* allocate(u32 n){require(n<=4096,"bounded original fixture allocation");auto* p=std::calloc(1,n);require(p,"fixture allocation");allocations.insert(p);return p;}
static void release(void* p){require(allocations.erase(p)==1,"original allocation released once");std::free(p);}
static int white(int n){return n==' '||n=='\t'||n=='\r'||n=='\n'||n=='\v'||n=='\f';}
static int digit(int n){return n>='0'&&n<='9';}
static int integer(const char* p){return std::atoi(p);}
static int originalCalls,nativeCalls,lastOpponent,nativeOpponent,originalKind,nativeKind,originalCreature,nativeCreature;static int backendValue;
static int __attribute__((thiscall)) ranged(void* receiver,u32 n){originalCreature=int((reinterpret_cast<std::uintptr_t>(receiver)-0x6a5f80)/0x5c9);originalKind=int(g::Property::ranged);++originalCalls;lastOpponent=int(n);return backendValue;}
static int __attribute__((thiscall)) combat(void* receiver,u32 n){originalCreature=int((reinterpret_cast<std::uintptr_t>(receiver)-0x6a5f80)/0x5c9);originalKind=int(g::Property::combat);++originalCalls;lastOpponent=int(n);return backendValue;}
static std::array<u32,9> traits={0x6a5f98,0x6a5fa8,0x6a5f94,0x6a6531,0x6a5fb8,0x6a5fb4,0x6a653d,0x6a6541,0x6a6545};
struct FsScope {
    unsigned short previous=0;unsigned entry=0;std::array<u32,8> exceptionFrame{};
    explicit FsScope(){asm volatile("mov %%fs,%0":"=r"(previous));user_desc d{};d.entry_number=unsigned(-1);d.base_addr=u32(reinterpret_cast<std::uintptr_t>(exceptionFrame.data()));d.limit=sizeof(exceptionFrame)-1;d.seg_32bit=1;d.useable=1;require(syscall(SYS_set_thread_area,&d)==0,"private FS descriptor");entry=d.entry_number;unsigned short selector=static_cast<unsigned short>((d.entry_number<<3)|3);asm volatile("mov %0,%%fs"::"r"(selector));}
    ~FsScope(){asm volatile("mov %0,%%fs"::"r"(previous));user_desc d{};d.entry_number=entry;d.read_exec_only=1;d.seg_not_present=1;(void)syscall(SYS_set_thread_area,&d);}
};
int main(int argc,char** argv)try{
    require(argc==2,"pinned executable required");map_image(read(argv[1]));
    for(const auto& patch:dependencies){require(!std::memcmp(reinterpret_cast<void*>(patch.address),patch.bytes,8),"dependency prefix");void* target=nullptr;
        switch(patch.address){case 0x597850:target=reinterpret_cast<void*>(&allocate);break;case 0x597870:target=reinterpret_cast<void*>(&release);break;case 0x59c73c:target=reinterpret_cast<void*>(&white);break;case 0x59c6e7:target=reinterpret_cast<void*>(&digit);break;case 0x59c5a4:target=reinterpret_cast<void*>(&integer);break;case 0x505bf0:target=reinterpret_cast<void*>(&ranged);break;case 0x505c50:target=reinterpret_cast<void*>(&combat);break;default:throw std::runtime_error("unreviewed dependency");}
        *reinterpret_cast<u8*>(patch.address)=0xe9;put(patch.address+1,u32(reinterpret_cast<std::uintptr_t>(target))-patch.address-5);
    }
    require(!std::memcmp(reinterpret_cast<void*>(0x4c8950),parser_bytes,8)&&!std::memcmp(reinterpret_cast<void*>(0x4dd970),constructor_bytes,8),"unchanged original parser and constructor");
    const std::array<const char*,13> properties={"LRC","HTH","HEALTH","MAGIC_RESIST","INTELLIGENCE","WEIGHT","VIEW_RANGE","FIRE_RANGE","GROUND_SPEED","FLYING_SPEED","BRAVERY","SPELL_RANGE","MANA_COST"};
    unsigned cases=0,omitted=0,guards=0;
    std::array<u8,0x38> defaults{};using Constructor=u32(__attribute__((thiscall))*)(void*);reinterpret_cast<Constructor>(0x4dd970)(defaults.data());g::Bar nativeDefault{};
    require(get(defaults.data()+0x1c)==u32(nativeDefault.minimum)&&get(defaults.data()+0x20)==u32(nativeDefault.maximum)&&get(defaults.data()+0x24)==u32(nativeDefault.value)&&get(defaults.data()+0x28)==u32(nativeDefault.value),"original constructor defaults");
    for(unsigned mode:{0u,1u,7u})for(unsigned id:{0u,10u,45u})for(int pattern:{0,1,2,3})for(const auto* key:properties){
        g::Sources env;for(unsigned i=0;i<env.maxima.size();++i)env.maxima[i]=20+int((i*37+unsigned(pattern)*13)%101);if(pattern==3){std::swap(env.maxima[0],env.maxima[1]);std::swap(env.maxima[6],env.maxima[7]);}env.creatures.resize(64);env.spells.resize(93);env.highResolution=mode!=0;env.originX=pattern==2?-8:13;env.originY=17;env.font=0x12345678;
        const int value=pattern==0?0:pattern==1?31:pattern==2?251:107;auto& c=env.creatures[id];c={value,value+1,value+2,value+3,value+4,value+5,value+6,value,value+8};env.spells[id]={value+17,value+23};backendValue=value;
        for(unsigned i=0;i<11;++i){put(0x6e981c+4*i,u32(env.maxima[i]));}
        put(0x6f56a4,u32(env.maxima[11]));put(0x6f56a8,u32(env.maxima[12]));put(0x6de6d5,mode);put(0x6e2058,u32(env.originX));put(0x6e205c,u32(env.originY));
        const std::array<int,9> values={c.health,c.resistance,c.intelligence,c.weight,c.sight,c.fireRange,c.groundSpeed,c.flyingSpeed,c.bravery};for(unsigned i=0;i<traits.size();++i)put(traits[i]+id*0x5c9,u32(values[i]));put(0x6b1268+id*0x2d1,u32(env.spells[id].range));put(0x6b1270+id*0x2d1,u32(env.spells[id].manaCost));
        auto text=std::string(pattern==3?"d \"Resist Magic \" 123 163x9 92-56-69 177-133-150 ":"D \"Resist Magic \" 123x17 163x9 92-56-69 177-133-150 ")+key+" "+std::to_string(id)+(std::string(key)=="LRC"||std::string(key)=="HTH"?"v14":"")+" \r\nNext";
        std::vector<u8> helper(0x960);put(u32(reinterpret_cast<std::uintptr_t>(helper.data()+0x850)),env.font);auto* cursor=text.data();u32 pointer=0;originalCalls=nativeCalls=0;lastOpponent=nativeOpponent=originalKind=nativeKind=originalCreature=nativeCreature=-1;
        {FsScope fs;pointer=invoke_dynamic(0x4c8950,helper.data(),&cursor);require(!dynamic_abi_failure&&fs.exceptionFrame[0]==0,"original ABI/SEH restoration");}
        env.combatValue=[](g::Property property,unsigned creature,unsigned n){nativeCreature=int(creature);nativeKind=int(property);++nativeCalls;nativeOpponent=int(n);return backendValue;};auto result=g::decodeDynamic(text,env);
        require(bool(pointer)==bool(result.bar),std::string(key)+" omission");require(result.cursor==std::size_t(cursor-text.data()),std::string(key)+" input cursor");require(originalCalls==nativeCalls&&lastOpponent==nativeOpponent&&originalKind==nativeKind&&originalCreature==nativeCreature,"combat backend dispatch");
        if(pointer){const auto& b=*result.bar;auto* p=reinterpret_cast<u8*>(pointer);require(std::string(reinterpret_cast<char*>(get(p+0x2c)))==b.label,std::string(key)+" label");require(get(p+4)==b.font&&get(p+0x10)==u32(b.width)&&get(p+0x14)==u32(b.height)&&get(p+0x1c)==u32(b.minimum)&&get(p+0x20)==u32(b.maximum)&&get(p+0x24)==u32(b.value)&&get(p+0x28)==u32(b.value)&&get(p+0x30)==u32(b.x)&&get(p+0x34)==u32(b.y),std::string(key)+" descriptor fields");release(reinterpret_cast<void*>(get(p+0x2c)));release(p);}else ++omitted;
        require(allocations.empty(),"original omission frees both allocations");++cases;
    }
    std::vector<u8> unknownHelper(0x960);std::string unknown="D \"Unrecognised\" 1x2 3x4 0-0-0 0-0-0 UNKNOWN 0 \n";char* unknownCursor=unknown.data();u32 unknownNode=0;
    {FsScope fs;unknownNode=invoke_dynamic(0x4c8950,unknownHelper.data(),&unknownCursor);require(!dynamic_abi_failure,"unknown original ABI");}
    require(unknownNode&&get(reinterpret_cast<u8*>(unknownNode)+0x20)==400&&get(reinterpret_cast<u8*>(unknownNode)+0x24)==200,"legacy unknown property retains default bar");release(reinterpret_cast<void*>(get(reinterpret_cast<u8*>(unknownNode)+0x2c)));release(reinterpret_cast<void*>(unknownNode));require(allocations.empty(),"unknown allocations released");
    g::Sources env;env.creatures.resize(64);env.spells.resize(93);const auto prefix=std::string("D \"Health\" 120x10 160x5 92-56-69 177-133-150 ");
    for(const auto& text:{prefix+"HEALTH 64 \n",prefix+"MANA_COST 93 \n",prefix+"UNKNOWN 0 \n",prefix+"HEALTH 1",prefix+"HEALTH 9999999999 \n",std::string("D \"")+std::string(129,'x')+"\" 1 1x1 0-0-0 0-0-0 HEALTH 0 \n",std::string(4097,'x'),prefix+"HEALTH 0 \n"+std::string(1,'\0'),std::string("X \"Bad\" 1 1x1 0-0-0 0-0-0 HEALTH 0 \n"),prefix+"LRC 0v14 \n"}){
        bool refused=false;try{(void)g::decodeDynamic(text,env);}catch(const std::invalid_argument&){refused=true;}require(refused,"intentional native bounds");++guards;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"omitted\":"<<omitted<<",\"constructor_cases\":1,\"legacy_unknown_cases\":1,\"native_guards\":"<<guards<<",\"allocator_crt_and_combat_backends_stubbed\":true,\"live_validated\":false}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
