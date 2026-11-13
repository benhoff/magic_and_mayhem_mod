#include "native_render_worker.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QElapsedTimer>
#include <QtEndian>
#include <thread>
#include <iostream>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void word(QByteArray& b,unsigned v){char out[4];qToLittleEndian(v,out);b.append(out,4);}
static QByteArray frame(unsigned id,unsigned sequence,unsigned colour,bool first){
 QByteArray b;if(first){b="MNMCMD01";word(b,1);word(b,16);}
 word(b,1);word(b,sequence);word(b,30);for(auto v:{id,1u,1u,16u,0xf800u,0x7e0u,31u})word(b,v);b.append(char(colour));b.append(char(colour>>8));
 word(b,6);word(b,sequence+1);word(b,4);word(b,id);return b;
}
struct NativeRenderWorkerTestAccess {
 static QJsonObject published(NativeRenderWorker& w){std::lock_guard<std::mutex> lock(w.mutex_);return w.published_.report;}
 static auto commands(NativeRenderWorker& w){std::lock_guard<std::mutex> lock(w.mutex_);return w.published_.hasResult?w.published_.result.commands:0;}
};
int main(int argc,char** argv)try{
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
 QApplication app(argc,argv);QTemporaryDir dir;check(dir.isValid(),"Fixture directory");GlViewport view;view.resize(160,120);view.show();
 QElapsedTimer init;init.start();while(!view.ready()&&init.elapsed()<10000)app.processEvents();check(view.ready(),"GL viewport");
 NativeRenderWorker worker(view,dir.filePath("world"),dir.path(),dir.filePath("report.json"));check(worker.create(dir.filePath("commands"),12345,2,10000),"Create native worker");
 QFile file(dir.filePath("commands"));check(file.open(QIODevice::ReadWrite),"Command producer");auto* map=file.map(0,MNM_RENDER_COMMANDS_V2_SIZE);mnm_ring_writer producer{};check(mnm_ring_writer_bind(&producer,map,MNM_RENDER_COMMANDS_V2_SIZE),"Claim producer");
 unsigned shown=0;worker.framePresented=[&]{++shown;};
 const auto wait=[&](auto done,bool pickup,int timeout=10000){QElapsedTimer t;t.start();while(!done()&&t.elapsed()<timeout){if(pickup)check(worker.poll(false,true),"Render worker error");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}return done();};
 auto a=frame(1,1,0xf800,true);check(mnm_ring_write(&producer,a.data(),unsigned(a.size()))==1,"First frame");check(wait([&]{return shown==1;},true),"First output not delivered");
 auto b=frame(2,3,0x07e0,false);check(mnm_ring_write(&producer,b.data(),unsigned(b.size()))==1,"Second frame");
 check(wait([&]{return NativeRenderWorkerTestAccess::published(worker)["presentation_exports"].toString().toULongLong()>=2;},false),"Second output not queued");
 auto c=frame(3,5,0x001f,false);check(mnm_ring_write(&producer,c.data(),unsigned(c.size()))==1,"Third frame");
 check(wait([&]{return NativeRenderWorkerTestAccess::published(worker)["presentation_exports_skipped"].toString().toULongLong()>0;},false),"Two-lease backpressure not exercised");
 // Change the native surface to white without a PRESENT. The retained
 // completed blue presentation must remain the retry source.
 QByteArray partial;word(partial,2);word(partial,7);word(partial,22);for(auto v:{3u,0u,0u,1u,1u})word(partial,v);partial+=QByteArray::fromHex("ffff");
 check(mnm_ring_write(&producer,partial.data(),unsigned(partial.size()))==1,"Partial update");
 check(wait([&]{return NativeRenderWorkerTestAccess::commands(worker)>=7;},false),"Partial update not consumed");
 check(worker.poll(false,true)&&shown==2,"Queued second output not delivered");
 // No further commands, simulation updates or PRESENTs arrive. Merely returning
 // the GUI's first lease must allow the already completed third output through.
 check(wait([&]{return shown==3;},true,1000),"Completed image lost until another PRESENT");
 const auto image=view.grabFramebuffer();const auto pixel=image.pixelColor(image.width()/2,image.height()/2);check(pixel.blue()>240&&pixel.red()<8&&pixel.green()<8,"Latest completed image pixels differ");
 auto r=worker.report();check(r["presentation_export_allocations"].toString().toULongLong()==2&&r["presentation_lease_high_water"].toInt()==2,"Output storage grew beyond two leases");
 worker.abort();check(view.frameSize().isEmpty(),"GUI output lease survived abort");file.unmap(map);
 std::cout<<"{\"success\":true,\"outputs\":3,\"leases\":2,\"retry_without_new_present\":true,\"latest_pixels_blue\":true,\"partial_update_not_published\":true,\"abort_retires_output\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
