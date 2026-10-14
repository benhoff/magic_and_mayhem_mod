#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "multiplayer_setup_widget.hpp"
#include "../../protocols/include/mnm/menu_v15.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QKeyEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
void installLiveMultiplayerTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 struct Evidence {int stage=0,refreshes=0;quint32 thread=0;QString error;bool draft=false;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"draft_preserved",e->draft},{"draft_refreshes",e->refreshes},{"error",e->error},{"scope","Native Main/Quick/Create/Join form admission, original defaults and availability, bounded edits and draft retention, Cancel/Escape/window-close original return. No networking or live submission."}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& text){e->error=text;std::fprintf(stderr,"Multiplayer validation: %s\n",qPrintable(text));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& t){if(output)output(t);std::fprintf(stderr,"%s",qPrintable(t));};
 auto failure=session.failed;session.failed=[failure,fail](const QString& t){if(failure)failure(t);fail(t);};
 auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
  if(presentation)presentation(s);
  if(e->stage==3&&s.ready&&s.screen==15&&s.ack==2)++e->refreshes;
  const int screens[]={3,22,15,22,16,22,15};
  if(!s.ready||e->stage>=7||s.screen!=quint32(screens[e->stage])||s.ack!=quint32(e->stage))return;
  if((e->stage&&s.status!=MNM_MENU_OK)||(e->thread&&s.thread!=e->thread)){fail("Original thread or acknowledgement differs.");return;}
  if((s.screen==15||s.screen==16)&&(s.multiplayer.transport!=0||s.multiplayer.actions!=3||s.multiplayer.userLimit!=15)){fail("Original form defaults differ.");return;}
  const int stage=e->stage++;e->thread=s.thread;e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"transport",int(s.multiplayer.transport)},{"available",int(s.multiplayer.available)},{"game_name",s.multiplayer.gameName},{"user_name",s.multiplayer.userName}});
  QTimer::singleShot(0,&window,[stage,s,e,fail,&window,path]{
   window.grab().save(QFileInfo(path).dir().filePath(QString("qt-multiplayer-%1.png").arg(stage)));
   auto click=[&](const char* name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isVisible()||!b->isEnabled()){fail(QString("Unavailable control: ")+name);return;}b->click();};
   if(stage==0){click("mainMenuAction2");return;}
   if(stage==1||stage==5){click("quickBattleAction0");return;}
   if(stage==3){click("quickBattleAction1");return;}
   MultiplayerSetupWidget* w=nullptr;for(auto* candidate:window.findChildren<MultiplayerSetupWidget*>())if(candidate->isVisible())w=candidate;
   if(!w){fail("Native multiplayer form is not visible.");return;}
   const auto f=w->form();if(f.gameName!=s.multiplayer.gameName||f.userName!=s.multiplayer.userName||int(f.transport)!=int(s.multiplayer.transport)){fail("Native form differs from original defaults.");return;}
   for(int i=0;i<3;++i)if(w->findChild<QRadioButton*>(QString("multiplayerTransport%1").arg(i+1))->isEnabled()!=bool(s.multiplayer.available&(1u<<i))){fail("Transport availability differs.");return;}
   if(stage==2){auto* game=w->findChild<QLineEdit*>("multiplayerEdit1");auto* user=w->findChild<QLineEdit*>("multiplayerEdit2");if(game->maxLength()!=63||user->maxLength()!=15){fail("Live name bounds differ.");return;}
    game->setText(" Native game ");user->setText("1234567890123456");w->findChild<QRadioButton*>("multiplayerTransport3")->click();
    QTimer::singleShot(350,&window,[w,game,user,e,fail]{if(game->text()!=" Native game "||user->text()!="123456789012345"||w->form().transport!=MultiplayerSetupWidget::Transport::NullModem||e->refreshes<2){fail("Engine refresh replaced native draft.");return;}e->draft=true;w->findChild<QPushButton*>("multiplayerCancel")->click();});return;}
   if(stage==4){QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(w,&escape);return;}
   if(stage==6)window.close();
  });
 };
 auto finished=session.finished;session.finished=[e,save,finished,&app]{const bool success=e->stage==7&&e->draft&&e->error.isEmpty();const bool saved=save(success);if(finished)finished();app.exit(success&&saved?0:1);};
 auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Bounded multiplayer validation expired.");});};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
 QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
