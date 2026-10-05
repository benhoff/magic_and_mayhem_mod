#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "mini_menu_widget.hpp"
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
#include <memory>
void installLiveCampaignMiniTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 installLiveRegionEnterTest(app,window,session,path);
 struct Evidence {int menus=0,returns=0;bool game=false,waiting=false;QJsonArray states;QString error;};auto e=std::make_shared<Evidence>();
 auto failed=session.failed;auto fail=[e,failed](const QString& error){e->error=error;if(failed)failed(error);};
 auto* input=new QTimer(&window);input->setInterval(1000);
 QObject::connect(input,&QTimer::timeout,&window,[e,fail]{
  if(!e->game||e->waiting||e->menus>=2)return;
  if(QProcess::execute("python3",{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"})!=0)fail("Original Escape injection failed.");
 });input->start();
 auto battle=session.battleStarted;session.battleStarted=[e,battle](quint32 destination){if(battle)battle(destination);e->game=destination==3;};
 auto viewport=session.originalViewportRequested;session.originalViewportRequested=[e,viewport]{if(viewport)viewport();++e->returns;e->waiting=false;};
 auto state=session.stateChanged;session.stateChanged=[e,state,fail,&window,path](const MenuBridge::State& s){
  if(state)state(s);
  if(!s.ready||s.screen!=17||e->waiting)return;
  if(e->menus>=2||s.ack!=quint32(3+e->menus)||s.mini.battle||s.mini.context!=5||s.mini.mode!=2||s.mini.depth!=5||s.mini.actions!=1){fail("Unexpected campaign Mini ownership/acknowledgement.");return;}
  auto* w=window.findChild<MiniMenuWidget*>();if(!w||!w->isVisible()||w->mode()!=MiniMenuWidget::Mode::Campaign){fail("Native campaign Mini not presented.");return;}
  for(int i=1;i<=5;++i){auto* b=w->findChild<QPushButton*>(QString("miniMenuButton%1").arg(i));if(!b||b->isEnabled()!=(i==5)){fail("Unsupported campaign controls offered.");return;}}
  e->states.append(QJsonObject{{"screen",17},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"context",int(s.mini.context)},{"mode",int(s.mini.mode)},{"depth",int(s.mini.depth)},{"actions",int(s.mini.actions)}});
  int pass=e->menus++;e->waiting=true;
  QTimer::singleShot(0,&window,[w,pass,&window,path]{window.grab().save(QFileInfo(path).dir().filePath(QString("qt-campaign-mini-%1.png").arg(pass)));if(!pass)w->findChild<QPushButton*>("miniMenuButton5")->click();else{QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(w,&escape);}});
 };
 auto finished=session.finished;session.finished=[e,finished,path,&app]{
  if(finished){finished();}
  QFile f(path);if(!f.open(QIODevice::ReadOnly)){app.exit(1);return;}auto r=QJsonDocument::fromJson(f.readAll()).object();f.close();bool ok=r["success"].toBool()&&e->menus==2&&e->returns==2&&e->error.isEmpty();r["success"]=ok;r["mini_states"]=e->states;r["viewport_returns"]=e->returns;r["mini_error"]=e->error;r["scope"]="Qt New Game/Adept/Enter, original gameplay Escape to campaign Mini, native Cancel button and Escape to original World resume twice; bounded termination; save/load/preferences/quit and timer pause equivalence pending";
  if(!f.open(QIODevice::WriteOnly)||f.write(QJsonDocument(r).toJson())<0||!f.flush()){ok=false;}
  app.exit(ok?0:1);
 };
}
