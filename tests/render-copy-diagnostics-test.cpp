#include "commands.hpp"
#include "command_copy_diagnostics.hpp"
#include "../protocols/include/mnm/render_stream_v4.h"
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
SurfaceCommand command(unsigned op,unsigned seq,std::initializer_list<unsigned> fields){SurfaceCommand c;c.operation=op;c.version=4;c.sequence=seq;std::copy(fields.begin(),fields.end(),c.words.begin());return c;}
SurfaceCommand create(unsigned id,unsigned seq,const Image& image){auto c=command(1,seq,{id,unsigned(image.width),unsigned(image.height),16,0xf800,0x7e0,31});c.image=image;c.format={16,{0xf800,0x7e0,31}};return c;}
}
int main(int argc,char** argv){
 QSurfaceFormat f;f.setVersion(3,3);f.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(f);QGuiApplication app(argc,argv);
 try{
  QTemporaryDir dir;require(dir.isValid(),"No temporary output");GlBlitter renderer;Image image{800,600,std::vector<std::uint32_t>(800*600)};
  for(std::size_t i=0;i<image.pixels.size();++i)image.pixels[i]=(i+1)&65535;
  const auto world=renderer.create(image,{16,{0xf800,0x7e0,31}});auto frame=renderer.presentGpu(world);
  auto diagnostics=std::make_shared<CommandCopyDiagnostics>(dir.path());CommandConsumerOptions options;options.copyDiagnostics=diagnostics;
  options.residentWorld=[&](auto,auto){return frame;};CommandConsumer consumer(renderer,[](auto){},options);
  consumer.submit(create(1,1,image));consumer.submit(create(2,2,image));
  consumer.submit(command(3,3,{1,2,0,0,4,3,0,0,0,0}));require(renderer.stats().nativeReadbacks==0,"Non-World copy read pixels");
  consumer.submit(command(19,4,{1,1,0,1,0}));
  consumer.submit(command(3,5,{1,2,0,0,3,2,1,1,1,2}));
  consumer.submit(command(3,6,{2,1,1,1,4,3,0,0,0,0}));
  consumer.submit(command(6,7,{2}));consumer.submit(command(7,8,{1}));consumer.submit(command(7,9,{2}));consumer.submit(command(8,10,{}));consumer.finish();
  QFile file(dir.filePath("report.json"));require(file.open(QIODevice::ReadOnly),"Missing copy report");const auto report=QJsonDocument::fromJson(file.readAll()).object();
  require(report["success"].toBool() && report["copies_checked"].toInt()==2 && report["diagnostic_native_readbacks"].toInt()==6 && report["pixels_compared"].toInt()==960000,"Copy diagnostics incomplete");
  QFile records(dir.filePath("copies.jsonl"));require(records.open(QIODevice::ReadOnly),"Missing copy records");
  for(const auto& line:records.readAll().trimmed().split('\n')){
   const auto row=QJsonDocument::fromJson(line).object();const auto files=row["files"].toObject();
   for(const auto* role:{"source","before","actual","expected"}){
    QFile pixels(dir.filePath(files[role].toString()));require(pixels.open(QIODevice::ReadOnly),"Missing canonical witness");
    const auto data=pixels.readAll();
    require(QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex())==row[QString(role)+"_sha256"].toString(),"Witness lifetime/hash mismatch");
    require(data.size()==800*600*4 && qFromLittleEndian<quint32>(data.constData())==1,"Witness canonical word encoding");
   }
  }
  bool refused=false;try{CommandCopyDiagnostics duplicate(dir.path());}catch(const std::runtime_error&){refused=true;}require(refused,"Historical diagnostics overwritten");
  {
   CommandCopyDiagnostics bad(dir.filePath("wrong-output"));const auto dest=renderer.create(image,{16,{0xf800,0x7e0,31}});
   bad.committed(command(19,1,{1,1,0,1,0}));const auto copy=command(3,2,{1,2,0,0,3,2,1,1,1,2});
   require(bad.before(copy,renderer,world,dest),"Missing negative diagnostic");
   // Deliberately omit the GPU copy; a matching implementation-shaped counter
   // test would miss this. The independent destination must reject unchanged data.
   bool mismatch=false;try{bad.after(copy,renderer,dest);}catch(const std::runtime_error&){mismatch=true;}
   require(mismatch,"Incorrect GPU output passed independent comparison");
   QFile wrong(dir.filePath("wrong-output/copies.jsonl"));require(wrong.open(QIODevice::ReadOnly),"Missing mismatch witness");
   const auto badRow=QJsonDocument::fromJson(wrong.readAll().trimmed()).object();
   require(badRow["actual_sha256"]!=badRow["expected_sha256"],"Mismatch reused actual digest as expected");
   renderer.destroy(dest);
  }
  renderer.destroy(world);require(renderer.stats().surfaces==0,"Diagnostic resource leak");
  std::puts("{\"success\":true,\"copies_checked\":2,\"non_world_readbacks\":0,\"diagnostic_readbacks\":6,\"overwrites_refused\":true}");return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
}
