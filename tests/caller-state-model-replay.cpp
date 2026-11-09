// Publication replay against the separately original-compared state models.
// Owned source/history inputs only. This is not a new original raster proof.
#define main caller_state_unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "../runtime/scene/world_word_state.h"
#include "../runtime/scene/world_generic_state.h"
#include "../runtime/scene/world_rectangle_state.h"
#include "../runtime/scene/world_effect_prepare.h"
#include "../runtime/scene/drawing_caller_state_codec.h"
#include "../reconstruction/rendering/glyph_backend.h"
#include <array>
#include <algorithm>

static void need(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
static void receive(void* p,std::size_t n){need(bool(std::cin.read(static_cast<char*>(p),n)),"Truncated model request");}
static void send(const void* p,std::size_t n){need(bool(std::cout.write(static_cast<const char*>(p),n)),"Model response write");}
static void expandFx(const unsigned char* semantic,unsigned char* fx){
 std::memset(fx,0,512);std::memcpy(fx,semantic,5);std::memcpy(fx+24,semantic+8,4);
 for(unsigned i=0;i<8;++i)std::memcpy(fx+32+i*16,semantic+12+i*10,10);
 std::memcpy(fx+160,semantic+92,128);
}
static void knownGlyph(const MnmGlyphBackend& b,const std::vector<unsigned char>& known){
 if(b.result.branch!=MNM_GLYPH_ROWS)return;
 const auto* frame=b.state.frame;const int x=b.state.x-int(mnm_raster_word(frame+12)),y=b.state.y-int(mnm_raster_word(frame+16));
 const unsigned first=y<b.state.top?unsigned(b.state.top-y):0,last=first+b.result.visible_rows,w=mnm_raster_word(frame+4);
 for(unsigned row=first;row<last;++row){unsigned control=mnm_raster_word(frame+40+row*8),at=0,on=0;
  while(at<w){
   const unsigned count=frame[control++];
   if(on){
    for(unsigned i=0;i<count;++i){
     const auto offset=std::size_t(y+int(row))*b.width+unsigned(x)+at+i;
     need(offset<known.size()&&known[offset]==1,"Glyph consumes undefined independent history");
    }
   }
   at+=count;on=!on;
  }
 }
}
int main(int argc,char** argv)try{
 need(argc==2,"Expected pinned original initializer supplier");map_image(read(argv[1]));
 unsigned header[4];receive(header,16);need(!std::memcmp(header,"MNMCSR01",8)&&header[2]==1&&header[3]<=262144,"Model stream contract");
 unsigned responseHeader[4];std::memcpy(responseHeader,"MNMCSO01",8);responseHeader[2]=1;responseHeader[3]=header[3];send(responseHeader,16);std::cout.flush();
 for(unsigned ordinal=0;ordinal<header[3];++ordinal){
  unsigned packet[8];receive(packet,32);const auto total=packet[0],cpsBytes=packet[1],count=packet[2],knownCount=packet[3];
  need(total>=32+848+96&&total<=32*1024*1024&&cpsBytes>=96&&cpsBytes<=1048576+96+8704&&
       count<=4194304&&knownCount==count&&total==32+848+cpsBytes+count*3,"Model packet bounds");
  unsigned row[212],fields[24];receive(row,848);Bytes cps(cpsBytes);receive(cps.data(),cps.size());std::memcpy(fields,cps.data(),96);
  need(fields[0]==cpsBytes&&fields[18]==cpsBytes-96&&fields[22]==row[4],"Model source caller/extent");
  std::vector<std::uint16_t> baseline(count);std::vector<unsigned char> known(count);receive(baseline.data(),count*2);receive(known.data(),count);
  need(row[1]==1&&row[202]==1&&row[211]==fields[7]&&row[211]>=fields[5]&&row[211]<=4096,"Model state environment");
  need(fields[8]==(row[2]==26||row[2]==52?row[34]:row[2]==10||row[2]==18||row[2]==19||row[2]==20||row[2]==21||row[2]==25||row[2]>=57?row[21]:row[35]),"Model source X");
  unsigned saved[16];std::memcpy(saved,row+16,36);saved[9]=row[4];std::memcpy(saved+10,row+34,24);
  unsigned workspace[16];std::memcpy(workspace,row+46,64);
  alignas(16) unsigned char fx[512];expandFx(reinterpret_cast<unsigned char*>(row)+312,fx);
  const auto tag=row[2],width=fields[5],height=fields[6],stride=row[211];unsigned model=0,mask=packet[7];
  Bytes frame;if(fields[2]==8||fields[2]==9){need(fields[19]>=40&&cpsBytes==96+fields[19]+fields[20],"Model frame extent");frame.assign(cps.begin()+96,cps.begin()+96+fields[19]);}
  if(tag==22||tag==36){
   need(fields[2]==9&&!fields[17]&&!fields[20],"WORD model source");const unsigned marker=~0u;std::memcpy(frame.data()+28,&marker,4);
   MnmWordBackendInput input{frame.data(),frame.size(),saved[10],row[15],fields[12],fields[13],stride,tag==22?0x197086u:0x196cb8u,int(fields[10]),int(fields[11]),int(saved[11]),int(saved[12])};
   MnmWorldWordState state{};need(!mnm_world_word_prepare(&input,workspace,fx,&state),"WORD replay admission");mnm_world_word_commit(&state,workspace,saved,fx);model=1;
  }else if((tag>=12&&tag<=16)||tag==23||tag==24){
   need(fields[2]==9,"Generic model source");const unsigned family=tag==12?4:tag==23||tag==24?6:tag-13;
   MnmWorldGenericInput input{frame.data(),unsigned(frame.size()),saved[10],row[15],fields[12],fields[13],stride,family,fields[17],int(fields[10]),int(fields[11]),int(saved[11]),int(saved[12])};
   MnmWorldGenericState state{};need(!mnm_world_generic_prepare(&input,workspace,fx,&state),"Generic replay admission");mnm_world_generic_commit(&state,workspace,saved,fx);if(state.eax)mask|=0x10;model=2;
  }else if(tag==26){
   need(fields[2]==14&&cpsBytes==116,"Additive model source");MnmWorldAddInput input{int(u32(cps,96)),int(u32(cps,100)),int(saved[10]),int(saved[11]),int(fields[10]),int(fields[11]),int(fields[12]),int(fields[13]),stride};
   MnmWorldAddState state{};need(!mnm_world_add_prepare(&input,row[5],&state),"Additive replay admission");mnm_world_add_commit(&state,saved);model=3;
  }else if(tag==10){
   need(fields[2]==8&&fields[20]==272&&width&&height&&count==width*height&&stride<=2048,"Glyph model source/history");
   need(row[207]==0||row[207]==0x3f800000,"Glyph initializer sentinel");
   std::vector<std::uint16_t> pixels(std::size_t(stride)*height,0);for(unsigned y=0;y<height;++y)std::copy_n(baseline.data()+y*width,width,pixels.data()+y*stride);
   unsigned coverage[64];std::memcpy(coverage,cps.data()+96+fields[19]+16,256);alignas(16) unsigned char outgoing[512];
   MnmGlyphBackend b{};b.state={frame.data(),frame.size(),saved[6],0x6f5dac,row[5],saved[8],int(fields[10]),int(fields[11]),int(fields[12]),int(fields[13]),int(saved[5]),int(saved[10]),saved[11],saved[12],saved[13],unsigned(!row[207]),(fx[1]>>2)&3u};
   b.pixels=pixels.data();b.width=width;b.height=height;b.stride=stride;b.coverage=coverage;b.incoming_fx=fx;b.outgoing_fx=outgoing;b.format=row[210];
   need(!mnm_glyph_backend_state_format(&b.state,&b.result,b.format),"Glyph state admission");knownGlyph(b,known);
   need(!mnm_glyph_backend(&b),"Glyph replay admission");saved[7]=b.result.eax;saved[6]=b.result.ecx;saved[5]=b.result.edx;saved[8]=(saved[8]&~0xcd5u)|b.result.flags;
   std::memcpy(saved+10,b.result.arguments,16);std::memcpy(fx,outgoing,512);
   need(!std::memcmp(coverage,cps.data()+96+fields[19]+16,256),"Glyph emitted coverage differs");model=4;
  }else{
   need(tag==52||tag==17||tag==18||tag==19||tag==20||tag==21||tag==25||(tag>=57&&tag<=60),"Unknown state model");
   need(!(packet[4]&~0xffffu)&&!packet[5]&&!packet[6]&&!(packet[7]&~0x400u),"Unproved fast volatile obligation");
   if(tag==21&&row[205]==0x7d00){need(mnm_world_effect_fx_ready(fx),"Wave initializer FX admission");global(0x5f14d0,0x7d00);global(0x6c4830,row[208]);global(0x5e15f8,row[209]);mnm_world_effect_prepare(0x5806f0,0,fx);}
   if(tag==60&&row[206]==0x7d00){need(row[209]&&row[209]<=16&&mnm_world_effect_fx_ready(fx),"Source-row initializer FX admission");global(0x5f1450,0x7d00);global(0x6c4830,row[208]);global(0x5e15f8,row[209]);mnm_world_effect_prepare(0x580260,1,fx);}
   saved[7]=0;model=tag==52?6:5;
  }
  saved[3]=row[6];unsigned gprMask[8]={~0u,~0u,~0u,~0u,~0u,packet[6],packet[5],packet[4]};bool gpr=true;
  for(unsigned i=0;i<8;++i)gpr=gpr&&!((saved[i]^row[25+i])&gprMask[i]);
  const bool arguments=!(row[14]&1)||model==5||!std::memcmp(saved+10,row+40,row[11]*4);
  const bool words=!std::memcmp(workspace,row+62,64),flags=!((saved[8]^row[33])&mask);
  unsigned char semantic[224];mnm_caller_state_fx(semantic,fx);const bool floating=!std::memcmp(semantic,reinterpret_cast<unsigned char*>(row)+536,224);
  const bool error=row[12]==row[13];unsigned reply[8]={32+348,row[0],model,unsigned(gpr),unsigned(arguments),unsigned(words),unsigned(flags),unsigned(floating&&error)};
  send(reply,32);send(saved,36);send(saved+10,24);send(workspace,64);send(semantic,224);std::cout.flush();
  if(!(gpr&&arguments&&words&&flags&&floating&&error))std::cerr<<"state mismatch id="<<row[0]<<" tag="<<tag<<" gpr="<<gpr<<" args="<<arguments<<" workspace="<<words<<" flags="<<flags<<" FX="<<floating<<" LastError="<<error<<'\n';
 }
 need(std::cin.peek()==std::char_traits<char>::eof(),"Trailing model requests");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
