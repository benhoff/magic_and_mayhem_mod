#define main sprite_reference_unused_main
#define u32 sprite_original_word
#include "sprite-binary-reference.cpp"
#undef main
#undef u32
#include <cstdio>
using u32=std::uint32_t;using u8=unsigned char;
#ifdef MNM_SPRITE_CALLER_HEADER
#include MNM_SPRITE_CALLER_HEADER
#else
#include "../runtime/scene/world_raster_callers.h"
#endif
#ifndef MNM_EXPECT_CALLERS
#define MNM_EXPECT_CALLERS 43
#define MNM_EXPECT_PAIRS 50
#endif
namespace {
struct Span{u32 start,bytes;};std::vector<Span> spans;u32 imageBytes;
struct Limit{u32 start=0,bytes=0,visible=0;} limit;
static u32 get(const void* p){u32 value;std::memcpy(&value,p,4);return value;}
static int readable(u32 p,u32 n){
    if(!p||!n||p+n<p)return 0;
    if(limit.bytes && p<limit.start+limit.bytes && limit.start<p+n)
        return p>=limit.start&&n<=limit.visible&&p-limit.start<=limit.visible-n;
    if(p>=0x400000&&n<=imageBytes&&p-0x400000<=imageBytes-n)return 1;
    for(const auto& s:spans)if(p>=s.start&&n<=s.bytes&&p-s.start<=s.bytes-n)return 1;
    return 0;
}
#include "../runtime/scene/kind8_sprite_admission.h"
static u32 batch_guard_open=1,batch_owner=123,current_thread=123;
static u32 GetCurrentThreadId(){return current_thread;}
#include "sprite-batch-caller.inc"
#include "../reconstruction/rendering/kind8_class.h"
#include "sprite-observer-caller.inc"
u32 address(const void* p){return u32(reinterpret_cast<std::uintptr_t>(p));}
void set(u32 p,u32 value){std::memcpy(reinterpret_cast<void*>(p),&value,4);}
unsigned controls=0,protectedShort=0;
void check(bool value){++controls;if(!value)throw std::runtime_error("Sprite admission control "+std::to_string(controls)+" failed");}
}
int main(int argc,char** argv)try{
    if(argc!=2)throw std::invalid_argument("Expected pinned original image");
    const auto image=read(argv[1]);map_image(image);imageBytes=sprite_original_word(image,sprite_original_word(image,60)+24+56);
    auto* pages=static_cast<u8*>(mmap(nullptr,8192,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    check(pages!=MAP_FAILED);check(mprotect(pages+4096,4096,PROT_NONE)==0);
    u32 object[8]{},owner[17]{},bank[4]{},frame[8]{},row[9]{},stack[10]{};
    const auto objectAddress=address(object),ownerAddress=address(owner),bankAddress=address(bank),frameAddress=address(frame),rowAddress=address(row),stackAddress=address(stack);
    auto reset=[&](const SpriteRasterSite& site){
        frame[7]=site.parent_word==7?0xffffffffu:0;
        limit={};std::fill(std::begin(object),std::end(object),0);std::fill(std::begin(owner),std::end(owner),0);std::fill(std::begin(row),std::end(row),0);std::fill(std::begin(stack),std::end(stack),0);
        const auto type=mnm_kind8_sprite_class(site.table,site.method);object[0]=site.table;object[type.owner_offset/4]=ownerAddress;owner[9]=0x80000000u;bank[3]=1;
        row[1]=objectAddress;row[6]=8;row[7]=0x8ad08ad0;stack[0]=site.address;stack[1]=frameAddress;stack[site.parent_word]=0x5006b2;stack[site.parent_word-2]=rowAddress;
        set(0x5e1404,1);set(0x6e1f88,0);set(0x644520,0);set(site.bank,bankAddress);
        spans={{objectAddress,sizeof(object)},{ownerAddress,sizeof(owner)},{bankAddress,sizeof(bank)},{frameAddress,sizeof(frame)},{rowAddress,sizeof(row)},{stackAddress,sizeof(stack)},{address(pages),4096}};
    };
    unsigned pairs=0;
    for(const auto& site:sprite_raster_sites){
        reset(site);
        for(unsigned tag=0;tag<64;++tag)check(bool(raster_batch_caller(site.address,tag,stack))==bool(site.tags&(1ull<<tag)));
        check(!raster_batch_caller(site.address,64,stack));check(!raster_batch_caller(site.address,UINT32_MAX,stack));
        for(unsigned tag=0;tag<64;++tag){
            if(!(site.tags&(1ull<<tag)))continue;
            ++pairs;reset(site);const auto type=mnm_kind8_sprite_class(site.table,site.method);const auto n=(site.parent_word+1)*4;
            auto valid=[&]{return raster_batch_caller(site.address,tag,stack)!=0;};check(valid());check(kind8_supported(rowAddress));
            frame[7]^=0xffffffffu;check(!valid());frame[7]^=0xffffffffu;check(valid());
            check(!raster_caller(site.address,tag,stack));batch_guard_open=0;check(!valid());batch_guard_open=1;check(valid());
            current_thread=124;check(!valid());current_thread=123;check(valid());
            for(unsigned player=0;player<8;++player){std::fill(std::begin(owner),std::end(owner),0);owner[9+player]=1;set(0x644520,player);check(valid());owner[9+player]=0;owner[9+(player+1)%8]=1;check(!valid());}
            reset(site);
            for(auto range:std::initializer_list<Span>{{site.address,10},{site.method,9},{0x5006a2,16},{0x5006b2,10}})
                for(u32 i=0;i<range.bytes;++i){auto* p=reinterpret_cast<u8*>(range.start+i);const auto saved=*p;*p^=1;check(!valid());*p=saved;}
            for(auto range:std::initializer_list<Span>{{site.table,16},{objectAddress,type.root_bytes},{ownerAddress,68},{bankAddress,16},{frameAddress,32},{rowAddress,36},{stackAddress,n}}){
                for(u32 visible=0;visible<range.bytes;++visible){limit={range.start,range.bytes,visible};check(!valid());}limit={};check(valid());
            }
            for(u32 outer:{0u,1u,0x5006b3u,0x5006e0u,0x500f51u}){stack[site.parent_word]=outer;check(!valid());}stack[site.parent_word]=0x5006b2;
            for(u32 mode:{1u,2u,0xffffffffu}){set(0x5e1404,mode);check(valid());}set(0x5e1404,0);check(!valid());set(0x5e1404,1);
            for(u32 format:{1u,2u,0xffffffffu}){set(0x6e1f88,format);check(!valid());}set(0x6e1f88,0);
            for(u32 player:{8u,0xffffffffu}){set(0x644520,player);check(!valid());}set(0x644520,0);
            for(u32 count:{0u,4097u,0x80000000u,0xffffffffu}){bank[3]=count;check(!valid());}
            for(u32 count:{1u,8u,4096u}){bank[3]=count;check(valid());}bank[3]=1;
            row[6]=9;check(!valid());row[6]=8;row[7]=0;check(!valid());row[7]=0x8ad08ad0;
            for(u32 bad:{0u,1u,0xffffffffu}){
                row[1]=bad;check(!valid());row[1]=objectAddress;
                stack[site.parent_word-2]=bad;check(!valid());stack[site.parent_word-2]=rowAddress;
                stack[1]=bad;check(!valid());stack[1]=frameAddress;
                object[type.owner_offset/4]=bad;check(!valid());object[type.owner_offset/4]=ownerAddress;
                set(site.bank,bad);check(!valid());set(site.bank,bankAddress);
            }
            for(u32 table:{0u,1u,0x5c7420u,site.table^1u}){object[0]=table;check(!valid());check(!kind8_supported(rowAddress));}object[0]=site.table;
            const auto target=get(reinterpret_cast<void*>(site.table+12));set(site.table+12,target^1);check(!valid());set(site.table+12,target);
            check(!raster_batch_caller(site.address^1,tag,stack));check(!raster_batch_caller(site.address,tag,nullptr));
            for(u32 visible=0;visible<n;++visible){const auto* shortStack=reinterpret_cast<const u32*>(pages+4096-visible);check(!raster_batch_caller(site.address,tag,shortStack));++protectedShort;}
            for(u32 visible=0;visible<type.root_bytes;++visible){auto* shortObject=pages+4096-visible;if(visible)std::memset(shortObject,0,visible);if(visible>=4)std::memcpy(shortObject,&site.table,4);row[1]=address(shortObject);check(!valid());check(!kind8_supported(rowAddress));++protectedShort;}row[1]=objectAddress;
            check(valid());
        }
    }
    unsigned ordinaryPairs=0;for(const auto& site:raster_callers)for(unsigned tag=0;tag<64;++tag)ordinaryPairs+=bool(site.tags&(1ull<<tag));
    check(std::size(raster_callers)==MNM_EXPECT_CALLERS&&ordinaryPairs==MNM_EXPECT_PAIRS);
    batch_guard_open=0;for(const auto& site:raster_callers)for(unsigned tag=0;tag<64;++tag)if(site.tags&(1ull<<tag))check(raster_batch_caller(site.address,tag,nullptr));batch_guard_open=1;check(std::size(sprite_raster_sites)==6&&pairs==10);
    check(munmap(pages,8192)==0);
    std::printf("{\"success\":true,\"context_pairs\":%u,\"controls\":%u,\"protected_short_spans\":%u,\"ordinary_callers\":%u,\"ordinary_pairs\":%u,\"legacy_sprite_transport_refused\":true}\n",pairs,controls,protectedShort,unsigned(std::size(raster_callers)),ordinaryPairs);
    return 0;
}catch(const std::exception& error){std::fprintf(stderr,"%s\n",error.what());return 1;}
