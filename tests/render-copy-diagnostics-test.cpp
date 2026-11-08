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
  QTemporaryDir dir;require(dir.isValid(),"No temporary output");GlBlitter renderer;QJsonObject distinctResult;Image image{800,600,std::vector<std::uint32_t>(800*600)};
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
  diagnostics->finish();
  QFile file(dir.filePath("report.json"));require(file.open(QIODevice::ReadOnly),"Missing copy report");const auto report=QJsonDocument::fromJson(file.readAll()).object();
  require(report["success"].toBool() && report["copies_checked"].toInt()==2 && report["diagnostic_native_readbacks"].toInt()==6 && report["pixels_compared"].toInt()==960000,"Copy diagnostics incomplete");
  require(report["proof_processing"]=="bounded-fifo-worker" && report["copies_enqueued"].toInt()==2 && report["worker_peak_bytes"].toInteger()>0 && report["worker_peak_bytes"].toInteger()<=report["worker_queue_limit_bytes"].toInteger(),"Unbounded/incomplete diagnostic worker");
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
   bool mismatch=false;try{bad.after(copy,renderer,dest);bad.finish();}catch(const std::runtime_error&){mismatch=true;}
   require(mismatch,"Incorrect GPU output passed independent comparison");
   QFile wrong(dir.filePath("wrong-output/copies.jsonl"));require(wrong.open(QIODevice::ReadOnly),"Missing mismatch witness");
   const auto badRow=QJsonDocument::fromJson(wrong.readAll().trimmed()).object();
   require(badRow["actual_sha256"]!=badRow["expected_sha256"],"Mismatch reused actual digest as expected");
   renderer.destroy(dest);
  }
  {
   // No World command: these keyed CPU-backed COPYs were excluded by the
   // historical ancestry selector. Full-plane comparison must now run.
   const auto path=dir.filePath("all-copies");
   auto all=std::make_shared<CommandCopyDiagnostics>(path,CommandCopyDiagnostics::Selection::AllCopies);
   CommandConsumerOptions allOptions;allOptions.copyDiagnostics=all;
   CommandConsumer copies(renderer,[](auto){},allOptions);
   Image small{4,3,{1,2,3,4,5,6,7,8,9,10,11,12}};
   Image untouched{4,3,{101,102,103,104,105,106,107,108,109,110,111,112}};
   copies.submit(create(1,1,small));copies.submit(create(2,2,untouched));
   copies.submit(command(3,3,{1,2,0,0,3,2,1,1,1,2}));
   copies.submit(command(3,4,{2,1,1,1,4,3,0,0,0,0}));
   copies.submit(command(6,5,{2}));copies.submit(command(7,6,{1}));copies.submit(command(7,7,{2}));
   copies.submit(command(8,8,{}));copies.finish();all->finish();
   QFile summary(path+"/report.json");require(summary.open(QIODevice::ReadOnly),"Missing all-copy summary");
   const auto result=QJsonDocument::fromJson(summary.readAll()).object();
   require(result["success"].toBool() && result["selection"]=="all-copies" && result["copies_seen"].toInt()==2 && result["copies_checked"].toInt()==2 && result["diagnostic_native_readbacks"].toInt()==6 && result["pixels_compared"].toInt()==24,"CPU-keyed all-copy output not independently checked");
   QFile evidence(path+"/copies.jsonl");require(evidence.open(QIODevice::ReadOnly),"Missing CPU-keyed records");
   const auto rows=evidence.readAll().trimmed().split('\n');require(rows.size()==2,"Incomplete all-copy rows");
   for(const auto& row:rows)require(!QJsonDocument::fromJson(row).object()["source_world_ancestry"].toBool(),"CPU source invented World ancestry");
   CommandCopyDiagnostics badCpu(dir.filePath("wrong-cpu"),CommandCopyDiagnostics::Selection::AllCopies);
   const auto src=renderer.create(small,{16,{0xf800,0x7e0,31}}),dst=renderer.create(untouched,{16,{0xf800,0x7e0,31}});
   const auto keyed=command(3,1,{1,2,0,0,3,2,1,1,1,2});
   require(badCpu.before(keyed,renderer,src,dst),"CPU-keyed diagnostic was skipped");
   bool mismatch=false;try{badCpu.after(keyed,renderer,dst);badCpu.finish();}catch(const std::runtime_error&){mismatch=true;}
   require(mismatch,"Missing CPU-keyed draw passed independent output proof");renderer.destroy(src);renderer.destroy(dst);
  }
  {
   const auto path=dir.filePath("distinct-inputs");
   auto distinct=std::make_shared<CommandCopyDiagnostics>(path,CommandCopyDiagnostics::Selection::DistinctInputs);
   Image small=image,blank{800,600,std::vector<std::uint32_t>(800*600,100)};
   const auto resident=renderer.create(small,{16,{0xf800,0x7e0,31}});auto residentFrame=renderer.presentGpu(resident);
   CommandConsumerOptions selected;selected.copyDiagnostics=distinct;selected.exportImages=true;
   selected.residentWorld=[&](auto,auto){return residentFrame;};CommandConsumer copies(renderer,{},selected);unsigned seq=1;
   auto submit=[&](SurfaceCommand c){c.sequence=seq++;copies.submit(c);};
   submit(create(1,0,small));submit(create(2,0,blank));
   const auto full=command(3,0,{1,2,0,0,800,600,0,0,0,0});submit(full);
   auto changed=small;changed.pixels[0]=0x7bef;auto update=command(2,0,{1,0,0,800,600});update.image=changed;submit(update);
   submit(full);submit(command(6,0,{2}));
   require(qFromLittleEndian<quint16>(copies.result().native.constData())==0x7bef,"Skipped duplicate did not execute its changed actual input");
   submit(command(7,0,{1}));changed.pixels[0]=31;submit(create(3,0,changed));const auto replacement=command(3,0,{3,2,0,0,800,600,0,0,0,0});submit(replacement);submit(command(6,0,{2}));
   require(qFromLittleEndian<quint16>(copies.result().native.constData())==31,"Replacement handle lifetime did not execute");
   submit(command(19,0,{3,1,0,1,0}));submit(replacement);submit(replacement);
   for(unsigned i=0;i<9;++i)submit(command(3,0,{3,2,i%4,i/4,i%4+1,i/4+1,i%4,i/4,0,0}));
   submit(command(3,0,{3,2,0,0,1,1,0,0,0,0}));
   submit(command(3,0,{3,2,0,2,1,3,0,2,0,0}));
   submit(command(3,0,{3,2,0,0,800,600,0,0,1,50000}));submit(command(6,0,{2}));
   const auto actual=copies.result().native;
   require(actual.size()==800*600*2,"Distinct final native plane extent");
   for(unsigned i=0;i<image.pixels.size();++i)require(qFromLittleEndian<quint16>(actual.constData()+2*i)==image.pixels[i],"Selection altered actual World/key/budget output");
   submit(command(7,0,{3}));submit(command(7,0,{2}));submit(command(8,0,{}));copies.finish();distinct->finish();
   QFile summary(path+"/report.json");require(summary.open(QIODevice::ReadOnly),"Missing distinct input summary");distinctResult=QJsonDocument::fromJson(summary.readAll()).object();
   require(distinctResult["success"].toBool() && distinctResult["selection"]=="distinct-copy-inputs"
    && distinctResult["copies_seen"].toInt()==17 && distinctResult["copies_checked"].toInt()==11
    && distinctResult["copies_skipped_duplicate"].toInt()==3 && distinctResult["copies_skipped_class_budget"].toInt()==3
    && distinctResult["copies_skipped_byte_budget"].toInt()==0 && distinctResult["diagnostic_native_readbacks"].toInt()==33
    && distinctResult["retained_pixel_cases"].toInt()==11 && distinctResult["selected_plane_bytes"].toInt()==84480000,
    "Distinct input selector/readback/retention denominator differs");
   QFile evidence(path+"/copies.jsonl");require(evidence.open(QIODevice::ReadOnly),"Missing distinct input rows");
   const auto rows=evidence.readAll().trimmed().split('\n');require(rows.size()==11,"Selected plane lost");
   const auto first=QJsonDocument::fromJson(rows[0]).object(),reuse=QJsonDocument::fromJson(rows[1]).object(),ancestry=QJsonDocument::fromJson(rows[2]).object();
   require(first["source_create_sequence"].toInt()==1 && reuse["source_create_sequence"].toInt()==8
    && reuse["words"]==ancestry["words"] && !reuse["source_world_ancestry"].toBool() && ancestry["source_world_ancestry"].toBool(),
    "Replacement lifetime or ancestry transition was deduplicated as old input");
   for(const auto& encoded:rows){const auto row=QJsonDocument::fromJson(encoded).object();
    for(const auto* role:{"source","before","actual","expected"}){QFile pixels(path+"/"+row["files"].toObject()[role].toString());
     require(pixels.open(QIODevice::ReadOnly) && QString::fromLatin1(QCryptographicHash::hash(pixels.readAll(),QCryptographicHash::Sha256).toHex())==row[QString(role)+"_sha256"].toString(),"Distinct complete plane is missing/corrupt");}}
   renderer.destroy(resident);
  }
  {
   CommandCopyDiagnostics bad(dir.filePath("wrong-distinct"),CommandCopyDiagnostics::Selection::DistinctInputs);
   Image srcImage{4,3,{1,2,3,4,5,6,7,8,9,10,11,12}},dstImage{4,3,std::vector<std::uint32_t>(12,100)};
   const auto src=renderer.create(srcImage,{16,{0xf800,0x7e0,31}}),dst=renderer.create(dstImage,{16,{0xf800,0x7e0,31}});
   bad.committed(create(1,1,srcImage));bad.committed(create(2,2,dstImage));const auto copy=command(3,3,{1,2,0,0,3,2,1,1,0,0});
   require(bad.before(copy,renderer,src,dst),"First distinct input skipped");bool mismatch=false;
   try{bad.after(copy,renderer,dst);bad.finish();}catch(const std::runtime_error&){mismatch=true;}
   require(mismatch,"Distinct selector hid an omitted actual GPU copy");renderer.destroy(src);renderer.destroy(dst);
  }
  renderer.destroy(world);require(renderer.stats().surfaces==0,"Diagnostic resource leak");
  const auto driver=renderer.driver();
  std::puts(QJsonDocument(QJsonObject{{"success",true},{"copies_checked",2},{"non_world_readbacks",0},{"diagnostic_readbacks",6},{"overwrites_refused",true},{"cpu_keyed_all_copy_checks",2},{"renderer",QString::fromStdString(driver.renderer)},{"worker_peak_bytes",report["worker_peak_bytes"]},{"distinct_inputs",distinctResult}}).toJson(QJsonDocument::Compact).constData());return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
}
