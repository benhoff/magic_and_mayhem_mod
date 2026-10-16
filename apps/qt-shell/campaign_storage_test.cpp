#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include "../../protocols/include/mnm/menu_v13.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QAbstractButton>
#include <QLineEdit>
#include <QListWidget>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QProcess>
#include <QElapsedTimer>
#include <QRadioButton>
#include <QDateTime>
#include <QThread>
#include <QtEndian>
#include <memory>
#include <cstdio>
void installCampaignStorageTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path,std::function<CampaignPresentationProbe()> probe){
 if(qEnvironmentVariableIntValue("MNM_CAMPAIGN_STORAGE_BOUNDARIES")==1){installCampaignStorageBoundaryTest(app,window,session,path);return;}
 const bool restart=qEnvironmentVariableIntValue("MNM_CAMPAIGN_STORAGE_RESTART")==1;
 const bool resident=qEnvironmentVariable("MNM_PUBLIC_RESIDENT_WORLD")=="1";
 const int difficulty=qEnvironmentVariableIntValue("MNM_CAMPAIGN_DIFFICULTY");
 const int loadingDwell=qEnvironmentVariableIntValue("MNM_CAMPAIGN_LOADING_MENU_SECONDS");
 const bool recreateWorld=qEnvironmentVariableIntValue("MNM_CAMPAIGN_RECREATE_WORLD")==1;
 const int requestedTimeout=qEnvironmentVariableIntValue("MNM_CAMPAIGN_TIMEOUT_MS");
 const int deadlineMs=requestedTimeout>=25000&&requestedTimeout<=595000?requestedTimeout:240000+loadingDwell*1000;
 const int miniCaptureDelay=qBound(0,qEnvironmentVariableIntValue("MNM_CAMPAIGN_MINI_CAPTURE_DELAY_MS"),1000);
 struct Run {int stage=0,attempts=0,input=0,rebuilds=0;quint32 journalBase=0;bool helper=false,failed=false,reloaded=false;quint32 thread=0;quint64 frames=0,source=0,inputSource=0;QString name,hash,error;QJsonArray states,checkpoints;QElapsedTimer dwell,loadingDwell;int loadingDwellMs=0;};auto r=std::make_shared<Run>();r->name=qEnvironmentVariable("MNM_CAMPAIGN_STORAGE_NAME","native-menu-live");r->dwell.start();
 auto write=[r,path,probe,resident,recreateWorld,loadingDwell,deadlineMs](bool success){const auto p=probe();QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"deadline_ms",deadlineMs},{"stage",r->stage},{"error",r->error},{"states",r->states},{"checkpoints",r->checkpoints},{"saved_sha256",r->hash},{"saved_name",r->name},{"save_load",true},{"recreate_world",recreateWorld},{"loading_menu_seconds",loadingDwell},{"loading_menu_elapsed_ms",r->loadingDwellMs},{"display_rebuilds",r->rebuilds},{"reloaded_from_main",r->reloaded},{"normal_quit",success&&r->stage==12},{"steps",QJsonArray{}},{"native_command_frames",qint64(p.frames)},{"native_painted_frames",qint64(p.paints)},{"native_command_fallback",p.fallback},{"native_recoveries",int(p.recoveries)},{"native_error",p.error},{"renderer",p.renderer},{"resident_world_requested",resident},{"scope","Native campaign Save/Load, fresh post-action World sources and normal Quit; independent restored World raster comparison is a separate recorded check"}}).toJson())>0&&f.flush();};
 auto fail=[r,write,&app](const QString& why){if(r->failed)return;r->failed=true;r->error=why;std::fprintf(stderr,"Campaign storage failed: %s\n",qPrintable(why));write(false);app.exit(1);};
 session.miniRequestObserved=[r,fail](quint32 action,bool accepted,const QString& why,const MenuBridge::State& s){
  r->states.append(QJsonObject{{"event","mini-request"},{"time_ms",QDateTime::currentMSecsSinceEpoch()},{"action",int(action)},{"accepted",accepted},{"reason",why},{"screen",int(s.screen)},{"generation",int(s.generation)},{"ack",int(s.ack)},{"stage",r->stage}});
  if(!accepted)fail("Native Mini action rejected: "+why);
 };
 auto click=[&window,fail](const QString& name){auto* b=window.findChild<QAbstractButton*>(name);if(!b||!b->isVisible()||!b->isEnabled()){fail("Native control unavailable: "+name);return;}b->click();};
 auto input=[r,fail,probe,resident,difficulty,&session,&window,path](const QString& mode){
  auto* gl=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));QWidget* v=gl&&gl->isVisible()?static_cast<QWidget*>(gl):window.findChild<QWidget*>("legacyGameContainer");if(!v||!v->isVisible()){fail("Game viewport unavailable");return;}window.activateWindow();window.raise();v->setFocus();const QRectF rect=gl&&gl->isVisible()?gl->imageRect():QRectF((v->width()-800)/2,(v->height()-600)/2,800,600);const auto origin=v->mapToGlobal(rect.topLeft().toPoint());auto* p=new QProcess(&window);r->helper=true;
  QObject::connect(p,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[r,p,fail](int code,QProcess::ExitStatus status){r->helper=false;if(code||status!=QProcess::NormalExit)fail("Physical storage input failed: "+QString::fromLocal8Bit(p->readAllStandardError()));p->deleteLater();});
  const auto record=QString("storage-input-%1.json").arg(r->input++);if(mode=="mini"){r->inputSource=probe().sourceGeneration;r->states.append(QJsonObject{{"event","mini-input"},{"record",record},{"source_generation",QString::number(r->inputSource)},{"stage",r->stage}});}
  QStringList args{QDir::current().filePath("tools/menu-storage-input.py"),"--mode",mode,"--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--output",QFileInfo(path).dir().filePath(record)};
  if(resident)args<<"--input-channel"<<QDir(session.evidenceDirectory()).filePath("render-input.bin");
  // Focus was already assigned above. A scene click is a game command and
  // can overlap the newly opened original Mini while rendering is delayed.
  if(mode=="mini")args<<"--skip-click";
  if(mode=="mini"&&difficulty==0)args<<"--initiate-tutorial";
  if(mode=="mini"&&difficulty!=0)args<<"--dismiss-dialogue";
  if(mode=="mini"&&difficulty==0&&r->stage==3)args<<"--complete-opening";
  p->start("python3",args);
 };
 auto output=session.output;session.output=[output](const QString& s){if(output)output(s);std::fprintf(stderr,"%s",qPrintable(s));};auto failed=session.failed;session.failed=[failed,fail](const QString& s){if(failed)failed(s);fail(s);};
 auto old=session.stateChanged;session.stateChanged=[r,old,fail,click,probe,restart,difficulty,recreateWorld,loadingDwell,miniCaptureDelay,&session,&window,path](const MenuBridge::State& s){
  if(old){old(s);}if(!s.ready||r->failed){return;}if(r->thread&&r->thread!=s.thread){fail("Menu thread changed");return;}r->thread=s.thread;QString action;
  if((r->stage==4||r->stage==7||r->stage==10)&&s.screen==10){fail("Original Preferences detour while awaiting a Mini action");return;}
  if(r->stage==0&&s.screen==3){r->stage=1;action=restart?"mainMenuAction1":"mainMenuAction0";}
  else if(restart&&r->stage==1&&s.screen==12){auto at=s.storage.names.indexOf(r->name);auto* list=window.findChild<QListWidget*>("loadGameList");if(at<0||!list){fail("Persistent save missing from fresh Main Load");return;}list->setCurrentRow(at);r->stage=8;r->frames=probe().frames;action="loadGameLoad";}
  else if(r->stage==1&&s.screen==18){r->stage=difficulty?13:2;action=difficulty?QString("regionEntryDifficulty%1").arg(difficulty):QString("regionEntryEnter");}
  else if(r->stage==13&&s.screen==18&&s.ready&&s.ack==2){r->stage=2;action="regionEntryEnter";}
  else if(r->stage==3&&s.screen==17){r->stage=4;action="miniMenuButton2";}
  else if(r->stage==4&&s.screen==13){r->stage=5;auto* edit=window.findChild<QLineEdit*>("saveGameFileName");if(!edit){fail("Native Save edit unavailable");return;}edit->setText(r->name);r->frames=probe().frames;r->source=probe().sourceGeneration;r->dwell.restart();action="saveGameSave";}
  else if(r->stage==6&&s.screen==17){r->stage=7;action="miniMenuButton1";}
  else if(r->stage==7&&s.screen==12){auto at=s.storage.names.indexOf(r->name);auto* list=window.findChild<QListWidget*>("loadGameList");if(at<0||!list){fail("Original Load list lacks newly saved row");return;}list->setCurrentRow(at);r->stage=8;r->frames=probe().frames;r->source=probe().sourceGeneration;r->dwell.restart();action="loadGameLoad";}
  else if(r->stage==9&&s.screen==17){r->stage=10;action="miniMenuButton4";}
  else if(r->stage==10&&s.screen==6){r->stage=11;action="battleResultContinue";}
  else if(r->stage==11&&s.screen==3){r->stage=recreateWorld&&!r->reloaded?14:12;action=r->stage==14?"mainMenuAction3":"mainMenuAction4";}
  else if((r->stage==14||r->stage==16)&&s.screen==10){
   const int expected=r->stage==14?0:1;
   if(s.preferences.values[2]!=expected||(s.preferences.available&3)!=3){fail("Original display modes unavailable or unexpected resolution");return;}
   auto* radio=window.findChild<QRadioButton*>(QString("preferencesRadio%1").arg(2-expected));
   if(!radio||!radio->isEnabled()){fail("Resolution control unavailable");return;}radio->click();++r->rebuilds;++r->stage;action="preferencesOk";
  }
  else if(r->stage==15&&s.screen==3){r->stage=16;action="mainMenuAction3";}
  else if(r->stage==17&&s.screen==3){r->stage=18;action="mainMenuAction1";}
  else if(r->stage==18&&s.screen==12){
   auto at=s.storage.names.indexOf(r->name);auto* list=window.findChild<QListWidget*>("loadGameList");if(at<0||!list){fail("Saved map missing after original display rebuild");return;}
   list->setCurrentRow(at);r->reloaded=true;r->stage=8;r->frames=probe().frames;r->source=probe().sourceGeneration;r->dwell.restart();action="loadGameLoad";
  }
  if(action.isEmpty()){return;}r->attempts=0;r->journalBase=s.sequence;r->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"thread",int(s.thread)},{"generation",int(s.generation)},{"stage",r->stage},{"parent",int(s.storage.parent)},{"depth",int(s.storage.depth)},{"resolution",int(s.preferences.values[2])},{"rebuilds",r->rebuilds},{"reloaded",r->reloaded}});
  const int delay=loadingDwell>0&&r->reloaded&&r->stage==8&&action=="loadGameLoad"?loadingDwell*1000:0;
  if(delay)r->loadingDwell.start();
  QTimer::singleShot(delay,Qt::PreciseTimer,&window,[click,fail,action,&window,path,r,delay,miniCaptureDelay]{
   if(r->failed)return;
   if(delay){r->loadingDwellMs=int(r->loadingDwell.elapsed());if(r->loadingDwellMs<delay){fail("Load dwell timer fired early");return;}}
   // Submit against current menu ownership before optional diagnostic rendering
   // can block the GUI and age the host lease or the displayed generation.
   const auto submitted=QDateTime::currentMSecsSinceEpoch();click(action);if(r->failed)return;
   const auto begin=QDateTime::currentMSecsSinceEpoch();QElapsedTimer captureClock;captureClock.start();const int blocked=action.startsWith("miniMenuButton")?miniCaptureDelay:0;
   if(blocked)QThread::msleep(blocked); // Explicit bounded test stall after dispatch.
   window.grab().save(QFileInfo(path).dir().filePath(QString("campaign-storage-%1.png").arg(r->stage)));
   r->states.append(QJsonObject{{"event","click"},{"action",action},{"stage",r->stage},{"time_ms",submitted},{"capture_begin_ms",begin},{"capture_end_ms",QDateTime::currentMSecsSinceEpoch()},{"capture_duration_ms",captureClock.elapsed()},{"injected_capture_delay_ms",blocked}});
  });
 };
 auto battle=session.battleStarted;session.battleStarted=[r,battle,probe,fail](quint32 destination){if(battle)battle(destination);if(destination!=3||r->stage!=2){fail("Unexpected campaign handoff");return;}r->stage=3;r->frames=probe().frames;r->source=probe().sourceGeneration;r->dwell.restart();};
 auto viewport=session.originalViewportRequested;session.originalViewportRequested=[r,viewport,input,&window]{if(viewport)viewport();if(r->stage==10)QTimer::singleShot(800,&window,[input]{input("yes");});};
 auto* timer=new QTimer(&window);timer->setInterval(1500);QObject::connect(timer,&QTimer::timeout,&window,[r,probe,fail,input,resident,difficulty,&session]{
  if(r->failed||r->helper){return;}const auto p=probe();
  QFile events(QDir(session.evidenceDirectory()).filePath("events.bin"));bool originalWorld=false,returned=false;
  if(events.open(QIODevice::ReadOnly)){const auto bytes=events.readAll();for(int at=16;at+64<=bytes.size();at+=64){auto word=[&](int i){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(bytes.constData()+at+4*i));};if((word(1)==12||word(1)==20)&&word(3)==2&&word(6)==1)originalWorld=true;if(word(1)==4&&word(8)==1&&((r->stage==5&&word(3)==13)||(r->stage==8&&word(3)==12)))returned=true;}}
  const bool presenting=p.active?(p.frames>r->frames+10&&(!resident||campaignResumeConfirmed(p,r->frames,r->source,true))):(!resident&&originalWorld);if(p.fallback||!p.error.isEmpty()||p.recoveries){fail("Native presentation failed: "+p.error);return;}
  if((r->stage==5||r->stage==8)&&presenting&&returned){
   QFile saved(QDir(session.evidenceDirectory()).filePath("game/Save/"+r->name+".sav"));if(!saved.open(QIODevice::ReadOnly)||saved.size()==0){fail("Original writer produced no nonempty save");return;}const auto bytes=saved.readAll();const auto hash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());if(r->hash.isEmpty())r->hash=hash;else if(r->hash!=hash){fail("Load changed saved bytes");return;}r->stage=r->stage==5?6:9;r->attempts=0;
  }
  if((r->stage==3||r->stage==6||r->stage==9)&&presenting&&(!resident||p.sourceGeneration>r->inputSource)&&r->dwell.elapsed()>=6000){if(++r->attempts>(difficulty==0&&r->stage==3?24:8)){fail("Original Mini ingress timed out");return;}if(r->attempts==1)r->checkpoints.append(QJsonObject{{"phase",r->stage==3?"before-save":r->stage==6?"after-save":r->reloaded?"after-recreate-load":"after-load"},{"frames",qint64(p.frames)},{"source_generation",QString::number(p.sourceGeneration)},{"source_base",QString::number(r->source)},{"frame_base",qint64(r->frames)},{"native_active",p.active},{"fallback",p.fallback},{"error",p.error}});input("mini");}
 });timer->start();
 auto finished=session.finished;session.finished=[r,finished,write,&app]{if(finished)finished();const bool success=!r->failed&&r->stage==12&&!r->hash.isEmpty();app.exit(success&&write(success)?0:1);};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;QTimer::singleShot(deadlineMs,&window,[fail]{fail("Campaign Save/Load deadline expired");});QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
