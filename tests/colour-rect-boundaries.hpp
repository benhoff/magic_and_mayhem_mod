// Independent original-wrapper/production-splice boundary comparisons.
// The original runs first from the same owned seed; no expected words enter
// the native route. Entire physical allocations and caller state are compared.
struct ColourBoundaries {
 unsigned cases=0,native=0,changed=0;
 std::uint64_t words=0;
 std::ostringstream rows;
};
static ColourBoundaries colourBoundaries(const unsigned char* original,const unsigned char* patched){
 ColourBoundaries result;
 const std::array<std::array<unsigned,2>,12> dimensions={{{1,1},{2,2},{63,64},{64,64},{65,1},{1,65},{65,65},{127,73},{800,600},{2048,1},{1,2048},{2048,2048}}};
 for(const auto& size:dimensions)for(unsigned format=0;format<2;++format)
 for(unsigned mode=0;mode<4;++mode)for(unsigned plane=0;plane<2;++plane)for(unsigned clipped=0;clipped<2;++clipped){
  const unsigned w=size[0],h=size[1],width=clipped?std::min(w+4,100u):std::min(w+4,2048u),height=clipped?std::min(h+4,84u):std::min(h+4,2048u),stride=width+7;
  std::vector<unsigned short> seed(stride*(height+2)+512);
  for(unsigned i=0;i<seed.size();++i)seed[i]=i*7919+result.cases*107;
  auto pixels=seed;ownedPixels=pixels.data()+256;ownedStride=stride;ownedHeight=height;
  unsigned char shape[22]={};std::memcpy(shape,&mode,4);std::memcpy(shape+4,&w,4);std::memcpy(shape+8,&h,4);
  std::vector<unsigned short> triples(w*h*3);
  for(unsigned i=0;i<w*h;++i){triples[i*3]=(i+17)%32;triples[i*3+1]=(i*13+result.cases*7)%(format?32:64);triples[i*3+2]=(i*29+11)%32;}
  std::memcpy(shape+12,triples.data(),6);const auto address=plane?reinterpret_cast<unsigned>(triples.data()):0;std::memcpy(shape+18,&address,4);
  const auto source=triples;
  global(0x658174,reinterpret_cast<unsigned>(ownedPixels));global(0x6a2dc8,stride);global(0x6e1f88,format);
  global(0x6e0008,clipped?1:0);global(0x6cbb6c,clipped?1:0);global(0x6a49b8,width-(clipped?1:0));global(0x656618,height-(clipped?1:0));
  pixel_function=0x547d80;pixel_object=reinterpret_cast<unsigned>(shape);pixel_argc=2;
  pixel_args[0]=clipped?unsigned(-int(w/3)):w==2048?0:2;pixel_args[1]=clipped?unsigned(-int(h/3)):h==2048?0:2;
  pixel_seed_flags=0x202|(result.cases&1?0x8d5:0);threadId=1;
  std::memcpy(reinterpret_cast<void*>(0x547e53),original,5);const auto before=call();const auto expected=pixels;
  pixels=seed;const auto count=mnm_colour_rect_native;
  std::memcpy(reinterpret_cast<void*>(0x547e53),patched,5);const auto after=call();
  if(pixels!=expected||triples!=source||before.gpr!=after.gpr||((before.flags^after.flags)&0xcd5)||before.error!=after.error||before.fx!=after.fx)
   throw std::runtime_error("Extended original colour boundary mismatch case="+std::to_string(result.cases));
  const bool native=mnm_colour_rect_native==count+1,changed=pixels!=seed;
  if(!clipped&&(!native||!changed))throw std::runtime_error("Full colour boundary did not draw natively");
  if(result.cases)result.rows<<',';
  result.rows<<"{\"format\":"<<format<<",\"mode\":"<<mode<<",\"plane\":"<<plane<<",\"width\":"<<w<<",\"height\":"<<h<<",\"clipped\":"<<clipped<<",\"native\":"<<(native?"true":"false")<<",\"changed\":"<<(changed?"true":"false")<<",\"physical_words\":"<<pixels.size()<<'}';
  ++result.cases;result.native+=native;result.changed+=changed;result.words+=pixels.size();
 }
 return result;
}
