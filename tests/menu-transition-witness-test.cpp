#include "live_menu_session.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QtEndian>
#include <iostream>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct LiveMenuSessionTestAccess {
 static void scenario(const QString& root,bool control){
  qputenv("MNM_MENU_TIMELINE_CONTROL",control?"1":"0");
  LiveMenuSession s("/unused");s.bridge_=std::make_unique<MenuBridge>();
  const auto channel=QDir(root).filePath(control?"control-wire.bin":"normal-wire.bin");
  check(s.bridge_->create(channel),"wire create");s.active_=true;s.clock_.start();
  QFile f(channel);check(f.open(QIODevice::ReadWrite),"wire open");auto* bytes=f.map(0,f.size());check(bytes,"wire map");
  auto put=[&](unsigned i,quint32 v){qToLittleEndian(v,bytes+4*i);};auto get=[&](unsigned i){return qFromLittleEndian<quint32>(bytes+4*i);};
  put(16,2);put(17,1);put(18,3);put(19,1);put(20,0);put(21,0);put(22,123);s.poll();
  check(s.menuTimeline_.open(QDir(root).filePath(control?"control-host.bin":"normal-host.bin")),"witness create");s.menuWitness(MNM_MTL_CREATED);
  check(s.request(MNM_MENU_OPEN_QUICK),"one Main publication");check(get(7)==1,"request identity");
  put(16,4);put(17,2);put(18,22);put(19,1);put(20,1);put(21,1);const auto beat=get(6);
  if(control)s.lastState_=-2001; // Synthetic time only; the live guard is untouched.
  s.poll();check(get(6)>beat,"heartbeat continues during diagnostic suppression");
  check(control?!s.active_:s.state_.screen==22&&s.state_.ack==1,"control timeout or real ACK consumption");
  check(control?get(5)==0:get(5)==1,"control retires client without changing ACK");check(get(20)==1,"observer never writes engine ACK");
  s.retireBridge();s.menuTimeline_.close();f.unmap(bytes);
 }
};
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);check(argc==2,"output root required");const QString root=QString::fromLocal8Bit(argv[1]);check(QDir().mkpath(root),"output directory");
 qputenv("MNM_MENU_TIMELINE","1");qputenv("MNM_ACCEPTANCE_PILOT","1");qunsetenv("MNM_ACCEPTANCE_DIAGNOSTIC");
 {MenuTransitionWitness w;check(!w.open(QDir(root).filePath("nominal-forbidden.bin")),"diagnostic gate");check(!w.suppressRead(),"control cannot activate nominally");}
 qputenv("MNM_ACCEPTANCE_DIAGNOSTIC","1");LiveMenuSessionTestAccess::scenario(root,false);LiveMenuSessionTestAccess::scenario(root,true);
 {MenuTransitionWitness w;check(w.open(QDir(root).filePath("overflow-host.bin")),"overflow file");std::array<quint32,16> row{};for(unsigned i=0;i<513;++i)w.record(MNM_MTL_CREATED,row);w.close();}
 std::cout<<"{\"success\":true,\"live_original_executed\":false,\"unchanged_guard_ms\":2000,\"cases\":4}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
