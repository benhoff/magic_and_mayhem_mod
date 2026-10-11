// Execute the original render postlude and real clip/frame setters; external
// canvas/font/Unlock operations are explicit synthetic bindings.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <algorithm>
#include <array>
#include <limits>
#include <string>

struct PostludeCall {std::uint32_t tag,ecx,return_pc,args[7];};
static std::vector<PostludeCall> postlude_calls;
static std::string postlude_text;
static std::uint32_t postlude_lock,postlude_width,postlude_unlock;
extern "C" void capture_postlude(std::uint32_t tag,std::uint32_t* regs){
    PostludeCall call{tag,regs[6],regs[9],{}};std::memcpy(call.args,regs+10,28);postlude_calls.push_back(call);
    std::uint32_t value=0x80004005;
    if(tag==0x58b660)value=postlude_lock;
    if(tag==0x4a5ce0){
        const auto* text=reinterpret_cast<const char*>(call.args[0]);
        const auto* end=static_cast<const char*>(std::memchr(text,0,128));
        if(!end)throw std::runtime_error("Unterminated original postlude text");
        postlude_text.assign(text,end);value=postlude_width;
    }
    if(tag==0x80000080)value=postlude_unlock;
    regs[7]=value;
}
static std::uint32_t postlude_pointer(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
static void* postlude_stub(std::uint32_t tag,unsigned pop){
    auto* code=static_cast<unsigned char*>(mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if(code==MAP_FAILED)throw std::runtime_error("Stub allocation");
    const unsigned char head[]={0x9c,0x60,0x89,0xe0,0x50,0x68};std::memcpy(code,head,sizeof(head));
    std::memcpy(code+6,&tag,4);code[10]=0xe8;
    const auto relative=postlude_pointer(reinterpret_cast<void*>(&capture_postlude))-postlude_pointer(code+15);
    std::memcpy(code+11,&relative,4);
    const unsigned char tail[]={0x83,0xc4,0x08,0x61,0x9d,0xc2};std::memcpy(code+15,tail,sizeof(tail));
    const std::uint16_t cleanup=pop;std::memcpy(code+21,&cleanup,2);return code;
}
static void postlude_bind(std::uintptr_t address,unsigned pop){
    auto* destination=postlude_stub(address,pop);auto* entry=reinterpret_cast<unsigned char*>(address);
    const auto relative=postlude_pointer(destination)-std::uint32_t(address+5);entry[0]=0xe9;std::memcpy(entry+1,&relative,4);
}
static void postlude_check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static std::uint32_t postlude_global(std::uintptr_t address){std::uint32_t value;std::memcpy(&value,reinterpret_cast<void*>(address),4);return value;}
static std::string postlude_hex(const std::string& text){const char* digits="0123456789abcdef";std::string result;
    for(unsigned char value:text){result+=digits[value>>4];result+=digits[value&15];}return result;}

int main(int argc,char** argv)try{
    postlude_check(argc==2,"Expected pinned original PE");map_image(read(argv[1]));
    std::vector<std::pair<std::uintptr_t,Bytes>> originals;
    for(auto range:std::vector<std::pair<std::uintptr_t,std::uintptr_t>>{
        {0x4d8ab0,0x4d8cad},{0x57dc20,0x57dc40},{0x57dc60,0x57dc82},{0x57dc90,0x57dcb2},{0x59de2f,0x59de5c},{0x59de5c,0x59deb8}}){
        const auto* start=reinterpret_cast<const unsigned char*>(range.first);originals.push_back({range.first,Bytes(start,start+(range.second-range.first))});}
    postlude_bind(0x58bac0,8);postlude_bind(0x58b660,0);postlude_bind(0x4a5ce0,28);
    std::array<std::uint32_t,33> vtable{};vtable[32]=postlude_pointer(postlude_stub(0x80000080,8));
    std::uint32_t surface=postlude_pointer(vtable.data());std::array<std::uint32_t,32> buffer{};
    std::string prefix(reinterpret_cast<const char*>(0x5e2fe4),19);
    const auto zero=prefix.find('\0');if(zero!=std::string::npos)prefix.resize(zero);
    using Postlude=void (__attribute__((thiscall)) *)(void*,std::int32_t,std::uint32_t,std::uint32_t);
    using GetClip=void (__attribute__((thiscall)) *)(void*);
    unsigned cases=0;
    for(unsigned enabled:{0u,1u})for(unsigned ready:{0u,1u})for(unsigned lock:{0u,1u})
    for(unsigned colour:{0u,1u})for(unsigned large:{0u,1u})for(unsigned width:{0u,1u,13u,127u})
    for(unsigned unlock:{0u,0x80004005u})for(std::int32_t value:{0,1,-1,1234,std::numeric_limits<std::int32_t>::min(),std::numeric_limits<std::int32_t>::max()})
    for(unsigned rows:{3u,5u})for(unsigned height:{8u,20u})for(unsigned unused:{0u,1u}){
        std::array<unsigned char,512> object{};auto put=[&](unsigned offset,std::uint32_t v){std::memcpy(object.data()+offset,&v,4);};
        std::array<std::uint32_t,6> widths,locations;std::array<std::uint32_t,24> rectangles;
        widths.fill(0xc0ffee42);locations.fill(0xc0ffee42);rectangles.fill(0xc0ffee42);
        const auto original_widths=widths,original_locations=locations;const auto original_rectangles=rectangles;
        put(4,ready?0x12345678:0);put(8,0x13572468);put(12,postlude_pointer(widths.data()));
        put(16,postlude_pointer(rectangles.data()));put(20,postlude_pointer(locations.data()));
        put(0x128,postlude_pointer(&surface));put(0x138,80);put(0x13c,160);
        put(0x156,height);put(0x15a,rows);put(0x15e,enabled);const auto original_object=object;
        global(0x6de6dd,colour);global(0x6de6d5,large);
        for(auto address:{0x658174u,0x6a2dc8u,0x6def50u,0x6e0008u,0x6cbb6cu,0x6a49b8u,0x656618u})global(address,0xdeadface);
        postlude_lock=lock?postlude_pointer(buffer.data()):0;postlude_width=width;postlude_unlock=unlock;
        postlude_calls.clear();postlude_text.clear();reinterpret_cast<Postlude>(0x4d8ab0)(object.data(),value,unused?0xdeadbeef:0,unused?0x12345678:0);
        const bool acquire=enabled&&ready,draw=acquire&&lock;
        postlude_check(postlude_calls.size()==(draw?4u:acquire?2u:0u),"Original postlude early exits/call count");
        postlude_check(object==original_object,"Postlude unexpectedly mutated owning object fields");
        auto expected_widths=original_widths,expected_locations=original_locations;auto expected_rectangles=original_rectangles;
        std::array<std::uint32_t,4> clip{};reinterpret_cast<GetClip>(0x57dc90)(clip.data());
        if(draw){
            const auto index=rows-2;expected_widths[index]=100;
            expected_rectangles[index*4]=0;expected_rectangles[index*4+1]=height*index;
            expected_rectangles[index*4+2]=(width+3)&~3u;expected_rectangles[index*4+3]=height*(index+1);
            expected_locations[index]=(((large?800:640)/2-(width>>1))+3)&~3u;
            postlude_check(postlude_text==prefix+std::to_string(value),"Original signed decimal text/prefix");
            postlude_check(clip==std::array<std::uint32_t,4>{0,height*index,404,height*(index+1)},"Real original clip setter/getter state");
            postlude_check(postlude_global(0x658174)==postlude_pointer(buffer.data())&&postlude_global(0x6a2dc8)==80&&postlude_global(0x6def50)==80,"Real original framebuffer setter state");
            const auto& font=postlude_calls[2];const auto& release=postlude_calls[3];
            postlude_check(font.tag==0x4a5ce0&&font.ecx==0x13572468&&font.return_pc==0x4d8c1a,"Original postlude font caller/receiver");
            postlude_check(std::all_of(font.args+2,font.args+7,[](auto v){return v==0;}),"Original font trailing arguments");
            postlude_check(release.tag==0x80000080&&release.return_pc==0x4d8c3a&&release.args[0]==postlude_pointer(&surface)&&release.args[1]==postlude_pointer(buffer.data()),"Original Unlock caller/pointer arguments");
        }else{
            postlude_check(clip==std::array<std::uint32_t,4>{0xdeadface,0xdeadface,0xdeadface,0xdeadface},"Early exit changed clip state");
            for(auto address:{0x658174u,0x6a2dc8u,0x6def50u})postlude_check(postlude_global(address)==0xdeadface,"Early exit changed frame state");
        }
        postlude_check(widths==expected_widths&&locations==expected_locations&&rectangles==expected_rectangles,"Original aligned postlude metadata or untouched rows");
        if(acquire){const auto& fill=postlude_calls[0];const auto& borrowed=postlude_calls[1];
            postlude_check(fill.tag==0x58bac0&&fill.ecx==postlude_pointer(object.data()+0x124)&&fill.args[1]==(colour?31u:0u),"Original fill receiver/flags");
            postlude_check(borrowed.tag==0x58b660&&borrowed.ecx==postlude_pointer(object.data()+0x124),"Original Lock receiver/order");}
        for(const auto& original:originals){const auto* address=reinterpret_cast<const unsigned char*>(original.first);
            postlude_check(Bytes(address,address+original.second.size())==original.second,"Original helper instructions changed");}
        std::cout<<"{\"enabled\":"<<enabled<<",\"ready\":"<<ready<<",\"lock\":"<<lock<<",\"colour\":"<<colour
                 <<",\"large\":"<<large<<",\"width\":"<<width<<",\"unlock\":"<<unlock<<",\"value\":"<<value
                 <<",\"rows\":"<<rows<<",\"height\":"<<height<<",\"unused\":"<<unused<<",\"draw\":"<<(draw?"true":"false")
                 <<",\"text_hex\":\""<<postlude_hex(postlude_text)<<"\",\"calls\":[";
        for(unsigned i=0;i<postlude_calls.size();++i){if(i)std::cout<<',';const auto& c=postlude_calls[i];
            std::cout<<"{\"tag\":"<<c.tag<<",\"return_pc\":"<<c.return_pc<<"}";}
        std::cout<<"]}\n";++cases;
    }
    postlude_check(cases==12288,"Postlude case count");return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
