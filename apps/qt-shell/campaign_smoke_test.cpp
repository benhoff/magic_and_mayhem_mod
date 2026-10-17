#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include "frame_stream.hpp"
#include "native_frame_trace.hpp"
#include "native_image_encoder.hpp"
#include "native_capture_request.hpp"
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
#include <stdexcept>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <cstdio>

// Public controls and ordinary session feature flags. No engine readiness fix,
// protocol downgrade or fallback is enabled by this automation.
void installCampaignSmokeTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,
                             const QString& path,std::function<CampaignPresentationProbe()> probe){
    if(qEnvironmentVariableIntValue("MNM_CAMPAIGN_STORAGE")==1){installCampaignStorageTest(app,window,session,path,probe);return;}
    if(qEnvironmentVariableIntValue("MNM_NATIVE_HEALING")==1){installNativeHealingSmokeTest(app,window,session,path,probe);return;}
    const int difficulty=qEnvironmentVariableIntValue("MNM_CAMPAIGN_DIFFICULTY");
    const int stressSeconds=qEnvironmentVariableIntValue("MNM_CAMPAIGN_STRESS_SECONDS");
    const int cycles=qMax(1,qEnvironmentVariableIntValue("MNM_CAMPAIGN_MENU_CYCLES"));
    const bool normalQuit=qEnvironmentVariableIntValue("MNM_CAMPAIGN_NORMAL_QUIT")==1;
    const bool castingCombat=qEnvironmentVariableIntValue("MNM_CAMPAIGN_CASTING_COMBAT")==1;
    const bool synchronousRouteCapture=qEnvironmentVariableIntValue("MNM_SYNC_ROUTE_CAPTURE")==1;
    const bool noRouteScreenshots=qEnvironmentVariableIntValue("MNM_NO_ROUTE_SCREENSHOTS")==1;
    struct Run {NativeImageEncoder encoder;QString renderer;std::future<void> captureJob,diagnosticJob;std::future<QJsonObject> routeJob;std::future<QString> requestJob;QString captureName;QString routeName;int routeCycle=0;QElapsedTimer routeWait;int escapeCounter=0;int quitPass=0;bool quitAnswered=false;int cycle=0;bool stressRunning=false,stressDone=false;quint64 sampleFrames=0,samplePaints=0;qint64 sampleHostUs=0;quint64 resumeSourceBase=0;QElapsedTimer sampleClock,snapshotWait;QJsonArray rates;int stage=0;QElapsedTimer diagnosticClock;quint32 thread=0;quint64 frameBase=0;bool done=false;QJsonArray steps;};
    auto run=std::make_shared<Run>();
    auto finish=[run,path,probe,&app,normalQuit,noRouteScreenshots,synchronousRouteCapture](bool success,const QString& reason){
        if(run->done)return;
        run->done=true;const auto p=probe();
        QSaveFile file(path);
        const QJsonObject result{{"route_capture_policy",noRouteScreenshots?"omitted":synchronousRouteCapture?"synchronous":"bounded-background-encoding"},{"route_screenshots",!noRouteScreenshots},{"success",success},{"normal_quit",normalQuit&&success&&run->stage==11},{"error",reason},{"steps",run->steps},
            {"native_command_frames",qint64(p.frames)},{"native_painted_frames",qint64(p.paints)},{"native_presentation_pixels",qint64(p.presentationPixels)},{"native_recoveries",int(p.recoveries)},{"native_error",p.error},{"renderer",p.renderer.isEmpty()?run->renderer:p.renderer},{"gameplay_rates",run->rates},{"native_command_fallback",p.fallback},
            {"movement_verified",false},{"original_drawing_retained",qEnvironmentVariable("MNM_PUBLIC_RESIDENT_WORLD")!="1"},{"original_ui_drawing_retained",true},{"original_simulation_retained",true},{"resident_world_requested",qEnvironmentVariable("MNM_PUBLIC_RESIDENT_WORLD")=="1"},
            {"scope","Native Qt New Game/Enter, native command presentation, Escape/Mini/Cancel and resumed presentation; no movement, pixel equivalence or complete drawing replacement claim"}};
        const bool saved=file.open(QIODevice::WriteOnly)&&file.write(QJsonDocument(result).toJson())>0&&file.commit();
        std::fprintf(stderr,"Campaign smoke %s: %s\n",success&&saved?"complete":"failed",qPrintable(reason));
        // The runner owns bounded cleanup of its private Wine prefix.
        if(!saved)app.exit(1);
        else if(normalQuit&&success&&run->stage==11)app.exit(0);
    };
    auto snapshot=[run,&window,path,&session,noRouteScreenshots,synchronousRouteCapture,finish](const QString& name){
        auto* activityViewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
        if(noRouteScreenshots){run->steps.append(QJsonObject{{"step",name},{"route_capture_skipped",true}});return true;}
        if(run->routeJob.valid()){
            if(run->routeName!=name||run->routeCycle!=run->cycle){finish(false,"Overlapping route capture identity");return false;}
            if(run->routeJob.wait_for(std::chrono::seconds(0))!=std::future_status::ready){
                if(run->routeWait.elapsed()>5000)finish(false,"Route image encoding exceeded deadline");
                return false;
            }
            try{auto result=run->routeJob.get();
                if(!result["screenshot_saved"].toBool()||(!result["native_image"].toString().isEmpty()&&(!result["native_image_saved"].toBool()||!result["original_image_saved"].toBool()))){finish(false,"Route image encoding failed");return false;}
                run->steps.append(result);return true;
            }catch(const std::exception& e){finish(false,QString("Route image encoding refused: ")+e.what());return false;}
        }
        GlViewport::ActivityScope capture(activityViewport,GlViewport::Activity::RouteCapture);
        QElapsedTimer guiClock;guiClock.start();QImage originalImage,nativeImage,screenImage;
        const bool gameplay=name=="gameplay"||name=="gameplay-resumed"||name=="quit-no-resumed";
        if(gameplay){
            GlViewport::ActivityScope read(activityViewport,GlViewport::Activity::ReferenceRead);
            FrameStream reference;
            if(reference.open(QDir(session.evidenceDirectory()).filePath("render-frame.bin")))originalImage=reference.nextFrame();
            if(originalImage.isNull())return false;
        }
        window.raise();
        const auto file=QFileInfo(path).dir().filePath(name+QString("-%1.png").arg(run->cycle));
        {GlViewport::ActivityScope read(activityViewport,GlViewport::Activity::ScreenCapture);if(auto* screen=window.screen())screenImage=screen->grabWindow(window.winId()).toImage();}
        QString nativeFile,originalFile;
        if(gameplay){
            if(activityViewport&&activityViewport->isVisible()){
                const auto rect=activityViewport->imageRect();const auto ratio=activityViewport->devicePixelRatioF();
                GlViewport::ActivityScope read(activityViewport,GlViewport::Activity::NativeReadback);
                nativeImage=activityViewport->grabFramebuffer().copy(QRect(qRound(rect.x()*ratio),qRound(rect.y()*ratio),qRound(rect.width()*ratio),qRound(rect.height()*ratio))).scaled(activityViewport->frameSize(),Qt::IgnoreAspectRatio,Qt::FastTransformation);
            }
            nativeFile=QFileInfo(path).dir().filePath(name+QString("-native-%1.png").arg(run->cycle));
            originalFile=QFileInfo(path).dir().filePath(name+QString("-original-%1.png").arg(run->cycle));
        }
        const double guiMs=guiClock.nsecsElapsed()/1000000.;
        auto encode=[screenImage,nativeImage,originalImage,file,nativeFile,originalFile,name,guiMs]{
            QElapsedTimer clock;clock.start();const bool saved=screenImage.save(file);
            const bool nativeSaved=!nativeFile.isEmpty()&&nativeImage.save(nativeFile);
            const bool originalSaved=!originalFile.isEmpty()&&originalImage.save(originalFile);
            return QJsonObject{{"native_image",nativeFile},{"native_image_saved",nativeSaved},{"original_image",originalFile},{"original_image_saved",originalSaved},{"step",name},{"screenshot",file},{"screenshot_saved",saved},{"capture_gui_ms",guiMs},{"encoding_ms",clock.nsecsElapsed()/1000000.}};
        };
        if(synchronousRouteCapture){GlViewport::ActivityScope encodeTiming(activityViewport,GlViewport::Activity::ImageEncoding);run->steps.append(encode());return true;}
        run->routeName=name;run->routeCycle=run->cycle;run->routeWait.start();
        run->routeJob=run->encoder.submit(std::move(encode));return false;
    };
    auto click=[&window,finish](const QString& name){
        auto* button=window.findChild<QAbstractButton*>(name);
        if(!button||!button->isVisible()||!button->isEnabled()){finish(false,"Public control unavailable: "+name);return;}
        button->click();
    };
    auto sendEscape=[run,&window,&session,path,finish]{
        window.activateWindow();window.raise();
        QStringList arguments{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"};
        if(qEnvironmentVariableIntValue("MNM_PIXEL_CAMERA_SCENARIOS")==1){
            auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
            if(!viewport||!viewport->isVisible()){finish(false,"Native camera Mini viewport unavailable");return;}
            const auto rect=viewport->imageRect();const auto origin=viewport->mapToGlobal(rect.topLeft().toPoint());
            arguments={QDir::current().filePath("tools/campaign-camera-menu-input.py"),"--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--experiment",session.evidenceDirectory(),"--output",QFileInfo(path).dir().filePath(QString("mini-input-%1.json").arg(run->escapeCounter++))};
        }
        auto* input=new QProcess(&window);
        QObject::connect(input,&QProcess::errorOccurred,&window,[finish](QProcess::ProcessError error){
            if(error==QProcess::FailedToStart)finish(false,"Could not start Escape input helper");
        });
        QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[input,finish](int code,QProcess::ExitStatus status){
            if(code||status!=QProcess::NormalExit)finish(false,"Escape input failed: "+QString::fromLocal8Bit(input->readAllStandardError()));
            input->deleteLater();
        });
        input->start("python3",arguments);
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
        const auto advance=[snapshot,click,step,action,run,&window](auto self)->void {
            if(run->done)return;
            if(!step.isEmpty()&&!snapshot(step)){QTimer::singleShot(10,&window,[self]{self(self);});return;}
            click(action);
        };
        QTimer::singleShot(0,&window,[advance]{advance(advance);});
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
            QObject::connect(input,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[input,run,finish,probe](int code,QProcess::ExitStatus status){
                if(code||status!=QProcess::NormalExit)finish(false,"Native Quit input failed: "+QString::fromLocal8Bit(input->readAllStandardError()));
                else{run->resumeSourceBase=probe().sourceGeneration;run->quitAnswered=true;}
                input->deleteLater();
            });
            input->start("python3",{QDir::current().filePath("tools/campaign-quit-input.py"),"--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),"--answer",run->quitPass?"yes":"no","--experiment",session.evidenceDirectory(),"--output",QFileInfo(path).dir().filePath(QString("quit-input-%1.json").arg(run->quitPass))});
            return;
        }
        if(run->stage!=5){finish(false,"Unexpected original viewport handoff");return;}
        run->stage=6;run->stressDone=false;run->frameBase=probe().frames;++run->cycle;
    };
    const auto captureRequestPath=QFileInfo(path).dir().filePath("capture-request.json");
    auto* poll=new QTimer(&window);poll->setInterval(100);
    QObject::connect(poll,&QTimer::timeout,&window,[run,probe,finish,snapshot,path,stressSeconds,cycles,castingCombat,normalQuit,captureRequestPath,sendEscape,&session,&window]{
        if(run->done)return;
        auto* phaseViewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
        GlViewport::ActivityScope campaignActivity(phaseViewport,GlViewport::Activity::CampaignPoll);
        const auto p=probe();
        if(!run->diagnosticClock.isValid() || run->diagnosticClock.elapsed()>=1000){
            GlViewport::ActivityScope progressActivity(phaseViewport,GlViewport::Activity::ProgressPublication);
            run->diagnosticClock.start();
            const bool ready=!run->diagnosticJob.valid() || run->diagnosticJob.wait_for(std::chrono::seconds(0))==std::future_status::ready;
            if(ready){
                try{if(run->diagnosticJob.valid())run->diagnosticJob.get();}
                catch(const std::exception& e){finish(false,QString::fromUtf8(e.what()));return;}
                const auto bytes=QJsonDocument(QJsonObject{{"stage",run->stage},{"active",p.active},{"frames",qint64(p.frames)},{"paints",qint64(p.paints)},{"stress_running",run->stressRunning},{"stress_done",run->stressDone},{"error",p.error}}).toJson();
                // Progress is diagnostic: keep one immutable owned write and
                // skip intermediates while busy. Every FPS sample stays intact.
                run->diagnosticJob=std::async(std::launch::async,[path,bytes]{
                    QSaveFile diagnostic(path+".progress.json");
                    if(!diagnostic.open(QIODevice::WriteOnly)||diagnostic.write(bytes)!=bytes.size()||!diagnostic.commit())
                        throw std::runtime_error("Campaign progress publication failed");
                });
            }
        }
        if(p.active&&!p.renderer.isEmpty())run->renderer=p.renderer;
        {GlViewport::ActivityScope requestActivity(phaseViewport,GlViewport::Activity::CaptureRequest);
        const auto captureRequest=captureRequestPath;
        const bool captureReady=!run->captureJob.valid()||run->captureJob.wait_for(std::chrono::seconds(0))==std::future_status::ready;
        try{
            if(run->requestJob.valid()&&run->requestJob.wait_for(std::chrono::seconds(0))==std::future_status::ready)run->captureName=run->requestJob.get();
            if(captureReady&&run->captureJob.valid())run->captureJob.get();
        }catch(const std::exception& e){finish(false,QString("Diagnostic request refused: ")+e.what());return;}
        if(p.active&&captureReady&&run->captureName.isEmpty()&&!run->requestJob.valid())
            run->requestJob=run->encoder.trySubmit([captureRequest]{return readNativeCaptureRequest(captureRequest);});
        if(p.active&&captureReady&&!run->captureName.isEmpty()){
                const auto name=run->captureName;
                    auto* activityViewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
                    GlViewport::ActivityScope capture(activityViewport,GlViewport::Activity::DiagnosticCapture);
                    QElapsedTimer captureClock;captureClock.start();
                    FrameStream reference;
                    QImage originalImage;
                    {GlViewport::ActivityScope read(activityViewport,GlViewport::Activity::ReferenceRead);if(reference.open(QDir(session.evidenceDirectory()).filePath("render-frame.bin")))originalImage=reference.nextFrame();}
                    const double originalCopyMs=captureClock.nsecsElapsed()/1000000.0;
                    if(!originalImage.isNull()){
                    auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QOpenGLWidget*>("nativeGameViewport"));
                    QImage nativeImage;
                    if(viewport&&viewport->isVisible()){
                        const auto rect=viewport->imageRect();const auto ratio=viewport->devicePixelRatioF();
                        GlViewport::ActivityScope read(activityViewport,GlViewport::Activity::NativeReadback);
                        nativeImage=viewport->grabFramebuffer().copy(QRect(qRound(rect.x()*ratio),qRound(rect.y()*ratio),qRound(rect.width()*ratio),qRound(rect.height()*ratio))).scaled(viewport->frameSize(),Qt::IgnoreAspectRatio,Qt::FastTransformation);
                    }
                    const double captureOwnerMs=captureClock.nsecsElapsed()/1000000.0;
                    // Read back on the GL owner thread, then compress owned
                    // image copies off the event loop. Keep one bounded job.
                    const auto directory=QFileInfo(path).dir();
                    run->captureJob=run->encoder.submit([nativeImage,originalImage,directory,name,p,originalCopyMs,captureOwnerMs,captureRequest]{
                        if(!QFile::remove(captureRequest))throw std::runtime_error("Diagnostic request cleanup failed");
                        const bool nativeSaved=!nativeImage.isNull()&&nativeImage.save(directory.filePath(name+"-native.png"));
                        const bool originalSaved=!originalImage.isNull()&&originalImage.save(directory.filePath(name+"-original.png"));
                        QSaveFile result(directory.filePath(name+".json"));
                        if(result.open(QIODevice::WriteOnly)){result.write(QJsonDocument(QJsonObject{{"native_saved",nativeSaved},{"original_saved",originalSaved},{"native_frames",qint64(p.frames)},{"painted_frames",qint64(p.paints)},{"fallback",p.fallback},{"original_copy_ms",originalCopyMs},{"native_readback_ms",captureOwnerMs-originalCopyMs},{"capture_owner_ms",captureOwnerMs}}).toJson());result.commit();}
                    });
                    run->captureName.clear();
                    }
        }
        }
        if(p.fallback){finish(false,"Native rendering fell back to the original window");return;}
        if(normalQuit&&run->stage==9&&run->quitPass==0&&run->quitAnswered&&campaignResumeConfirmed(p,run->frameBase,run->resumeSourceBase,qEnvironmentVariable("MNM_PUBLIC_RESIDENT_WORLD")=="1")){
            if(!snapshot("quit-no-resumed"))return;
            run->quitPass=1;run->stage=7;
            sendEscape();
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
                const auto hostUs=NativeFrameTrace::now();
                run->rates.append(QJsonObject{{"host_begin_us",QString::number(run->sampleHostUs)},{"host_end_us",QString::number(hostUs)},{"cycle",run->cycle},{"monotonic_start_seconds",double(run->sampleClock.msecsSinceReference())/1000},{"monotonic_end_seconds",double(run->sampleClock.msecsSinceReference()+elapsed)/1000},{"seconds",double(elapsed)/1000},{"native_frames",qint64(p.frames-run->sampleFrames)},{"painted_frames",qint64(p.paints-run->samplePaints)},{"native_fps",1000.0*(p.frames-run->sampleFrames)/elapsed},{"paint_fps",1000.0*(p.paints-run->samplePaints)/elapsed}});
                run->sampleClock.restart();run->sampleHostUs=hostUs;run->sampleFrames=p.frames;run->samplePaints=p.paints;
            }
        }
        if(stressSeconds&&!run->stressDone){
            if(!run->stressRunning){
                run->stressRunning=true;run->sampleClock.start();run->sampleHostUs=NativeFrameTrace::now();run->sampleFrames=p.frames;run->samplePaints=p.paints;
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
                if(castingCombat&&run->cycle>0)arguments<<"--camera-only"<<"--keep-moving"<<"--experiment"<<session.evidenceDirectory();
                if(qEnvironmentVariableIntValue("MNM_PIXEL_CAMERA_SCENARIOS"))arguments<<"--experiment"<<session.evidenceDirectory();
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
        sendEscape();
    });poll->start();
    auto finished=session.finished;session.finished=[finished,finish,run,normalQuit]{if(finished)finished();finish(normalQuit&&run->stage==11,normalQuit&&run->stage==11?QString():QString("Game exited before campaign smoke completed"));};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    session.preferencesStorePath=QFileInfo(path).dir().filePath("config/engine-preferences.json");
    const int timeoutMs=qEnvironmentVariableIntValue("MNM_CAMPAIGN_TIMEOUT_MS");
    QTimer::singleShot(timeoutMs>0?timeoutMs:300000,&window,[finish]{finish(false,"Campaign flow exceeded configured deadline");});
    QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
