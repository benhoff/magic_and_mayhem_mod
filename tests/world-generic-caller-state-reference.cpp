#define MNM_PIXEL_TEST_UTILITIES
#include "pixel-splice-reference.cpp"
#include "../runtime/scene/world_generic_state.h"
extern "C" {unsigned generic_call_sp,generic_after_args[5];}

static void put32(Bytes& b,unsigned at,unsigned value){std::memcpy(b.data()+at,&value,4);}
static unsigned short half(const unsigned char* p){unsigned short v;std::memcpy(&v,p,2);return v;}
static Bytes encoded(unsigned w,unsigned h,unsigned seed,bool indexed){
    if(!w||!h){Bytes f(40);put32(f,0,40);put32(f,4,w);put32(f,8,h);put32(f,12,unsigned(-3));put32(f,16,2);put32(f,28,UINT32_MAX);return f;}
    Bytes control,colours;std::vector<unsigned> controls,pixels;
    for(unsigned y=0;y<h;++y){controls.push_back(control.size());pixels.push_back(colours.size());unsigned x=0;
        while(x<w){const unsigned skip=std::min(w-x,(x+y+seed)%4);control.push_back(skip);x+=skip;if(x==w)break;
            const unsigned n=std::min(w-x,1+(x*3+y+seed)%13);control.push_back(n);
            for(unsigned i=0;i<n;++i){const unsigned v=x+i+y*43+seed*71;colours.push_back(v);if(!indexed)colours.push_back(v>>8);}x+=n;}}
    Bytes f(40+h*8);put32(f,4,w);put32(f,8,h);put32(f,12,unsigned(-3));put32(f,16,2);put32(f,28,UINT32_MAX);
    for(unsigned y=0;y<h;++y){put32(f,40+y*8,f.size()+controls[y]);put32(f,44+y*8,f.size()+control.size()+pixels[y]);}
    f.insert(f.end(),control.begin(),control.end());f.insert(f.end(),colours.begin(),colours.end());put32(f,0,f.size());return f;
}
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

int main(int argc,char** argv)try{
    if(argc!=4)throw std::runtime_error("Expected original PE, state wire and comparison");
    map_image(read(argv[1]));std::ofstream raw(argv[2],std::ios::binary);if(!raw)throw std::runtime_error("State output open");
    raw.write("MNMGENS1",8);const unsigned version=1,rowBytes=320;raw.write(reinterpret_cast<const char*>(&version),4);raw.write(reinterpret_cast<const char*>(&rowBytes),4);
    const unsigned entries[]={0x5947b2,0x59521a,0x595b47,0x59603e,0x595677,0x596490,0x5968a4};
    unsigned cases=0,bad=0,nativePixels=0,changed=0;std::uint64_t words=0;std::array<unsigned,7> byEntry{};
    for(unsigned kind=0;kind<7;++kind)for(unsigned sourceSeed=0;sourceSeed<35;++sourceSeed)
    for(unsigned placement=0;placement<12;++placement)for(unsigned variant=0;variant<12;++variant){
        const unsigned family=kind<5?kind:6,fw=sourceSeed<32?std::array<unsigned,8>{1,2,3,7,16,39,129,255}[sourceSeed%8]:sourceSeed==34?3:0,
            fh=sourceSeed<32?std::array<unsigned,4>{1,2,3,5}[sourceSeed/8]:sourceSeed==33?2:0;
        const bool indexed=kind<4||(kind==4&&sourceSeed%2);Bytes frame=encoded(fw,fh,sourceSeed,indexed);
        const unsigned width=std::max(64u,fw+16),height=32,stride=width+3+sourceSeed%5,
            cl=sourceSeed/4%2?2:0,ct=sourceSeed/4%2?2:0,cr=width-sourceSeed/5%3,cb=height-1;
        const std::array<std::pair<int,int>,12> positions={{{int(cl+1),int(ct+1)},{int(cl),int(ct)},
            {int(cr-fw),int(ct+1)},{int(cl+1),int(cb-fh)},{int(cl)-1,int(ct+1)},{int(cl+1),int(ct)-1},
            {int(cr-fw+1),int(cb-fh+1)},{int(cl)-int(fw),int(ct+1)},{int(cl+1),int(ct)-int(fh)},
            {int(cr),int(ct+1)},{int(cl+1),int(cb)},{int(cl)-1,int(ct)-1}}};
        const auto [left,top]=positions[placement];const int ax=left-3,ay=top+2;
        Bytes palette(12+256*4);const unsigned step=family==2||family==3?4:2;
        for(unsigned i=0;i<256;++i){const unsigned value=((i*7919+sourceSeed*107)&65535)|
            (step==4&&sourceSeed/3%2?((i*5939+sourceSeed*123)&65535)<<16:0);std::memcpy(palette.data()+12+i*step,&value,step);}
        if(indexed){const unsigned pointer=reinterpret_cast<unsigned>(palette.data());put32(frame,28,pointer);}
        const auto sourceBefore=frame,paletteBefore=palette;
        std::vector<unsigned short> pixels(stride*(height+2)+512);for(unsigned i=0;i<pixels.size();++i)pixels[i]=i*7919+sourceSeed*107+placement*31;
        const auto beforePixels=pixels;auto* target=pixels.data()+256;
        const unsigned frameAddress=reinterpret_cast<unsigned>(frame.data()),canvasAddress=reinterpret_cast<unsigned>(target);
        global(0x658174,canvasAddress);global(0x6a2dc8,stride);global(0x6e1f88,0);global(0x6e1f68,0);
        global(0x6e0008,cl);global(0x6cbb6c,ct);global(0x6a49b8,cr);global(0x656618,cb);
        const unsigned phase=sourceSeed%16;global(0x6c4830,phase);for(unsigned i=0;i<16;++i)global(0x656640+i*4,i%7);
        unsigned old[16];for(unsigned i=0;i<16;++i)old[i]=0xabc00000+i*7919+sourceSeed;
        std::memcpy(reinterpret_cast<void*>(0x5f1e50),old,64);seedFx(variant,sourceSeed%16);
        MnmWorldGenericInput input{frame.data(),unsigned(frame.size()),frameAddress,canvasAddress,cr,cb,stride,family,unsigned(indexed),int(cl),int(ct),ax,ay};
        MnmWorldGenericState plan{};if(mnm_world_generic_prepare(&input,old,pixel_seed_fx,&plan))throw std::runtime_error("Model refused planned input kind="+std::to_string(kind)+" seed="+std::to_string(sourceSeed));
        pixel_function=entries[kind];pixel_args[0]=frameAddress;pixel_args[1]=unsigned(ax);pixel_args[2]=unsigned(ay);pixel_args[3]=0;pixel_args[4]=0;
        const auto original=call();const auto expectedPixels=pixels;unsigned originalWords[16];std::memcpy(originalWords,reinterpret_cast<void*>(0x5f1e50),64);
        unsigned actualArgs[5];std::memcpy(actualArgs,generic_after_args,20);
        unsigned saved[16]={0x71727374,0x51525354,0xe1e2e3e4,0x12345678,0xb1b2b3b4,0xd1d2d3d4,0xc1c2c3c4,0xa1a2a3a4,pixel_seed_flags,0x87654321,frameAddress,unsigned(ax),unsigned(ay),0,0,0xabcdef01};
        unsigned nativeWords[16];std::memcpy(nativeWords,old,64);alignas(16) unsigned char nativeFx[512];std::memcpy(nativeFx,pixel_seed_fx,512);
        mnm_world_generic_commit(&plan,nativeWords,saved,nativeFx);
        const unsigned wantedGpr[]={saved[7],saved[6],saved[5],saved[4],generic_call_sp,saved[2],saved[1],saved[0]};
        const unsigned wantedArgs[]={saved[10],saved[11],saved[12],saved[13],saved[14]};
        pixels=beforePixels;bool native=true;
        if(fw&&fh&&!plan.eax){
            struct MnmPixelPlane plane{target,width,height,stride,0};struct MnmRasterPixels raster{};
            raster.frame=frame.data();raster.bytes=frame.size();raster.indexed=indexed;raster.palette_step=step;raster.palette=palette.data()+12;
            raster.left=left;raster.top=top;raster.clip_left=cl;raster.clip_top=ct;raster.clip_right=cr;raster.clip_bottom=cb;
            raster.period=16;raster.mode=family==4?7:family==6?3:0;raster.strict_horizontal=family==4||family==6;raster.contiguous=family!=4;
            raster.paired_palette=family==2?1:family==3?2:0;
            const unsigned skip=top<int(ct)?ct-top:0;for(unsigned i=0;i<16;++i)raster.offsets[i]=((phase+1+i-skip)&15)%7;
            native=mnm_raster_pixels(&plane,&raster)!=0;if(native)++nativePixels;
        }
        const unsigned flagMask=plan.eax?0xcd5:0xcc5;unsigned mask=0;
        if(!std::memcmp(originalWords,nativeWords,64))mask|=1;
        if(!std::memcmp(actualArgs,wantedArgs,20))mask|=2;
        if(!std::memcmp(original.gpr.data(),wantedGpr,32))mask|=4;
        if(!((original.flags^saved[8])&flagMask))mask|=8;
        bool fp=half(original.fx.data())==half(nativeFx)&&half(original.fx.data()+2)==half(nativeFx+2)&&original.fx[4]==nativeFx[4]&&
            !std::memcmp(original.fx.data()+24,nativeFx+24,4)&&!std::memcmp(original.fx.data()+160,nativeFx+160,128);
        for(unsigned i=0;i<3;++i)fp=fp&&!std::memcmp(original.fx.data()+32+i*16,nativeFx+32+i*16,10);
        if(fp)mask|=16;
        if(pixels==expectedPixels&&frame==sourceBefore&&palette==paletteBefore)mask|=32;
        if(native)mask|=64;
        if(saved[3]==0x12345678&&saved[9]==0x87654321&&saved[15]==0xabcdef01)mask|=128;
        unsigned row[80]={++cases,entries[kind],family,sourceSeed,placement,variant,mask,flagMask};
        std::memcpy(row+8,originalWords,64);std::memcpy(row+24,nativeWords,64);std::memcpy(row+40,original.gpr.data(),32);std::memcpy(row+48,wantedGpr,32);
        std::memcpy(row+56,actualArgs,20);std::memcpy(row+61,wantedArgs,20);
        row[66]=original.flags;row[67]=saved[8];row[68]=half(original.fx.data());row[69]=half(nativeFx);row[70]=half(original.fx.data()+2);row[71]=half(nativeFx+2);
        row[72]=plan.clear_c1;row[73]=plan.eax;row[74]=frameAddress;row[75]=canvasAddress;row[76]=pixels.size();row[77]=pixels!=beforePixels;row[78]=indexed;
        raw.write(reinterpret_cast<const char*>(row),sizeof(row));
        if(mask!=255){if(bad<12){std::cerr<<"state kind="<<kind<<" seed="<<sourceSeed<<" placement="<<placement<<" variant="<<variant<<" mask="<<mask<<" flags="<<std::hex<<original.flags<<"/"<<saved[8]<<" FP="<<row[70]<<"/"<<row[71]<<std::dec<<"\n";
            for(unsigned i=0;i<16;++i)if(originalWords[i]!=nativeWords[i])std::cerr<<"workspace "<<i<<" "<<std::hex<<originalWords[i]<<"/"<<nativeWords[i]<<std::dec<<"\n";}++bad;}
        else ++byEntry[kind];words+=pixels.size();changed+=pixels!=beforePixels;
    }
    if(!raw)throw std::runtime_error("State output failed");std::ofstream report(argv[3]);
    report<<"{\"success\":"<<(bad?"false":"true")<<",\"cases\":"<<cases<<",\"failures\":"<<bad<<",\"native_pixel_cases\":"<<nativePixels<<",\"changed_cases\":"<<changed<<",\"physical_words\":"<<words<<",\"entries\":7,\"precision_rounding_modes\":12,\"condition_patterns\":16,\"installed_game\":false,\"by_entry\":[";
    for(unsigned i=0;i<7;++i){if(i)report<<',';report<<byEntry[i];}report<<"]}\n";
    std::cout<<cases<<" original/generic-state cases; "<<bad<<" mismatches\n";return bad?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
