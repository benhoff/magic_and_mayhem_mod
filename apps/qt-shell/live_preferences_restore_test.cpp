#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "preferences_widget.hpp"
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
void installLivePreferencesRestoreTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    struct Evidence {int stage=0;bool restored=false,unchanged=false;QByteArray before;QJsonArray states;QString error;};
    auto e=std::make_shared<Evidence>();QFile f(session.preferencesStorePath);if(f.open(QIODevice::ReadOnly))e->before=f.readAll();
    const auto saved=QJsonDocument::fromJson(e->before).object().value("values").toArray();std::array<int,7> expected{};
    for(int i=0;i<7&&i<saved.size();++i)expected[i]=saved[i].toInt();
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto fail=[e,&app](const QString& error){e->error=error;std::fprintf(stderr,"Preferences restore: %s\n",qPrintable(error));app.exit(1);};
    auto failure=session.failed;session.failed=[failure,fail](const QString& text){if(failure)failure(text);fail(text);};
    auto presentation=session.stateChanged;
    session.stateChanged=[e,presentation,expected,fail,&window](const MenuBridge::State& state){
        if(presentation)presentation(state);
        if(!state.ready)return;
        if((e->stage==0&&state.screen==3&&state.ack==0)||(e->stage==1&&state.screen==10&&state.ack==1)){
            const int stage=e->stage++;QJsonArray values;for(int v:state.preferences.values)values.append(v);
            e->states.append(QJsonObject{{"screen",int(state.screen)},{"ack",int(state.ack)},{"values",values}});
            QTimer::singleShot(0,&window,[e,stage,state,expected,fail,&window]{
                if(stage==0){if(auto* b=window.findChild<QPushButton*>("mainMenuAction3"))b->click();return;}
                auto* widget=window.findChild<PreferencesWidget*>();if(!widget||!widget->isVisible()){fail("Qt Preferences is not visible.");return;}
                const auto v=widget->draftSettings();const std::array<int,7> shown{v.musicLevel,v.soundLevel,int(v.resolution),int(v.animation),int(v.dialogueSpeed),int(v.gameSpeed),v.borderPicture?1:0};
                e->restored=expected==state.preferences.values&&shown==expected;if(!e->restored){fail("Fresh process did not restore persisted settings.");return;}window.close();
            });
        }
    };
    auto finished=session.finished;session.finished=[e,finished,path,&session,&app]{
        QFile stored(session.preferencesStorePath);if(stored.open(QIODevice::ReadOnly))e->unchanged=stored.readAll()==e->before;
        const bool success=e->stage==2&&e->restored&&e->unchanged&&e->error.isEmpty();QFile file(path);bool saved=file.open(QIODevice::WriteOnly);
        if(saved)saved=file.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"preferences_restored",e->restored},{"store_unchanged",e->unchanged},{"error",e->error}}).toJson())>0&&file.flush();
        if(finished)finished();
        app.exit(success&&saved?0:1);
    };
    auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Restore run expired.");});};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
    QTimer::singleShot(0,&window,[fail,saved,&window]{if(saved.size()!=7){fail("Missing saved preferences.");return;}if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
