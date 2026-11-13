#include "gui_surface_loss_probe.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QSaveFile>
#include <QtEndian>
#include <thread>
#include <cstdio>
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static void word(QByteArray& out,unsigned value){char b[4];qToLittleEndian(value,b);out.append(b,4);}
static QByteArray source(unsigned colour){
 QByteArray out("MNMCMD01");word(out,1);word(out,16);
 for(unsigned v:{1u,1u,30u,1u,1u,1u,16u,0xf800u,0x7e0u,31u})word(out,v);
 char pixel[2];qToLittleEndian(quint16(colour),pixel);out.append(pixel,2);
 for(unsigned v:{6u,2u,4u,1u})word(out,v);
 return out;
}
int main(int argc,char** argv)try{
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
 QApplication app(argc,argv);QTemporaryDir temporary;check(temporary.isValid(),"Temporary GUI source root");
 GlViewport view;view.resize(160,120);view.show();app.processEvents();check(view.ready(),"GUI native viewport");
 for(unsigned generation=0;generation<2;++generation){
  const auto root=temporary.filePath(QString::number(generation));QDir().mkpath(root);
  auto commands=std::make_unique<LiveCommandSession>(view);check(commands->create(root+"/commands",300+generation,3,10000),"GUI command create");
  QFile channel(root+"/commands");check(channel.open(QIODevice::ReadWrite),"GUI source file");auto* map=channel.map(0,MNM_RENDER_COMMANDS_V2_SIZE);check(map,"GUI source map");
  mnm_ring_writer producer{};check(mnm_ring_writer_bind(&producer,map,MNM_RENDER_COMMANDS_V2_SIZE),"GUI source claim");
  std::unique_ptr<GuiSurfaceLossProbe> probe;
  if(!generation){
   probe=std::make_unique<GuiSurfaceLossProbe>(root+"/commands",300,3);
   QSaveFile request(root+"/surface-loss-request-v1.json");check(request.open(QIODevice::WriteOnly),"GUI loss request");
   auto raw=QJsonDocument(QJsonObject{{"schema",1},{"producer",300},{"transport_version",3},{"request_id","0123456789abcdef0123456789abcdef"},{"action","destroy-render-target"}}).toJson();
   check(request.write(raw)==raw.size()&&request.commit(),"GUI atomic loss request");
   check(probe->poll(*commands,0,false).isEmpty(),"Unpresented loss request must wait");
   check(!QFile::exists(root+"/surface-loss-observation-v1.json"),"Unpresented target was destroyed");
  }
  unsigned presents=0;commands->framePresented=[&]{++presents;};auto bytes=source(generation?31:0xf800);
  check(mnm_ring_write(&producer,bytes.data(),bytes.size())==1,"GUI source publication");
  QElapsedTimer clock;clock.start();while(!presents&&clock.elapsed()<10000){check(commands->poll(),"GUI native poll");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
  check(presents,"GUI presentation deadline");view.repaint();auto image=view.grabFramebuffer();
  check(!image.isNull()&&image.pixelColor(80,60)==(generation?QColor(Qt::blue):QColor(Qt::red)),"Independent GUI/fresh native pixels");
  if(!generation){
   check(probe->poll(*commands,0,true).isEmpty(),"Borrowed World loss request must wait");
   check(!QFile::exists(root+"/surface-loss-observation-v1.json"),"Borrowed target was destroyed");
   check(probe->poll(*commands,0,false)=="OpenGL offscreen target is unavailable","GUI typed target loss");
   view.setGpuFrame({});commands->abort();probe->commandCleanup(commands->result());commands.reset();probe->retired(false,false,!view.frameSize().isEmpty());
   QFile file(root+"/surface-loss-retirement-v1.json");check(file.open(QIODevice::ReadOnly),"GUI retirement observation");auto observed=QJsonDocument::fromJson(file.readAll()).object();
   check(observed["provider_owner"].toString()=="gui-resident"&&!observed["worker_thread"].toBool()&&observed["render_owner_gui_thread"].toBool()&&observed["target_restored_for_deletion"].toBool()&&observed["command_owner_retired"].toBool()&&observed["frame_leases_retired"].toBool()&&observed["command_surfaces_after_cleanup"].toString()=="0"&&observed["renderer_surfaces_after_cleanup"].toString()=="0","GUI provider deletion/lifetime incomplete");
  }else{view.setGpuFrame({});commands->abort();commands.reset();}
  channel.unmap(map);
 }
 std::puts("{\"success\":true,\"actual_target_destructions\":1,\"primary_gui_provider\":true,\"independent_pixel_checks\":2,\"fresh_channel_identity\":true,\"deferred_unpresented_and_borrowed_requests\":2,\"synthetic_world_counter\":0,\"original_game_runs\":0}");return 0;
}catch(const std::exception& error){std::fprintf(stderr,"%s\n",error.what());return 1;}
