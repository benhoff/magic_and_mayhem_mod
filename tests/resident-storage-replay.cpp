#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../compat/legacy/canvas_world.hpp"
#include "../renderer/gpu_canvas_mirror.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtEndian>
#include <set>
#include <iostream>
using namespace mnm;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
QByteArray read(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly),"Input unavailable");require(f.size()<=16ll*1024*1024*1024,"Input too large");return f.readAll();}
void save(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(bytes)==bytes.size(),"New checkpoint output");}
void put(QByteArray& b,unsigned value){char v[4];qToLittleEndian<quint32>(value,v);b.append(v,4);}
QByteArray pixels(const render::Image& image){QByteArray out;out.reserve(image.pixels.size()*2);for(auto p:image.pixels){out.append(char(p));out.append(char(p>>8));}return out;}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==5,"Expected capture,asset root,checkpoint list,output directory");
 const auto requested=QJsonDocument::fromJson(read(argv[3])).array();
 std::set<quint64> selected;for(const auto& c:requested)selected.insert(c.toObject()["source_generation"].toString().toULongLong());
 require((requested.size()==3||requested.size()==4)&&selected.size()==unsigned(requested.size())&&*selected.begin()>1,"Three or four distinct World checkpoints required");
 auto opened=assets::AssetStore::create(argv[2]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
 const auto store=std::get<assets::AssetStore>(std::move(opened));QDir out(argv[4]);require(out.mkpath("."),"Output directory");
 QFile input(argv[1]);require(input.open(QIODevice::ReadOnly),"Resident capture input");const auto magic=input.read(8);const bool sources=magic=="MNMPROF3";require(sources||magic=="MNMPROF2","Compressed resident capture");
 quint64 chunks=0,blocks=0;
 render::GlBlitter renderer;quint64 generation=0;QJsonArray checkpoints;
 {
 render::GpuCanvasMirror mirror(renderer);
 legacy::CanvasProducerReplay producer([store](const auto& name){auto opened=store.open(name);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);auto f=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto bytes=assets::readWhole(*f,16*1024*1024);if(auto* e=std::get_if<assets::Error>(&bytes))throw std::runtime_error(e->detail);return std::get<std::vector<std::uint8_t>>(std::move(bytes));},&mirror);
 legacy::CanvasWorld world(renderer,store);
 while(generation<*selected.rbegin()){
  const auto h=input.read(sources?32:20);require(h.size()==(sources?32:20),"Truncated source header");
  const auto g=qFromLittleEndian<quint64>(h.constData());const auto chunk=sources?qFromLittleEndian<quint64>(h.constData()+8):0;const unsigned offset=sources?16:8;const auto s=qFromLittleEndian<quint32>(h.constData()+offset),r=qFromLittleEndian<quint32>(h.constData()+offset+4),n=qFromLittleEndian<quint32>(h.constData()+offset+8);
  require(!sources||!qFromLittleEndian<quint32>(h.constData()+28),"Reserved capture header");
  require(g==generation+1&&g<=8192&&++blocks<=8192&&s>=16&&s<=16777216&&(chunk?r==0:r>=64&&r<=197184)&&n>4&&n<=s+r+(s+r)/1000+128,"Source bounds/order");
  const auto packed=input.read(n);require(packed.size()==n&&qFromBigEndian<quint32>(packed.constData())==s+r,"Compressed extent");const auto data=qUncompress(packed);require(data.size()==s+r,"Source inflation");
  auto bytes=[&](unsigned at,unsigned size){return std::vector<std::uint8_t>(data.begin()+at,data.begin()+at+size);};
  const auto stream=legacy::decodeCanvasProducers(bytes(0,s),false);
  if(chunk){
   require(chunk==++chunks&&!world.active()&&stream.version==3&&!stream.operations.empty(),"Source capture ordering");
   for(const auto& c:stream.operations)require(c.fields[2]!=11&&c.fields[2]!=12&&c.fields[2]!=13&&c.fields[2]!=27,"World/failure in captured source prefix");
   renderer.batch([&]{for(const auto& c:stream.operations)producer.apply(c);producer.publishWrites();});continue;
  }
  ++generation;auto handoff=legacy::prepareRollingHandoff(stream,0,bytes(s,r),1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
  const bool capture=selected.count(g);QByteArray draws("MNMSCHK1",8);put(draws,800);put(draws,600);put(draws,0);unsigned count=0,raster=0;QString base=QString("world-%1").arg(g);render::SurfaceId surface=0;
  for(const auto& c:handoff.stream.operations){const auto& f=c.fields;
   if(!world.active()||(f[2]!=9&&f[2]!=14&&f[2]!=26))producer.apply(c);
   if(f[2]==11){producer.publishWrites();surface=mirror.surface(f[3]);if(capture)save(out.filePath(base+"-before.565"),pixels(renderer.read(surface)));producer.discardCpuWorld(f[3]);world.beginResident(c,surface,g);}
   else if(world.active()){
    if(f[2]==9||f[2]==14||f[2]==26){
     world.append(c);if(capture){put(draws,f[2]==9?handoff.batch.descriptors.at(raster)[1]:0);auto fields=f;fields[0]=96+c.bytes().size();fields[18]=c.bytes().size();for(auto v:fields)put(draws,v);draws.append(reinterpret_cast<const char*>(c.bytes().data()),c.bytes().size());++count;}if(f[2]==9)++raster;
    }else if(f[2]==10&&f[3]==world.canvas()){world.completeResident();if(capture)save(out.filePath(base+"-native.565"),pixels(renderer.read(surface)));}
    else if(f[2]==12)world.end(c);
   }
  }
  require(!world.active(),"Unclosed World");
  if(capture){qToLittleEndian<quint32>(count,draws.data()+16);save(out.filePath(base+"-draws.bin"),draws);checkpoints.append(QJsonObject{{"generation",QString::number(g)},{"draws",int(count)},{"rasters",int(raster)},{"canvas",int(world.canvas())},{"base",base}});}
 }
 }
 require(!renderer.stats().surfaces&&checkpoints.size()==int(selected.size()),"Checkpoint/retirement proof");
 save(out.filePath("replay.json"),QJsonDocument(QJsonObject{{"success",true},{"worlds",QString::number(generation)},{"source_chunks",QString::number(chunks)},{"checkpoints",checkpoints},{"remaining_surfaces",0},{"original_oracles_read",false},{"scope","Source-only resident World replay and diagnostic entry/final canvases; no presented whole-session equivalence"}}).toJson());return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
