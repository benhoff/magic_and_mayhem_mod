#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "preferences_widget.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
void installLivePreferencesMenuTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    struct Evidence {int stage=0;quint32 thread=0;bool consistent=true,cancelUnchanged=false;QString error;QByteArray beforeCancel;std::array<int,7> initial{},committed{};QJsonArray states;};
    auto e=std::make_shared<Evidence>();
    auto save=[e,path](bool success){QFile f(path);if(!f.open(QIODevice::WriteOnly))return false;
        QJsonArray initial,committed;for(int v:e->initial)initial.append(v);for(int v:e->committed)committed.append(v);
        QJsonObject report{{"success",success},{"preferences",true},{"states",e->states},{"cancel_file_unchanged",e->cancelUnchanged},
            {"initial",initial},{"committed",committed},{"error",e->error},{"scope","Main Preferences audio preview, Cancel rollback, OK, reopen and window-close Cancel/Quit"}};
        return f.write(QJsonDocument(report).toJson())>0&&f.flush();};
    auto fail=[e,save,&app](const QString& reason){e->error=reason;std::fprintf(stderr,"Preferences validation: %s\n",qPrintable(reason));save(false);app.exit(1);};
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failure=session.failed;session.failed=[failure,fail](const QString& text){if(failure)failure(text);fail(text);};
    auto presentation=session.stateChanged;
    session.stateChanged=[e,presentation,fail,&window,&session,path](const MenuBridge::State& state){
        if(presentation)presentation(state);
        if(!state.ready||e->stage>=8)return;
        static const int screens[]={3,10,10,3,10,10,3,10};const int stage=e->stage;
        if(state.screen!=quint32(screens[stage])||state.ack!=quint32(stage))return;
        if(stage>0&&state.status!=MNM_MENU_OK){fail("Engine rejected Preferences action.");return;}
        if(e->thread&&e->thread!=state.thread)e->consistent=false;
        e->thread=state.thread;
        QJsonArray values;for(int v:state.preferences.values)values.append(v);
        e->states.append(QJsonObject{{"screen",int(state.screen)},{"ack",int(state.ack)},{"generation",int(state.generation)},
            {"thread",int(state.thread)},{"status",int(state.status)},{"values",values},{"available",int(state.preferences.available)}});
        ++e->stage;
        QTimer::singleShot(0,&window,[e,stage,state,fail,&window,&session,path]{
            auto* widget=window.findChild<PreferencesWidget*>();
            const auto prefs=QDir(session.evidenceDirectory()).filePath("game/CFG/prefs.cfg");
            auto bytes=[&](){QFile f(prefs);if(!f.open(QIODevice::ReadOnly)){fail("Cannot read staged prefs.cfg.");return QByteArray{};}return f.readAll();};
            auto click=[&](const QString& name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isEnabled()){fail("Unavailable Qt action: "+name);return;}b->click();};
            window.grab().save(QFileInfo(path).dir().filePath(QString("qt-preferences-%1.png").arg(stage)));
            if(stage==0||stage==3||stage==6){
                if(stage==3){e->cancelUnchanged=bytes()==e->beforeCancel;if(!e->cancelUnchanged){fail("Cancel changed staged preferences file.");return;}}
                click("mainMenuAction3");return;
            }
            if(!widget||!widget->isVisible()){fail("Qt Preferences is not visible.");return;}
            if(stage==1||stage==4){
                if(stage==1){e->initial=state.preferences.values;e->beforeCancel=bytes();if(e->beforeCancel.isEmpty())return;
                    if(!(state.preferences.available&(1u<<13))){fail("Effects slider unavailable in this run.");return;}
                    e->committed=e->initial;e->committed[1]=e->initial[1]==-500?-1000:-500;e->committed[4]=(e->initial[4]+1)%3;
                }else if(state.preferences.values!=e->initial){fail("Cancel did not restore engine entry values.");return;}
                auto* r=widget->findChild<QRadioButton*>(QString("preferencesRadio%1").arg(5+e->committed[4]));
                auto* fx=widget->findChild<QSlider*>("preferencesSlider2");
                if(!r||!r->isEnabled()||!fx||!fx->isEnabled()){fail("Requested test controls unavailable.");return;}
                r->click();fx->setValue(e->committed[1]);return;
            }
            if(stage==2||stage==5){
                if(state.preferences.values[1]!=e->committed[1]||state.preferences.values[4]!=e->initial[4]||
                   int(widget->draftSettings().dialogueSpeed)!=e->committed[4]){fail("Preview or preserved radio draft disagrees with engine.");return;}
                click(stage==2?"preferencesCancel":"preferencesOk");return;
            }
            if(stage==7){if(state.preferences.values!=e->committed){fail("Reopened Preferences disagrees with committed settings.");return;}window.close();}
        });
    };
    auto finished=session.finished;session.finished=[e,finished,save,&app]{
        const bool success=e->stage==8&&e->consistent&&e->cancelUnchanged&&e->error.isEmpty();const bool saved=save(success);
        if(finished)finished();
        app.exit(success&&saved?0:1);
    };
    auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Preferences bounded run expired.");});};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
    QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
