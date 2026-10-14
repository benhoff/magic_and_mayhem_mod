#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "grimoire_widget.hpp"
#include "region_entry_widget.hpp"
#include "../../protocols/include/mnm/menu_v14.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QTextBrowser>
#include <QScrollBar>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QKeyEvent>
#include <memory>
#include <cstdio>
void installLiveGrimoireTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 struct Evidence {int stage=0;quint32 thread=0;QString error;bool scrollPreserved=false;int scrollRefreshes=0;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"scroll_preserved",e->scrollPreserved},{"scroll_refreshes",e->scrollRefreshes},{"error",e->error},{"scope","Native fresh Celtic Region Entry Grimoire: Introduction contextual Forest of Pain, contents/four static Introduction sections, Previous/Next/Contents, Close/Escape and window-close return; original lifecycle retained, native text wrapping/scrolling policy"}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& text){e->error=text;std::fprintf(stderr,"Grimoire validation: %s\n",qPrintable(text));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
 auto failure=session.failed;session.failed=[failure,fail](const QString& text){if(failure)failure(text);fail(text);};
 auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
  if(presentation)presentation(s);
  if(e->stage==6&&s.ready&&s.screen==27&&s.ack==5)++e->scrollRefreshes;
  const int screens[]={3,18,27,27,27,27,27,27,27,27,27,18,27,18,27};
  const int sections[]={0,0,1,0,1,2,3,4,3,0,1,0,1,0,1};
  const int chapters[]={0,0,1,0,0,0,0,0,0,0,1,0,1,0,1};
  if(!s.ready||e->stage>=15||s.screen!=quint32(screens[e->stage])||s.ack!=quint32(e->stage))return;
  const int stage=e->stage;
  if((stage&&s.status!=MNM_MENU_OK)||(e->thread&&s.thread!=e->thread)||(s.screen==27&&(s.grimoire.section!=quint32(sections[stage])||s.grimoire.chapter!=quint32(chapters[stage])))){fail("Original acknowledgement/location/thread differs.");return;}
  e->thread=s.thread;e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"chapter",int(s.grimoire.chapter)},{"section",int(s.grimoire.section)},{"available",int(s.grimoire.available)}});++e->stage;
  QTimer::singleShot(0,&window,[stage,e,fail,&window,path]{
   window.grab().save(QFileInfo(path).dir().filePath(QString("qt-grimoire-%1.png").arg(stage)));
   auto click=[&](const char* name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isEnabled()||!b->isVisible()){fail(QString("Unavailable native control: ")+name);return false;}b->click();return true;};
   if(stage==0){click("mainMenuAction0");return;}
   if(stage==1||stage==11||stage==13){click("regionEntryAuxiliary0");return;}
   auto* book=window.findChild<GrimoireWidget*>();if(!book||!book->isVisible()){fail("Native Grimoire does not own presentation.");return;}
   if(book->findChild<QPushButton*>("grimoireTab2")->isEnabled()||book->findChild<QPushButton*>("grimoireArtwork")->isEnabled()){fail("Unmapped chapter/artwork enabled.");return;}
   if(stage==2){click("grimoireTab0");return;}
   if(stage==3){auto* entries=book->findChild<QListWidget*>("grimoireEntries");if(entries->count()!=4){fail("Introduction catalog mismatch.");return;}entries->setCurrentRow(0);QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(entries,&enter);return;}
   if(stage==5){auto* text=book->findChild<QTextBrowser*>("grimoireText");auto* bar=text->verticalScrollBar();if(bar->maximum()<=0){fail("Portmanteau introduction does not exercise scrolling.");return;}bar->setValue(bar->maximum());const int position=bar->value();
    QTimer::singleShot(300,&window,[e,bar,position,fail,&window]{if(bar->value()!=position||e->scrollRefreshes<2){fail("Engine refresh reset reader scrolling.");return;}e->scrollPreserved=true;window.findChild<QPushButton*>("grimoireNext")->click();});return;}
   if(stage==4||stage==6){click("grimoireNext");return;}
   if(stage==7){click("grimoirePrevious");return;}
   if(stage==8){click("grimoireContents");return;}
   if(stage==9){click("grimoireTab1");return;}
   if(stage==10){click("grimoireClose");return;}
   if(stage==12){QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(book,&escape);return;}
   if(stage==14)window.close();
  });
 };
 auto finished=session.finished;session.finished=[e,save,finished,&app]{const bool success=e->stage==15&&e->scrollPreserved&&e->error.isEmpty();const bool saved=save(success);if(finished)finished();app.exit(success&&saved?0:1);};
 auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Bounded Grimoire run expired.");});};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
 QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
