#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QTimer>
#include <QtEndian>
#include <cstdio>

void installCampaignRestartTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,
                               const QString& path,std::function<CampaignPresentationProbe()> probe){
    struct Run {int stage=0,restarts=0;bool done=false;quint64 frames=0,source=0;QJsonArray states;};
    auto r=std::make_shared<Run>();
    auto finish=[r,path,&app,&session](bool ok,const QString& error){
        if(r->done)return;r->done=true;
        QFile file(path);if(file.open(QIODevice::WriteOnly))file.write(QJsonDocument(QJsonObject{
            {"success",ok},{"error",error},{"restarts",r->restarts},{"states",r->states},
            {"experiment",session.evidenceDirectory()},{"bounded_termination",true}}).toJson());
        session.forceStop([&app,ok](bool stopped,const QString&){app.exit(ok&&stopped?0:1);});
    };
    auto click=[&window,finish](const QString& name){
        auto* button=window.findChild<QAbstractButton*>(name);
        if(!button||!button->isVisible()||!button->isEnabled()){finish(false,"Unavailable control: "+name);return;}
        button->click();
    };
    auto previous=session.stateChanged;
    session.stateChanged=[r,previous,click,finish,probe,&window](const MenuBridge::State& s){
        if(previous)previous(s);if(r->done||!s.ready)return;
        r->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"actions",int(s.mini.actions)},
                                    {"generation",int(s.generation)},{"stage",r->stage}});
        QString action;
        if(r->stage==0&&s.screen==3){r->stage=1;action="mainMenuAction0";}
        else if(r->stage==1&&s.screen==18){r->stage=2;action="regionEntryDifficulty2";}
        else if(r->stage==2&&s.screen==18){r->stage=3;action="regionEntryEnter";}
        else if(r->stage==5&&s.screen==17){
            if(!(s.mini.actions&32)){finish(false,"Level-start checkpoint unavailable");return;}
            r->stage=6;
            QTimer::singleShot(0,&window,[r,click,finish,probe,&window]{
                click("miniMenuRestart");
                auto* dialog=window.findChild<QMessageBox*>("restartLevelConfirmation");
                if(!dialog||dialog->defaultButton()!=dialog->button(QMessageBox::No)){finish(false,"Restart confirmation missing safe default");return;}
                dialog->done(QMessageBox::No);dialog->setObjectName("cancelledRestartConfirmation");
                // No must leave the same Mini visit active, with Restart usable.
                click("miniMenuRestart");
                dialog=window.findChild<QMessageBox*>("restartLevelConfirmation");
                if(!dialog||!dialog->isVisible()){finish(false,"Restart confirmation did not reopen");return;}
                const auto p=probe();r->frames=p.frames;r->source=p.sourceGeneration;r->stage=7;
                window.grab().save(QFileInfo(qEnvironmentVariable("MNM_RESTART_TEST_REPORT")).dir().filePath(QString("restart-confirm-%1.png").arg(r->restarts)));
                dialog->done(QMessageBox::Yes);
            });
        }
        if(!action.isEmpty())QTimer::singleShot(0,&window,[click,action]{click(action);});
    };
    auto battle=session.battleStarted;session.battleStarted=[r,battle](quint32 destination){if(battle)battle(destination);r->stage=4;};
    auto viewport=session.originalViewportRequested;session.originalViewportRequested=[r,viewport]{if(viewport)viewport();if(r->stage==7)r->stage=8;};
    auto failure=session.failed;session.failed=[failure,finish](const QString& error){if(failure)failure(error);finish(false,error);};
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto finished=session.finished;session.finished=[finished,finish,r]{if(finished)finished();if(!r->done)finish(false,"Game exited before restart validation completed");};
    auto* poll=new QTimer(&window);poll->setInterval(200);
    QObject::connect(poll,&QTimer::timeout,&window,[r,probe,finish,&session,&window]{
        if(r->done||(r->stage!=4&&r->stage!=8))return;
        const auto p=probe();if(p.fallback||!p.error.isEmpty()){finish(false,"Restart rendering failed: "+p.error);return;}
        if(!campaignResumeConfirmed(p,r->frames,r->source,true))return;
        if(r->stage==8){
            QFile events(QDir(session.evidenceDirectory()).filePath("events.bin"));
            if(!events.open(QIODevice::ReadOnly))return;
            const auto bytes=events.readAll();int loads=0;
            for(int at=16;at+64<=bytes.size();at+=64)
                if(qFromLittleEndian<quint32>(bytes.constData()+at+4)==35)++loads;
            if(loads!=r->restarts+1)return;
            ++r->restarts;if(r->restarts==2){finish(true,{});return;}
        }
        r->stage=9;
        QTimer::singleShot(2000,&window,[r,&window,finish]{
            auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");
            if(!viewport||!viewport->isVisible()){finish(false,"Native viewport absent");return;}
            window.activateWindow();window.raise();viewport->setFocus();
            auto* input=new QProcess(&window);
            QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[input,finish](int code,QProcess::ExitStatus status){
                if(code||status!=QProcess::NormalExit)finish(false,"Escape failed: "+QString::fromLocal8Bit(input->readAllStandardError()));input->deleteLater();
            });
            r->stage=5;
            input->start("python3",{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"});
        });
    });poll->start();
    session.preferencesStorePath=QFileInfo(path).dir().filePath("preferences.json");
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    QTimer::singleShot(150000,&window,[finish]{finish(false,"Restart test deadline exceeded");});
    QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
