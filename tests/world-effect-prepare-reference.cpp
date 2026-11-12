#define MNM_PIXEL_TEST_UTILITIES
#include "pixel-splice-reference.cpp"
#include "../runtime/scene/world_effect_prepare.h"
extern "C" {unsigned generic_call_sp,generic_after_args[5];}
static unsigned effect_entry,effect_source_rows;
alignas(16) static unsigned char published_fx[512];
static unsigned direct(unsigned frame,unsigned x,unsigned y,unsigned amplitude,unsigned){
    if(effect_source_rows){
        using Function=unsigned(__attribute__((fastcall))*)(void*,unsigned,unsigned,unsigned,unsigned);
        return reinterpret_cast<Function>(effect_entry)(reinterpret_cast<void*>(frame),x,y,0,0);
    }
    using Function=unsigned(__attribute__((fastcall))*)(void*,unsigned,unsigned,unsigned);
    return reinterpret_cast<Function>(effect_entry)(reinterpret_cast<void*>(frame),x,y,amplitude);
}
static unsigned prepared(unsigned,unsigned,unsigned,unsigned,unsigned){
    mnm_world_effect_prepare(effect_entry,effect_source_rows,published_fx);return 0;
}
static unsigned short gethalf(const unsigned char* p){unsigned short value;std::memcpy(&value,p,2);return value;}
static void seed(unsigned variant,unsigned condition){
    alignas(16) unsigned char host[512];const double a=1.25,b=-2.0,c=3.141592653589793;
    asm volatile("fxsave %0;fninit;fldl %2;fldl %3;fldl %4;fxsave %1;fxrstor %0"
        :"=m"(host),"=m"(pixel_seed_fx):"m"(a),"m"(b),"m"(c):"memory");
    const unsigned precision[]={0,2,3};const unsigned short cw=0x7f|(precision[variant/4]<<8)|((variant%4)<<10);
    std::memcpy(pixel_seed_fx,&cw,2);unsigned short sw=gethalf(pixel_seed_fx+2)&~0x4700u;
    sw|=(condition&1)<<8;sw|=(condition&2)<<8;sw|=(condition&4)<<8;sw|=(condition&8)<<11;
    if(condition&1)sw|=0x21;std::memcpy(pixel_seed_fx+2,&sw,2);
    const unsigned mxcsr=0x1f80|((variant%4)<<13);std::memcpy(pixel_seed_fx+24,&mxcsr,4);
    for(unsigned i=0;i<128;++i)pixel_seed_fx[160+i]=i*37+condition+variant;
    pixel_seed_flags=0x202;
}
static bool same(const unsigned char* a,const unsigned char* b){
    if(std::memcmp(a,b,5)||std::memcmp(a+24,b+24,4)||std::memcmp(a+160,b+160,128))return false;
    for(unsigned i=0;i<3;++i)if(std::memcmp(a+32+i*16,b+32+i*16,10))return false;
    return true;
}
int main(int argc,char** argv)try{
    if(argc!=4)throw std::runtime_error("Expected original PE, state wire and result");
    map_image(read(argv[1]));std::ofstream raw(argv[2],std::ios::binary);
    raw.write("MNMEFX01",8);const unsigned version=1,rowBytes=3392;raw.write(reinterpret_cast<const char*>(&version),4);raw.write(reinterpret_cast<const char*>(&rowBytes),4);
    unsigned cases=0,bad=0,changed=0,refusals=0;
    for(unsigned kind=0;kind<2;++kind)for(unsigned amplitude=1;amplitude<=16;++amplitude)
    for(unsigned variant=0;variant<12;++variant)for(unsigned condition=0;condition<16;++condition)for(unsigned positive=0;positive<2;++positive){
        effect_source_rows=kind;effect_entry=kind?0x580260:0x5806f0;
        unsigned frame[11]={44,positive,positive};
        /* A positive frame outside the clip proves that actual post-initializer
         * normal-return control preserves the same FP result. No raster data is
         * accessed, and this test gives no native pixel-comparison credit. */
        unsigned palette[131]={0};frame[7]=reinterpret_cast<unsigned>(palette);
        global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,64);global(0x656618,8);
        global(0x5e15f8,amplitude);global(0x6c4830,condition);
        unsigned before[288];for(unsigned i=0;i<288;++i)before[i]=0x7d00+i*7919;
        before[0]=before[32]=0x7d00;std::memcpy(reinterpret_cast<void*>(0x5f1450),before,sizeof(before));
        seed(variant,condition);
        if(!mnm_world_effect_fx_ready(pixel_seed_fx))throw std::runtime_error("Legal FPU state refused");
        pixel_args[0]=reinterpret_cast<unsigned>(frame);pixel_args[1]=1000;pixel_args[2]=1000;pixel_args[3]=amplitude;pixel_args[4]=0;
        pixel_function=reinterpret_cast<unsigned>(direct);const auto original=call();unsigned originalTable[288];
        std::memcpy(originalTable,reinterpret_cast<void*>(0x5f1450),sizeof(originalTable));
        std::memcpy(reinterpret_cast<void*>(0x5f1450),before,sizeof(before));std::memcpy(published_fx,pixel_seed_fx,512);
        pixel_function=reinterpret_cast<unsigned>(prepared);const auto native=call();unsigned nativeTable[288];
        std::memcpy(nativeTable,reinterpret_cast<void*>(0x5f1450),sizeof(nativeTable));
        const bool fp=same(original.fx.data(),published_fx)&&same(native.fx.data(),published_fx);
        const bool table=!std::memcmp(originalTable,nativeTable,sizeof(nativeTable));
        const bool mutation=std::memcmp(before,nativeTable,sizeof(before));
        unsigned row[16]={++cases,kind,amplitude,variant,condition,positive,unsigned(fp),unsigned(table),unsigned(mutation),gethalf(original.fx.data()+2),gethalf(published_fx+2)};
        raw.write(reinterpret_cast<const char*>(row),sizeof(row));raw.write(reinterpret_cast<const char*>(original.fx.data()),512);
        raw.write(reinterpret_cast<const char*>(published_fx),512);raw.write(reinterpret_cast<const char*>(originalTable),sizeof(originalTable));raw.write(reinterpret_cast<const char*>(nativeTable),sizeof(nativeTable));
        if(!fp||!table||!mutation){if(bad<8)std::cerr<<"effect kind="<<kind<<" amplitude="<<amplitude<<" variant="<<variant<<" condition="<<condition<<" positive="<<positive<<" FP="<<fp<<" table="<<table<<'\n';++bad;}
        changed+=!same(original.fx.data(),pixel_seed_fx);
    }
    seed(0,0);alignas(16) unsigned char invalid[512];
    for(unsigned variant=0;variant<5;++variant){std::memcpy(invalid,pixel_seed_fx,512);
        if(!variant)invalid[0]&=~1u;else if(variant==1)invalid[1]=(invalid[1]&~3u)|1u;
        else invalid[4]|=1u<<((((invalid[3]>>3)&7)+8-(variant-1))&7);
        if(mnm_world_effect_fx_ready(invalid))throw std::runtime_error("Unsupported FPU state admitted");++refusals;}
    if(!raw)throw std::runtime_error("State output failed");std::ofstream result(argv[3]);
    result<<"{\"success\":"<<(bad?"false":"true")<<",\"cases\":"<<cases<<",\"failures\":"<<bad<<",\"changed_fp_cases\":"<<changed<<",\"policy_refusals\":"<<refusals<<",\"original_routines_unchanged\":true,\"original_conversion_helper_unchanged\":true,\"native_pixel_comparison\":false}\n";
    std::cout<<cases<<" retained initializer comparisons; "<<bad<<" mismatches; "<<changed<<" changed FP states\n";return bad?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
