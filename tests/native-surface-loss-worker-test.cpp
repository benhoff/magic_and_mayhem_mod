#include "native_render_worker.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QtEndian>
#include <thread>
#include <cstdio>
static void check(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static void word(QByteArray& out,unsigned value){char bytes[4];qToLittleEndian(value,bytes);out.append(bytes,4);}
static QByteArray first(unsigned colour){
 QByteArray out("MNMCMD01");word(out,1);word(out,16);
 for(unsigned v:{1u,1u,30u,1u,1u,1u,16u,0xf800u,0x7e0u,31u})word(out,v);
 char p[2];qToLittleEndian(quint16(colour),p);out.append(p,2);
 for(unsigned v:{6u,2u,4u,1u})word(out,v);
 return out;
}
static QJsonObject read(const QString& path){QFile file(path);check(file.open(QIODevice::ReadOnly),"Missing worker loss receipt");return QJsonDocument::fromJson(file.readAll()).object();}
int main(int argc,char** argv)try{
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
 QApplication app(argc,argv);QTemporaryDir temporary;check(temporary.isValid(),"Temporary root");
 qputenv("MNM_ACCEPTANCE_NORMAL","1");qputenv("MNM_SURFACE_LOSS_TEST","1");
 GlViewport view;view.resize(160,120);view.show();app.processEvents();check(view.ready(),"Native viewport");
 for(unsigned generation=0;generation<2;++generation){
  const auto root=temporary.filePath(QString::number(generation));QDir().mkpath(root);
  NativeRenderWorker worker(view,root+"/world",root,root+"/report.json");
  check(worker.create(root+"/commands",100+generation,3,10000),"Native worker create");
  QFile channel(root+"/commands");check(channel.open(QIODevice::ReadWrite),"Producer file");auto* map=channel.map(0,MNM_RENDER_COMMANDS_V2_SIZE);check(map,"Producer map");
  mnm_ring_writer producer{};check(mnm_ring_writer_bind(&producer,map,MNM_RENDER_COMMANDS_V2_SIZE),"Producer claim");
  unsigned presented=0;worker.framePresented=[&]{++presented;};
  auto bytes=first(generation?31:0xf800);check(mnm_ring_write(&producer,bytes.data(),bytes.size())==1,"Initial source publication");
  auto wait=[&](auto predicate){QElapsedTimer clock;clock.start();while(!predicate()&&clock.elapsed()<10000){worker.poll(false,true);app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}check(predicate(),"Worker loss deadline");};
  wait([&]{return presented>0;});view.repaint();auto image=view.grabFramebuffer();
  check(!image.isNull()&&image.pixelColor(80,60)==(generation?QColor(Qt::blue):QColor(Qt::red)),"Independent actual worker pixel output");
  if(!generation){
   QSaveFile request(root+"/surface-loss-request-v1.json");check(request.open(QIODevice::WriteOnly),"Unique request");
   const auto raw=QJsonDocument(QJsonObject{{"schema",1},{"producer",100},{"transport_version",3},{"request_id","0123456789abcdef0123456789abcdef"},{"action","destroy-render-target"}}).toJson();
   check(request.write(raw)==raw.size()&&request.commit(),"Atomic diagnostic request publication");
   wait([&]{return !worker.error().isEmpty();});
   if(worker.error()!="OpenGL offscreen target is unavailable" || worker.recoveries()!=0)
       throw std::runtime_error(("Typed failure/retry mismatch: "+worker.error()+"; recoveries="+QString::number(worker.recoveries())).toStdString());
   const auto before=read(root+"/surface-loss-observation-v1.json");
   check(before["surface_valid_before"].toBool()&&!before["surface_valid_after"].toBool()&&before["gui_destroyed_target"].toBool()&&before["worker_thread"].toBool()&&before["graphics_failure"].toBool(),"Not actual target loss on correct owners");
  }
  worker.abort();check(view.frameSize().isEmpty(),"Terminal GUI frame retained");
  if(!generation){auto final=read(root+"/surface-loss-retirement-v1.json");check(final["target_restored_for_deletion"].toBool()&&final["command_owner_retired"].toBool()&&final["world_owner_retired"].toBool()&&final["frame_leases_retired"].toBool()&&final["worker_teardown_complete"].toBool()&&final["command_surfaces_after_cleanup"].toString()=="0"&&final["renderer_surfaces_after_cleanup"].toString()=="0","Lost worker did not retire all native owners/storage");}
  channel.unmap(map);
 }
 std::puts("{\"success\":true,\"actual_target_destructions\":1,\"actual_worker_retirements\":2,\"independent_pixel_checks\":2,\"fresh_channel_identity\":true,\"no_in_place_retry\":true,\"original_game_runs\":0}");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
