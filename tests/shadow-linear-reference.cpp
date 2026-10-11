// Isolated unmodified PE32 shadow execution; independently owned gradient.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
int main(int argc,char** argv)try{
    if(argc!=4)throw std::runtime_error("Expected pinned PE, owned frame, new report");
    map_image(read(argv[1]));const unsigned char anchor[]={0x83,0xec,0x18};
    if(std::memcmp(reinterpret_cast<void*>(0x57e540),anchor,3))throw std::runtime_error("Shadow entry bytes");
    auto input=read(argv[2]);const auto n=u32(input,0),w=u32(input,4),h=u32(input,8);
    const bool covered=w==48&&h==29&&std::int32_t(u32(input,12))==-28&&std::int32_t(u32(input,16))==-10;
    const bool padding=w==68&&h==38&&std::int32_t(u32(input,12))==-18&&std::int32_t(u32(input,16))==-23;
    if(input.size()!=n||(!covered&&!padding))throw std::runtime_error("Captured shadow geometry");
    constexpr unsigned width=800,height=600;const int anchorX=covered?723:700,anchorY=covered?242:179;
    const int left=anchorX-std::int32_t(u32(input,12)),top=anchorY-std::int32_t(u32(input,16))+h/2;
    std::vector<std::uint16_t> pixels(width*height);for(unsigned i=0;i<pixels.size();++i)pixels[i]=(i*97+13)&65535;
    const auto before=pixels;auto hypothetical=pixels;unsigned wrapped=0,samples=0;
    for(unsigned row=0;row<(h+1)/2;++row){const unsigned sourceRow=row*2;unsigned x=0;bool opaque=false;
        const auto delta=u32(input,40+sourceRow*8),end=sourceRow+1<h?u32(input,48+sourceRow*8):u32(input,44);
        for(unsigned at=delta;at<end;++at){const auto run=input.at(at);
            if(opaque)for(unsigned col=x;col<x+run;++col){const unsigned dx=left+col+h/2-row,dy=top+row;const auto to=dy*width+dx;
                if(to>=pixels.size())throw std::runtime_error("Shadow sample beyond allocation");
                hypothetical.at(to)=(hypothetical.at(to)>>1)&0x7bef;wrapped+=dx>=width;++samples;
            }
            x+=run;opaque=!opaque;
        }
        if(x!=w)throw std::runtime_error("Shadow row coverage");
    }
    if(wrapped!=(covered?2u:0u)||samples!=(covered?225u:206u))throw std::runtime_error("Captured shadow coverage count");
    if(left+w+h/2<=width)throw std::runtime_error("Missing projected horizontal refusal");
    global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);
    global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,width);global(0x656618,covered?329:312);global(0x6e1f88,0);
    using Shadow=void (__attribute__((fastcall)) *)(void*,int,int);reinterpret_cast<Shadow>(0x57e540)(input.data(),anchorX,anchorY);
    if(pixels!=before)throw std::runtime_error("Original projected refusal changed destination");
    save(std::string(argv[3])+".565",pixels);
    // Equality at the full projected edge is admitted. Translate the original
    // model's covered samples left to that exact edge, independently of native.
    const int adjustment=left+w+h/2-width;auto boundary=before;
    for(unsigned row=0;row<(h+1)/2;++row){const unsigned sourceRow=row*2;unsigned x=0;bool opaque=false;
        const auto delta=u32(input,40+sourceRow*8),end=sourceRow+1<h?u32(input,48+sourceRow*8):u32(input,44);
        for(unsigned at=delta;at<end;++at){const auto run=input.at(at);
            if(opaque)for(unsigned col=x;col<x+run;++col){const auto to=(top+row)*width+left-adjustment+col+h/2-row;boundary.at(to)=(boundary.at(to)>>1)&0x7bef;}
            x+=run;opaque=!opaque;
        }
    }
    pixels=before;reinterpret_cast<Shadow>(0x57e540)(input.data(),anchorX-adjustment,anchorY);
    if(pixels!=boundary)throw std::runtime_error("Original exact projected edge differs");
    save(std::string(argv[3])+".boundary.565",pixels);
    pixels=before;global(0x656618,top+4);reinterpret_cast<Shadow>(0x57e540)(input.data(),anchorX,anchorY);
    if(pixels!=before)throw std::runtime_error("Vertical crop incorrectly relaxed full projected refusal");
    std::ofstream out(argv[3]);out<<"{\"success\":true,\"original_entries\":1,\"cases\":3,\"canvas_words_compared\":1440000,\"hypothetical_covered_samples\":"<<samples<<",\"hypothetical_wrapped_samples\":"<<wrapped<<",\"original_whole_shadow_refused\":true,\"original_changed_words\":0,\"equality_admitted\":true,\"vertical_crop_preserves_full_envelope\":true,\"live_replacement\":false}\n";
    if(!out)throw std::runtime_error("Shadow report write");
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
