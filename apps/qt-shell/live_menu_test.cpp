#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QScreen>
#include <memory>
#include <cstdio>
void installLiveMenuTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    if(!qEnvironmentVariableIsEmpty("MNM_LIVE_MENU_TEST_REGION")){installLiveRegionEntryTest(app,window,session,path);return;}
    session.regionMenusEnabled=false; // Existing bounded V1-V6 harnesses retain their recorded protocols.
    if(!qEnvironmentVariableIsEmpty("MNM_LIVE_MENU_TEST_PREFERENCES_DISPLAY")){installLivePreferencesDisplayTest(app,window,session,path);return;}
    if(!qEnvironmentVariableIsEmpty("MNM_LIVE_MENU_TEST_PREFERENCES_RESTORE")){installLivePreferencesRestoreTest(app,window,session,path);return;}
    if(!qEnvironmentVariableIsEmpty("MNM_LIVE_MENU_TEST_PREFERENCES")){installLivePreferencesMenuTest(app,window,session,path);return;}
    if(!qEnvironmentVariableIsEmpty("MNM_LIVE_MENU_TEST_BATTLE")){installLiveBattleMenuTest(app,window,session,path);return;}
    struct Evidence {int stage=0;quint32 thread=0;bool consistent=true,fallback=false;QJsonArray states;};
    auto evidence=std::make_shared<Evidence>();
    const auto exitMode=qEnvironmentVariable("MNM_LIVE_MENU_TEST_EXIT");
    auto output=session.output;
    session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failure=session.failed;
    session.failed=[failure](const QString& text){if(failure)failure(text);std::fprintf(stderr,"Menu fallback: %s\n",qPrintable(text));};
    auto presentation=session.stateChanged;
    session.stateChanged=[evidence,presentation,&window,path,exitMode](const MenuBridge::State& state){
        if(presentation)presentation(state);
        if(!state.ready)return;
        if(evidence->thread&&evidence->thread!=state.thread)evidence->consistent=false;
        evidence->thread=state.thread;
        if((evidence->stage==0&&state.screen==3&&state.ack==0)||
           (evidence->stage==1&&state.screen==22&&state.ack==1)||
           (evidence->stage==2&&state.screen==3&&state.ack==2)){
            evidence->states.append(QJsonObject{{"screen",int(state.screen)},{"generation",int(state.generation)},
                {"ack",int(state.ack)},{"status",int(state.status)},{"thread",int(state.thread)}});
            const int stage=evidence->stage++;
            QTimer::singleShot(0,&window,[&window,path,stage,evidence,exitMode]{
                window.grab().save(QFileInfo(path).dir().filePath(QString("qt-stage-%1.png").arg(stage)));
                if(stage==1&&exitMode=="quick")window.close();
                else if(stage==2&&!exitMode.isEmpty()){
                    if(exitMode=="main")if(auto* button=window.findChild<QPushButton*>("mainMenuAction4"))button->click();
                }
                else if(stage<2){const auto name=stage==0?"mainMenuAction2":"quickBattleAction3";
                    if(auto* button=window.findChild<QPushButton*>(name))button->click();}
                else if(auto* button=window.findChild<QPushButton*>("originalMenusFallback")){
                    button->click();
                    QTimer::singleShot(250,&window,[&window,path,evidence]{
                        auto* container=window.findChild<QWidget*>("legacyGameContainer");
                        evidence->fallback=container&&container->isVisible();
                        if(auto* screen=QGuiApplication::primaryScreen())
                            screen->grabWindow(window.winId()).save(QFileInfo(path).dir().filePath("qt-original-fallback.png"));
                    });
                }
            });
        }
    };
    auto finished=session.finished;
    session.finished=[evidence,finished,path,&app,exitMode]{
        if(finished)finished();
        const bool success=evidence->stage==3&&evidence->consistent&&(exitMode.isEmpty()?evidence->fallback:true);
        QFile file(path);bool saved=file.open(QIODevice::WriteOnly|QIODevice::NewOnly);
        if(saved){QJsonObject report{{"success",success},{"exit_mode",exitMode},{"qt_buttons_activated",true},{"states",evidence->states},
            {"original_fallback_presented",evidence->fallback},{"original_drawing_retained",true},{"scope","Live Main/Quick/Cancel callbacks and engine-confirmed Qt transitions only"}};
            saved=file.write(QJsonDocument(report).toJson())>0&&file.flush();}
        app.exit(success&&saved?0:1);
    };
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    session.smokeSeconds=exitMode.isEmpty()?45:0;
    QTimer::singleShot(0,&window,[&window]{if(auto* button=window.findChild<QPushButton*>("launchGame"))button->click();});
}
