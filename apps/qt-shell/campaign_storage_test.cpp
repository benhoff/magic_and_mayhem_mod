#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include "../../protocols/include/mnm/menu_v13.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
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
#include <QtEndian>
#include <memory>
#include <cstdio>
void installCampaignStorageTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path,std::function<CampaignPresentationProbe()> probe){
 if(qEnvironmentVariableIntValue("MNM_CAMPAIGN_STORAGE_BOUNDARIES")==1){installCampaignStorageBoundaryTest(app,window,session,path);return;}
 const bool restart=qEnvironmentVariableIntValue("MNM_CAMPAIGN_STORAGE_RESTART")==1;
 struct Run {int stage=0,attempts=0,input=0;quint32 journalBase=0;bool helper=false,failed=false;quint32 thread=0;quint64 frames=0;QString name,hash,error;QJsonArray states;};auto r=std::make_shared<Run>();r->name=qEnvironmentVariable("MNM_CAMPAIGN_STORAGE_NAME","native-menu-live");
 auto write=[r,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"stage",r->stage},{"error",r->error},{"states",r->states},{"saved_sha256",r->hash},{"saved_name",r->name},{"scope","Native fresh campaign Save/new file/Load/World presentation/Mini/Quit and original confirmation; full restored state equivalence pending"}}).toJson())>0&&f.flush();};
 auto fail=[r,write,&app](const QString& why){r->failed=true;r->error=why;std::fprintf(stderr,"Campaign storage failed: %s\n",qPrintable(why));write(false);app.exit(1);};
 auto click=[&window,fail](const QString& name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isVisible()||!b->isEnabled()){fail("Native control unavailable: "+name);return;}b->click();};
 auto input=[r,fail,&window,path](const QString& mode){
  auto* gl=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));QWidget* v=gl&&gl->isVisible()?static_cast<QWidget*>(gl):window.findChild<QWidget*>("legacyGameContainer");if(!v||!v->isVisible()){fail("Game viewport unavailable");return;}window.activateWindow();window.raise();v->setFocus();const QRectF rect=gl&&gl->isVisible()?gl->imageRect():QRectF((v->width()-800)/2,(v->height()-600)/2,800,600);const auto origin=v->mapToGlobal(rect.topLeft().toPoint());auto* p=new QProcess(&window);r->helper=true;
  QObject::connect(p,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[r,p,fail](int code,QProcess::ExitStatus status){r->helper=false;if(code||status!=QProcess::NormalExit)fail("Physical storage input failed: "+QString::fromLocal8Bit(p->readAllStandardError()));p->deleteLater();});
  p->start("python3",{QDir::current().filePath("tools/menu-storage-input.py"),"--mode",mode,"--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--output",QFileInfo(path).dir().filePath(QString("storage-input-%1.json").arg(r->input++))});
 };
 auto output=session.output;session.output=[output](const QString& s){if(output)output(s);std::fprintf(stderr,"%s",qPrintable(s));};auto failed=session.failed;session.failed=[failed,fail](const QString& s){if(failed)failed(s);fail(s);};
 auto old=session.stateChanged;session.stateChanged=[r,old,fail,click,probe,restart,&session,&window,path](const MenuBridge::State& s){
  if(old){old(s);}if(!s.ready||r->failed){return;}if(r->thread&&r->thread!=s.thread){fail("Menu thread changed");return;}r->thread=s.thread;QString action;
  if(r->stage==0&&s.screen==3){r->stage=1;action=restart?"mainMenuAction1":"mainMenuAction0";}
  else if(restart&&r->stage==1&&s.screen==12){auto at=s.storage.names.indexOf(r->name);auto* list=window.findChild<QListWidget*>("loadGameList");if(at<0||!list){fail("Persistent save missing from fresh Main Load");return;}list->setCurrentRow(at);r->stage=8;r->frames=probe().frames;action="loadGameLoad";}
  else if(r->stage==1&&s.screen==18){r->stage=2;action="regionEntryEnter";}
  else if(r->stage==3&&s.screen==17){r->stage=4;action="miniMenuButton2";}
  else if(r->stage==4&&s.screen==13){r->stage=5;auto* edit=window.findChild<QLineEdit*>("saveGameFileName");if(!edit){fail("Native Save edit unavailable");return;}edit->setText(r->name);r->frames=probe().frames;action="saveGameSave";}
  else if(r->stage==6&&s.screen==17){r->stage=7;action="miniMenuButton1";}
  else if(r->stage==7&&s.screen==12){auto at=s.storage.names.indexOf(r->name);auto* list=window.findChild<QListWidget*>("loadGameList");if(at<0||!list){fail("Original Load list lacks newly saved row");return;}list->setCurrentRow(at);r->stage=8;r->frames=probe().frames;action="loadGameLoad";}
  else if(r->stage==9&&s.screen==17){r->stage=10;action="miniMenuButton4";}
  else if(r->stage==10&&s.screen==6){r->stage=11;action="battleResultContinue";}
  else if(r->stage==11&&s.screen==3){r->stage=12;action="mainMenuAction4";}
  if(action.isEmpty()){return;}r->attempts=0;r->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"thread",int(s.thread)},{"generation",int(s.generation)},{"stage",r->stage},{"parent",int(s.storage.parent)},{"depth",int(s.storage.depth)}});
  QTimer::singleShot(0,&window,[click,action,&window,path,r]{window.grab().save(QFileInfo(path).dir().filePath(QString("campaign-storage-%1.png").arg(r->stage)));click(action);});
 };
 auto battle=session.battleStarted;session.battleStarted=[r,battle,probe,fail](quint32 destination){if(battle)battle(destination);if(destination!=3||r->stage!=2){fail("Unexpected campaign handoff");return;}r->stage=3;r->frames=probe().frames;};
 auto viewport=session.originalViewportRequested;session.originalViewportRequested=[r,viewport,input,&window]{if(viewport)viewport();if(r->stage==10)QTimer::singleShot(800,&window,[input]{input("yes");});};
 auto* timer=new QTimer(&window);timer->setInterval(1500);QObject::connect(timer,&QTimer::timeout,&window,[r,probe,fail,input,&session]{
  if(r->failed||r->helper){return;}const auto p=probe();
  QFile events(QDir(session.evidenceDirectory()).filePath("events.bin"));bool originalWorld=false,returned=false;
  if(events.open(QIODevice::ReadOnly)){const auto bytes=events.readAll();for(int at=16;at+64<=bytes.size();at+=64){auto word=[&](int i){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(bytes.constData()+at+4*i));};if((word(1)==12||word(1)==20)&&word(3)==2&&word(6)==1)originalWorld=true;if(word(1)==4&&word(8)==1&&((r->stage==5&&word(3)==13)||(r->stage==8&&word(3)==12)))returned=true;}}
  const bool presenting=p.active?p.frames>r->frames+10:originalWorld;if(p.fallback||!p.error.isEmpty()){fail("Native presentation failed: "+p.error);return;}
  if((r->stage==5||r->stage==8)&&presenting&&returned){
   QFile saved(QDir(session.evidenceDirectory()).filePath("game/Save/"+r->name+".sav"));if(!saved.open(QIODevice::ReadOnly)||saved.size()==0){fail("Original writer produced no nonempty save");return;}const auto bytes=saved.readAll();const auto hash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());if(r->hash.isEmpty())r->hash=hash;else if(r->hash!=hash){fail("Load changed saved bytes");return;}r->stage=r->stage==5?6:9;r->attempts=0;
  }
  if((r->stage==3||r->stage==6||r->stage==9)&&presenting){if(++r->attempts>8){fail("Original Mini ingress timed out");return;}input("mini");}
 });timer->start();
 auto finished=session.finished;session.finished=[r,finished,write,&app]{if(finished)finished();const bool success=!r->failed&&r->stage==12&&!r->hash.isEmpty();app.exit(success&&write(success)?0:1);};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;QTimer::singleShot(240000,&window,[fail]{fail("Campaign Save/Load deadline expired");});QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
