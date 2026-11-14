// Native outputs are compared with retained independent original results.
// Original/native destination pixels are never supplied to the native renderer.
#include "../compat/legacy/world_rolling_session.hpp"
#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../apps/qt-shell/profile_capture_codec.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtEndian>
#include <cstdio>
using namespace mnm;
namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
QByteArray bytes(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly)&&f.size()<32*1024*1024,"Bounded retained reference");return f.readAll();}
}
int main(int argc,char** argv){
 QJsonObject report{{"original_game_runs",0},{"original_reference_executions",0},{"original_pixels_used_as_native_inputs",false}};
 try{
  require(argc==5,"Expected retained source corpus, exact assets, reference directory and new output directory");
  QGuiApplication app(argc,argv);QDir reference(argv[3]),output(argv[4]);require(!output.exists()&&QDir().mkpath(output.path()),"New bounded native replay output required");
  const auto index=QJsonDocument::fromJson(bytes(reference.filePath("worlds.json"))).object();require(index["success"].toBool()&&!index["native_player_pixels_read"].toBool()&&!index["original_destination_capture_read"].toBool(),"Independent retained reference required");
  auto opened=assets::AssetStore::create(argv[2]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
  const auto store=std::get<assets::AssetStore>(std::move(opened));render::GlBlitter renderer;
  legacy::WorldRollingSession native(renderer,store,false,true);
  QFile input(argv[1]);require(input.open(QIODevice::ReadOnly)&&input.read(8)=="MNMPROF4","Exact current owned source corpus required");profile::Decoder codec;
  quint64 generation=0,chunks=0;QJsonArray mismatches;quint64 pixels=0;
  while(!input.atEnd()&&generation<433){
   auto header=input.read(profile::HeaderSize);require(header.size()==profile::HeaderSize,"Complete source header");
   const auto g=qFromLittleEndian<quint64>(header.constData()),chunk=qFromLittleEndian<quint64>(header.constData()+8);
   const auto s=qFromLittleEndian<quint32>(header.constData()+16),r=qFromLittleEndian<quint32>(header.constData()+20),n=qFromLittleEndian<quint32>(header.constData()+24);
   require(g==generation+1&&s>=16&&s<=16777216&&(chunk?r==0:r>=64&&r<=197184)&&n>4&&n<=s+r+(s+r)/128+1024,"Ordered bounded source record");
   const auto packed=input.read(n);require(packed.size()==n,"Complete source body");const auto data=codec.decode(header,packed,s+r);
   const auto part=[&](unsigned offset,unsigned size){return std::vector<std::uint8_t>(data.begin()+offset,data.begin()+offset+size);};
   const auto source=legacy::decodeCanvasProducers(part(0,s),false);
   if(chunk){require(chunk==++chunks,"Source chunk order");native.consumeSource(source);continue;}
   ++generation;const auto handoff=legacy::prepareRollingHandoff(source,0,part(s,r),1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
   const auto frame=native.consumeResident(generation,handoff.stream);require(frame.valid(),"Native replay frame missing");
   const auto actual=native.diagnosticReadResident(renderer);
   const auto row=index["worlds"].toArray()[int(generation-1)].toObject();require(row["generation"].toString().toULongLong()==generation,"Reference generation order");
   const auto original=qUncompress(bytes(reference.filePath(row["path"].toString())));require(original.size()==qsizetype(actual.pixels.size()*2),"Complete original/native plane dimensions");
   unsigned count=0;QJsonArray examples;
   for(std::size_t i=0;i<actual.pixels.size();++i){const auto expected=qFromLittleEndian<quint16>(original.constData()+2*i);if(actual.pixels[i]!=expected){++count;if(examples.size()<8)examples.append(QJsonObject{{"x",int(i%actual.width)},{"y",int(i/actual.width)},{"actual",int(actual.pixels[i])},{"expected",int(expected)}});}}
   pixels+=actual.pixels.size();if(count)mismatches.append(QJsonObject{{"generation",QString::number(generation)},{"count",int(count)},{"examples",examples}});
   if(generation==432){QByteArray raw;raw.reserve(int(actual.pixels.size()*2));for(auto value:actual.pixels){raw.append(char(value));raw.append(char(value>>8));}QFile f(output.filePath("world-432-native.565"));require(f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(raw)==raw.size(),"Retain actual target native plane");}
  }
  const auto driver=renderer.driver();report["success"]=true;report["worlds"]=QString::number(generation);report["source_chunks"]=QString::number(chunks);report["pixels_compared"]=QString::number(pixels);report["mismatching_worlds"]=mismatches;report["renderer"]=QString::fromStdString(driver.renderer);report["vendor"]=QString::fromStdString(driver.vendor);
  std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 0;
 }catch(const std::exception& e){report["success"]=false;report["error"]=e.what();std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 1;}
}
