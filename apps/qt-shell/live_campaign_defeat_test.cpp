#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "mini_menu_widget.hpp"
#include "battle_result_widget.hpp"
#include <QLabel>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QKeyEvent>
#include <QProcess>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtEndian>
#include <memory>
void installLiveCampaignDefeatTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 session.campaignDefeatEnabled=true;
 installLiveRegionEnterTest(app,window,session,path);
 session.smokeSeconds=0; // This bounded harness requires normal Quit, not deadline survival.
 struct Evidence {int menus=0,returns=0;bool game=false,waiting=false,main=false,defeat=false;QJsonArray states;QString error;};auto e=std::make_shared<Evidence>();
 auto failed=session.failed;auto fail=[e,failed](const QString& error){e->error=error;if(failed)failed(error);};
 auto* input=new QTimer(&window);input->setInterval(1000);
 QObject::connect(input,&QTimer::timeout,&window,[e,fail,&session]{
  if(!e->game||e->waiting||e->menus>=2)return;
  // Region Enter admission precedes actual World initialization. Wait for
  // observed gameplay updates before sending original input into the viewport.
  QFile events(QDir(session.evidenceDirectory()).filePath("events.bin"));
  if(!events.open(QIODevice::ReadOnly))return;
  const QByteArray bytes=events.readAll();int ticks=0;
  for(int at=16;at+64<=bytes.size();at+=64){
   const auto word=[&bytes,at](int index){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(bytes.constData()+at+4*index));};
   if(word(1)==12&&word(3)==2&&word(6)==1&&word(11)==0x6cbb78)++ticks;
  }
  if(ticks<3)return;
  if(QProcess::execute("python3",{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"})!=0)fail("Original Escape injection failed.");
 });input->start();
 auto battle=session.battleStarted;session.battleStarted=[e,battle](quint32 destination){if(battle)battle(destination);e->game=destination==3;};
 auto viewport=session.originalViewportRequested;session.originalViewportRequested=[e,viewport,fail,&window,&session,path]{
  if(viewport){viewport();}++e->returns;if(e->returns==3)return;
  auto* answer=new QProcess(&window);answer->setProcessChannelMode(QProcess::MergedChannels);
  QObject::connect(answer,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[e,answer,fail](int code,QProcess::ExitStatus status){if(code||status!=QProcess::NormalExit)fail("Original confirmation input failed: "+QString::fromLocal8Bit(answer->readAll()));e->waiting=false;answer->deleteLater();});
  auto* container=window.findChild<QWidget*>("legacyGameContainer");if(!container||container->width()<800||container->height()<600){fail("Original confirmation viewport unavailable.");answer->deleteLater();return;}
  const QPoint origin=container->mapToGlobal(QPoint((container->width()-800)/2,(container->height()-600)/2));
  answer->start("python3",{QDir::current().filePath("tools/test-live-qt-campaign-defeat.py"),"--origin-x",QString::number(origin.x()),"--origin-y",QString::number(origin.y()),"--answer",e->menus==1?"no":"yes","--experiment",session.evidenceDirectory(),"--capture",QFileInfo(path).dir().filePath(QString("original-confirmation-%1.png").arg(e->menus))});
 };
 auto state=session.stateChanged;session.stateChanged=[e,state,fail,&window,path](const MenuBridge::State& s){
  if(state)state(s);
  if(e->menus==2&&s.ready&&s.screen==6&&!e->defeat){
   if(s.ack!=5||s.defeat.actions!=1||s.defeat.context!=5||s.defeat.depth!=5||s.defeat.outcome||s.defeat.title.isEmpty()||s.defeat.texts[19]!="You quit the battle."){fail("Unexpected original campaign defeat report.");return;}
   auto* report=window.findChild<BattleResultWidget*>();if(!report||!report->isVisible()||report->outcome()!=BattleResultWidget::Outcome::Defeat){fail("Native defeat report unavailable.");return;}
   if(report->findChild<QLabel*>("battleResultTitle")->text()!=s.defeat.title){fail("Original report title changed.");return;}
   for(int i=0;i<21;++i){auto* label=report->findChild<QLabel*>(QString("battleResultText%1").arg(i+1));if(!label||label->text()!=s.defeat.texts[i]||label->isVisible()!=!s.defeat.texts[i].isEmpty()){fail("Original report display text changed.");return;}}
   e->defeat=true;QJsonArray texts;for(const auto& t:s.defeat.texts)texts.append(t);e->states.append(QJsonObject{{"screen",6},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"title",s.defeat.title},{"texts",texts}});
   QTimer::singleShot(0,&window,[report,&window,path]{window.grab().save(QFileInfo(path).dir().filePath("qt-campaign-defeat.png"));report->findChild<QPushButton*>("battleResultContinue")->click();});return;
  }
  if(e->menus==2&&s.ready&&s.screen==3&&!e->main){
   if(s.ack!=6||s.handoff||s.thread!=quint32(e->states[0].toObject()["thread"].toInt())){fail("Campaign Quit did not restore original Main ownership.");return;}e->main=true;e->game=false;
   QTimer::singleShot(0,&window,[fail,&window,path]{window.grab().save(QFileInfo(path).dir().filePath("qt-campaign-quit-main.png"));auto* b=window.findChild<QPushButton*>("mainMenuAction4");if(!b||!b->isEnabled()){fail("Native Main Quit unavailable after campaign exit.");return;}b->click();});return;
  }
  if(!s.ready||s.screen!=17||e->waiting)return;
  if(e->menus>=2||s.ack!=quint32(3+e->menus)||s.mini.battle||s.mini.context!=5||s.mini.mode!=2||s.mini.depth!=5||s.mini.actions!=5){fail("Unexpected campaign Mini ownership/acknowledgement.");return;}
  auto* w=window.findChild<MiniMenuWidget*>();if(!w||!w->isVisible()||w->mode()!=MiniMenuWidget::Mode::Campaign){fail("Native campaign Mini not presented.");return;}
  for(int i=1;i<=5;++i){auto* b=w->findChild<QPushButton*>(QString("miniMenuButton%1").arg(i));if(!b||b->isEnabled()!=(i==4||i==5)){fail("Unsupported campaign controls offered.");return;}}
  e->states.append(QJsonObject{{"screen",17},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"context",int(s.mini.context)},{"mode",int(s.mini.mode)},{"depth",int(s.mini.depth)},{"actions",int(s.mini.actions)}});
  int pass=e->menus++;e->waiting=true;
  QTimer::singleShot(0,&window,[w,pass,&window,path]{window.grab().save(QFileInfo(path).dir().filePath(QString("qt-campaign-mini-%1.png").arg(pass)));w->findChild<QPushButton*>("miniMenuButton4")->click();});
 };
 auto finished=session.finished;session.finished=[e,finished,path,&app]{
  if(finished){finished();}
  QFile f(path);if(!f.open(QIODevice::ReadOnly)){app.exit(1);return;}auto r=QJsonDocument::fromJson(f.readAll()).object();f.close();bool ok=r["success"].toBool()&&e->menus==2&&e->returns==3&&e->defeat&&e->main&&e->error.isEmpty();r["success"]=ok;r["mini_states"]=e->states;r["viewport_returns"]=e->returns;r["mini_error"]=e->error;r["main_return"]=e->main;r["native_defeat"]=e->defeat;r["scope"]="Qt New Game/Adept/Enter, original gameplay Escape, native campaign Quit to original confirmation No and World resume, repeat Quit/Yes to original defeat report, native report with all original text and original OK callback, World/Realm return, native Main and normal Main Quit; save/load/preferences and timer pause equivalence pending";
  if(!f.open(QIODevice::WriteOnly)||f.write(QJsonDocument(r).toJson())<0||!f.flush()){ok=false;}
  app.exit(ok?0:1);
 };
}
