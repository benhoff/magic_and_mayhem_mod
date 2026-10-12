#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "load_game_widget.hpp"
#include "../../protocols/include/mnm/menu_v13.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <cstdio>
void installLiveStorageMenuTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 struct Evidence {int stage=0;quint32 thread=0;QString error;QJsonArray states;};auto e=std::make_shared<Evidence>();
 auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"error",e->error},{"scope","Native Main Load/Cancel twice and normal Main Quit; actual serializer I/O and campaign Save/Load remain separate"}}).toJson())>0&&f.flush();};
 auto fail=[e,save,&app](const QString& error){e->error=error;std::fprintf(stderr,"Storage live validation: %s\n",qPrintable(error));save(false);app.exit(1);};
 auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
 auto failure=session.failed;session.failed=[failure,fail](const QString& text){if(failure)failure(text);fail(text);};
 auto present=session.stateChanged;session.stateChanged=[e,present,fail,&window,path](const MenuBridge::State& s){
  if(present){present(s);}
  if(!s.ready||e->stage>=5)return;
  const quint32 screens[]={3,MNM_MENU_LOAD_SCREEN,3,MNM_MENU_LOAD_SCREEN,3};if(s.screen!=screens[e->stage]||s.ack!=quint32(e->stage))return;
  if((s.ack&&s.status!=MNM_MENU_OK)||!s.thread||(e->thread&&e->thread!=s.thread)){fail("Engine ACK/thread mismatch");return;}e->thread=s.thread;
  if(s.screen==MNM_MENU_LOAD_SCREEN&&(s.storage.parent!=3||s.storage.depth!=3||s.storage.context!=5)){fail("Original Load parent mismatch");return;}
  e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"status",int(s.status)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"rows",s.storage.names.size()},{"parent",int(s.storage.parent)},{"depth",int(s.storage.depth)}});const int stage=e->stage++;
  QTimer::singleShot(0,&window,[stage,fail,&window,path]{window.grab().save(QFileInfo(path).dir().filePath(QString("qt-storage-%1.png").arg(stage)));const auto name=stage==4?"mainMenuAction4":stage%2?"loadGameCancel":"mainMenuAction1";auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isVisible()||!b->isEnabled()){fail("Native Load/Cancel/Quit unavailable");return;}b->click();});
 };
 auto finished=session.finished;session.finished=[e,finished,save,&app]{if(finished)finished();const bool success=e->stage==5&&e->error.isEmpty();app.exit(success&&save(success)?0:1);};
 session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
 QTimer::singleShot(210000,&window,[fail]{fail("Main Load round-trip deadline expired");});
 QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
