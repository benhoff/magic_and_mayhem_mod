#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "multiplayer_setup_widget.hpp"
#include "multiplayer_lobby_widget.hpp"
#include "../../protocols/include/mnm/menu_v17.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QSlider>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
void installLiveHostLobbyTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 session.hostLobbyEnabled=true;
 struct Evidence{int stage=0,handoffs=0;quint32 thread=0;QString error;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"handoffs",e->handoffs},{"error",e->error},{"scope","Original TCP Create to native one-local-player host view twice, rules/roster/name projection, disabled unsupported controls and Cancel/window-close to normal Quit; no peer/chat/rule/map/start/gameplay validation."}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& t){e->error=t;std::fprintf(stderr,"Host lobby validation: %s\n",qPrintable(t));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& t){if(output)output(t);std::fprintf(stderr,"%s",qPrintable(t));};auto failure=session.failed;session.failed=[failure,fail](const QString& t){if(failure)failure(t);fail(t);};
 auto handoff=session.originalViewportRequested;session.originalViewportRequested=[e,handoff]{++e->handoffs;if(handoff)handoff();};
 auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
  if(presentation)presentation(s);
  const int screens[]={3,22,15,24,22,15,24};
  if(!s.ready||e->stage>=7||s.screen!=quint32(screens[e->stage])||s.ack!=quint32(e->stage))return;
  if((e->stage&&s.status!=MNM_MENU_OK)||(e->thread&&e->thread!=s.thread)){fail("Original thread/acknowledgement differs.");return;}
  if(s.screen==24&&(s.lobby.provider!=2||s.lobby.actions!=1||!s.lobby.players[0].active||s.lobby.players[0].name!="NativeHost")){fail("Original one-player TCP host view differs.");return;}
  const int stage=e->stage++;e->thread=s.thread;e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"game_name",s.lobby.gameName},{"player",s.lobby.players[0].name},{"transport",int(s.lobby.provider)}});
  QTimer::singleShot(0,&window,[stage,e,fail,&window,path]{
   window.grab().save(QFileInfo(path).dir().filePath(QString("qt-session-%1.png").arg(stage)));
   auto click=[&](const char* name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isVisible()||!b->isEnabled()){fail(QString("Unavailable control: ")+name);return;}b->click();};
   if(stage==0){click("mainMenuAction2");return;}if(stage==1||stage==4){click("quickBattleAction0");return;}
   if(stage==2||stage==5){MultiplayerSetupWidget* w=nullptr;for(auto* c:window.findChildren<MultiplayerSetupWidget*>())if(c->isVisible())w=c;if(!w){fail("Create form missing.");return;}w->findChild<QLineEdit*>("multiplayerEdit1")->setText(QString("NativeRoom%1").arg(stage));w->findChild<QLineEdit*>("multiplayerEdit2")->setText("NativeHost");auto* tcp=w->findChild<QRadioButton*>("multiplayerTransport2");if(!tcp->isEnabled()){fail("TCP unavailable.");return;}tcp->click();w->findChild<QPushButton*>("multiplayerOk")->click();return;}
   auto* lobby=window.findChild<MultiplayerLobbyWidget*>();if(!lobby||!lobby->isVisible()||lobby->mode()!=MultiplayerLobbyWidget::Mode::Host||!lobby->lobby().players[0].active||lobby->lobby().players[0].name!="NativeHost"){fail("Native host view does not own presentation.");return;}
   for(auto* slider:lobby->findChildren<QSlider*>())if(slider->isEnabled()){fail("Unmapped lobby rule control enabled.");return;}
   if(lobby->findChild<QPushButton*>("multiplayerLobbyStart")->isEnabled()||lobby->findChild<QPushButton*>("multiplayerLobbyMap")->isEnabled()){fail("Unmapped lobby action enabled.");return;}
   if(stage==3)click("multiplayerLobbyCancel");else if(stage==6)window.close();

  });
 };
 auto finished=session.finished;session.finished=[e,save,finished,&app]{const bool success=e->stage==7&&e->handoffs==2&&e->error.isEmpty();const bool saved=save(success);if(finished)finished();app.exit(success&&saved?0:1);};
 auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Bounded host view validation expired.");});};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
