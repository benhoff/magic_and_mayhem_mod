#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScreen>
#include <memory>
#include <cstdio>

// Public controls and ordinary session feature flags. No engine readiness fix,
// protocol downgrade or fallback is enabled by this automation.
void installCampaignSmokeTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,
                             const QString& path,std::function<CampaignPresentationProbe()> probe){
    struct Run {int stage=0;quint32 thread=0;quint64 frameBase=0;bool done=false;QJsonArray steps;};
    auto run=std::make_shared<Run>();
    auto finish=[run,path,probe,&app](bool success,const QString& reason){
        if(run->done)return;
        run->done=true;const auto p=probe();
        QSaveFile file(path);
        const QJsonObject result{{"success",success},{"error",reason},{"steps",run->steps},
            {"native_command_frames",qint64(p.frames)},{"native_command_fallback",p.fallback},
            {"movement_verified",false},{"original_drawing_retained",true},
            {"scope","Native Qt New Game/Enter, native command presentation, Escape/Mini/Cancel and resumed presentation; no movement, pixel equivalence or complete drawing replacement claim"}};
        const bool saved=file.open(QIODevice::WriteOnly)&&file.write(QJsonDocument(result).toJson())>0&&file.commit();
        std::fprintf(stderr,"Campaign smoke %s: %s\n",success&&saved?"complete":"failed",qPrintable(reason));
        // The runner owns bounded cleanup of its private Wine prefix.
        if(!saved)app.exit(1);
    };
    auto snapshot=[run,&window,path](const QString& name){
        const auto file=QFileInfo(path).dir().filePath(name+".png");
        bool saved=false;if(auto* screen=window.screen())saved=screen->grabWindow(window.winId()).save(file);
        run->steps.append(QJsonObject{{"step",name},{"screenshot",file},{"screenshot_saved",saved}});
    };
    auto click=[&window,finish](const QString& name){
        auto* button=window.findChild<QPushButton*>(name);
        if(!button||!button->isVisible()||!button->isEnabled()){finish(false,"Public control unavailable: "+name);return;}
        button->click();
    };
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failed=session.failed;session.failed=[failed,finish](const QString& text){if(failed)failed(text);finish(false,"Native menu fallback: "+text);};
    auto state=session.stateChanged;
    session.stateChanged=[run,state,finish,snapshot,click,probe,&window](const MenuBridge::State& s){
        if(state)state(s);
        if(run->done||!s.ready)return;
        if(run->thread&&run->thread!=s.thread){finish(false,"Menu engine thread changed");return;}
        run->thread=s.thread;
        QString action,step;
        if(run->stage==0&&s.screen==3&&s.ack==0){run->stage=1;step="main";action="mainMenuAction0";}
        else if(run->stage==1&&s.screen==18&&s.ack==1){run->stage=2;step="region";action="regionEntryEnter";}
        else if(run->stage==4&&s.screen==17&&s.ack==2){run->stage=5;run->frameBase=probe().frames;step="campaign-menu";action="miniMenuButton5";}
        if(action.isEmpty())return;
        QTimer::singleShot(0,&window,[snapshot,click,step,action]{snapshot(step);click(action);});
    };
    auto battle=session.battleStarted;
    session.battleStarted=[run,battle,finish,probe](quint32 destination){
        if(battle)battle(destination);
        if(run->done)return;
        if(destination!=3||run->stage!=2){finish(false,"Unexpected campaign handoff");return;}
        run->stage=3;run->frameBase=probe().frames;
        run->steps.append(QJsonObject{{"step","campaign-handoff"}});
    };
    auto viewport=session.originalViewportRequested;
    session.originalViewportRequested=[run,viewport,finish,probe]{
        if(viewport)viewport();
        if(run->done)return;
        if(run->stage!=5){finish(false,"Unexpected original viewport handoff");return;}
        run->stage=6;run->frameBase=probe().frames;
    };
    auto* poll=new QTimer(&window);poll->setInterval(100);
    QObject::connect(poll,&QTimer::timeout,&window,[run,probe,finish,snapshot,&window]{
        if(run->done)return;
        const auto p=probe();
        if(p.fallback){finish(false,"Native rendering fell back to the original window");return;}
        if(run->stage!=3&&run->stage!=6)return;
        if(!p.active||p.frames<run->frameBase+3)return;
        auto* legacy=window.findChild<QWidget*>("legacyGameContainer");
        if(legacy&&legacy->isVisible()){finish(false,"Gameplay still uses the original Wine viewport");return;}
        if(run->stage==6){snapshot("gameplay-resumed");finish(true,QString());return;}
        run->stage=4;snapshot("gameplay");
        // XTest input traverses the public viewport forwarding path. The runner
        // provides a private X display by default; explicit --display opts out.
        window.activateWindow();window.raise();
        auto* input=new QProcess(&window);
        QObject::connect(input,&QProcess::errorOccurred,&window,[finish](QProcess::ProcessError error){
            if(error==QProcess::FailedToStart)finish(false,"Could not start Escape input helper");
        });
        QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,
            [input,finish](int code,QProcess::ExitStatus status){
                if(code||status!=QProcess::NormalExit)finish(false,"Escape input failed: "+QString::fromLocal8Bit(input->readAllStandardError()));
                input->deleteLater();
            });
        input->start("python3",{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"});
    });poll->start();
    auto finished=session.finished;session.finished=[finished,finish]{if(finished)finished();finish(false,"Game exited before campaign smoke completed");};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    session.preferencesStorePath=QFileInfo(path).dir().filePath("config/engine-preferences.json");
    QTimer::singleShot(300000,&window,[finish]{finish(false,"Campaign flow exceeded five minutes");});
    QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
