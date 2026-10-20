#include "../compat/legacy/canvas_world.hpp"
#include "../compat/legacy/world_raster_batch.hpp"
#include "resource-fixtures.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <iostream>
#include <algorithm>
using namespace mnm;using namespace resource_test;
static Bytes load(const char* path){QFile f(QString::fromLocal8Bit(path));require(f.open(QIODevice::ReadOnly),"Sprite input");const auto b=f.readAll();return {b.begin(),b.end()};}
static unsigned get(const Bytes& b,unsigned at){require(at+4<=b.size(),"Sprite word");return b[at]|unsigned(b[at+1])<<8|unsigned(b[at+2])<<16|unsigned(b[at+3])<<24;}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==4,"Expected actual CPS, batch request, original expected pixels");
 const auto wire=load(argv[1]),request=load(argv[2]),expected=load(argv[3]);auto stream=legacy::decodeCanvasProducers(wire,false);std::vector<const legacy::CanvasProducer*> rows;
 for(const auto& c:stream.operations){if(c.fields[2]==9)rows.push_back(&c);}
 require(rows.size()==15,"Sprite packet routes");
 const auto batch=legacy::validateWorldRasterBatch(request,get(request,16),1,get(request,24),99,800,600,0,rows);
 const unsigned entries[]={0x5947b2,0x59521a,0x57de00,0x597086,0x596cb8};
 QTemporaryDir directory;require(directory.isValid(),"Sprite asset root");const auto root=std::filesystem::path(directory.path().toStdString());const auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
 legacy::WorldFrame frame{};frame.width=frame.stride=800;frame.height=600;std::vector<Bytes> seen;
 for(unsigned i=0;i<rows.size();++i){const auto& c=*rows[i];require(batch.descriptors[i][1]==entries[i%5],"Actual entry order");require(c.fields[4]==0&&!c.payload.size()&&c.ownedPayload,"Source packet reference must resolve to owned bytes");
  auto d=legacy::producerWorldDraw(c);require(!legacy::worldRasterAX(c)&&d.x==int(1+i%5*3)&&d.y==int(i<5?1:i<10?5:8),"Sprite anchor/AX");
  const auto& p=c.bytes();require(get(p,28)==0,"Guest frame pointer escaped owned packet");Bytes encoded(p.begin(),p.begin()+c.fields[19]);const bool indexed=c.fields[17];word(encoded,28,indexed?0:0xffffffffu);
  if(std::find(seen.begin(),seen.end(),encoded)==seen.end()){seen.push_back(encoded);const unsigned base=28+(indexed?768:0);Bytes asset(base,0);word(asset,0,0x00525053);word(asset,8,4);word(asset,12,1);word(asset,16,indexed?1:0);word(asset,base-4,0);asset.insert(asset.end(),encoded.begin(),encoded.end());word(asset,4,asset.size());
   const auto name=indexed?"sprite-indexed.spr":"sprite-word.spr";write(root,name,asset);bindings.add({assets::ResourceKind::ui,indexed?"sprite/indexed":"sprite/word"},name,QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(asset.data()),asset.size()),QCryptographicHash::Sha256).toHex());}
  frame.draws.push_back(std::move(d));
 }
 require(seen.size()==2,"Expected two authored sources");render::GlBlitter renderer;render::Image zero{800,600,std::vector<std::uint32_t>(480000,0)};render::SceneRenderer scene(renderer,resources,zero);const auto reads=renderer.stats().nativeReadbacks;
 scene.draw(legacy::worldDisplay(frame,bindings));require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Ordinary sprite draw/present readback");const auto actual=scene.read();Bytes output;for(auto p:actual.pixels){output.push_back(p);output.push_back(p>>8);}require(output==expected,"Actual owned-packet native pixels differ from original");
 require(std::count_if(actual.pixels.begin(),actual.pixels.end(),[](auto p){return p!=0;})==60,"Empty or misplaced sprite output");
 std::cout<<"{\"success\":true,\"actual_owned_draws\":15,\"original_pixel_words\":480000,\"nonzero_words\":60,\"ordinary_readbacks\":0,\"canonical_batch_validated\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
