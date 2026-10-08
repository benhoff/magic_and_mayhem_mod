// Independent unmodified PE32 raster execution. Actual palettes/clip/ordered
// requests are inputs. An optional before-canvas is a diagnostic input to this
// private original process only; it must never initialize the native renderer.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../protocols/include/mnm/world_frame_v1.h"
int main(int argc,char** argv)try{
    if(argc!=4&&argc!=5)throw std::runtime_error("Expected pinned PE, World inputs, new output, optional diagnostic before-canvas");
    map_image(read(argv[1]));auto input=read(argv[2]);
    if(input.size()<80||std::memcmp(input.data(),MNM_WORLD_MAGIC,8)||u32(input,40)||u32(input,72)||u32(input,16)!=input.size())throw std::runtime_error("World input admission");
    const auto width=u32(input,24),height=u32(input,28);if(!width||width>2048||!height||height>2048)throw std::runtime_error("World extent");
    std::vector<std::uint16_t> pixels(width*height,0);
    if(argc==5){const auto before=read(argv[4]);const auto stride=u32(input,32);if(stride<width||before.size()!=std::size_t(stride)*height*2)throw std::runtime_error("Diagnostic before-canvas extent");for(unsigned y=0;y<height;++y)std::memcpy(pixels.data()+y*width,before.data()+std::size_t(y)*stride*2,width*2);}
    global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f68,0);global(0x6e1f88,0);
    std::size_t at=80;
    for(unsigned i=0;i<u32(input,36);++i){
        const auto size=u32(input,at),op=u32(input,at+4),n=u32(input,at+32),indexed=u32(input,at+36),payload=u32(input,at+40);
        if(size!=64+n+payload||(op<6&&n<40)||at+size>input.size()||op>7)throw std::runtime_error("World raster record");
        if(op==7){
            if(n||indexed||payload<14||u32(input,at+52)!=12)throw std::runtime_error("Colour rectangle record");
            const auto w=u32(input,at+64),h=u32(input,at+68),mode=u32(input,at+72);
            if(!w||!h||w>64||h>64||mode>3||payload!=12+w*h*2)throw std::runtime_error("Colour rectangle extent");
            std::vector<std::uint16_t> rgb(w*h*3);for(unsigned j=0;j<w*h;++j){const auto p=input.at(at+76+j*2)|unsigned(input.at(at+77+j*2))<<8;rgb[j*3]=p>>11;rgb[j*3+1]=(p>>5)&63;rgb[j*3+2]=p&31;}
            unsigned char object[80]={};const std::uint32_t fields[]={mode,w,h};std::memcpy(object+28,fields,12);const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(rgb.data()));std::memcpy(object+46,&pointer,4);
            for(const auto& pair:std::vector<std::pair<std::uintptr_t,unsigned>>{{0x6e0008,16},{0x6cbb6c,20},{0x6a49b8,24},{0x656618,28}})global(pair.first,u32(input,at+pair.second));
            using Colour=unsigned (__attribute__((thiscall)) *)(void*,int,int);const auto x=std::int32_t(u32(input,at+8)),y=std::int32_t(u32(input,at+12));
            const auto prior=pixels;reinterpret_cast<Colour>(0x54ab60)(object,x,y);const auto expected=pixels;
            bool uniform=true;for(unsigned j=1;j<w*h;++j)if(std::memcmp(rgb.data(),rgb.data()+j*3,6))uniform=false;
            if(uniform){pixels=prior;std::memcpy(object+40,rgb.data(),6);const std::uint32_t zero=0;std::memcpy(object+46,&zero,4);reinterpret_cast<Colour>(0x54ab60)(object,x,y);if(pixels!=expected)throw std::runtime_error("Uniform/plane colour source differs");}
            at+=size;continue;
        }
        if(op==6){
            if(n||indexed||payload!=20||u32(input,at+52)!=11)throw std::runtime_error("Additive record");
            std::uint32_t object[18]={};object[0]=0x5c7420;
            for(unsigned j=0;j<5;++j)object[12+j]=u32(input,at+64+j*4);
            if(!object[12]||object[12]>2048||!object[13]||object[13]>2048)throw std::runtime_error("Additive extent");
            for(const auto& pair:std::vector<std::pair<std::uintptr_t,unsigned>>{{0x6e0008,16},{0x6cbb6c,20},{0x6a49b8,24},{0x656618,28}})global(pair.first,u32(input,at+pair.second));
            using Additive=unsigned (__attribute__((thiscall)) *)(void*,int,int);
            const auto prior=pixels;reinterpret_cast<Additive>(0x54ab00)(object,std::int32_t(u32(input,at+8)),std::int32_t(u32(input,at+12)));
            if(reinterpret_cast<unsigned char*>(object)[0x44]!=1)throw std::runtime_error("Original drawn side effect missing");
            // Independently exercise the second additive wrapper's owner/player gates.
            const auto expected=pixels;std::uint32_t gated[26]={},owner[58]={};for(unsigned j=0;j<5;++j)gated[13+j]=object[12+j];
            using Gated=unsigned (__attribute__((thiscall)) *)(void*,int,int);const auto x=std::int32_t(u32(input,at+8)),y=std::int32_t(u32(input,at+12));
            pixels=prior;reinterpret_cast<Gated>(0x546540)(gated,x,y);if(pixels!=expected)throw std::runtime_error("Owner-free additive wrapper differs");
            gated[24]=reinterpret_cast<std::uintptr_t>(owner);owner[57]=1;
            for(int player:{-1,0,8}){global(0x644520,player);pixels=prior;reinterpret_cast<Gated>(0x546540)(gated,x,y);if(pixels!=prior)throw std::runtime_error("Disabled owner/player additive gate drew pixels");}
            global(0x644520,0);owner[9]=1;pixels=prior;reinterpret_cast<Gated>(0x546540)(gated,x,y);if(pixels!=expected)throw std::runtime_error("Active owner/player additive wrapper differs");
            at+=size;continue;
        }
        Bytes frame(input.begin()+at+64,input.begin()+at+64+n);std::vector<std::uint32_t> palette(3+128,0);
        if(indexed&&op!=3){if(payload!=512)throw std::runtime_error("Palette extent");std::memcpy(palette.data()+3,input.data()+at+64+n,512);const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(palette.data()));std::memcpy(frame.data()+28,&pointer,4);}
        else if(!indexed){std::uint32_t marker=UINT32_MAX;std::memcpy(frame.data()+28,&marker,4);}
        for(const auto& pair:std::vector<std::pair<std::uintptr_t,unsigned>>{{0x6e0008,16},{0x6cbb6c,20},{0x6a49b8,24},{0x656618,28}})global(pair.first,u32(input,at+pair.second));
        using Draw=unsigned (__attribute__((fastcall)) *)(void*,int,int,int,int);
        const int x=std::int32_t(u32(input,at+8)),y=std::int32_t(u32(input,at+12));
        if(op==5){using Shadow=unsigned (__attribute__((fastcall)) *)(void*,int,int);reinterpret_cast<Shadow>(0x57e540)(frame.data(),x,y);}
        else if(op==3){
            const auto period=u32(input,at+60),amplitude=period/2;if(!period||period>16||payload!=64)throw std::runtime_error("Wave extent");
            // Keep the original initializer inactive; the observed owned table
            // already contains this request's effective phase.
            global(0x5f14d0,0);for(unsigned j=0;j<16;++j)global(0x5f14d0+(amplitude*16+j)*4,u32(input,at+64+n+j*4));
            const auto left=x-std::int32_t(u32(frame,12));const bool clipped=left<std::int32_t(u32(input,at+16))||left+int(u32(frame,4))>std::int32_t(u32(input,at+24));
            global(0x6c4830,clipped?u32(input,at+48):0);
            using Wave=unsigned (__attribute__((fastcall)) *)(void*,int,int,int);reinterpret_cast<Wave>(0x5806f0)(frame.data(),x,y,amplitude);
        }else if(!indexed){using Direct=unsigned (*)(void*,int,int);reinterpret_cast<Direct>(0x597086)(frame.data(),x,y);}
        else reinterpret_cast<Draw>(op==0?0x57e1b0:op==1?0x57ec90:op==2?0x57f5f0:0x57f0f0)(frame.data(),x,y,0,0);
        at+=size;
    }
    {
        unsigned char flagObject[80]={};const auto prior=pixels;
        using Noop=unsigned (__attribute__((thiscall)) *)(void*,int,int);reinterpret_cast<Noop>(0x54ac00)(flagObject,-123,456);
        if(pixels!=prior||flagObject[64]!=1)throw std::runtime_error("Original flag-only draw changed pixels or missed flag");
        for(unsigned i=0;i<80;++i)if(i!=64&&flagObject[i])throw std::runtime_error("Original flag-only draw changed other fields");
    }
    if(at!=input.size())throw std::runtime_error("Trailing raster bytes");
    save(argv[3],pixels);std::cout<<"Independent original World raster canvas complete\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
