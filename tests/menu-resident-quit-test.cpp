#include "live_menu_session.hpp"
#include "resident_world_session.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/menu_v1.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QtEndian>
#include <iostream>
#include <stdexcept>
void check(bool ok){if(!ok)throw std::runtime_error("Resident Quit fixture");}
template<class F> void refuses(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
struct LiveMenuSessionTestAccess {
 static void initialize(LiveMenuSession& s,const QString& path){s.bridge_=std::make_unique<MenuBridge>();check(s.bridge_->create(path));s.active_=true;s.clock_.start();}
 static void poll(LiveMenuSession& s){s.poll();}
};
struct ResidentWorldSessionTestAccess {
 static void counts(ResidentWorldSession& s,unsigned complete,unsigned imports,unsigned returns,bool pending){s.completed_=complete;s.imports_=imports;s.returns_=returns;if(pending)s.pending_.emplace();}
};
int main(int argc,char** argv) try {
 QApplication app(argc,argv);QTemporaryDir root;check(root.isValid());unsigned ackCases=0,refusals=0,retired=0;
 for(unsigned variant=0;variant<5;++variant){
  LiveMenuSession session("/unused");const auto path=root.filePath(QString("menu-%1.bin").arg(variant));LiveMenuSessionTestAccess::initialize(session,path);
  QFile peer(path);check(peer.open(QIODevice::ReadWrite));auto* words=peer.map(0,MNM_MENU_V1_SIZE);check(words);
  const auto state=[&](unsigned sequence,unsigned ack,unsigned status,unsigned thread){
   const unsigned fields[]={sequence,1,3,1,ack,status,thread};
   for(unsigned i=0;i<7;++i)qToLittleEndian<quint32>(fields[i],words+(MNM_MENU_V1_ENGINE_WORD+i)*4);
  };
  state(2,0,MNM_MENU_OK,123);LiveMenuSessionTestAccess::poll(session);check(!session.normalQuitAccepted());
  check(session.request(variant==4?MNM_MENU_OPEN_QUICK:MNM_MENU_QUIT));check(!session.normalQuitAccepted());
  state(4,variant==1?2:1,variant==2?MNM_MENU_UNAVAILABLE:MNM_MENU_OK,variant==3?124:123);
  check(session.normalQuitAccepted()==(variant==0));check(session.normalQuitAccepted()==(variant==0));++ackCases;
  peer.unmap(words);
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
 std::cout<<"{\"ack_cases\":"<<ackCases<<",\"retired_cases\":"<<retired<<",\"refusals\":"<<refusals<<",\"idempotent\":true}"<<std::endl;return 0;
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
