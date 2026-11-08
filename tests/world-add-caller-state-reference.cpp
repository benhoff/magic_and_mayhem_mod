#define MNM_PIXEL_TEST_UTILITIES
#include "pixel-splice-reference.cpp"
#include "../runtime/scene/world_rectangle_state.h"
static unsigned short half(const unsigned char* p){unsigned short value;std::memcpy(&value,p,2);return value;}
static void seedFx(unsigned variant,unsigned condition){
    alignas(16) unsigned char host[512];const double a=1.25,b=-2.0,c=3.141592653589793;
    asm volatile("fxsave %0;fninit;fldl %2;fldl %3;fldl %4;fxsave %1;fxrstor %0"
        :"=m"(host),"=m"(pixel_seed_fx):"m"(a),"m"(b),"m"(c):"memory");
    const unsigned precision[]={0,2,3};const unsigned short cw=0x7f|(precision[variant/4]<<8)|((variant%4)<<10);
    std::memcpy(pixel_seed_fx,&cw,2);unsigned short sw=half(pixel_seed_fx+2)&~0x4700u;
    sw|=(condition&1)<<8;sw|=(condition&2)<<8;sw|=(condition&4)<<8;sw|=(condition&8)<<11;
    if(condition&1)sw|=0x21;std::memcpy(pixel_seed_fx+2,&sw,2);
    const unsigned mxcsr=0x1f80|((variant%4)<<13);std::memcpy(pixel_seed_fx+24,&mxcsr,4);
    for(unsigned i=0;i<128;++i)pixel_seed_fx[160+i]=i*37+condition+variant;
    const unsigned flags[]={0x202,0xed7,0x602,0xa97};pixel_seed_flags=flags[variant%4];
}
extern "C" {
unsigned _producer_trampolines[61];
void _producer_enter_26();
void _producer_leave_observe(unsigned*){throw std::runtime_error("Unexpected rectangle return observer");}
static unsigned admissions;
static unsigned plane_width,plane_height;
int _producer_enter_observe(unsigned* saved,unsigned tag,unsigned char*){
    if(tag!=26)throw std::runtime_error("Unexpected rectangle entry");
    const auto* shape=reinterpret_cast<const unsigned*>(saved[6]);
    MnmWorldAddInput input{int(shape[0]),int(shape[1]),int(saved[10]),int(saved[11]),
        int(*reinterpret_cast<unsigned*>(0x6e0008)),int(*reinterpret_cast<unsigned*>(0x6cbb6c)),
        int(*reinterpret_cast<unsigned*>(0x6a49b8)),int(*reinterpret_cast<unsigned*>(0x656618)),
        *reinterpret_cast<unsigned*>(0x6a2dc8)};
    MnmWorldAddState state{};
    if(mnm_world_add_prepare(&input,reinterpret_cast<unsigned>(saved+9),&state))throw std::runtime_error("Add state refused corpus");
    const int left=std::max(input.x,input.clip_left),top=std::max(input.y,input.clip_top),
        right=std::min(input.x+input.width,input.right),bottom=std::min(input.y+input.height,input.bottom);
    if(left<right&&top<bottom){
        MnmPixelPlane plane{*reinterpret_cast<unsigned short**>(0x658174),plane_width,plane_height,input.stride,*reinterpret_cast<unsigned*>(0x6e1f88)};
        if(!mnm_rgb_add(&plane,left,top,right,bottom,shape[2],shape[3],shape[4]))throw std::runtime_error("Native add pixels refused corpus");
    }
    mnm_world_add_commit(&state,saved);++admissions;return 1;
}
}

int main(int argc,char** argv)try{
    if(argc!=4)throw std::runtime_error("Expected original PE, state wire and comparison");
    map_image(read(argv[1]));std::ofstream raw(argv[2],std::ios::binary);raw.write("MNMRECT1",8);
    const unsigned version=1,rowBytes=1344;raw.write(reinterpret_cast<const char*>(&version),4);raw.write(reinterpret_cast<const char*>(&rowBytes),4);
    unsigned cases=0,bad=0,changed=0;std::uint64_t words=0;
    for(unsigned format=0;format<2;++format)for(unsigned clip=0;clip<2;++clip)for(unsigned sourceSeed=0;sourceSeed<32;++sourceSeed)
    for(unsigned placement=0;placement<12;++placement)for(unsigned variant=0;variant<12;++variant){
        const unsigned fw=std::array<unsigned,8>{0,1,2,7,16,39,63,129}[sourceSeed%8],fh=std::array<unsigned,4>{0,1,3,7}[sourceSeed/8];
        plane_width=std::max(64u,fw+16);plane_height=32;const unsigned stride=plane_width+3+sourceSeed%5;
        const int cl=clip?2:0,ct=clip?2:0,cr=plane_width-1,cb=31;
        const std::array<std::pair<int,int>,12> positions={{{cl+1,ct+1},{cl,ct},{cr-int(fw),ct+1},{cl+1,cb-int(fh)},
            {cl-1,ct+1},{cl+1,ct-1},{cr-int(fw)+1,cb-int(fh)+1},{cl-int(fw),ct+1},{cl+1,ct-int(fh)},
            {cr,ct+1},{cl+1,cb},{cl-1,ct-1}}};
        const auto [x,y]=positions[placement];unsigned shape[5]={fw,fh,sourceSeed%32,sourceSeed%(format?32:64),sourceSeed*7%32};
        const auto sourceBefore=std::array<unsigned,5>{shape[0],shape[1],shape[2],shape[3],shape[4]};
        std::vector<unsigned short> pixels(stride*(plane_height+2)+512);for(unsigned i=0;i<pixels.size();++i)pixels[i]=i*7919+sourceSeed*107+placement*31;
        const auto beforePixels=pixels;auto* target=pixels.data()+256;
        global(0x658174,reinterpret_cast<unsigned>(target));global(0x6a2dc8,stride);global(0x6e1f88,format);
        global(0x6e0008,cl);global(0x6cbb6c,ct);global(0x6a49b8,cr);global(0x656618,cb);
        unsigned workspace[16];for(unsigned i=0;i<16;++i)workspace[i]=0xabc00000+i*7919+sourceSeed;
        std::memcpy(reinterpret_cast<void*>(0x5f1e50),workspace,64);seedFx(variant,sourceSeed%16);
        pixel_object=reinterpret_cast<unsigned>(shape);pixel_args[0]=x;pixel_args[1]=y;pixel_function=0x545c10;
        const auto original=call();const auto expected=pixels;unsigned originalWorkspace[16];std::memcpy(originalWorkspace,reinterpret_cast<void*>(0x5f1e50),64);
        pixels=beforePixels;pixel_function=reinterpret_cast<unsigned>(_producer_enter_26);const auto native=call();
        unsigned nativeWorkspace[16];std::memcpy(nativeWorkspace,reinterpret_cast<void*>(0x5f1e50),64);
        bool fx=true;for(unsigned i=0;i<512;++i)if(!(i>=6&&i<24)&&original.fx[i]!=native.fx[i])fx=false;
        const unsigned mask=unsigned(original.gpr==native.gpr)|unsigned(!((original.flags^native.flags)&0xcd5))<<1|
            unsigned(fx)<<2|unsigned(pixels==expected)<<3|unsigned(!std::memcmp(shape,sourceBefore.data(),20))<<4|
            unsigned(!std::memcmp(originalWorkspace,nativeWorkspace,64)&&!std::memcmp(workspace,nativeWorkspace,64))<<5;
        unsigned row[80]={++cases,format,clip,sourceSeed,placement,variant,mask,0xcd5};
        std::memcpy(row+8,original.gpr.data(),32);std::memcpy(row+16,native.gpr.data(),32);row[24]=original.flags;row[25]=native.flags;
        row[30]=half(original.fx.data());row[31]=half(native.fx.data());row[32]=half(original.fx.data()+2);row[33]=half(native.fx.data()+2);
        row[34]=x;row[35]=y;row[36]=fw;row[37]=fh;row[38]=pixels.size();row[39]=pixels!=beforePixels;
        std::memcpy(row+48,originalWorkspace,64);std::memcpy(row+64,nativeWorkspace,64);
        raw.write(reinterpret_cast<const char*>(row),sizeof(row));raw.write(reinterpret_cast<const char*>(original.fx.data()),512);raw.write(reinterpret_cast<const char*>(native.fx.data()),512);
        if(mask!=63){if(bad<12){std::cerr<<"add seed="<<sourceSeed<<" placement="<<placement<<" variant="<<variant<<" mask="<<mask<<" flags="<<std::hex<<original.flags<<'/'<<native.flags<<std::dec<<'\n';
            for(unsigned i=0;i<8;++i)if(original.gpr[i]!=native.gpr[i])std::cerr<<"GPR "<<i<<' '<<std::hex<<original.gpr[i]<<'/'<<native.gpr[i]<<std::dec<<'\n';}++bad;}
        changed+=pixels!=beforePixels;words+=pixels.size();
    }
    if(!raw||admissions!=cases)throw std::runtime_error("Incomplete shared-entry corpus");std::ofstream result(argv[3]);
    result<<"{\"success\":"<<(bad?"false":"true")<<",\"cases\":"<<cases<<",\"failures\":"<<bad<<",\"native_entries\":"<<admissions<<",\"changed_cases\":"<<changed<<",\"physical_words\":"<<words<<",\"original_unchanged\":true,\"full_callback\":false}\n";
    std::cout<<cases<<" original/native shared additive entries; "<<bad<<" mismatches\n";return bad?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
