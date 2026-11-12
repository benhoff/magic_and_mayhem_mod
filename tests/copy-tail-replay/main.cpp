// Offline source-only replay. There is no original/native oracle input path.
#include "../../renderer/commands.hpp"
#include "../../renderer/command_copy_diagnostics.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include "../../compat/legacy/world_rolling_session.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QtEndian>
#include <QOpenGLContext>
#include <iostream>
using namespace mnm;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void line(QFile& file,const QJsonObject& row){auto data=QJsonDocument(row).toJson(QJsonDocument::Compact)+'\n';require(file.write(data)==data.size(),"Replay record write");}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==5,"Expected source profile, raw V5 commands, assets, new output directory");
 auto opened=assets::AssetStore::create(argv[3]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
 auto store=std::get<assets::AssetStore>(std::move(opened));
 QDir out(argv[4]);require(!out.exists()&&QDir().mkpath(out.path()),"New replay output required");
 QFile profile(argv[1]),input(argv[2]),cpu(out.filePath("cpu.jsonl")),report(out.filePath("native.json"));
 require(profile.open(QIODevice::ReadOnly)&&profile.size()<=2ll*1024*1024*1024&&profile.read(8)=="MNMPROF3","Bounded V3 source profile");
 require(input.open(QIODevice::ReadOnly)&&input.size()<=1024ll*1024*1024,"Bounded raw command input");
 require(cpu.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&report.open(QIODevice::WriteOnly|QIODevice::NewOnly),"New replay records");
 // Match the player's separate World and facade surface owners and budgets.
 QOpenGLContext share;QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);
 share.setFormat(format);require(share.create(),"Replay context sharing root");
 render::GlBlitter renderer(&share),worldRenderer(&share);quint64 generation=0,chunks=0,blocks=0,cpuReads=0;unsigned submitted=0;
 auto diagnostic=std::make_shared<render::CommandCopyDiagnostics>(out.filePath("copies"),render::CommandCopyDiagnostics::Selection::AllCopies);
 {
 legacy::WorldRollingSession world(worldRenderer,store,false,true,true,true);
 render::CommandConsumerOptions options;options.stream=render::CommandStreamMode::Streaming;
 options.prepareResidentFrames=true;options.reuseSealedUi=true;options.copyDiagnostics=diagnostic;
 options.residentWorld=[&](std::uint64_t requested,std::uint64_t nonce){
  require(nonce&&requested==generation+1,"Sequential requested source generation");
  render::GpuFrame result;
  while(generation<requested){
   auto h=profile.read(32);require(h.size()==32,"Missing source profile generation");
   auto g=qFromLittleEndian<quint64>(h.constData()),chunk=qFromLittleEndian<quint64>(h.constData()+8);
   auto s=qFromLittleEndian<quint32>(h.constData()+16),r=qFromLittleEndian<quint32>(h.constData()+20),n=qFromLittleEndian<quint32>(h.constData()+24);
   require(!qFromLittleEndian<quint32>(h.constData()+28)&&g==generation+1&&g<=8192&&++blocks<=8192&&s>=16&&s<=16777216&&(chunk?r==0:r>=64&&r<=197184)&&n>4&&n<=s+r+(s+r)/1000+128,"Source profile identity/bounds");
   auto packed=profile.read(n);require(packed.size()==n&&qFromBigEndian<quint32>(packed.constData())==s+r,"Source compressed extent");
   auto data=qUncompress(packed);require(data.size()==s+r,"Source inflation");
   auto bytes=[&](unsigned at,unsigned size){return std::vector<std::uint8_t>(data.begin()+at,data.begin()+at+size);};
   auto stream=legacy::decodeCanvasProducers(bytes(0,s),false);
   if(chunk){require(chunk==++chunks,"Source chunk order");world.consumeSource(stream);continue;}
   auto handoff=legacy::prepareRollingHandoff(stream,0,bytes(s,r),1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
   result=world.consumeResident(g,handoff.stream);++generation;
   require(!world.worldReadbacks(),"Unexpected ordinary World readback");
  }
  return result;
 };
 options.cpuRead=[&](const render::SurfaceCommand& c,const render::Image& image){
  const auto& w=c.words;require(image.width==int(w[4])&&image.height==int(w[5]),"CPU reply extent");
  QByteArray reply("MNMCREAD",8);reply.resize(64+qsizetype(image.pixels.size())*2);
  const quint32 header[]={1,64,w[3],w[1],w[2],w[0],w[4],w[5],quint32(image.pixels.size()*2),w[6],w[7],w[8],w[9],1};
  for(unsigned i=0;i<14;++i)qToLittleEndian(header[i],reply.data()+8+i*4);
  for(std::size_t i=0;i<image.pixels.size();++i){require(image.pixels[i]<=65535,"CPU WORD reply");qToLittleEndian(quint16(image.pixels[i]),reply.data()+64+i*2);}
  line(cpu,{{"sequence",int(c.sequence)},{"sha256",QString::fromLatin1(QCryptographicHash::hash(reply,QCryptographicHash::Sha256).toHex())}});++cpuReads;
 };
 // Discover only the session identifier from decoded requests, before execution.
 // Images and actual reply bytes have no input path into this executable.
 render::CommandDecoder scan(render::CommandStreamMode::Streaming);
 while(!input.atEnd())for(const auto& c:scan.append(input.read(65536)))if(c.operation==21){require(!options.cpuReadSession||options.cpuReadSession==c.words[3],"CPU session changed");options.cpuReadSession=c.words[3];}
 scan.finish();require(input.seek(0),"Command rewind");
 render::CommandConsumer consumer(renderer,[](render::GpuFrame){},options);
 render::CommandDecoder decoder(render::CommandStreamMode::Streaming);
 while(!input.atEnd())for(const auto& c:decoder.append(input.read(65536))){consumer.submit(c);++submitted;}
 decoder.finish();consumer.finish();diagnostic->finish();
 require(profile.atEnd()&&generation>0&&!consumer.result().liveSurfaces,"Complete source history and command retirement");
 }
 require(!renderer.stats().surfaces&&!worldRenderer.stats().surfaces,"Source resources retired");
 line(report,{{"success",true},{"commands",int(submitted)},{"worlds",int(generation)},{"source_chunks",int(chunks)},{"cpu_reads",int(cpuReads)},
              {"scope","Offline native replay; captured outputs are not inputs and historical capture status is unchanged"},{"original_pixels_used_as_native_inputs",false},{"remaining_surfaces",0}});
 std::cout<<submitted<<" commands, "<<generation<<" source Worlds, "<<cpuReads<<" CPU replies\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
