#include "../compat/legacy/canvas_world.hpp"
#include "../compat/legacy/world_raster_batch.hpp"
#include "resource-fixtures.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <algorithm>
#include <iostream>
using namespace mnm;using namespace resource_test;
static Bytes load(const char* path){QFile f(QString::fromLocal8Bit(path));require(f.open(QIODevice::ReadOnly),"Secondary input");const auto b=f.readAll();return{b.begin(),b.end()};}
static unsigned get(const Bytes& b,unsigned at){require(at+4<=b.size(),"Secondary word");return b[at]|unsigned(b[at+1])<<8|unsigned(b[at+2])<<16|unsigned(b[at+3])<<24;}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==3,"Expected actual secondary CPS and batch");
 const auto wire=load(argv[1]),request=load(argv[2]);auto stream=legacy::decodeCanvasProducers(wire,false);std::vector<const legacy::CanvasProducer*> rows;
 for(const auto& c:stream.operations){if(c.fields[2]==9)rows.push_back(&c);}
 require(rows.size()==5,"Secondary packet count");
 const auto batch=legacy::validateWorldRasterBatch(request,get(request,16),1,get(request,24),99,800,600,0,rows);
 const unsigned entries[]={0x595677,0x595677,0x57de00,0x595677,0x57de00},callers[]={0x500a16,0x500a16,0x500a32,0x500a16,0x500a32},returns[]={0,1,0,1,0};const int xs[]={5,-1,-1,799,799};
 QTemporaryDir directory;require(directory.isValid(),"Secondary asset root");const auto root=std::filesystem::path(directory.path().toStdString());const auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
 legacy::WorldFrame frame{};frame.width=frame.stride=800;frame.height=600;Bytes identity;
 for(unsigned i=0;i<rows.size();++i){const auto& c=*rows[i];require(batch.descriptors[i][1]==entries[i]&&c.fields[22]==callers[i]&&legacy::worldRasterAX(c)==returns[i],"Secondary routing/AX");
  auto d=legacy::producerWorldDraw(c);require(d.x==xs[i]&&d.y==2,"Secondary anchor");const auto& p=c.bytes();require(get(p,28)==0&&c.fields[17]==1,"Secondary owned indexed source");Bytes encoded(p.begin(),p.begin()+c.fields[19]);
  if(!i){identity=encoded;Bytes asset(796,0);word(asset,0,0x00525053);word(asset,8,4);word(asset,12,1);word(asset,16,1);asset.insert(asset.end(),encoded.begin(),encoded.end());word(asset,4,asset.size());write(root,"secondary.spr",asset);bindings.add({assets::ResourceKind::ui,"secondary"},"secondary.spr",QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(asset.data()),asset.size()),QCryptographicHash::Sha256).toHex());}
  require(encoded==identity,"Changed secondary frame");frame.draws.push_back(std::move(d));
 }
 render::GlBlitter renderer;render::Image base{800,600,std::vector<std::uint32_t>(480000,0x4321)};auto expected=base;
 for(int y:{2,3})for(int x:{0,5,6,799})expected.pixels[y*800+x]=0;
 render::SceneRenderer scene(renderer,resources,base);const auto reads=renderer.stats().nativeReadbacks;
 scene.draw(legacy::worldDisplay(frame,bindings));require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Secondary ordinary readback");require(scene.read().pixels==expected.pixels,"Secondary full canvas differs from authored scalar image");
 std::cout<<"{\"success\":true,\"actual_owned_draws\":5,\"compared_pixel_words\":480000,\"changed_words\":8,\"ordinary_readbacks\":0,\"canonical_batch_validated\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
