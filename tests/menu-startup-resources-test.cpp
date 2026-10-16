#include "live_menu_session.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QtEndian>
#include <iostream>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void script(const QString& path,const QByteArray& body){QFile f(path);check(f.open(QIODevice::WriteOnly),"script open");check(f.write("#!/usr/bin/python3\n"+body)>0,"script write");f.close();check(f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner),"script permissions");}
struct LiveMenuSessionTestAccess {
 static void media(const QString& path){
  LiveMenuSession s("/unused");s.bridge_=std::make_unique<MenuBridge>();check(s.bridge_->create(path),"wire create");s.active_=true;s.clock_.start();
  QFile f(path);check(f.open(QIODevice::ReadWrite),"wire open");auto* data=f.map(0,f.size());check(data,"wire map");
  auto put=[&](unsigned word,quint32 value){qToLittleEndian(value,data+word*4);};auto get=[&](unsigned word){return qFromLittleEndian<quint32>(data+word*4);};
  put(16,2);put(17,1);put(18,3);put(19,1);put(20,0);put(21,0);put(22,123);
  unsigned notifications=0;s.stateChanged=[&](const MenuBridge::State& state){++notifications;check(!s.mediaActive_,"menu delivery during movie");check(state.thread==123,"original owner retained");};
  s.setMediaActive(true);const auto beat=get(6);s.poll();check(get(6)>beat,"movie must retain host heartbeat");check(!notifications&&!s.sequence_,"movie must defer readiness and retain latest unread state");check(!s.request(MNM_MENU_OPEN_QUICK),"movie input must not dispatch a menu action");
  QThread::msleep(20);s.setMediaActive(false);s.poll();check(notifications==1&&s.sequence_==2&&s.state_.ready,"completion must deliver retained ready state");
  check(s.request(MNM_MENU_OPEN_QUICK)&&get(7)==1,"resumed action must publish once");check(!s.request(MNM_MENU_OPEN_QUICK)&&get(7)==1,"resumed duplicate action must refuse");s.active_=false;f.unmap(data);
 }
};
int main(int argc,char** argv) try {
 QCoreApplication app(argc,argv);
 for(bool refuse:{false,true}){
  QTemporaryDir repo;check(repo.isValid(),"temporary repo");check(QDir().mkpath(repo.filePath("tools")),"tools directory");check(QDir().mkpath(repo.filePath("evidence/game")),"owned staged assets");
  script(repo.filePath("tools/prepare-menu-observer.py"),"from pathlib import Path\nprint('Evidence directory: '+str(Path.cwd()/'evidence'),flush=True)\n");
  script(repo.filePath("tools/run-menu-observer.py"),"from pathlib import Path\nr=Path.cwd()\nassert (r/'assets-ready').is_file() and (r/'channels-ready').is_file()\n(r/'original-launched').write_text('ok')\n");
  LiveMenuSession s(repo.path());s.preferencesMenusEnabled=false;s.nativeCommandsEnabled=true;bool done=false;unsigned assets=0,channels=0;
  s.prepareMenuAssets=[&](const QString& root){++assets;check(root==repo.filePath("evidence/game"),"assets must use staged game");check(!QFile::exists(repo.filePath("evidence/channel.bin"))&&!QFile::exists(repo.filePath("original-launched")),"assets must precede channels and original launch");QThread::msleep(20);if(refuse)return false;QFile f(repo.filePath("assets-ready"));check(f.open(QIODevice::WriteOnly),"assets marker");return true;};
  s.prepareNativePresentation=[&](const QString&){++channels;check(assets==1&&QFile::exists(repo.filePath("assets-ready")),"channels must follow complete asset admission");QFile f(repo.filePath("channels-ready"));return f.open(QIODevice::WriteOnly);};
  s.finished=[&]{done=true;};check(s.start(),"start preparation");QElapsedTimer bound;bound.start();while(!done&&bound.elapsed()<5000){app.processEvents();QThread::msleep(2);}check(done,"bounded startup completion");check(assets==1,"assets prepared exactly once");check(channels==unsigned(!refuse)&&QFile::exists(repo.filePath("original-launched"))==!refuse,"failed assets must prevent channels and original execution");
 }
 QTemporaryDir wire;check(wire.isValid(),"wire directory");LiveMenuSessionTestAccess::media(wire.filePath("movie.bin"));
 std::cout<<"{\"success\":true,\"asset_admission_cases\":2,\"movie_readiness_cases\":1,\"original_execution\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
