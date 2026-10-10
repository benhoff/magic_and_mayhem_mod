#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include "frame_stream.hpp"
#include "../../protocols/include/mnm/menu_v11.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QAbstractButton>
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
#include <future>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <cstdio>

// Public controls and ordinary session feature flags. No engine readiness fix,
// protocol downgrade or fallback is enabled by this automation.
void installCampaignSmokeTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,
                             const QString& path,std::function<CampaignPresentationProbe()> probe){
    if(qEnvironmentVariableIntValue("MNM_NATIVE_HEALING")==1){installNativeHealingSmokeTest(app,window,session,path,probe);return;}
    const int difficulty=qEnvironmentVariableIntValue("MNM_CAMPAIGN_DIFFICULTY");
    const int stressSeconds=qEnvironmentVariableIntValue("MNM_CAMPAIGN_STRESS_SECONDS");
    const int cycles=qMax(1,qEnvironmentVariableIntValue("MNM_CAMPAIGN_MENU_CYCLES"));
    const bool normalQuit=qEnvironmentVariableIntValue("MNM_CAMPAIGN_NORMAL_QUIT")==1;
    const bool castingCombat=qEnvironmentVariableIntValue("MNM_CAMPAIGN_CASTING_COMBAT")==1;
    struct Run {std::future<void> captureJob;int quitPass=0;bool quitAnswered=false;int cycle=0;bool stressRunning=false,stressDone=false;quint64 sampleFrames=0,samplePaints=0;QElapsedTimer sampleClock,snapshotWait;QJsonArray rates;int stage=0;quint32 thread=0;quint64 frameBase=0;bool done=false;QJsonArray steps;};
    auto run=std::make_shared<Run>();
    auto finish=[run,path,probe,&app,normalQuit](bool success,const QString& reason){
        if(run->done)return;
        run->done=true;const auto p=probe();
        QSaveFile file(path);
        const QJsonObject result{{"success",success},{"normal_quit",normalQuit&&success&&run->stage==11},{"error",reason},{"steps",run->steps},
            {"native_command_frames",qint64(p.frames)},{"native_painted_frames",qint64(p.paints)},{"native_presentation_pixels",qint64(p.presentationPixels)},{"native_recoveries",int(p.recoveries)},{"native_error",p.error},{"renderer",p.renderer},{"gameplay_rates",run->rates},{"native_command_fallback",p.fallback},
            {"movement_verified",false},{"original_drawing_retained",true},
            {"scope","Native Qt New Game/Enter, native command presentation, Escape/Mini/Cancel and resumed presentation; no movement, pixel equivalence or complete drawing replacement claim"}};
        const bool saved=file.open(QIODevice::WriteOnly)&&file.write(QJsonDocument(result).toJson())>0&&file.commit();
        std::fprintf(stderr,"Campaign smoke %s: %s\n",success&&saved?"complete":"failed",qPrintable(reason));
        // The runner owns bounded cleanup of its private Wine prefix.
        if(!saved)app.exit(1);
        else if(normalQuit&&success&&run->stage==11)app.exit(0);
    };
    auto snapshot=[run,&window,path,&session](const QString& name){
        QImage originalImage;
        if(name=="gameplay"||name=="gameplay-resumed"||name=="quit-no-resumed"){
            FrameStream reference;
            if(reference.open(QDir(session.evidenceDirectory()).filePath("render-frame.bin")))originalImage=reference.nextFrame();
            // A busy seqlock is normal. Retry on the next UI poll before
            // capturing/saving any image or advancing the campaign route.
            if(originalImage.isNull())return false;
        }
        window.raise();
        const auto file=QFileInfo(path).dir().filePath(name+QString("-%1.png").arg(run->cycle));
        bool saved=false;if(auto* screen=window.screen())saved=screen->grabWindow(window.winId()).save(file);
        QString nativeFile,originalFile;bool nativeSaved=false,originalSaved=false;
        if(name=="gameplay"||name=="gameplay-resumed"||name=="quit-no-resumed"){
            auto* widget=window.findChild<QOpenGLWidget*>("nativeGameViewport");
            auto* viewport=dynamic_cast<GlViewport*>(widget);
            if(viewport&&viewport->isVisible()){
                const auto rect=viewport->imageRect();const auto ratio=viewport->devicePixelRatioF();
                nativeFile=QFileInfo(path).dir().filePath(name+QString("-native-%1.png").arg(run->cycle));
                nativeSaved=viewport->grabFramebuffer().copy(QRect(qRound(rect.x()*ratio),qRound(rect.y()*ratio),qRound(rect.width()*ratio),qRound(rect.height()*ratio))).scaled(viewport->frameSize(),Qt::IgnoreAspectRatio,Qt::FastTransformation).save(nativeFile);
            }
            originalFile=QFileInfo(path).dir().filePath(name+QString("-original-%1.png").arg(run->cycle));
            originalSaved=originalImage.save(originalFile);
        }
        run->steps.append(QJsonObject{{"native_image",nativeFile},{"native_image_saved",nativeSaved},{"original_image",originalFile},{"original_image_saved",originalSaved},{"step",name},{"screenshot",file},{"screenshot_saved",saved}});
        return true;
    };
    auto click=[&window,finish](const QString& name){
        auto* button=window.findChild<QAbstractButton*>(name);
        if(!button||!button->isVisible()||!button->isEnabled()){finish(false,"Public control unavailable: "+name);return;}
        button->click();
    };
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failed=session.failed;session.failed=[failed,finish](const QString& text){if(failed)failed(text);finish(false,"Native menu fallback: "+text);};
    auto state=session.stateChanged;
    session.stateChanged=[run,state,finish,snapshot,click,probe,difficulty,normalQuit,&window](const MenuBridge::State& s){
        if(state)state(s);
        if(run->done||!s.ready)return;
        if(run->thread&&run->thread!=s.thread){finish(false,"Menu engine thread changed");return;}
        run->thread=s.thread;
        if(run->stage>=3&&run->stage<=7&&(s.screen==3||s.screen==MNM_MENU_DEFEAT_SCREEN||s.screen==MNM_MENU_RESULT_SCREEN)){
            finish(false,QString("Campaign left gameplay unexpectedly (screen %1)").arg(s.screen));return;
        }
        QString action,step;
        if(run->stage==0&&s.screen==3&&s.ack==0){run->stage=1;step="main";action="mainMenuAction0";}
        else if(run->stage==1&&s.screen==18&&s.ack==1){run->stage=difficulty?10:2;step="region";action=difficulty?QString("regionEntryDifficulty%1").arg(difficulty):QString("regionEntryEnter");}
        else if(run->stage==10&&s.screen==18&&s.ack==2){run->stage=2;action="regionEntryEnter";}
        else if(run->stage==4&&s.screen==17&&s.ack==quint32(2+run->cycle+(difficulty?1:0))){run->stage=5;run->frameBase=probe().frames;step="campaign-menu";action="miniMenuButton5";}
        if(normalQuit&&run->stage==7&&s.screen==17&&s.ack==quint32(2+run->cycle+(difficulty?1:0)+run->quitPass)){
            run->stage=8;run->frameBase=probe().frames;step="campaign-quit-menu";action="miniMenuButton4";
        }else if(normalQuit&&run->stage==9&&run->quitPass==1&&s.screen==MNM_MENU_DEFEAT_SCREEN){
            run->stage=10;step="campaign-defeat";action="battleResultContinue";
        }else if(normalQuit&&run->stage==10&&s.screen==3){
            run->stage=11;step="quit-main";action="mainMenuAction4";
        }
        if(action.isEmpty())return;
        QTimer::singleShot(0,&window,[snapshot,click,step,action]{if(!step.isEmpty())snapshot(step);click(action);});
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
    session.originalViewportRequested=[run,viewport,finish,probe,normalQuit,path,&window,&session]{
        if(viewport)viewport();
        if(run->done)return;
        if(normalQuit&&run->stage==10)return; // native report Continue awaits fresh Main
        if(normalQuit&&run->stage==8){
            run->stage=9;run->quitAnswered=false;window.activateWindow();window.raise();
            auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
            if(!viewport||!viewport->isVisible()){finish(false,"Native Quit viewport unavailable");return;}
            const auto rect=viewport->imageRect();const auto origin=viewport->mapToGlobal(rect.topLeft().toPoint());
            auto* input=new QProcess(&window);
            QObject::connect(input,&QProcess::errorOccurred,&window,[finish](QProcess::ProcessError e){if(e==QProcess::FailedToStart)finish(false,"Could not start native Quit input");});
            QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[input,run,finish](int code,QProcess::ExitStatus status){
                if(code||status!=QProcess::NormalExit)finish(false,"Native Quit input failed: "+QString::fromLocal8Bit(input->readAllStandardError()));
                else run->quitAnswered=true;
                input->deleteLater();
            });
            input->start("python3",{QDir::current().filePath("tools/campaign-quit-input.py"),"--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--answer",run->quitPass?"yes":"no","--experiment",session.evidenceDirectory(),"--output",QFileInfo(path).dir().filePath(QString("quit-input-%1.json").arg(run->quitPass))});
            return;
        }
        if(run->stage!=5){finish(false,"Unexpected original viewport handoff");return;}
        run->stage=6;run->stressDone=false;run->frameBase=probe().frames;++run->cycle;
    };
    auto* poll=new QTimer(&window);poll->setInterval(100);
    QObject::connect(poll,&QTimer::timeout,&window,[run,probe,finish,snapshot,path,stressSeconds,cycles,castingCombat,normalQuit,&session,&window]{
        if(run->done)return;
        const auto p=probe();
        const auto captureRequest=QFileInfo(path).dir().filePath("capture-request.json");
        if(p.active&&QFileInfo::exists(captureRequest)&&(!run->captureJob.valid()||run->captureJob.wait_for(std::chrono::seconds(0))==std::future_status::ready)){
            QFile file(captureRequest);
            if(file.open(QIODevice::ReadOnly)){
                const auto request=QJsonDocument::fromJson(file.readAll()).object();const auto name=request.value("name").toString();file.close();
                if(!name.isEmpty()&&name.size()<80&&!name.contains('/')&&!name.contains('\\')&&!name.contains("..")){
                    FrameStream reference;
                    QImage originalImage;
                    if(reference.open(QDir(session.evidenceDirectory()).filePath("render-frame.bin")))originalImage=reference.nextFrame();
                    if(!originalImage.isNull()){
                    auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
                    QImage nativeImage;
                    if(viewport&&viewport->isVisible()){
                        const auto rect=viewport->imageRect();const auto ratio=viewport->devicePixelRatioF();
                        nativeImage=viewport->grabFramebuffer().copy(QRect(qRound(rect.x()*ratio),qRound(rect.y()*ratio),qRound(rect.width()*ratio),qRound(rect.height()*ratio))).scaled(viewport->frameSize(),Qt::IgnoreAspectRatio,Qt::FastTransformation);
                    }
                    // Read back on the GL owner thread, then compress owned
                    // image copies off the event loop. Keep one bounded job.
                    const auto directory=QFileInfo(path).dir();
                    run->captureJob=std::async(std::launch::async,[nativeImage,originalImage,directory,name,p]{
                        const bool nativeSaved=!nativeImage.isNull()&&nativeImage.save(directory.filePath(name+"-native.png"));
                        const bool originalSaved=!originalImage.isNull()&&originalImage.save(directory.filePath(name+"-original.png"));
                        QSaveFile result(directory.filePath(name+".json"));
                        if(result.open(QIODevice::WriteOnly)){result.write(QJsonDocument(QJsonObject{{"native_saved",nativeSaved},{"original_saved",originalSaved},{"native_frames",qint64(p.frames)},{"painted_frames",qint64(p.paints)},{"fallback",p.fallback}}).toJson());result.commit();}
                    });
                    QFile::remove(captureRequest);
                    }
                }else QFile::remove(captureRequest);
            }
        }
        if(p.fallback){finish(false,"Native rendering fell back to the original window");return;}
        if(normalQuit&&run->stage==9&&run->quitPass==0&&run->quitAnswered&&p.active&&p.frames>=run->frameBase+3){
            if(!snapshot("quit-no-resumed"))return;
            run->quitPass=1;run->stage=7;
            window.activateWindow();window.raise();
            auto* input=new QProcess(&window);
            QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[input,finish](int code,QProcess::ExitStatus status){if(code||status!=QProcess::NormalExit)finish(false,"Quit repeat Escape failed");input->deleteLater();});
            input->start("python3",{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"});
            return;
        }
        if(run->stage!=3&&run->stage!=6)return;
        if(!p.active||p.frames<run->frameBase+3)return;
        if(run->stage==3&&!QFileInfo::exists(path+".world-ready"))return;
        auto* legacy=window.findChild<QWidget*>("legacyGameContainer");
        if(legacy&&legacy->isVisible()){finish(false,"Gameplay still uses the original Wine viewport");return;}
        // Include bounded capture retry time in the ordinary rate windows.
        if(run->sampleClock.isValid()){
            const auto elapsed=run->sampleClock.elapsed();
            if(elapsed>=1000){
                run->rates.append(QJsonObject{{"cycle",run->cycle},{"seconds",double(elapsed)/1000},{"native_frames",qint64(p.frames-run->sampleFrames)},{"painted_frames",qint64(p.paints-run->samplePaints)},{"native_fps",1000.0*(p.frames-run->sampleFrames)/elapsed},{"paint_fps",1000.0*(p.paints-run->samplePaints)/elapsed}});
                run->sampleClock.restart();run->sampleFrames=p.frames;run->samplePaints=p.paints;
            }
        }
        if(stressSeconds&&!run->stressDone){
            if(!run->stressRunning){
                run->stressRunning=true;run->sampleClock.start();run->sampleFrames=p.frames;run->samplePaints=p.paints;
                window.activateWindow();window.raise();
                auto* viewport=window.findChild<QOpenGLWidget*>("nativeGameViewport");
                if(!viewport||!viewport->isVisible()){finish(false,"Native stress viewport unavailable");return;}
                viewport->setFocus();auto* native=dynamic_cast<GlViewport*>(viewport);const auto rect=native?native->imageRect():QRectF(viewport->rect());const auto origin=viewport->mapToGlobal(rect.topLeft().toPoint());
                auto* input=new QProcess(&window);
                QObject::connect(input,&QProcess::errorOccurred,&window,[finish](QProcess::ProcessError error){if(error==QProcess::FailedToStart)finish(false,"Could not start gameplay input helper");});
                QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[run,input,finish](int code,QProcess::ExitStatus status){
                    run->stressRunning=false;run->stressDone=true;
                    if(code||status!=QProcess::NormalExit)finish(false,"Gameplay input failed: "+QString::fromLocal8Bit(input->readAllStandardError()));
                    input->deleteLater();
                });
                QStringList arguments{QDir::current().filePath(castingCombat&&run->cycle==0?"tools/campaign-combat-input.py":"tools/campaign-gameplay-input.py"),"--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--seconds",QString::number(stressSeconds),"--output",QFileInfo(path).dir().filePath(QString("input-%1.json").arg(run->cycle))};
                if(castingCombat&&run->cycle>0)arguments<<"--camera-only";
                if(castingCombat&&run->cycle==0)arguments<<"--experiment"<<session.evidenceDirectory()<<"--portrait-stress-seconds"<<QString::number(qEnvironmentVariableIntValue("MNM_CAMPAIGN_PORTRAIT_SECONDS"));
                if(castingCombat&&run->cycle==0&&qEnvironmentVariableIntValue("MNM_CAMPAIGN_SPELL_CASES")==1)arguments<<"--spell-cases";
                input->start("python3",arguments);
            }
            return;
        }
        if(!snapshot(run->stage==6?"gameplay-resumed":"gameplay")){
            if(!run->snapshotWait.isValid())run->snapshotWait.start();
            if(run->snapshotWait.elapsed()>5000)finish(false,"Independent original capture did not become readable");
            return;
        }
        run->snapshotWait.invalidate();run->sampleClock.invalidate();
        if(run->stage==6){
            if(run->cycle>=cycles&&!normalQuit){finish(true,QString());return;}
        }
        run->stage=normalQuit&&run->cycle>=cycles?7:4;
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
    auto finished=session.finished;session.finished=[finished,finish,run,normalQuit]{if(finished)finished();finish(normalQuit&&run->stage==11,normalQuit&&run->stage==11?QString():QString("Game exited before campaign smoke completed"));};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    session.preferencesStorePath=QFileInfo(path).dir().filePath("config/engine-preferences.json");
    const int timeoutMs=qEnvironmentVariableIntValue("MNM_CAMPAIGN_TIMEOUT_MS");
    QTimer::singleShot(timeoutMs>0?timeoutMs:300000,&window,[finish]{finish(false,"Campaign flow exceeded configured deadline");});
    QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
