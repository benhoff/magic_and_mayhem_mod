#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "preferences_widget.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <cstdio>
void installLivePreferencesDisplayTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    struct Evidence {int stage=0;quint32 thread=0;std::array<int,7> initial{};QByteArray profile;QJsonArray states;QString error;bool cancel=false;};
    auto e=std::make_shared<Evidence>();
    auto save=[e,path](bool success){QJsonArray initial;for(int v:e->initial)initial.append(v);
        QFile file(path);return file.open(QIODevice::WriteOnly)&&file.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"initial",initial},{"cancel_file_unchanged",e->cancel},{"error",e->error}}).toJson())>0&&file.flush();};
    auto fail=[e,save,&app](const QString& error){e->error=error;std::fprintf(stderr,"Display validation: %s\n",qPrintable(error));save(false);app.exit(1);};
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failure=session.failed;session.failed=[failure,fail](const QString& error){if(failure)failure(error);fail(error);};
    auto presentation=session.stateChanged;
    session.stateChanged=[e,presentation,fail,&session,&window,path](const MenuBridge::State& state){
        if(presentation)presentation(state);
        if(!state.ready||e->stage>=12||state.screen!=quint32(e->stage%2?10:3)||state.ack!=quint32(e->stage))return;
        if(e->stage&&state.status!=MNM_MENU_OK){fail("Original display action rejected.");return;}
        if(e->thread&&e->thread!=state.thread){fail("Engine thread changed.");return;}e->thread=state.thread;
        const int stage=e->stage++;QJsonArray values;for(int v:state.preferences.values)values.append(v);
        e->states.append(QJsonObject{{"screen",int(state.screen)},{"ack",int(state.ack)},{"thread",int(state.thread)},{"values",values},{"available",int(state.preferences.available)}});
        QTimer::singleShot(0,&window,[e,stage,state,fail,&session,&window,path]{
            auto click=[&](const QString& name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isEnabled()){fail("Unavailable action: "+name);return;}b->click();};
            auto bytes=[&](){QFile f(QDir(session.evidenceDirectory()).filePath("game/CFG/prefs.cfg"));if(!f.open(QIODevice::ReadOnly)){fail("Missing original profile.");return QByteArray{};}return f.readAll();};
            if(stage%2==0){
                if(stage==2){e->cancel=bytes()==e->profile&&!QFile::exists(session.preferencesStorePath);if(!e->cancel){fail("Cancelled resolution draft wrote settings.");return;}}
                click("mainMenuAction3");return;
            }
            auto* widget=window.findChild<PreferencesWidget*>();if(!widget||!widget->isVisible()){fail("Reopened controls are unavailable.");return;}
            const auto shown=widget->draftSettings();const auto expected=stage<=3?e->initial[2]:stage==5||stage==9?1-e->initial[2]:e->initial[2];
            if(stage==1){e->initial=state.preferences.values;e->profile=bytes();}
            else {auto values=e->initial;values[2]=expected;if(state.preferences.values!=values||int(shown.resolution)!=expected){fail("Rebuilt Preferences settings disagree.");return;}}
            window.grab().save(QFileInfo(path).dir().filePath(QString("qt-display-%1.png").arg(stage)));
            if(stage==11){window.close();return;}
            if((state.preferences.available&3)!=3){fail("Both resolution modes must be available; no forced mode flags.");return;}
            const int target=1-state.preferences.values[2];auto* radio=widget->findChild<QRadioButton*>(QString("preferencesRadio%1").arg(1+target));
            if(!radio||!radio->isEnabled()){fail("Resolution radio unavailable.");return;}radio->click();
            if(int(widget->draftSettings().resolution)!=target){fail("Resolution draft did not change.");return;}
            click(stage==1?"preferencesCancel":"preferencesOk");
        });
    };
    auto finished=session.finished;session.finished=[e,finished,save,&app]{const bool ok=e->stage==12&&e->cancel&&e->error.isEmpty();const bool saved=save(ok);if(finished)finished();app.exit(ok&&saved?0:1);};
    auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(240000,&window,[fail]{fail("Display run expired.");});};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
    QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
