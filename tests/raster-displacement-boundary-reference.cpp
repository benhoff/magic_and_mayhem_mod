// Captured real RLE shape, original sine initializer and physical-plane edge.
#define MNM_RASTER_TEST_UTILITIES
#include "raster-splice-reference.cpp"
#include "../reconstruction/rendering/raster_pixels.h"
extern "C" {extern unsigned mnm_raster_retained_calls[],mnm_raster_retained_pixels[];}

int main(int argc,char** argv)try{
 if(argc!=4)throw std::runtime_error("Expected pinned original, captured frame and result");
 map_image(read(argv[1]));const Bytes frame=read(argv[2]);
 auto word=[&](unsigned at){unsigned v;std::memcpy(&v,frame.data()+at,4);return v;};
 if(frame.size()!=2040||word(0)!=2040||word(4)!=61||word(8)!=47||word(12)!=31||word(16)!=31)
  throw std::runtime_error("Unexpected captured frame geometry");
 asm volatile("fninit");
 reinterpret_cast<void(*)()>(0x57dbd0)(); // Original non-drawing table preparation.
 std::array<unsigned,16> table{};std::memcpy(table.data(),reinterpret_cast<void*>(0x656640),64);
 for(unsigned v:table)if(v>16)throw std::runtime_error("Original sine table exceeds recovered range");
 constexpr unsigned site=14;
 std::array<unsigned char,9> original{},patched{};
 std::memcpy(original.data(),reinterpret_cast<void*>(sites[site].at),sites[site].length);
 if(!RasterWritersInstallForTest())throw std::runtime_error("Production installer refused");
 std::memcpy(patched.data(),reinterpret_cast<void*>(sites[site].at),sites[site].length);
 asm volatile("fninit; fxsave %0":"=m"(pixel_seed_fx));pixel_seed_stack=1;
 unsigned native=0,forwarded=0,atomic=0,cases=0,retainedCases=0,retainedPixels=0;std::uint64_t compared=0;
 std::array<unsigned,3> native_geometry{},forward_geometry{};
 for(unsigned format=0;format<2;++format)for(unsigned df=0;df<2;++df)
  for(unsigned geometry=0;geometry<3;++geometry)for(unsigned phase=0;phase<16;++phase){
   constexpr unsigned width=800,height=600,stride=800,prefix=256;
   const unsigned left=geometry==2?700:730,top=geometry==1?500:570;
   std::vector<unsigned short> seed(prefix+stride*(height+2)+256);
   for(unsigned i=0;i<seed.size();++i)seed[i]=i*391+phase*17+format*7;
   auto pixels=seed;ownedPixels=pixels.data()+prefix;ownedWidth=width;ownedHeight=height;ownedStride=stride;
   global(0x658174,reinterpret_cast<unsigned>(ownedPixels));global(0x6a2dc8,stride);global(0x6e1f88,format);
   global(0x6e1f68,0);global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,width);global(0x656618,height);global(0x6c4830,phase);
   MnmPixelPlane plane{ownedPixels,width,height,stride,format};MnmRasterPixels input{};
   input.frame=frame.data();input.bytes=frame.size();input.indexed=1;input.palette_step=2;
   input.mode=3;input.strict_horizontal=1;input.contiguous=1;input.left=left;input.top=top;
   input.clip_right=width;input.clip_bottom=height;input.period=16;
   for(unsigned y=0;y<16;++y)input.offsets[y]=table[(phase+1+y)&15];
   // The pinned frame's visible source row29 has an opaque pixel at x60.
   // At (730,570) this is physical(790,599), leaving9 valid sample WORDs.
   // Earlier rows and the two controls have enough physical room for all16
   // displacement WORDs. Padding is observation storage, never plane ownership.
   const bool outside=geometry==0&&table[(phase+8)&15]>9;
   const int admitted=mnm_raster_pixels(&plane,&input);
   if(bool(admitted)==outside)throw std::runtime_error("Native physical ownership admission differs");
   if(outside){if(pixels!=seed)throw std::runtime_error("Refusal wrote pixels before fallback");++atomic;}
   pixels=seed;ownedPixels=pixels.data()+prefix;global(0x658174,reinterpret_cast<unsigned>(ownedPixels));
   pixel_function=0x5968a4;pixel_object=reinterpret_cast<unsigned>(frame.data());raster_cdecl=1;pixel_argc=5;
   auto args=[&]{pixel_args[0]=reinterpret_cast<unsigned>(frame.data());pixel_args[1]=left+word(12);pixel_args[2]=top+word(16);pixel_args[3]=pixel_args[4]=0;};
   pixel_seed_flags=0x202|(df?0x400:0);threadId=1;args();
   std::memcpy(reinterpret_cast<void*>(sites[site].at),original.data(),sites[site].length);
   const auto before=call();const auto originalExpected=pixels;auto expected=pixels;
   // Only the original's unowned sample values are outside recovered parity.
   // Independently decode the captured RLE, retain these destinations, and
   // require every other physical WORD to equal the actual original result.
   unsigned retained=0;
   for(unsigned y=0;y<word(8);++y){
    unsigned control=word(40+y*8),x=0;bool opaque=false;
    while(x<word(4)){
     const unsigned run=frame.at(control++);
     if(opaque)for(unsigned i=0;i<run;++i){
      const unsigned dx=left+x+i,dy=top+y;
      if(dx<width&&dy<height){const unsigned at=dy*stride+dx;
       if(at+input.offsets[dy%16]>=stride*height){expected[prefix+at]=seed[prefix+at];++retained;}
      }
     }
     x+=run;opaque=!opaque;
    }
   }
   if(bool(retained)!=outside)throw std::runtime_error("Independent undefined-tap classification differs");
   if(!outside&&expected!=originalExpected)throw std::runtime_error("In-plane original parity changed");
   pixels=seed;ownedPixels=pixels.data()+prefix;global(0x658174,reinterpret_cast<unsigned>(ownedPixels));args();
   const unsigned n=mnm_raster_native[site],f=mnm_raster_forwarded[site],rc=mnm_raster_retained_calls[site],rp=mnm_raster_retained_pixels[site];
   std::memcpy(reinterpret_cast<void*>(sites[site].at),patched.data(),sites[site].length);
   const auto after=call();
   if(pixels!=expected)throw std::runtime_error("Original/native-once output mismatch");
   bool abi=(before.gpr[0]&65535)==(after.gpr[0]&65535)&&before.error==after.error;
   for(unsigned i=3;i<8;++i)abi&=before.gpr[i]==after.gpr[i];
   if(!abi||((before.flags^after.flags)&0xcd5))throw std::runtime_error("Caller ABI/flags changed");
   for(unsigned i=0;i<512;++i)if(!(i>=6&&i<24)&&before.fx[i]!=after.fx[i])throw std::runtime_error("Caller FX changed");
   if(mnm_raster_native[site]!=n+1||mnm_raster_forwarded[site]!=f||mnm_raster_retained_calls[site]!=rc+unsigned(outside)||mnm_raster_retained_pixels[site]!=rp+retained)
    throw std::runtime_error("Expected exactly one native or original-once fallback");
   ++native;++native_geometry[geometry];if(outside){++retainedCases;retainedPixels+=retained;}
   for(unsigned i=0;i<prefix;++i)if(pixels[i]!=seed[i])throw std::runtime_error("Prefix guard changed");
   for(unsigned i=prefix+stride*height;i<pixels.size();++i)if(pixels[i]!=seed[i])throw std::runtime_error("Tail guard changed");
   ++cases;compared+=pixels.size();
  }
 const unsigned expected_forward=4*std::count_if(table.begin(),table.end(),[](unsigned v){return v>9;});
 if(cases!=192||!expected_forward||native!=192||forwarded||atomic!=expected_forward||retainedCases!=expected_forward||!retainedPixels||forward_geometry[1]||forward_geometry[2])
  throw std::runtime_error("Unexpected recovered phase/ownership boundary");
 std::ofstream out(argv[3]);out<<"{\"success\":true,\"cases\":"<<cases<<",\"native_cases\":"<<native
  <<",\"retained_cases\":"<<retainedCases<<",\"retained_pixels\":"<<retainedPixels<<",\"original_once_cases\":"<<forwarded<<",\"atomic_refusals\":"<<atomic<<",\"physical_words\":"<<compared
  <<",\"formats\":2,\"direction_flags\":2,\"phases\":16,\"live_acceptance\":false,\"sine_table\":[";
 for(unsigned i=0;i<16;++i){if(i)out<<',';out<<table[i];}out<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
