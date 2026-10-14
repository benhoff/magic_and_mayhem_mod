#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "multiplayer_setup_widget.hpp"
#include "multiplayer_game_selection_widget.hpp"
#include "../../protocols/include/mnm/menu_v16.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
void installLiveMultiplayerSessionsTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 struct Evidence{int stage=0,handoffs=0;quint32 thread=0;QString error;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"handoffs",e->handoffs},{"error",e->error},{"scope","Original TCP Join submission/initial empty enumeration and native session-list Cancel/window-close; no peer joins or lobby/gameplay validation."}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& t){e->error=t;std::fprintf(stderr,"Session validation: %s\n",qPrintable(t));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& t){if(output)output(t);std::fprintf(stderr,"%s",qPrintable(t));};auto failure=session.failed;session.failed=[failure,fail](const QString& t){if(failure)failure(t);fail(t);};
 auto handoff=session.originalViewportRequested;session.originalViewportRequested=[e,handoff]{++e->handoffs;if(handoff)handoff();};
 auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
  if(presentation)presentation(s);
  const int screens[]={3,22,15,22,16,21,22,16,21};
  if(!s.ready||e->stage>=9||s.screen!=quint32(screens[e->stage])||s.ack!=quint32(e->stage))return;
  if((e->stage&&s.status!=MNM_MENU_OK)||(e->thread&&e->thread!=s.thread)){fail("Original thread/acknowledgement differs.");return;}
  if(s.screen==21&&(s.sessions.transport!=2||s.sessions.actions!=3||s.sessions.busy||s.sessions.pending||!s.sessions.active||!s.sessions.names.isEmpty())){fail("Original enumeration is not the expected empty TCP session list.");return;}
  const int stage=e->stage++;e->thread=s.thread;e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"count",s.sessions.names.size()},{"transport",int(s.sessions.transport)}});
  QTimer::singleShot(0,&window,[stage,e,fail,&window,path]{
   window.grab().save(QFileInfo(path).dir().filePath(QString("qt-session-%1.png").arg(stage)));
   auto click=[&](const char* name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isVisible()||!b->isEnabled()){fail(QString("Unavailable control: ")+name);return;}b->click();};
   if(stage==0){click("mainMenuAction2");return;}if(stage==1){click("quickBattleAction0");return;}if(stage==3||stage==6){click("quickBattleAction1");return;}
   if(stage==2){for(auto* w:window.findChildren<MultiplayerSetupWidget*>())if(w->isVisible())w->findChild<QPushButton*>("multiplayerCancel")->click();return;}
   if(stage==4||stage==7){MultiplayerSetupWidget* w=nullptr;for(auto* c:window.findChildren<MultiplayerSetupWidget*>())if(c->isVisible())w=c;if(!w){fail("Join form missing.");return;}w->findChild<QLineEdit*>("multiplayerEdit1")->setText("NativeJoin");auto* tcp=w->findChild<QRadioButton*>("multiplayerTransport2");if(!tcp->isEnabled()){fail("TCP transport unavailable.");return;}tcp->click();w->findChild<QPushButton*>("multiplayerOk")->click();return;}
   auto* list=window.findChild<MultiplayerGameSelectionWidget*>();if(!list||!list->isVisible()||list->findChild<QListWidget*>("multiplayerGameSelectionList")->count()||list->findChild<QPushButton*>("multiplayerGameSelectionOk")->isEnabled()){fail("Native empty session list did not own presentation.");return;}
   if(stage==5)click("multiplayerGameSelectionCancel");else if(stage==8)window.close();
  });
 };
 auto finished=session.finished;session.finished=[e,save,finished,&app]{const bool success=e->stage==9&&e->handoffs==2&&e->error.isEmpty();const bool saved=save(success);if(finished)finished();app.exit(success&&saved?0:1);};
 auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Bounded TCP session validation expired.");});};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
