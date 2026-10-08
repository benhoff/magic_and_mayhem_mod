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
        if(size!=64+n+payload||n<40||at+size>input.size()||op>5)throw std::runtime_error("World raster record");
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
    if(at!=input.size())throw std::runtime_error("Trailing raster bytes");
    save(argv[3],pixels);std::cout<<"Independent original World raster canvas complete\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
