#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "region_entry_widget.hpp"
#include "../../protocols/include/mnm/menu_v7.h"
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
void installLiveRegionEntryTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 struct Evidence {int stage=0;quint32 thread=0;QString error;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"error",e->error},{"scope","Qt New Game, four original difficulty selections, Cancel to Main, reopen and window-close Cancel/Quit; fresh Celtic region 1 only"}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& reason){e->error=reason;std::fprintf(stderr,"Region validation: %s\n",qPrintable(reason));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
 auto failure=session.failed;session.failed=[failure,fail](const QString& text){if(failure)failure(text);fail(text);};
 auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
  if(presentation)presentation(s);
  if(!s.ready||e->stage>=8)return;
  const int stage=e->stage;const int screens[]={3,18,18,18,18,18,3,18};const int difficulties[]={0,0,1,2,3,0,0,0};
  if(s.screen!=quint32(screens[stage])||s.ack!=quint32(stage))return;
  if((stage>0&&s.status!=MNM_MENU_OK)||(e->thread&&e->thread!=s.thread)){fail("Engine acknowledgement/thread mismatch.");return;}e->thread=s.thread;
  if(s.screen==18&&s.region.difficulty!=quint32(difficulties[stage])){fail("Original difficulty does not match Qt request.");return;}
  e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"difficulty",int(s.region.difficulty)},{"name",s.region.name}});++e->stage;
  QTimer::singleShot(0,&window,[stage,fail,&window,path]{
   window.grab().save(QFileInfo(path).dir().filePath(QString("qt-region-%1.png").arg(stage)));
   if(stage==0||stage==6){auto* b=window.findChild<QPushButton*>("mainMenuAction0");if(!b||!b->isEnabled()){fail("Qt New Game unavailable.");return;}b->click();return;}
   auto* widget=window.findChild<RegionEntryWidget*>();if(!widget||!widget->isVisible()||widget->findChild<QPushButton*>("regionEntryEnter")->isEnabled()){fail("Qt Region Entry ownership/policy mismatch.");return;}
   if(stage==7){window.close();return;}
   if(stage==5){widget->findChild<QPushButton*>("regionEntryCancel")->click();return;}
   auto* radio=widget->findChild<QRadioButton*>(QString("regionEntryDifficulty%1").arg(stage%4));if(!radio||!radio->isEnabled()){fail("Qt difficulty unavailable.");return;}radio->click();
  });
 };
 auto finished=session.finished;session.finished=[e,finished,save,&app]{const bool success=e->stage==8&&e->error.isEmpty();const bool saved=save(success);if(finished)finished();app.exit(success&&saved?0:1);};
 auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Region bounded run expired.");});};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
 QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
