#include "../reconstruction/rendering/startup_canvas.hpp"
#include "../reconstruction/rendering/generic_producer_caller.h"
#include "../compat/legacy/canvas_producers.hpp"
#include "rendering-wave-fixture.hpp"
#include "../assets/sprite_run_clipping.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

static void check(bool b){if(!b)throw std::runtime_error("Startup native contract assertion");}
template<class F>static void refuses(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected);}
int main(int argc,char** argv)try{
  std::ofstream pixelFile;
  if(argc==3 && std::string(argv[1])=="--wave-pixels")pixelFile.open(argv[2],std::ios::binary);
  else check(argc==1);
  check(argc==1||bool(pixelFile));
  unsigned guards=0;
  const unsigned char valid[]={0x83,0xc4,0x14,0x66,0x3d,1,0,0x0f,0x85,0x29};
  check(mnm_generic_producer_caller(0x500512,13,valid));++guards;
  for(unsigned byte=0;byte<10;++byte){auto changed=std::array<unsigned char,10>{};std::copy_n(valid,10,changed.begin());changed[byte]^=1;check(!mnm_generic_producer_caller(0x500512,13,changed.data()));++guards;}
  for(unsigned caller:{0u,0x500511u,0x500513u,0x57e8fbu,0xffffffffu}){check(!mnm_generic_producer_caller(caller,13,valid));++guards;}
  for(unsigned tag:{0u,12u,15u,64u}){check(!mnm_generic_producer_caller(0x500512,tag,valid));++guards;}
  check(!mnm_generic_producer_caller(0x500512,13,nullptr));++guards;
  using namespace mnm;
  refuses([]{reconstruction::paddedStartupBinding(false,-1,0);});
  const auto table=reconstruction::startupWaveTable();
  refuses([&]{reconstruction::startupWaveOffsets(table,0,0);});refuses([&]{reconstruction::startupWaveOffsets(table,9,0);});
  unsigned offsets=0;
  for(unsigned a=1;a<=8;++a)for(unsigned phase=0;phase<32;++phase){const auto rows=reconstruction::startupWaveOffsets(table,a,phase);for(unsigned y=0;y<16;++y){check(rows[y]==table[a*16+(y+phase)%(2*a)]);++offsets;}}
  render::CanvasSequence canvas;
  canvas.create(1,7,5);refuses([&]{canvas.read(1);});
  canvas.fill(1,{0,0,7,5},0);for(auto p:canvas.read(1).pixels)check(p==0);
  canvas.fill(1,{0,0,7,5},0x5678);for(auto p:canvas.read(1).pixels)check(p==0x5678);
  canvas.release(1);refuses([&]{canvas.read(1);});canvas.create(1,7,5);refuses([&]{canvas.read(1);});
  canvas.create(2,7,5);refuses([&]{canvas.copy(1,2,{0,0,7,5},0,0);});
  bool requestedMissing=false;
  legacy::CanvasProducerReplay missing([&](const std::string&)->std::vector<unsigned char>{requestedMissing=true;throw std::runtime_error("Missing owned encoded asset");});
  legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=1;create.fields[5]=7;create.fields[6]=5;missing.apply(create);
  legacy::CanvasProducer fill{};fill.fields[2]=5;fill.fields[3]=99;fill.fields[12]=7;fill.fields[13]=5;
  refuses([&]{missing.apply(fill);}); // A writer cannot precede its generation.
  refuses([&]{missing.apply(create);}); // A live identity cannot be silently recreated.
  legacy::CanvasProducer jpeg{};jpeg.fields[2]=7;jpeg.fields[3]=1;jpeg.payload={'m','i','s','s','i','n','g',0};refuses([&]{missing.apply(jpeg);});check(requestedMissing);
  std::uint64_t pixels=0;unsigned uniformCases=0;
  for(int x:{-8,-1,0,3,12,21})for(int y:{-4,0,2,13,17})for(int w:{1,4,11})for(int h:{1,5}){
    render::CanvasSequence c;c.create(1,21,17);c.fill(1,{0,0,21,17},0x1234);
    const render::Rect clipped{std::max(2,x),std::max(1,y),std::min(19,x+w),std::min(16,y+h)};
    if(clipped.left<clipped.right&&clipped.top<clipped.bottom)c.fill(1,clipped,std::uint16_t((23<<11)|(45<<5)|17));
    const auto image=c.read(1);
    for(int row=0;row<17;++row)for(int col=0;col<21;++col){const bool hit=col>=2&&col<19&&row>=1&&row<16&&col>=x&&col<x+w&&row>=y&&row<y+h;check(image.pixels[row*21+col]==(hit?((23<<11)|(45<<5)|17):0x1234));++pixels;}
    ++uniformCases;
  }
  std::vector<std::uint32_t> waveHashes;
  {
    assets::SpriteLimits limits;limits.decodedBytes=233;
    check(std::holds_alternative<assets::SpriteError>(assets::decodeSprite(wave_fixture::sprite(),limits)));
    limits.decodedBytes=234;
    auto decoded=assets::decodeSprite(wave_fixture::sprite(),limits);check(std::holds_alternative<assets::Sprite>(decoded));
    auto f=std::get<assets::Sprite>(decoded).frames.at(0);
    f.opaqueRuns.clear();refuses([&]{assets::clipWholeOpaqueRuns(f,2,40);});
    f=std::get<assets::Sprite>(decoded).frames.at(0);f.opaqueRuns.push_back(f.opaqueRuns[0]);
    refuses([&]{assets::clipWholeOpaqueRuns(f,2,40);});
    f=std::get<assets::Sprite>(decoded).frames.at(0);f.opaqueMask[0]=0;
    refuses([&]{assets::clipWholeOpaqueRuns(f,2,40);});
  }
  wave_fixture::cases([&](unsigned amplitude,unsigned,unsigned phase,int x,int y,unsigned split){
    auto decoded=assets::decodeSprite(wave_fixture::sprite(split));check(std::holds_alternative<assets::Sprite>(decoded));
    const auto& f=std::get<assets::Sprite>(decoded).frames.at(0);
    check(f.opaqueRuns.size()==(split?18u:9u));
    render::Image initial{64,32,std::vector<std::uint32_t>(64*32)};wave_fixture::pattern(initial.pixels);
    render::CanvasSequence c;c.create(1,64,32);c.update(1,0,0,initial);
    auto shifts=reconstruction::startupWaveOffsets(table,amplitude,phase);unsigned period=2*amplitude;
    if(x<2||x+7>40){period=16;for(unsigned row=0;row<16;++row)shifts[row]=std::abs(8-int((row+phase)&15));}
    c.raster(1,f,x,y,{2,2,40,25},3,7,{},shifts,period);
    const auto completed=c.read(1);waveHashes.push_back(wave_fixture::hash(completed.pixels));
    if(pixelFile.is_open())wave_fixture::write(pixelFile,completed.pixels);
  });
  std::cout<<"{\"success\":true,\"caller_guard_controls\":"<<guards<<",\"effective_offsets\":"<<offsets<<",\"uniform_cases\":"<<uniformCases<<",\"uniform_words\":"<<pixels<<",\"undefined_missing_retired_inputs_refused\":true,\"wave_table\":[";
  for(unsigned i=0;i<256;++i){if(i)std::cout<<',';std::cout<<table[i];}std::cout<<"],\"wave_pixel_hashes\":[";
  for(unsigned i=0;i<waveHashes.size();++i){if(i)std::cout<<',';std::cout<<waveHashes[i];}std::cout<<"]}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
