// Source-only CPU/GPU histories. Original oracle planes have no input path.
#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QtEndian>
#include <cstdlib>
#include <iostream>
using namespace mnm;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void save(const QString& name,const QByteArray& data){QFile f(name);require(f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(data)==data.size(),"New diagnostic output");}
QByteArray pixels(const render::Image& image){QByteArray b;for(auto p:image.pixels){require(p<=65535,"RGB565 words");b.append(char(p));b.append(char(p>>8));}return b;}
void put(QByteArray& b,unsigned value){char raw[4];qToLittleEndian<quint32>(value,raw);b.append(raw,4);}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==4,"Expected source profile, asset root, new output directory");
 auto opened=assets::AssetStore::create(argv[2]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
 const auto store=std::get<assets::AssetStore>(std::move(opened));
 auto source=[store](const auto& name){auto file=store.open(name);if(auto* e=std::get_if<assets::Error>(&file))throw std::runtime_error(e->detail);auto bytes=assets::readWhole(*std::get<std::unique_ptr<assets::AssetFile>>(file),16*1024*1024);if(auto* e=std::get_if<assets::Error>(&bytes))throw std::runtime_error(e->detail);return std::get<std::vector<std::uint8_t>>(std::move(bytes));};
 QDir out(argv[3]);require(!out.exists()&&QDir().mkpath(out.path()),"New output directory");
 QFile input(argv[1]);require(input.open(QIODevice::ReadOnly)&&input.size()<=2ll*1024*1024*1024&&input.read(8)=="MNMPROF3","Bounded complete-source profile");
 render::GlBlitter renderer;quint64 generation=0,chunks=0,blocks=0;QJsonArray worlds,trace;
 const auto* selectedText=std::getenv("MNM_DIVERGENCE_CAPTURE_DRAW");const auto selected=selectedText?std::strtoul(selectedText,nullptr,10):0;
 {
 legacy::CanvasProducerReplay cpu(source);
 legacy::WorldRollingSession gpu(renderer,store,false,true);
 while(!input.atEnd()){
  auto h=input.read(32);require(h.size()==32,"Profile header");
  auto g=qFromLittleEndian<quint64>(h.constData()),chunk=qFromLittleEndian<quint64>(h.constData()+8);
  auto s=qFromLittleEndian<quint32>(h.constData()+16),r=qFromLittleEndian<quint32>(h.constData()+20),n=qFromLittleEndian<quint32>(h.constData()+24);
  require(!qFromLittleEndian<quint32>(h.constData()+28)&&g==generation+1&&g<=8192&&++blocks<=8192&&s>=16&&s<=16777216&&(chunk?r==0:r>=64&&r<=197184)&&n>4&&n<=s+r+(s+r)/1000+128,"Profile identity/bounds");
  auto packed=input.read(n);require(packed.size()==n&&qFromBigEndian<quint32>(packed.constData())==s+r,"Compressed extent");
  auto data=qUncompress(packed);require(data.size()==s+r,"Profile inflation");
  auto bytes=[&](unsigned at,unsigned size){return std::vector<std::uint8_t>(data.begin()+at,data.begin()+at+size);};
  auto stream=legacy::decodeCanvasProducers(bytes(0,s),false);
  if(chunk){require(chunk==++chunks,"Source chunk order");gpu.consumeSource(stream);for(const auto& c:stream.operations)cpu.apply(c);continue;}
  ++generation;auto handoff=legacy::prepareRollingHandoff(stream,0,bytes(s,r),1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
  unsigned canvas=0,draw=0,raster=0;bool active=false;QByteArray draws("MNMSCHK1",8);
  for(const auto& c:handoff.stream.operations){
   const auto& f=c.fields;const bool traced=g==50&&active&&(f[2]==9||f[2]==14||f[2]==26);
   if(traced&&selected==draw+1)save(out.filePath("selected-native-before.565"),pixels(cpu.read(canvas)));
   cpu.apply(c);
   if(f[2]==11){canvas=f[3];active=true;if(g==50){save(out.filePath("world-50-cpu-before.565"),pixels(cpu.read(canvas)));put(draws,f[5]);put(draws,f[6]);put(draws,0);}}
   if(traced){
    ++draw;const auto entry=f[2]==9?handoff.batch.descriptors.at(raster)[1]:0;put(draws,entry);
    auto fields=f;fields[0]=96+c.bytes().size();fields[18]=c.bytes().size();for(auto value:fields)put(draws,value);draws.append(reinterpret_cast<const char*>(c.bytes().data()),c.bytes().size());
    const auto image=pixels(cpu.read(canvas));trace.append(QJsonObject{{"ordinal",int(draw)},{"record",int(f[1])},{"entry",int(entry)},{"kind",int(f[2])},{"mode",int(f[14])},{"backend",int(f[15])},{"sha256",QString::fromLatin1(QCryptographicHash::hash(image,QCryptographicHash::Sha256).toHex())}});
    if(selected==draw)save(out.filePath("selected-native-after.565"),image);
   }
   if(active&&f[2]==9)++raster;
   if(f[2]==12)active=false;
  }
  if(g==50){qToLittleEndian<quint32>(draw,draws.data()+16);save(out.filePath("world-50-draws.bin"),draws);}
  require(canvas!=0,"World canvas");
  auto reads=renderer.stats().nativeReadbacks;gpu.consumeResident(g,handoff.stream);
  require(renderer.stats().nativeReadbacks==reads&&!gpu.worldReadbacks(),"Ordinary delivery readback");
  auto actual=gpu.diagnosticReadResident(renderer);auto expected=cpu.read(canvas);
  require(actual.width==expected.width&&actual.height==expected.height,"CPU/GPU extent");
  auto stem=QString("world-%1").arg(g);save(out.filePath(stem+"-cpu.565"),pixels(expected));save(out.filePath(stem+"-gpu.565"),pixels(actual));
  worlds.append(QJsonObject{{"generation",int(g)},{"canvas",int(canvas)},{"width",actual.width},{"height",actual.height},{"cpu_gpu_equal",actual.pixels==expected.pixels}});
 }
 }
 require(generation>0&&!renderer.stats().surfaces,"Completed bounded history/resource retirement");
 save(out.filePath("native.json"),QJsonDocument(QJsonObject{{"success",true},{"worlds",worlds},{"trace",trace},{"source_chunks",int(chunks)},{"ordinary_delivery_readbacks",0},{"original_oracles_read",false},{"remaining_surfaces",0}}).toJson());
 std::cout<<generation<<" source-only CPU/GPU World comparisons\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
