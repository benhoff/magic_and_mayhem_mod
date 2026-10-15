#include "live_menu_session.hpp"
#include "resident_world_session.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/menu_v1.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QtEndian>
#include <iostream>
#include <QJsonDocument>
#include <future>
#include <stdexcept>
void check(bool ok){if(!ok)throw std::runtime_error("Resident Quit fixture");}
template<class F> void refuses(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
struct LiveMenuSessionTestAccess {
 static void initialize(LiveMenuSession& s,const QString& path){s.bridge_=std::make_unique<MenuBridge>();check(s.bridge_->create(path));s.active_=true;s.clock_.start();}
 static void poll(LiveMenuSession& s){s.poll();}
 static void cpuRead(LiveMenuSession& s){
  const auto state=s.lastState_,request=s.requestedAt_;
  s.renderingProgress(7);const auto start=s.cpuReadStarted_;
  check(s.cpuReadWaiting(start+9999)&&!s.cpuReadWaiting(start+10001));
  s.renderingProgress(7);check(s.cpuReadStarted_==start); // Same ticket cannot renew its budget.
  s.renderingProgress(0);check(s.cpuReadWaiting(s.cpuReadGrace_-1)&&!s.cpuReadWaiting(s.cpuReadGrace_));
  check(s.lastState_==state&&s.requestedAt_==request); // Progress cannot certify an old snapshot or resend.
  s.cpuReadGrace_=0;
 }
};
struct ResidentWorldSessionTestAccess {
 static bool production(bool asynchronous,unsigned complete,const mnm::render::GpuProductionQueue::Stats& p){return ResidentWorldSession::productionRetired(asynchronous,complete,p);}
 static void queue(ResidentWorldSession& s,const QString& path){s.queueReport(path);}
 static void busy(ResidentWorldSession& s,std::shared_ptr<std::promise<void>> gate){s.reportWork_=std::async(std::launch::async,[gate]{gate->get_future().wait();});}
 static QString latest(ResidentWorldSession& s){return s.latestReport_?QJsonDocument::fromJson(s.latestReport_->bytes).object()["completed"].toString():QString();}
 static void fail(ResidentWorldSession& s){s.reportWork_=std::async(std::launch::async,[]{throw std::runtime_error("authored report failure");});s.reportWork_.wait();}
 static void service(ResidentWorldSession& s){s.serviceReport();}
 static void counts(ResidentWorldSession& s,unsigned complete,unsigned imports,unsigned returns,bool pending){s.completed_=complete;s.imports_=imports;s.returns_=returns;if(pending)s.pending_.emplace();}
};
int main(int argc,char** argv) try {
 QApplication app(argc,argv);QTemporaryDir root;check(root.isValid());unsigned ackCases=0,refusals=0,retired=0;
 {
  mnm::render::GpuProductionQueue::Stats p;
  // A synchronous owner with completed Worlds has no GPU queue. This must not
  // inherit the asynchronous zero-vs-nonzero queue-count rejection.
  check(ResidentWorldSessionTestAccess::production(false,4498,p));
  check(!ResidentWorldSessionTestAccess::production(true,4498,p));
  p.submitted=p.produced=p.taken=4498;check(ResidentWorldSessionTestAccess::production(true,4498,p));
  p.outstanding=1;check(!ResidentWorldSessionTestAccess::production(true,4498,p));p.outstanding=0;
  --p.taken;check(!ResidentWorldSessionTestAccess::production(true,4498,p));++p.taken;
  p.error="authored producer failure";check(!ResidentWorldSessionTestAccess::production(true,4498,p));
 }
 for(unsigned variant=0;variant<5;++variant){
  LiveMenuSession session("/unused");const auto path=root.filePath(QString("menu-%1.bin").arg(variant));LiveMenuSessionTestAccess::initialize(session,path);
  QFile peer(path);check(peer.open(QIODevice::ReadWrite));auto* words=peer.map(0,MNM_MENU_V1_SIZE);check(words);
  const auto state=[&](unsigned sequence,unsigned ack,unsigned status,unsigned thread){
   const unsigned fields[]={sequence,1,3,1,ack,status,thread};
   for(unsigned i=0;i<7;++i)qToLittleEndian<quint32>(fields[i],words+(MNM_MENU_V1_ENGINE_WORD+i)*4);
  };
  state(2,0,MNM_MENU_OK,123);LiveMenuSessionTestAccess::poll(session);check(!session.normalQuitAccepted());
  check(session.request(variant==4?MNM_MENU_OPEN_QUICK:MNM_MENU_QUIT));check(!session.normalQuitAccepted());
  std::array<quint32,3> request{};for(unsigned i=0;i<3;++i)request[i]=qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+3+i)*4);
  const auto heartbeat=qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+2)*4);session.renderingProgress();session.renderingProgress();
  check(qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+2)*4)==heartbeat+2);
  for(unsigned i=0;i<3;++i)check(qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+3+i)*4)==request[i]);
  LiveMenuSessionTestAccess::cpuRead(session);
  for(unsigned i=0;i<3;++i)check(qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+3+i)*4)==request[i]);
  state(4,variant==1?2:1,variant==2?MNM_MENU_UNAVAILABLE:MNM_MENU_OK,variant==3?124:123);
  check(session.normalQuitAccepted()==(variant==0));check(session.normalQuitAccepted()==(variant==0));++ackCases;
  session.fallback("synthetic retirement");const auto retired=qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+2)*4);session.renderingProgress();check(qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+2)*4)==retired);check(!qFromLittleEndian<quint32>(words+(MNM_MENU_V1_HOST_WORD+1)*4));
  peer.unmap(words);
 }
 {
  GlViewport viewport;const auto directory=root.filePath("owned-cold-start");check(QDir().mkpath(directory));
  ResidentWorldSession session(viewport,directory+"/world.bin",directory+"/game",true);
  check(session.independent());session.poll(false);
  const auto report=session.report();check(report["delivery_mode"].toString()=="owned-v5");
  check(report["source_accepted"].toString()=="0" && report["source_outstanding"].toInt()==0);
  check(!report["pending"].toBool() && !report["normal_quit_retired"].toBool());
 }
 for(unsigned variant=0;variant<8;++variant){
  GlViewport viewport;const auto directory=root.filePath(QString("resident-%1").arg(variant));check(QDir().mkpath(directory));
  const auto path=directory+"/world.bin";ResidentWorldSession session(viewport,path,directory+"/game");
  ResidentWorldSessionTestAccess::counts(session,variant==0?0:4498,variant==0?0:variant==3?4497:4498,variant==0?0:variant==4?4497:4498,variant==5);
  // Simulate the producer's cancel flag through its owned mapping. Creating a
  // second native channel owner is correctly refused by the lease contract.
  QFile peer(path);check(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_CONT_HEADER);check(bytes);
  const auto nonce=std::uint64_t(qFromLittleEndian<quint32>(bytes+24)) | (std::uint64_t(qFromLittleEndian<quint32>(bytes+28))<<32);
  qToLittleEndian<quint32>(1,bytes+40);if(variant==6)session.abort("prior error");
  const bool accepted=variant!=2;
  if(variant<=1 || variant==7){
   session.poll(accepted);check(session.report()["normal_quit_retired"].toBool());check(session.report()["error"].toString().isEmpty());
   session.poll(accepted);refuses([&]{session.import(1,nonce);});++refusals;++retired;
  }else{
   refuses([&]{session.poll(accepted);});check(!session.report()["normal_quit_retired"].toBool());check(!session.report()["error"].toString().isEmpty());++refusals;
  }
  peer.unmap(bytes);
 }
 {
  GlViewport viewport;const auto directory=root.filePath("report-worker");check(QDir().mkpath(directory));
  ResidentWorldSession session(viewport,directory+"/world.bin",directory+"/game");
  auto gate=std::make_shared<std::promise<void>>();ResidentWorldSessionTestAccess::busy(session,gate);
  for(unsigned n:{64u,128u,256u}){ResidentWorldSessionTestAccess::counts(session,n,n,n,false);ResidentWorldSessionTestAccess::queue(session,directory+"/periodic.json");}
  check(ResidentWorldSessionTestAccess::latest(session)=="256"); // did not wait for the blocked writer
  gate->set_value();session.saveReport(directory+"/final.json");
  QFile final(directory+"/final.json");check(final.open(QIODevice::ReadOnly));check(QJsonDocument::fromJson(final.readAll()).object()["completed"].toString()=="256");
  ResidentWorldSessionTestAccess::counts(session,512,512,512,false);ResidentWorldSessionTestAccess::queue(session,directory+"/periodic.json");session.saveReport(directory+"/final-512.json");
  QFile periodic(directory+"/periodic.json");check(periodic.open(QIODevice::ReadOnly));check(QJsonDocument::fromJson(periodic.readAll()).object()["completed"].toString()=="512");
  ResidentWorldSessionTestAccess::fail(session);refuses([&]{ResidentWorldSessionTestAccess::service(session);});++refusals;
 }
 std::cout<<"{\"success\":true,\"owned_sync_nonzero_retirement\":true,\"gpu_retirement_refusals\":4,\"report_coalescing\":true,\"report_final_current\":true,\"report_error_propagation\":true,\"ack_cases\":"<<ackCases<<",\"retired_cases\":"<<retired<<",\"refusals\":"<<refusals<<",\"idempotent\":true}"<<std::endl;return 0;
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
