#include "resident_restart_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include "frame_stream.hpp"
#include "native_capture_request.hpp"
#include "../../protocols/include/mnm/menu_v11.h"
#include "../../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QMainWindow>
#include <QAbstractButton>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <memory>
#include <cstdio>
#include <csignal>
#include <cerrno>

void installResidentRestartTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path,
    std::function<CampaignPresentationProbe()> probe,std::function<QJsonObject()> cleanup){
    struct Run {
        int stage=0,cycle=0;bool done=false;quint64 frames=0,paints=0,source=0;
        QElapsedTimer clock,terminal;QJsonArray events;QString firstRoot;quint32 firstIdentity=0;qint64 firstPid=0;
    };
    auto r=std::make_shared<Run>();r->clock.start();
    auto event=[r](QJsonObject e){e["milliseconds"]=r->clock.elapsed();r->events.append(e);};
    auto finish=[r,path,&app](bool ok,const QString& error){
        if(r->done)return;
        r->done=true;QSaveFile out(path);
        const QJsonObject result{{"success",ok},{"error",error},{"events",r->events},
            {"scope","Actual original-game producer failure after resident World presentation, terminal cleanup, public Hard restart, fresh World and normal Quit. No physical context loss, freeze, in-place recovery or pixel equivalence claim."}};
        const auto bytes=QJsonDocument(result).toJson();
        const bool saved=out.open(QIODevice::WriteOnly)&&out.write(bytes)==bytes.size()&&out.commit();
        std::fprintf(stderr,"Resident restart %s: %s\n",ok&&saved?"complete":"failed",qPrintable(error));app.exit(ok&&saved?0:1);
    };
    auto click=[&window,finish](const QString& name){
        auto* button=window.findChild<QAbstractButton*>(name);
        if(!button||!button->isVisible()||!button->isEnabled()){finish(false,"Public control unavailable: "+name);return;}
        button->click();
    };
    auto helper=[&window,finish](const QStringList& args,std::function<void()> done){
        auto* p=new QProcess(&window);
        QObject::connect(p,&QProcess::errorOccurred,&window,[finish](QProcess::ProcessError e){if(e==QProcess::FailedToStart)finish(false,"Input helper did not start");});
        QObject::connect(p,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[p,finish,done](int code,QProcess::ExitStatus status){
            if(code||status!=QProcess::NormalExit)finish(false,"Input helper failed: "+QString::fromLocal8Bit(p->readAllStandardError()));else if(done)done();p->deleteLater();
        });p->start("python3",args);
    };
    auto output=session.output;session.output=[output](const QString& s){if(output)output(s);std::fprintf(stderr,"%s",qPrintable(s));};
    auto failed=session.failed;session.failed=[failed,finish](const QString& s){if(failed)failed(s);finish(false,"Menu session failed: "+s);};
    auto state=session.stateChanged;session.stateChanged=[r,state,event,click,&window](const MenuBridge::State& s){
        if(state)state(s);
        if(r->done||!s.ready)return;
        QString button;
        if(r->stage==0&&s.screen==3&&s.ack==0){r->stage=1;button="mainMenuAction0";event({{"step","main"},{"cycle",r->cycle}});}
        else if(r->stage==1&&s.screen==18&&s.ack==1){r->stage=2;button="regionEntryDifficulty1";}
        else if(r->stage==2&&s.screen==18&&s.ack==2){r->stage=3;button="regionEntryEnter";}
        else if(r->stage==7&&s.screen==17){r->stage=8;button="miniMenuButton4";}
        else if(r->stage==9&&s.screen==MNM_MENU_DEFEAT_SCREEN){r->stage=10;button="battleResultContinue";}
        else if(r->stage==10&&s.screen==3){r->stage=11;button="mainMenuAction4";event({{"step","normal-main-quit"}});}
        if(!button.isEmpty())QTimer::singleShot(0,&window,[click,button]{click(button);});
    };
    auto battle=session.battleStarted;session.battleStarted=[r,battle,probe,finish,event,&session](quint32 destination){
        if(battle)battle(destination);
        if(r->done)return;
        if(r->stage!=3||destination!=3){finish(false,"Unexpected campaign handoff");return;}
        r->stage=4;auto p=probe();r->frames=p.frames;r->paints=p.paints;r->source=p.sourceGeneration;
        event({{"step","handoff"},{"cycle",r->cycle},{"root",session.evidenceDirectory()}});
    };
    auto viewport=session.originalViewportRequested;session.originalViewportRequested=[r,viewport,helper,finish,path,&window,&session]{
        if(viewport)viewport();
        if(r->done||r->stage!=8)return;
        r->stage=9;window.activateWindow();window.raise();
        auto* view=window.findChild<GlViewport*>("nativeGameViewport");
        if(!view||!view->isVisible()){finish(false,"Quit viewport unavailable");return;}
        const auto rect=view->imageRect();const auto pos=view->mapToGlobal(rect.topLeft().toPoint());
        helper({QDir::current().filePath("tools/campaign-quit-input.py"),"--rect",QString::number(pos.x()),QString::number(pos.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--answer","yes","--experiment",session.evidenceDirectory(),"--output",QFileInfo(path).dir().filePath("quit-input.json")},{});
    };
    auto finished=session.finished;session.finished=[r,finished,finish,event,&session]{
        const bool quit=session.normalQuitAccepted();if(finished)finished();
        if(r->done)return;
        event({{"step","producer-finished"},{"normal_quit_accepted",quit},{"stage",r->stage}});
        finish(r->stage==11&&quit,r->stage==11&&quit?QString():QString("Producer exited before completed restart journey"));
    };
    auto* poll=new QTimer(&window);poll->setInterval(50);
    QObject::connect(poll,&QTimer::timeout,&window,[r,probe,cleanup,event,finish,click,helper,path,&window,&session]{
        if(r->done)return;
        const auto p=probe();
        // The ordinary Quit input helper requires retained native/original
        // confirmation images before clicking. These are diagnostic reads.
        if(r->stage==9){
            const auto directory=QFileInfo(path).dir();const auto request=directory.filePath("capture-request.json");
            try{
                const auto name=readNativeCaptureRequest(request);
                if(!name.isEmpty()){
                    auto* view=window.findChild<GlViewport*>("nativeGameViewport");
                    FrameStream reference;QImage original;
                    if(reference.open(QDir(session.evidenceDirectory()).filePath("render-frame.bin")))original=reference.nextFrame();
                    if(!view||!view->isVisible()||original.isNull())return;
                    const auto rect=view->imageRect();const auto ratio=view->devicePixelRatioF();
                    const auto native=view->grabFramebuffer().copy(QRect(qRound(rect.x()*ratio),qRound(rect.y()*ratio),qRound(rect.width()*ratio),qRound(rect.height()*ratio))).scaled(view->frameSize());
                    const bool a=native.save(directory.filePath(name+"-native.png")),b=original.save(directory.filePath(name+"-original.png"));
                    QSaveFile result(directory.filePath(name+".json"));
                    const auto bytes=QJsonDocument(QJsonObject{{"native_saved",a},{"original_saved",b},{"fallback",p.fallback}}).toJson();
                    if(!a||!b||!QFile::remove(request)||!result.open(QIODevice::WriteOnly)||result.write(bytes)!=bytes.size()||!result.commit())finish(false,"Quit diagnostic capture failed");
                }
            }catch(const std::exception& error){finish(false,QString::fromUtf8(error.what()));}
        }
        if(r->stage==4){
            if(p.fallback||!p.error.isEmpty()){finish(false,"Rendering failed before injection: "+p.error);return;}
            if(!p.active||p.frames<r->frames+3||p.paints<r->paints+3||p.sourceGeneration<r->source+3)return;
            QFile channel(QDir(session.evidenceDirectory()).filePath("render-commands.bin"));
            if(!channel.open(QIODevice::ReadWrite)||channel.size()!=MNM_RENDER_COMMANDS_V2_SIZE){finish(false,"Command channel unavailable");return;}
            auto* bytes=channel.map(0,channel.size());if(!bytes){finish(false,"Command channel mapping failed");return;}
            auto* words=reinterpret_cast<uint32_t*>(bytes);const auto identity=words[4];
            const bool valid=mnm_ring_identity(words,identity)&&mnm_ring_load(words+6)==1;
            event({{"step","live-world"},{"cycle",r->cycle},{"root",session.evidenceDirectory()},{"identity",int(identity)},
                {"launcher_pid",session.launcherProcessId()},{"frames",qint64(p.frames)},{"paints",qint64(p.paints)},{"source_generation",qint64(p.sourceGeneration)},{"renderer",p.renderer}});
            if(!valid){channel.unmap(bytes);finish(false,"Live producer identity/state invalid");return;}
            if(!r->cycle){
                r->firstRoot=session.evidenceDirectory();r->firstIdentity=identity;r->firstPid=session.launcherProcessId();
                // Test-only terminal fault. Never claim the real writer or modify its cursors/payload.
                mnm_ring_writer fault{};fault.map=words;fault.session=identity;fault.version=words[2];
                mnm_ring_fail(&fault,MNM_RENDER_COMMANDS_V2_REASON_INVALID);
                const bool injected=mnm_ring_load(words+6)==3;channel.unmap(bytes);
                if(!injected){finish(false,"Producer fault injection refused");return;}
                event({{"step","producer-failure-injected"},{"reason",int(MNM_RENDER_COMMANDS_V2_REASON_INVALID)}});
                r->stage=5;return;
            }
            channel.unmap(bytes);
            if(identity==r->firstIdentity||session.evidenceDirectory()==r->firstRoot||session.launcherProcessId()<=0||session.launcherProcessId()==r->firstPid){finish(false,"Restart reused failed session identity/process");return;}
            if(r->firstPid<=0||::kill(pid_t(r->firstPid),0)!=-1||errno!=ESRCH){finish(false,"Old launcher still exists after restart");return;}
            event({{"step","fresh-session-confirmed"},{"old_launcher_gone",true}});r->stage=7;window.activateWindow();window.raise();
            helper({QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"},{});return;
        }
        if(r->stage==5){
            auto s=cleanup();if(!s.value("terminal").toBool())return;
            if(!s.value("restart_available").toBool())return; // Native-only force-stop must finish first.
            for(auto it=s.begin();it!=s.end();++it)if(!it.value().toBool()){event(s);finish(false,"Incomplete terminal cleanup: "+it.key());return;}
            if(!p.fallback||p.active||p.error.isEmpty()||p.recoveries){finish(false,"Failed generation retried or remained active");return;}
            event({{"step","terminal-cleanup"},{"state",s},{"error",p.error}});r->frames=p.frames;r->paints=p.paints;r->terminal.start();r->stage=6;
        }
        if(r->stage==6){
            auto s=cleanup();for(auto it=s.begin();it!=s.end();++it)if(!it.value().toBool()){finish(false,"Terminal state resurrected: "+it.key());return;}
            if(p.frames!=r->frames||p.paints!=r->paints){finish(false,"Failed generation presented again");return;}
            if(r->terminal.elapsed()<500)return;
            event({{"step","terminal-stable"},{"milliseconds_observed",r->terminal.elapsed()}});
            r->cycle=1;r->stage=0;event({{"step","hard-restart-click"}});click("hardRestartGame");
        }
    });poll->start();
    if(!qEnvironmentVariableIsSet("MNM_NATIVE_OUTPUT_OWNER"))session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    session.preferencesStorePath=QFileInfo(path).dir().filePath("config/engine-preferences.json");
    QTimer::singleShot(590000,&window,[finish]{finish(false,"Restart journey deadline exceeded");});
    QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
