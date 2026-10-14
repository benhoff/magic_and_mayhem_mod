// Isolated unmodified PE32 distortion leaves; no game process or file patching.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
int main(int argc,char** argv)try{
    if(argc!=4&&argc!=5)throw std::runtime_error("Expected pinned PE, owned frame+offsets, new JSON report, optional bottom-edge mode");
    const bool edge=argc==5&&std::string(argv[4])=="--bottom-edge";
    if(argc==5&&!edge)throw std::runtime_error("Unknown original fixture mode");
    map_image(read(argv[1]));const unsigned char anchor[]={0x55,0x8b,0xec,0x56,0x57,0x53};
    for(auto entry:{0x596490,0x5968a4})if(std::memcmp(reinterpret_cast<void*>(entry),anchor,6))throw std::runtime_error("Distortion entry bytes");
    auto input=read(argv[2]);const auto n=u32(input,0),w=u32(input,4),h=u32(input,8);
    if(input.size()!=n+64||w!=61||h!=47||u32(input,12)!=31||u32(input,16)!=31)throw std::runtime_error("Captured request geometry");
    constexpr unsigned width=800,height=600;const int left=edge?732:733,top=edge?572:425;
    // Explicit owned guard words isolate the original's read beyond its logical
    // canvas. They are evidence only, never inputs to public native rendering.
    std::vector<std::uint16_t> before(width*height+(edge?32:0));for(unsigned i=0;i<before.size();++i)before[i]=(i*97+13)&65535;
    auto expected=before;unsigned wrapped=0,covered=0,outside=0;
    for(unsigned row=0;row<h;++row){unsigned x=0;bool opaque=false;
        const auto delta=u32(input,40+row*8),end=row+1<h?u32(input,48+row*8):u32(input,44);
        for(unsigned at=delta;at<end;++at){const auto run=input.at(at);
            if(opaque)for(unsigned col=x;col<x+run;++col){const unsigned dx=left+col,dy=top+row;if(dy>=height)continue;const unsigned shift=u32(input,n+(dy%16)*4);const auto to=dy*width+dx,from=to+shift;
                if(shift>16||from+1>=before.size())throw std::runtime_error("Original DWORD sample exceeds owned storage");
                expected.at(to)=before.at(from);wrapped+=dx+shift>=width;outside+=from>=width*height;++covered;
            }
            x+=run;opaque=!opaque;
        }
        if(x!=w)throw std::runtime_error("Captured row coverage");
    }
    if(edge?(outside!=1):(wrapped!=10))throw std::runtime_error("Captured edge sample count");
    global(0x6a2dc8,width);global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,width);global(0x656618,height);global(0x6c4830,0);
    for(unsigned i=0;i<16;++i)global(0x656640+((i+1)&15)*4,u32(input,n+i*4));
    for(auto entry:{0x596490,0x5968a4}){
        auto pixels=before;global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));
        using Draw=unsigned (*)(void*,int,int);if(reinterpret_cast<Draw>(entry)(input.data(),left+31,top+31)||pixels!=expected)throw std::runtime_error("Original displacement differs from linear model");
    }
    std::ofstream out(argv[3],std::ios::out);out<<"{\"success\":true,\"original_entries\":2,\"canvas_words_compared\":"<<2*before.size()<<",\"covered_samples\":"<<covered<<",\"wrapped_samples\":"<<wrapped<<",\"outside_logical_samples\":"<<outside<<",\"live_replacement\":false}\n";
    if(!out)throw std::runtime_error("Report write");
    save(std::string(argv[3])+".565",expected);
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
