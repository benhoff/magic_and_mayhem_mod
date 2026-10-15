#include "native_render_worker.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QtEndian>
#include <thread>
#include <cstdio>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void word(QByteArray& bytes,unsigned value){char out[4];qToLittleEndian(value,out);bytes.append(out,4);}
static QByteArray firstFrame(){
 QByteArray out("MNMCMD01");word(out,1);word(out,16);
 word(out,1);word(out,1);word(out,30);for(unsigned v:{1u,1u,1u,16u,0xf800u,0x7e0u,31u})word(out,v);out+=QByteArray::fromHex("00f8");
 word(out,6);word(out,2);word(out,4);word(out,1);return out;
}
int main(int argc,char** argv)try{
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
 QApplication app(argc,argv);QTemporaryDir dir;check(dir.isValid(),"temporary directory");
 GlViewport view;view.resize(160,120);view.show();app.processEvents();check(view.ready(),"GL initialized");
 // Each failure is followed by a fresh worker, new nonce, new command identity
 // and a visible frame. No retry can import the old worker's textures/channels.
 for(unsigned cause=0;cause<4;++cause){
  const auto root=dir.filePath(QString::number(cause));QDir().mkpath(root);
  NativeRenderWorker failed(view,root+"/world",root,root+"/report.json");check(failed.create(root+"/commands",100+cause,2,cause==2?20:10000),"failed channel create");
  QFile file(root+"/commands");check(file.open(QIODevice::ReadWrite),"open producer");auto* map=file.map(0,MNM_RENDER_COMMANDS_V2_SIZE);mnm_ring_writer producer{};check(mnm_ring_writer_bind(&producer,map,MNM_RENDER_COMMANDS_V2_SIZE),"claim producer");
  unsigned shown=0;failed.framePresented=[&]{++shown;};
  const auto wait=[&](auto done){QElapsedTimer deadline;deadline.start();while(!done()&&deadline.elapsed()<10000){failed.poll(false,true);app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}check(done(),"failure deadline");};
  if(cause!=2){const auto bytes=firstFrame();check(mnm_ring_write(&producer,bytes.data(),unsigned(bytes.size()))==1,"first frame");wait([&]{return shown>0;});check(!view.frameSize().isEmpty(),"presented lease");}
  if(cause==0){const QByteArray invalid(16,'x');check(mnm_ring_write(&producer,invalid.data(),unsigned(invalid.size()))==1,"malformed publish");}
  if(cause==1)mnm_ring_fail(&producer,MNM_RENDER_COMMANDS_V2_REASON_INVALID);
  if(cause<3){wait([&]{return !failed.error().isEmpty();});check(failed.recoveries()==0,"resident failure retried old producer");}
  // cause3 is the GUI's terminal GL-error cancellation path after presentation.
  failed.abort();check(view.frameSize().isEmpty(),"failed GUI lease retained");file.unmap(map);
  NativeRenderWorker fresh(view,root+"/fresh-world",root,root+"/fresh-report.json");check(fresh.create(root+"/fresh-commands",200+cause,2,10000),"fresh channel create");
  QFile next(root+"/fresh-commands");check(next.open(QIODevice::ReadWrite),"fresh producer file");auto* nextMap=next.map(0,MNM_RENDER_COMMANDS_V2_SIZE);mnm_ring_writer nextProducer{};check(mnm_ring_writer_bind(&nextProducer,nextMap,MNM_RENDER_COMMANDS_V2_SIZE),"fresh claim");
  unsigned freshShown=0;fresh.framePresented=[&]{++freshShown;};const auto bytes=firstFrame();check(mnm_ring_write(&nextProducer,bytes.data(),unsigned(bytes.size()))==1,"fresh first frame");
  QElapsedTimer deadline;deadline.start();while(!freshShown&&deadline.elapsed()<10000){check(fresh.poll(false,true),"fresh worker refused");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
  check(freshShown&&!view.frameSize().isEmpty(),"fresh generation did not present");fresh.abort();check(view.frameSize().isEmpty(),"fresh shutdown lease retained");next.unmap(nextMap);
 }
 std::puts("{\"success\":true,\"failure_cases\":4,\"fresh_sessions\":4,\"malformed\":true,\"producer_failure\":true,\"first_frame_timeout\":true,\"gui_abort_after_presentation\":true,\"no_in_place_retry\":true,\"gui_leases_cleared\":true,\"scope\":\"Synthetic actual GL worker lifecycle; GUI abort models the GL error caller, not physical context loss. No actual-game freeze/restart equivalence.\"}");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
