#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "region_entry_widget.hpp"
#include "../../protocols/include/mnm/menu_v8.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
void installLiveRegionEnterTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 struct Evidence {int stage=0;quint32 thread=0;bool handoff=false,viewport=false;QString error;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"error",e->error},{"campaign_handoff",e->handoff},{"original_viewport_presented",e->viewport},{"scope","Qt New Game, Adept difficulty and original fresh Celtic region 1 Enter; original gameplay ticks checked independently; bounded termination, campaign return pending"}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& reason){e->error=reason;std::fprintf(stderr,"Region Enter validation: %s\n",qPrintable(reason));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
 auto failure=session.failed;session.failed=[failure,fail](const QString& text){if(failure)failure(text);fail(text);};
 auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
  if(presentation)presentation(s);
  if(!s.ready||e->stage>=3)return;
  const int stage=e->stage;const quint32 screens[]={3,18,18};
  if(s.screen!=screens[stage]||s.ack!=quint32(stage))return;
  if((stage>0&&s.status!=MNM_MENU_OK)||(e->thread&&e->thread!=s.thread)){fail("Engine acknowledgement/thread mismatch.");return;}e->thread=s.thread;
  if(s.screen==18&&(s.region.difficulty!=quint32(stage==2?2:0)||!(s.region.actions&2))){fail("Original difficulty/Enter availability mismatch.");return;}
  e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"difficulty",int(s.region.difficulty)}});++e->stage;
  QTimer::singleShot(0,&window,[stage,fail,&window,path]{
   window.grab().save(QFileInfo(path).dir().filePath(QString("qt-region-enter-%1.png").arg(stage)));
   if(stage==0){auto* b=window.findChild<QPushButton*>("mainMenuAction0");if(!b||!b->isEnabled()){fail("Qt New Game unavailable.");return;}b->click();return;}
   auto* w=window.findChild<RegionEntryWidget*>();if(!w||!w->isVisible()){fail("Qt Region Entry not presented.");return;}
   if(stage==1){auto* r=w->findChild<QRadioButton*>("regionEntryDifficulty2");if(!r||!r->isEnabled()){fail("Qt difficulty unavailable.");return;}r->click();}
   else{auto* b=w->findChild<QPushButton*>("regionEntryEnter");if(!b||!b->isEnabled()){fail("Qt Enter unavailable.");return;}b->click();}
  });
 };
 auto battle=session.battleStarted;session.battleStarted=[e,battle,fail,&window,path](quint32 destination){
  if(battle)battle(destination);
  if(destination!=3||e->stage!=3){fail("Unexpected campaign handoff.");return;}e->handoff=true;
  QTimer::singleShot(1000,&window,[e,fail,&window,path]{auto* v=window.findChild<QWidget*>("legacyGameContainer");e->viewport=v&&v->isVisible();if(!e->viewport){fail("Original game viewport not presented.");return;}window.grab().save(QFileInfo(path).dir().filePath("qt-campaign-viewport.png"));});
 };
 auto finished=session.finished;session.finished=[e,finished,save,&app]{const bool success=e->stage==3&&e->handoff&&e->viewport&&e->error.isEmpty();const bool saved=save(success);if(finished)finished();app.exit(success&&saved?0:1);};
 auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(210000,&window,[fail]{fail("Region Enter bounded run expired.");});};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=90;
 QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
