#include "../compat/legacy/canvas_world.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
namespace {
const render::PixelFormat rgb{16,{0xf800,0x7e0,31}};
std::uint16_t half(std::uint16_t a,std::uint16_t b){return ((a&0xf7de)>>1)+((b&0xf7de)>>1);}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir tmp;require(tmp.isValid(),"Fixture root");
 const auto root=std::filesystem::path(tmp.path().toStdString());populate(root);
 render::GlBlitter g;std::uint64_t checks=0,frames=0,repeats=0;
 // Each service is constructed after its explicit native route selection.
 for(unsigned replay=0;replay<3;++replay){
  qputenv("MNM_NATIVE_ALTERNATE_CANVAS","1");
  {
   legacy::CanvasWorld w(g,store(root));if(replay)w.enablePreparedReplay(replay==2);
   std::vector<render::SurfaceId> storage;
   render::GpuFrame oldFrame;render::Image oldPixels;
   for(unsigned step=0;step<6;++step){
    const int width=(step==2||step==3)?800:640,height=width==800?600:480;
    render::Image history{width,height,{}};history.pixels.resize(std::size_t(width)*height);
    for(std::size_t i=0;i<history.pixels.size();++i)history.pixels[i]=(i*73+step*101)&65535;
    const auto ordinary=g.create(history,rgb);storage.push_back(ordinary);
    legacy::CanvasProducer entry{};auto& e=entry.fields;e[2]=11;e[3]=step==3?9:7;e[5]=width;e[6]=height;e[14]=1;
    const auto reads=g.stats().nativeReadbacks,rgba=g.stats().rgbaReadbacks;
    w.beginResident(entry,ordinary,step+1);
    legacy::CanvasProducer draw{};auto& d=draw.fields;d[2]=26;d[3]=e[3];d[5]=width;d[6]=height;d[8]=width-2;d[9]=height-2;d[12]=width;d[13]=height;d[15]=2;d[19]=2;d[20]=2;draw.payload={0,248,0,248,0,248,0,248};
    w.append(draw);w.completeResident();
    // A second subqueue must use the alternate's current pixels. Its native
    // copy-back cannot re-seed history or reapply an earlier batch.
    if(step==1){w.append(draw);w.completeResident();}
    auto returned=entry;returned.fields[2]=12;w.end(returned);++frames;
    auto expected=history.pixels;
    for(auto i:{std::size_t(height-2)*width+width-2,std::size_t(height-2)*width+width-1,std::size_t(height-1)*width+width-2,std::size_t(height)*width-1}){
     expected[i]=half(0xf800,std::uint16_t(expected[i]));if(step==1)expected[i]=half(0xf800,std::uint16_t(expected[i]));
    }
    require(g.stats().nativeReadbacks==reads&&g.stats().rgbaReadbacks==rgba,"Alternate route added a frame readback");
    require(g.read(ordinary).pixels==expected,"Alternate copy-back lost native history or draw ordering");checks+=expected.size();
    if(oldFrame.valid()){
     const auto copy=g.allocate(oldPixels.width,oldPixels.height,rgb);g.importGpu565(copy,oldFrame);
     require(g.read(copy).pixels==oldPixels.pixels,"Retired alternate changed an exported ordinary lease");g.destroy(copy);checks+=oldPixels.pixels.size();
    }
    if(replay)for(unsigned n=0;n<4;++n){
     g.update(ordinary,0,0,history);const auto before=g.stats().nativeReadbacks;
     require(w.replayPrepared().valid(),"Alternate prepared replay missing");
     require(g.stats().nativeReadbacks==before,"Alternate prepared replay read back");
     require(g.read(ordinary).pixels==expected,"Alternate replay accumulated or used stale history");checks+=expected.size();++repeats;
    }
    oldFrame=w.presentGpu();oldPixels={width,height,expected};
   }
   require(w.alternateStats().publications==6&&w.alternateStats().frames==6,"Alternate generation accounting");
   for(auto id:storage)g.destroy(id);
  }
  require(g.stats().surfaces==0,"Alternate teardown leaked surfaces");
 }
 // Busy, uninitialized/wrong-extent history and unsupported video refuse.
 {
  rejected([&]{render::GpuAlternateCanvas bad(g,736,480);});
  render::GpuAlternateCanvas owner(g,640,480);
  auto unknown=g.allocate(640,480,rgb),wrong=g.create({800,600,std::vector<std::uint32_t>(480000,0)},rgb);
  rejected([&]{owner.begin(unknown);});rejected([&]{owner.begin(wrong);});
  rejected([&]{owner.copyBack(unknown);});rejected([&]{owner.end();});
  g.update(unknown,0,0,{640,480,std::vector<std::uint32_t>(307200,0x1234)});owner.begin(unknown);
  rejected([&]{owner.begin(unknown);});owner.copyBack(unknown);owner.end();
  g.destroy(unknown);g.destroy(wrong);
 }
 require(g.stats().surfaces==0,"Admission controls leaked surfaces");
 // A malformed source can abandon an active frame. Destruction must release
 // its lease and GPU plane without copying partial World pixels to ordinary.
 {
  const auto ordinary=g.create({640,480,std::vector<std::uint32_t>(307200,31)},rgb);
  {
   legacy::CanvasWorld failed(g,store(root));legacy::CanvasProducer e{};
   e.fields[2]=11;e.fields[3]=7;e.fields[5]=640;e.fields[6]=480;e.fields[14]=1;
   failed.beginResident(e,ordinary,1);auto bad=e;bad.fields[2]=26;bad.fields[3]=8;
   rejected([&]{failed.append(bad);});
  }
  require(g.read(ordinary).pixels==std::vector<std::uint32_t>(307200,31),"Abandoned alternate frame published pixels");
  g.destroy(ordinary);
 }
 require(g.stats().surfaces==0,"Abandoned frame leaked native storage");
 qunsetenv("MNM_NATIVE_ALTERNATE_CANVAS");
 std::cout<<"{\"frames\":"<<frames<<",\"prepared_replays\":"<<repeats<<",\"pixel_checks\":"<<checks<<",\"frame_readbacks\":0,\"remaining_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
