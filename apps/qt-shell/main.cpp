#include "menu_audio_controller.hpp"
#include "menu_music_controller.hpp"
#include <QMediaDevices>
#include <QSettings>
#include "audio_cli.hpp"
#include "voice_bridge.hpp"
#include "window_host.hpp"
#include "main_menu_widget.hpp"
#include "menu_preview.hpp"
#include "quick_battle_menu_widget.hpp"
#include "live_menu_session.hpp"
#include "live_battle_menu_controller.hpp"
#include "live_spell_menu_controller.hpp"
#include "live_mini_menu_controller.hpp"
#include "live_result_menu_controller.hpp"
#include "live_campaign_defeat_controller.hpp"
#include "../../protocols/include/mnm/menu_v11.h"
#include "live_preferences_menu_controller.hpp"
#include "live_region_entry_controller.hpp"
#include "../../protocols/include/mnm/menu_v7.h"
#include "../../protocols/include/mnm/menu_v6.h"
#include "../../protocols/include/mnm/menu_v5.h"
#include "live_menu_test.hpp"
#include "../../protocols/include/mnm/menu_v2.h"
#include <QStackedWidget>
#include "media_cli.hpp"
#include "media_broker.hpp"
#include "gl_viewport.hpp"
#include "frame_stream.hpp"
#include "input_forwarder.hpp"
#include "blit.hpp"
#include "commands.hpp"
#include "command_replay.hpp"
#include "live_command_renderer.hpp"
#include "live_command_session.hpp"
#include <QFile>
#include <QSurfaceFormat>
#include <QUuid>
#include <QRandomGenerator>
#include <memory>
#include <QApplication>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QDir>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLabel>
#include <QMainWindow>
#include <QMetaEnum>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWindow>
#include <cstdio>
#include <exception>

class Shell final:public QMainWindow {
public:
    explicit Shell(QString repository,bool opengl=false,bool captureDraws=false,bool captureHistory=false,bool skipMovies=false,bool noReadback=false,bool captureLocks=false,bool nativeMedia=false,bool nativeVoices=false,bool liveMenus=false,bool nativeCommands=false):repo_(std::move(repository)),opengl_(opengl),captureDraws_(captureDraws),captureHistory_(captureHistory),skipMovies_(skipMovies),noReadback_(noReadback),captureLocks_(captureLocks),nativeMedia_(nativeMedia),nativeVoices_(nativeVoices),nativeCommands_(nativeCommands) {
        setWindowTitle("Magic & Mayhem Workshop");resize(1100,850);
        viewport_=new QWidget(this);layout_=new QVBoxLayout(viewport_);
        layout_->setContentsMargins(0,0,0,0);viewport_->setMinimumSize(800,600);
        placeholder_=new QLabel("Launch the game to open its viewport.",viewport_);
        placeholder_->setAlignment(Qt::AlignCenter);layout_->addWidget(placeholder_);setCentralWidget(viewport_);
        if(liveMenus){
            menuStack_=new QStackedWidget(this);takeCentralWidget();menuStack_->addWidget(viewport_);setCentralWidget(menuStack_);
            liveMain_=new MainMenuWidget(menuStack_);liveQuick_=new QuickBattleMenuWidget(menuStack_);
            menuStack_->addWidget(liveMain_);menuStack_->addWidget(liveQuick_);
            liveSetup_=new SinglePlayerBattleWidget(menuStack_);liveMap_=new MapSelectionWidget(menuStack_);
            menuStack_->addWidget(liveSetup_);menuStack_->addWidget(liveMap_);
            liveSpells_=new SpellboxWidget(menuStack_);liveSpells_->setObjectName("liveSpellSelection");menuStack_->addWidget(liveSpells_);
            liveMenus_=std::make_unique<LiveMenuSession>(repo_,this);
            battleMenus_=std::make_unique<LiveBattleMenuController>(*liveMenus_,*liveSetup_,*liveMap_);
            spellMenus_=std::make_unique<LiveSpellMenuController>(*liveMenus_,*liveSpells_);
            liveDefeat_=new BattleResultWidget(menuStack_);menuStack_->addWidget(liveDefeat_);
            defeatMenus_=std::make_unique<LiveCampaignDefeatController>(*liveMenus_,*liveDefeat_);
            liveResults_=new QuickBattleResultWidget(menuStack_);menuStack_->addWidget(liveResults_);
            resultMenus_=std::make_unique<LiveResultMenuController>(*liveMenus_,*liveResults_);
            livePreferences_=new PreferencesWidget(menuStack_);menuStack_->addWidget(livePreferences_);
            preferencesMenus_=std::make_unique<LivePreferencesMenuController>(*liveMenus_,*livePreferences_);
            liveRegion_=new RegionEntryWidget(menuStack_);menuStack_->addWidget(liveRegion_);
            regionMenus_=std::make_unique<LiveRegionEntryController>(*liveMenus_,*liveRegion_);
            if(liveMenus_->miniMenusEnabled()){
                liveMini_=new MiniMenuWidget(menuStack_);menuStack_->addWidget(liveMini_);
                miniMenus_=std::make_unique<LiveMiniMenuController>(*liveMenus_,*liveMini_);
            }
            connect(liveMain_,&MainMenuWidget::actionRequested,this,[this](MainMenuWidget::Action action){
                if(action==MainMenuWidget::Action::NewGame)liveMenus_->request(MNM_MENU_NEW_GAME);
                else if(action==MainMenuWidget::Action::QuickBattle)liveMenus_->request(MNM_MENU_OPEN_QUICK);
                else if(action==MainMenuWidget::Action::Preferences)liveMenus_->request(MNM_MENU_OPEN_PREFERENCES);
                else if(action==MainMenuWidget::Action::Quit)close();
            });
            connect(liveQuick_,&QuickBattleMenuWidget::actionRequested,this,[this](QuickBattleMenuWidget::Action action){
                if(action==QuickBattleMenuWidget::Action::Cancel)liveMenus_->request(MNM_MENU_BACK);
                else if(action==QuickBattleMenuWidget::Action::CreateSinglePlayer)liveMenus_->request(MNM_MENU_OPEN_SINGLE);
            });
            liveMenus_->launched=[this]{elapsed_.restart();poll_.start();placeholder_->setText("Checking launch files and starting Wine…\nStartup may take a few minutes. See the launch log below.");statusBar()->showMessage("Checking launch files and starting Wine…");};
            liveMenus_->output=[this](const QString& text){log_->appendPlainText(text.trimmed());};
            liveMenus_->failed=[this](const QString& error){menuStack_->setCurrentWidget(viewport_);fallback_->setEnabled(false);if(!foreign_)placeholder_->setText(error+"\nSee the launch log below.");statusBar()->showMessage(error);};
            liveMenus_->battleStarted=[this](quint32 destination){menuStack_->setCurrentWidget(viewport_);fallback_->setEnabled(true);if(container_)container_->setFocus(Qt::OtherFocusReason);if(foreign_)foreign_->requestActivate();statusBar()->showMessage(destination==3?"Original campaign startup active. Use original game controls.":destination==1?"Original game active. Complete spell selection; Qt menus return after battle.":"Original game active. Qt menus return when the engine reaches Main or Quick Battle.");};
            liveMenus_->originalViewportRequested=[this]{menuStack_->setCurrentWidget(viewport_);fallback_->setEnabled(true);if(container_)container_->setFocus(Qt::OtherFocusReason);if(foreign_)foreign_->requestActivate();statusBar()->showMessage("Original game controls active. Confirmations and Preferences use this viewport.");};
            liveMenus_->finished=[this]{menuStack_->setCurrentWidget(viewport_);fallback_->setEnabled(false);finished();if(closeAfterGame_)close();};
            liveMenus_->stateChanged=[this](const MenuBridge::State& state){
                if(!menuAssetsLoaded_){QString error;const auto root=QDir(repo_).filePath("working/game-nocd");
                    if(!liveDefeat_->loadAssets(root,BattleResultWidget::Outcome::Defeat,&error)||!liveRegion_->loadAssets(root,&error)||!livePreferences_->loadAssets(root,&error)||!liveResults_->loadAssets(root,&error)||!liveMain_->loadAssets(root,&error)||!liveQuick_->loadAssets(root,&error)||!battleMenus_->loadAssets(root,&error)||!liveSpells_->loadAssets(root,&error)){liveMenus_->fallback(error);return;}
                    if(liveMini_&&!liveMini_->loadAssets(root,&error)){liveMenus_->fallback(error);return;}
                    for(int i=0;i<6;++i)if(i!=0&&i!=2&&i!=3&&i!=4)liveMain_->findChild<QPushButton*>(QString("mainMenuAction%1").arg(i))->setEnabled(false);
                    for(int i=0;i<2;++i)liveQuick_->findChild<QPushButton*>(QString("quickBattleAction%1").arg(i))->setEnabled(false);
                    menuAssetsLoaded_=true;
                }
                QString error;if(!defeatMenus_->present(state,&error)||!regionMenus_->present(state,&error)||!preferencesMenus_->present(state,&error)||!resultMenus_->present(state,&error)||!battleMenus_->present(state,&error)||!spellMenus_->present(state,&error)||(miniMenus_&&!miniMenus_->present(state,&error))){liveMenus_->fallback(error);return;}
                QWidget* screen=state.screen==MNM_MENU_DEFEAT_SCREEN?static_cast<QWidget*>(liveDefeat_):state.screen==18?static_cast<QWidget*>(liveRegion_):state.screen==10?static_cast<QWidget*>(livePreferences_):state.screen==MNM_MENU_RESULT_SCREEN?static_cast<QWidget*>(liveResults_):liveMini_&&state.screen==MNM_MENU_MINI_SCREEN?static_cast<QWidget*>(liveMini_):state.screen==7?static_cast<QWidget*>(liveSpells_):state.screen==14?static_cast<QWidget*>(liveSetup_):state.screen==25?static_cast<QWidget*>(liveMap_):state.screen==3?static_cast<QWidget*>(liveMain_):state.screen==22?static_cast<QWidget*>(liveQuick_):viewport_;
                fallback_->setEnabled(true);const bool changed=menuStack_->currentWidget()!=screen;const bool gainedReady=state.ready&&(changed||!screen->isEnabled());menuStack_->setCurrentWidget(screen);
                liveSpells_->setEnabled(state.ready&&state.screen==7);liveMain_->setEnabled(state.ready&&state.screen==3);liveQuick_->setEnabled(state.ready&&state.screen==22);liveSetup_->setEnabled(state.ready&&state.screen==14);liveMap_->setEnabled(state.ready&&state.screen==25);
                liveRegion_->setEnabled(state.ready&&state.screen==18);
                livePreferences_->setEnabled(state.ready&&state.screen==10);
                liveDefeat_->setEnabled(state.ready&&state.screen==MNM_MENU_DEFEAT_SCREEN);
                liveResults_->setEnabled(state.ready&&state.screen==MNM_MENU_RESULT_SCREEN);
                if(liveMini_)liveMini_->setEnabled(state.ready&&state.screen==MNM_MENU_MINI_SCREEN);
                if(gainedReady){if(state.screen==MNM_MENU_DEFEAT_SCREEN)liveDefeat_->focusContinue();else if(state.screen==18)liveRegion_->focusFirstControl();else if(state.screen==10)livePreferences_->focusFirstControl();else if(state.screen==MNM_MENU_RESULT_SCREEN)liveResults_->focusFirstAction();else if(liveMini_&&state.screen==MNM_MENU_MINI_SCREEN)liveMini_->focusFirstAction();else if(state.screen==7)liveSpells_->focusFirstControl();else if(state.screen==14)liveSetup_->focusFirstControl();else if(state.screen==25)liveMap_->focusSelection();else if(state.screen==22)liveQuick_->focusFirstAction();else if(state.screen==3)liveMain_->findChild<QPushButton*>("mainMenuAction2")->setFocus();}
                statusBar()->showMessage(state.ready?(state.screen==7?QString("Spell selection connected. Edits apply on Start battle. Time remaining: %1").arg(state.spells.seconds<0?QString("expired"):QString::number(state.spells.seconds)):state.screen==14?"Battle setup connected. Edits apply when opening Map, changing a player or starting.":"Native menu connected to the engine. Other actions are available through Use original menus."):"Waiting for the engine menu transition…");
            };
        }
        if(opengl_){gl_=new GlViewport(viewport_);layout_->addWidget(gl_);gl_->hide();input_=std::make_unique<InputForwarder>(*gl_,host_);}
        auto* toolbar=addToolBar("Game");toolbar->setMovable(false);
        launch_=new QPushButton("Launch game",this);launch_->setObjectName("launchGame");toolbar->addWidget(launch_);
        check_=new QPushButton("Check installation",this);toolbar->addWidget(check_);
        detach_=new QPushButton("Detach game",this);toolbar->addWidget(detach_);detach_->setEnabled(false);
        retry_=new QPushButton("Attach game",this);toolbar->addWidget(retry_);retry_->setEnabled(false);
        if(opengl_){detach_->hide();retry_->hide();}
        if(liveMenus_){fallback_=new QPushButton("Use original menus",this);fallback_->setObjectName("originalMenusFallback");toolbar->addWidget(fallback_);fallback_->setEnabled(false);
            connect(fallback_,&QPushButton::clicked,this,[this]{liveMenus_->fallback("Original menus active for this session.");});}
        auto* dock=new QDockWidget("Launch log",this);log_=new QPlainTextEdit(dock);
        log_->setReadOnly(true);log_->setMaximumBlockCount(2000);dock->setWidget(log_);addDockWidget(Qt::BottomDockWidgetArea,dock);
        process_.setProcessChannelMode(QProcess::MergedChannels);
        connect(launch_,&QPushButton::clicked,this,[this]{start(false);});
        connect(check_,&QPushButton::clicked,this,[this]{start(true);});
        connect(detach_,&QPushButton::clicked,this,[this]{detach();statusBar()->showMessage("Game detached; use Attach game to restore the viewport.");});
        connect(retry_,&QPushButton::clicked,this,[this]{elapsed_.restart();poll_.start();});
        connect(&process_,&QProcess::readyReadStandardOutput,this,[this]{
            auto cursor=log_->textCursor();cursor.movePosition(QTextCursor::End);
            cursor.insertText(QString::fromLocal8Bit(process_.readAllStandardOutput()));log_->setTextCursor(cursor);
        });
        connect(&process_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){
            statusBar()->showMessage("Launcher error: "+process_.errorString());
            if(error==QProcess::FailedToStart)finished();
        });
        connect(&process_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus){
            finished();statusBar()->showMessage(QString("Launcher exited with status %1.").arg(code));
        });
        poll_.setInterval(250);connect(&poll_,&QTimer::timeout,this,[this]{discover();});
        inputTimer_.setInterval(50);connect(&inputTimer_,&QTimer::timeout,this,[this]{if(input_)input_->heartbeat();});
        frames_.setInterval(16);connect(&frames_,&QTimer::timeout,this,[this]{
            if(!stream_ || (media_ && media_->movieActive()))return;
            if(!gl_->error().isEmpty()){frames_.stop();statusBar()->showMessage("OpenGL initialization failed: "+gl_->error());return;}
            if(commands_){
                if(!commands_->ended() && commands_->error().isEmpty()){
                    if(!commands_->poll()){
                        log_->appendPlainText("Native command session refused: "+commands_->error());
                        statusBar()->showMessage("Native command session ended; use the original game window.");
                        input_->suspend(true);gl_->setGpuFrame({});gl_->hide();placeholder_->show();
                        placeholder_->setText("Native command session incomplete. Use the original game window.");
                    }else if(commands_->ended()){
                        input_->suspend(true);statusBar()->showMessage("Native preview finished. Continue in the original game window.");
                    }
                }
                return;
            }
            auto frame=stream_->nextFrame();
            if(!frame.isNull()){gl_->setFrame(std::move(frame));placeholder_->hide();gl_->show();
                statusBar()->showMessage(input_->target()?"OpenGL presentation active. Click the viewport to control the game.":"OpenGL presentation active. Waiting for the game input window…");}
            else if(stream_->status()==2 || stream_->status()==3 || stream_->status()==8 || stream_->status()==9 || stream_->status()==10 || stream_->status()==13 || elapsed_.elapsed()>10000){
                const QString diagnostic=(noReadback_ && !captureLocks_)?"Readback disabled for diagnosis. Use the Wine game window; Qt frames are disabled.":stream_->diagnostic();statusBar()->showMessage(diagnostic);
                if(placeholder_->isVisible())placeholder_->setText(diagnostic);
            }
        });
        if(!host_.available() && !opengl_){
            launch_->setEnabled(false);placeholder_->setText("Game embedding requires an X11 session or XWayland.\nStart this shell with QT_QPA_PLATFORM=xcb.");
            statusBar()->showMessage("Viewport unavailable on this display backend.");
        }else statusBar()->showMessage("Ready. The game starts only when you click Launch game.");
    }
    bool attach(xcb_window_t id){
        if(foreign_ || !host_.exists(id))return false;
        auto* window=QWindow::fromWinId(WId(id));
        if(!window)return false;
        container_=QWidget::createWindowContainer(window,viewport_);container_->setObjectName("legacyGameContainer");
        foreign_=window;windowId_=id;container_->setFocusPolicy(Qt::StrongFocus);
        if(liveMenus_){container_->setFixedSize(800,600);viewport_->setStyleSheet("background: black; color: white;");}
        layout_->addWidget(container_);if(liveMenus_)layout_->setAlignment(container_,Qt::AlignCenter);placeholder_->hide();container_->show();
        poll_.stop();detach_->setEnabled(true);retry_->setEnabled(false);
        statusBar()->showMessage("Game attached. Click the viewport to focus game input.");return true;
    }
    void detach(){
        poll_.stop();
        if(!foreign_)return;
        if(host_.exists(windowId_)){foreign_->setParent(nullptr);foreign_->show();}
        layout_->removeWidget(container_);delete container_;container_=nullptr;foreign_=nullptr;windowId_=0;
        placeholder_->show();detach_->setEnabled(false);
        retry_->setEnabled((process_.state()!=QProcess::NotRunning||(liveMenus_&&liveMenus_->running())) && !checking_);
    }
    WindowHost& host(){return host_;}
    LiveMenuSession* liveMenuSession(){return liveMenus_.get();}
protected:
    void closeEvent(QCloseEvent* event) override {
        if(process_.state()!=QProcess::NotRunning||(liveMenus_&&liveMenus_->running())){
            if(closeAfterGame_){event->ignore();return;}
            if(commands_)commands_->requestStop();
            if(liveMenus_&&liveMenus_->requestExit()){
                closeAfterGame_=true;statusBar()->showMessage("Exiting through the original game menu…");
            }else{
                if(liveMenus_)liveMenus_->fallback("Use the original game's Quit action, then close the shell.");
                statusBar()->showMessage("Exit the game through its original menu, then close the shell.");
            }
            event->ignore();return;
        }
        detach();event->accept();
    }
private:
    void start(bool check){
        if(process_.state()!=QProcess::NotRunning||(liveMenus_&&liveMenus_->running()))return;
        if(liveMenus_&&!check){
            excluded_=host_.windows();launch_->setEnabled(false);check_->setEnabled(false);fallback_->setEnabled(true);
            placeholder_->setText("Verifying original files and preparing the game…\nStartup may take a few minutes. See the launch log below.");
            log_->appendPlainText("Starting menu session: verifying original files, staging a disposable installation, then checking launch files. This may take a few minutes.");
            liveMenus_->start();statusBar()->showMessage("Verifying original files and preparing the menu adapter…");return;
        }
        const QString launcher=QDir(repo_).filePath(opengl_ && !check?"tools/run-opengl-game.py":"tools/run-game.sh");
        if(!QFileInfo(launcher).isExecutable()){statusBar()->showMessage("Cannot find tools/run-game.sh. Set --repo to the repository directory.");return;}
        checking_=check;excluded_=host_.windows();launch_->setEnabled(false);check_->setEnabled(false);
        auto env=QProcessEnvironment::systemEnvironment();
        // An inherited custom runner could bypass the windowed Wine launch.
        env.remove("MNM_RUNNER");env.insert("WINEDEBUG",env.value("WINEDEBUG","fixme-all"));
        process_.setProcessEnvironment(env);process_.setWorkingDirectory(repo_);
        QStringList arguments{check?"check":"launch","--no-gamescope","--window-size","800x600","--prefix",QDir(repo_).filePath("working/wineprefix-x86_64")};
        log_->appendPlainText(check?"Checking installation…":"Starting game…");
        if(opengl_ && !check){
            const QString directory=QDir(repo_).filePath("working/runtime/render");
            QDir().mkpath(directory);const QString path=directory+"/frame-"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".bin";
            stream_=std::make_unique<FrameStream>();
            if(!stream_->create(path)){finished();statusBar()->showMessage(stream_->error());return;}
            inputState_=std::make_unique<InputState>();
            const auto inputPath=path+".input";
            if(!inputState_->create(inputPath)){finished();statusBar()->showMessage("Cannot create game input channel.");return;}
            input_->setState(inputState_.get());inputTimer_.start();
            arguments={"--stream",path,"--input",inputPath};elapsed_.restart();frames_.start();
            if(nativeVoices_){
                voices_=std::make_unique<mnm::audio::VoiceBroker>();
                voices_->failed=[this](const QString& error){log_->appendPlainText("Native voice output failed: "+error);};
                if(!voices_->create(path+".audio")){const auto error=voices_->lastError();finished();statusBar()->showMessage(error);return;}
                arguments.append({"--voice-channel",path+".audio"});
            }
            if(nativeMedia_){
                media_=std::make_unique<MediaBroker>(*gl_,QDir(repo_).filePath("working/game-nocd"));
                media_->frame=[this](QImage image){gl_->setFrame(std::move(image));placeholder_->hide();gl_->show();gl_->setFocus();};
                media_->movieChanged=[this](bool playing){input_->suspend(playing || (commands_ && commands_->state()!=LiveCommandSession::State::Active));statusBar()->showMessage(playing?"Playing movie in Qt. Press Escape to skip.":"Movie finished; resuming game frames.");};
                if(!media_->create(path+".media")){finished();statusBar()->showMessage("Cannot create native media channel.");return;}
                arguments.append({"--media-channel",path+".media"});
            }
            if(captureDraws_)arguments.append("--capture-draws");
            if(captureHistory_)arguments.append("--capture-history");
            if(skipMovies_)arguments.append("--skip-movies");
            if(noReadback_)arguments.append("--no-readback");
            if(captureLocks_)arguments.append("--capture-locks");
            if(nativeCommands_){
                commands_=std::make_unique<LiveCommandSession>(*gl_);
                commands_->stateChanged=[this](LiveCommandSession::State state){if(state==LiveCommandSession::State::Stopping){input_->suspend(true);return;}if(state==LiveCommandSession::State::Recovering || state==LiveCommandSession::State::WaitingFrame){input_->suspend(true);gl_->hide();placeholder_->setText("Recovering native presentation. Original game window remains available.");placeholder_->show();statusBar()->showMessage("Recovering native presentation.");}};
                commands_->framePresented=[this]{input_->suspend(media_ && media_->movieActive());placeholder_->hide();gl_->show();statusBar()->showMessage("Native command presentation active. Original rendering retained.");};
                const bool continuous=qEnvironmentVariable("MNM_RENDER_CONTINUOUS")==QStringLiteral("1");
                if(!commands_->create(path+".commands",(QRandomGenerator::global()->generate()&0x7fffffffu)|1u,continuous?2:1)){const auto error=commands_->error();finished();statusBar()->showMessage(error);return;}
                arguments.append({"--command-channel",path+".commands"});
                if(continuous){input_->suspend(true);arguments.append({"--render-control",path+".commands.control"});}
            }
            if(nativeCommands_)gl_->show();else gl_->hide();placeholder_->show();
            placeholder_->setText((noReadback_ && !captureLocks_)?"Readback disabled for diagnosis. Use the Wine game window; Qt frames are disabled.":"Waiting for the first DirectDraw frame…");
        }
        process_.start(launcher,arguments);
        if(!check){elapsed_.restart();poll_.start();statusBar()->showMessage("Waiting for the game window…");}
    }
    void discover(){
        const auto candidates=host_.desktops(excluded_);
        if(opengl_){
            xcb_window_t target=0;
            if(candidates.size()==1 && !gl_->frameSize().isEmpty())target=host_.inputWindow(candidates.front(),gl_->frameSize());
            input_->setTarget(target);return;
        }
        if(candidates.size()==1 && attach(candidates.front()))return;
        if(candidates.size()>1){poll_.stop();retry_->setEnabled(true);statusBar()->showMessage("Multiple new game desktops found. Close extra desktops, then click Attach game.");return;}
        if(elapsed_.elapsed()>(liveMenus_?120000:30000)){poll_.stop();retry_->setEnabled(true);statusBar()->showMessage("Window not found yet. Check the launch log, then click Attach game.");}
    }
    void finished(){
        if(commands_){if(!commands_->finishProducer())log_->appendPlainText("Native command producer stopped: "+commands_->error());commands_.reset();}
        voices_.reset();media_.reset();if(input_)input_->suspend(false);
        if(input_)input_->setTarget(0);
        if(input_)input_->setState(nullptr);
        inputTimer_.stop();inputState_.reset();
        if(opengl_ && !checking_ && stream_ && placeholder_->isVisible())
            placeholder_->setText((noReadback_ && !captureLocks_)?"Diagnostic game launcher stopped. Qt frame capture was disabled.":"Game launcher stopped before a frame was captured. Last state:\n"+stream_->diagnostic());
        detach();poll_.stop();frames_.stop();retry_->setEnabled(false);checking_=false;
        launch_->setEnabled(opengl_ || host_.available());check_->setEnabled(true);
    }
    SinglePlayerBattleWidget* liveSetup_=nullptr;MapSelectionWidget* liveMap_=nullptr;
    RegionEntryWidget* liveRegion_=nullptr;std::unique_ptr<LiveRegionEntryController> regionMenus_;
    PreferencesWidget* livePreferences_=nullptr;std::unique_ptr<LivePreferencesMenuController> preferencesMenus_;
    BattleResultWidget* liveDefeat_=nullptr;std::unique_ptr<LiveCampaignDefeatController> defeatMenus_;
    QuickBattleResultWidget* liveResults_=nullptr;std::unique_ptr<LiveResultMenuController> resultMenus_;
    MiniMenuWidget* liveMini_=nullptr;std::unique_ptr<LiveMiniMenuController> miniMenus_;
    SpellboxWidget* liveSpells_=nullptr;std::unique_ptr<LiveSpellMenuController> spellMenus_;
    std::unique_ptr<LiveBattleMenuController> battleMenus_;
    std::unique_ptr<LiveMenuSession> liveMenus_;QStackedWidget* menuStack_=nullptr;MainMenuWidget* liveMain_=nullptr;QuickBattleMenuWidget* liveQuick_=nullptr;QPushButton* fallback_=nullptr;bool menuAssetsLoaded_=false,closeAfterGame_=false;
    QString repo_;bool opengl_=false,captureDraws_=false,captureHistory_=false,skipMovies_=false,noReadback_=false,captureLocks_=false,nativeMedia_=false,nativeVoices_=false,nativeCommands_=false;std::unique_ptr<LiveCommandSession> commands_;GlViewport* gl_=nullptr;std::unique_ptr<FrameStream> stream_;QTimer frames_;WindowHost host_;std::unique_ptr<InputState> inputState_;std::unique_ptr<InputForwarder> input_;std::unique_ptr<MediaBroker> media_;std::unique_ptr<mnm::audio::VoiceBroker> voices_;QTimer inputTimer_;QProcess process_;QTimer poll_;QElapsedTimer elapsed_;
    QSet<xcb_window_t> excluded_;bool checking_=false;
    QWidget* viewport_=nullptr;QVBoxLayout* layout_=nullptr;QLabel* placeholder_=nullptr;
    QWidget* container_=nullptr;QWindow* foreign_=nullptr;xcb_window_t windowId_=0;
    QPushButton *launch_=nullptr,*check_=nullptr,*detach_=nullptr,*retry_=nullptr;
    QPlainTextEdit* log_=nullptr;
};

int main(int argc,char** argv){
    // Help remains available from terminals without a graphical display.
    for(int i=1;i<argc;++i)if(QString::fromLocal8Bit(argv[i])=="--help" || QString::fromLocal8Bit(argv[i])=="-h"){
        std::printf("Usage: mnm-qt-shell [--repo DIRECTORY] [--renderer opengl|native]\n"
                    "  --menu-audio          Opt in to native preview click/page-turn cues\n"
                    "  --menu-music FILE     Loop an explicit local music track in the menu\n"
                    "  --menu-audio-policy POLICY  literal or dequote-missing-leaf\n"
                    "  --menu-click-sound ID / --menu-page-sound ID  Preview cue IDs (822/830)\n"
                    "  --audio-catalog DIR   Preview reconstructed manager through Qt audio output\n"
                    "  --audio-preflight     Report playable WAVs and missing assets without output\n"
                    "  --audio-path-policy POLICY  literal or dequote-missing-leaf\n"
                    "  --audio-report FILE   Save catalog/startup JSON\n"
                    "  --audio-map ID        Map classifications/preload (default 1)\n"
                    "  --audio-sound ID      Select a sound or group in the preview\n"
                    "  --native-media        Opt in to Qt movies and supported file sounds\n"
                    "  --native-voices       Experimental native mixer/Qt output for DirectSound voices\n"
                    "  --main-menu           Preview the native main menu without launching a game\n"
                    "  --quick-battle-menu   Preview the native Quick Battle menu\n"
                    "  --mini-menu MODE      Preview the campaign or battle Mini Menu\n"
                    "  --quick-battle-results MODE Preview results: continue or spectate\n"
                    "  --join-multiplayer    Preview Join Multiplayer with sample input\n"
                    "  --create-multiplayer  Preview Create Multiplayer with sample input\n"
                    "  --preferences         Preview Preferences with local sample settings\n"
                    "  --save-game           Preview Save Game with sample saves\n"
                    "  --load-game           Preview Load Game with sample saves\n"
                    "  --multiplayer-game-selection  Preview sample multiplayer sessions\n"
                    "  --multiplayer-lobby MODE  Preview host or join lobby with local chat\n"
                    "  --realm-viewer        Preview the campaign map and region navigation\n"
                    "  --spell-research      Preview spell research with a supplied catalog\n"
                    "  --spellbox            Preview a local Portmanteau loadout\n"
                    "  --grimoire            Preview installed Grimoire pages and contents\n"
                    "  --character-screen    Preview local character improvements\n"
                    "  --region-entry        Preview a sample region and difficulty choices\n"
                    "  --single-player-battle Preview battle setup with sample players\n"
                    "  --map-selection       Preview map selection with sample entries\n"
                    "  --battle-results MODE Preview victory or defeat with sample results\n"
                    "  --menu-assets DIR     Installed assets for the main menu preview\n"
                    "  --menu-command-line   Show the conditional CommandLine Battle preview button\n"
                    "  --media FILE          Preview AVI/WAV media without the game\n"
                    "  --media-test          Decode a preview silently and write --media-report FILE\n"
                    "  --software-rendering  Use Mesa software rendering for Qt and Wine\n"
                    "  --native-commands     Bounded live native command/GPU presentation\n"
                    "  --capture-locks       Capture bounded game-owned Lock/Unlock buffers\n"
                    "  --no-readback         Diagnostic: disable extra surface locks; use the Wine window\n"
                    "  --skip-movies         Disable movies in the disposable OpenGL installation\n"
                    "  --capture-draws       Record a small blit when launching with OpenGL\n"
                    "  --capture-history     Record a bounded indexed/RGB surface history when launching\n"
                    "  --smoke-test          Open and close the shell without launching a game\n"
                    "  --opengl-test         Check texture presentation with known pixels\n"
                    "  --commands FILE       Replay captured surface commands in a standalone viewport\n"
                    "  --command-checks      Compare CHECK records during replay (diagnostic readback)\n"
                    "  --surface-demo        Show persistent renderer surfaces and palette cycling\n"
                    "  --surface-test        Verify rendered surfaces through Qt framebuffer readback\n"
                    "  --stream-test FILE    Check a synthetic frame stream through OpenGL\n"
                    "  --embedding-test      Check an external fixture window\n");return 0;
    }
    // Select the vendor before QApplication creates any graphics/display context.
    for(int i=1;i<argc;++i)if(QString::fromLocal8Bit(argv[i])=="--software-rendering"){
        qputenv("LIBGL_ALWAYS_SOFTWARE","1");qputenv("__GLX_VENDOR_LIBRARY_NAME","mesa");
        const QString vendor="/usr/share/glvnd/egl_vendor.d/50_mesa.json";
        if(QFile::exists(vendor))qputenv("__EGL_VENDOR_LIBRARY_FILENAMES",vendor.toLocal8Bit());
    }
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);QCoreApplication::setApplicationName("mnm-qt-shell");
    QCommandLineParser parser;parser.setApplicationDescription("Magic & Mayhem Qt development shell");parser.addHelpOption();
    addMediaOptions(parser);
    addAudioOptions(parser);
    parser.addOption({"menu-music","Loop a local music track in the native menu with separate saved volume.","file"});
    parser.addOption({"menu-audio","Native preview click/page-turn sounds through the manager and Qt sink."});
    parser.addOption({"menu-audio-policy","Preview source filenames: literal or dequote-missing-leaf.","policy","literal"});
    parser.addOption({"menu-click-sound","Native preview action cue catalog ID.","id","822"});
    parser.addOption({"menu-page-sound","Native preview page-turn cue catalog ID.","id","830"});
    parser.addOption({"native-voices","Experimental native DirectSound voices through the PCM mixer and Qt output."});
    parser.addOption({"live-menu-test","Bounded Qt-driven live menu round trip; write a new JSON report.","file"});
    parser.addOption({"live-menus","Connect native Main, Quick Battle, Single Player setup and Map to the engine (Wine viewport)."});
    parser.addOption({"main-menu","Preview the native main menu without launching the game."});
    parser.addOption({"quick-battle-menu","Preview the native Quick Battle menu without launching the game."});
    parser.addOption({"mini-menu","Preview the Mini Menu: campaign or battle.","mode"});
    parser.addOption({"quick-battle-results","Preview Quick Battle results: continue or spectate.","mode"});
    parser.addOption({"join-multiplayer","Preview Join Multiplayer without networking."});
    parser.addOption({"create-multiplayer","Preview Create Multiplayer without networking."});
    parser.addOption({"preferences","Preview Preferences with local sample settings."});
    parser.addOption({"save-game","Preview Save Game with sample saves."});
    parser.addOption({"load-game","Preview Load Game with sample saves."});
    parser.addOption({"multiplayer-game-selection","Preview multiplayer session selection without discovery."});
    parser.addOption({"multiplayer-lobby","Preview multiplayer lobby: host or join.","mode"});
    parser.addOption({"realm-viewer","Preview installed realm maps with sample campaign availability."});
    parser.addOption({"spell-research","Preview Spell Research with original spell text and sample availability."});
    parser.addOption({"spellbox","Preview Portmanteau with sample inventory and spells."});
    parser.addOption({"grimoire","Preview installed Grimoire chapters and entries."});
    parser.addOption({"character-screen","Preview Character Screen with supplied stats and upgrade costs."});
    parser.addOption({"region-entry","Preview Region Entry with supplied sample region data."});
    parser.addOption({"single-player-battle","Preview Single Player Battle setup without launching a game."});
    parser.addOption({"map-selection","Preview Map Selection with sample entries."});
    parser.addOption({"battle-results","Preview battle results: victory or defeat.","mode"});
    parser.addOption({"menu-assets","Installation root for main-menu assets.","directory"});
    parser.addOption({"menu-command-line","Show CommandLine Battle in the menu preview."});
    parser.addOption({"software-rendering","Use Mesa software rendering for this shell and its Wine child."});
    parser.addOption({"renderer","Presentation backend: opengl or native.","backend","opengl"});
    parser.addOption({"native-commands","Opt in to bounded live native command presentation; implies capture-locks."});
    parser.addOption({"capture-locks","Capture bounded game-owned locks; disables observer readback."});
    parser.addOption({"no-readback","Diagnostic: log game calls without extra surface locks or Qt frames."});
    parser.addOption({"skip-movies","Disable movies only in the disposable OpenGL installation."});
    parser.addOption({"capture-history","Record a bounded indexed/RGB surface history; implies draw capture."});
    parser.addOption({"capture-draws","Record bounded drawing evidence when the game is launched."});
    parser.addOption({"opengl-test","Test OpenGL texture presentation with known pixels."});
    parser.addOption({"commands","Replay a bounded surface-command file without launching the game.","file"});
    parser.addOption({"command-checks","Verify native/RGBA CHECK records during replay (diagnostic readback)."});
    parser.addOption({"surface-demo","Show native renderer surfaces and palette cycling without launching the game."});
    parser.addOption({"surface-test","Test persistent native surfaces through Qt presentation."});
    parser.addOption({"stream-test","Verify a bridge stream through the OpenGL viewport.","file"});
    parser.addOption({"repo","Repository directory.","directory",QDir::currentPath()});
    parser.addOption({"smoke-test","Open the shell briefly without launching the game."});
    parser.addOption({"embedding-test","Test an external fixture window; does not run the game."});
    parser.addOption({"fixture-window","Internal external-window fixture."});parser.process(app);
    if(parser.isSet("audio-catalog"))return runAudio(app,parser);
    if(parser.isSet("audio-preflight") || parser.isSet("audio-path-policy") || parser.isSet("audio-map") || parser.isSet("audio-sound") || parser.isSet("audio-report"))parser.showHelp(2);
    if(parser.isSet("media") || parser.isSet("media-server-test"))return runMedia(app,parser);
    if(parser.isSet("media-test") || parser.isSet("media-probe"))parser.showHelp(2);
    if(parser.isSet("live-menu-test")&&(!parser.isSet("live-menus")||parser.isSet("smoke-test")||QFileInfo::exists(parser.value("live-menu-test"))))parser.showHelp(2);
    const bool menuPreview=parser.isSet("main-menu") || parser.isSet("quick-battle-menu") || parser.isSet("mini-menu") || parser.isSet("battle-results") || parser.isSet("quick-battle-results") || parser.isSet("map-selection") || parser.isSet("load-game") || parser.isSet("save-game") || parser.isSet("preferences") || parser.isSet("join-multiplayer") || parser.isSet("create-multiplayer") || parser.isSet("multiplayer-game-selection") || parser.isSet("single-player-battle") || parser.isSet("multiplayer-lobby") || parser.isSet("region-entry") || parser.isSet("character-screen") || parser.isSet("grimoire") || parser.isSet("spellbox") || parser.isSet("spell-research") || parser.isSet("realm-viewer");
    if((parser.isSet("menu-assets") || parser.isSet("menu-command-line")) && !menuPreview)parser.showHelp(2);
    if((parser.isSet("menu-audio") || parser.isSet("menu-music")) && !menuPreview)parser.showHelp(2);
    if((parser.isSet("menu-audio-policy") || parser.isSet("menu-click-sound") || parser.isSet("menu-page-sound")) && !parser.isSet("menu-audio"))parser.showHelp(2);
    if(int(parser.isSet("main-menu"))+int(parser.isSet("quick-battle-menu"))+int(parser.isSet("mini-menu"))+int(parser.isSet("battle-results"))+int(parser.isSet("quick-battle-results"))+int(parser.isSet("map-selection"))+int(parser.isSet("load-game"))+int(parser.isSet("save-game"))+int(parser.isSet("preferences"))+int(parser.isSet("join-multiplayer"))+int(parser.isSet("create-multiplayer"))+int(parser.isSet("multiplayer-game-selection"))+int(parser.isSet("single-player-battle"))+int(parser.isSet("multiplayer-lobby"))+int(parser.isSet("region-entry"))+int(parser.isSet("character-screen"))+int(parser.isSet("grimoire"))+int(parser.isSet("spellbox"))+int(parser.isSet("spell-research"))+int(parser.isSet("realm-viewer"))>1)parser.showHelp(2);
    if(parser.isSet("multiplayer-lobby") && parser.value("multiplayer-lobby")!="host" && parser.value("multiplayer-lobby")!="join")parser.showHelp(2);
    if(parser.isSet("mini-menu") && parser.value("mini-menu")!="campaign" && parser.value("mini-menu")!="battle")parser.showHelp(2);
    if(parser.isSet("battle-results") && parser.value("battle-results")!="victory" && parser.value("battle-results")!="defeat")parser.showHelp(2);
    if(parser.isSet("quick-battle-results") && parser.value("quick-battle-results")!="continue" && parser.value("quick-battle-results")!="spectate")parser.showHelp(2);
    if(parser.isSet("live-menus")&&(menuPreview||parser.isSet("embedding-test")||parser.isSet("native-media")||parser.isSet("native-voices")||parser.isSet("capture-draws")||parser.isSet("capture-history")||parser.isSet("skip-movies")||parser.isSet("no-readback")||parser.isSet("capture-locks")||parser.isSet("native-commands")))parser.showHelp(2);
    if(menuPreview){
        MenuPreview preview;
        const auto root=parser.isSet("menu-assets")?parser.value("menu-assets"):QDir(parser.value("repo")).filePath("working/game-nocd");
        QString error;
        if(!preview.loadAssets(root,parser.isSet("quick-battle-menu"),&error)){
            std::fprintf(stderr,"Menu assets failed: %s\n",qPrintable(error));return 9;
        }
        std::unique_ptr<QSettings> audioSettings;
        std::unique_ptr<MenuAudioController> menuAudio;
        std::unique_ptr<MenuMusicController> menuMusic;
        if(parser.isSet("menu-audio") || parser.isSet("menu-music"))
            audioSettings=std::make_unique<QSettings>(QSettings::IniFormat,QSettings::UserScope,"MagicAndMayhemMod","QtShell");
        if(parser.isSet("menu-audio")){
            const auto policy=parser.value("menu-audio-policy");
            if(policy!="literal" && policy!="dequote-missing-leaf")parser.showHelp(2);
            bool clickOk=false,pageOk=false;
            const int click=parser.value("menu-click-sound").toInt(&clickOk),page=parser.value("menu-page-sound").toInt(&pageOk);
            if(!clickOk || !pageOk || click<=0 || page<=0)parser.showHelp(2);
            menuAudio=std::make_unique<MenuAudioController>(makeQtSessionOutput(),MenuAudioCues{click,page},nullptr,audioSettings.get());
            menuAudio->attach(preview);
            menuAudio->failed=[](const QString& message){std::fprintf(stderr,"Menu audio: %s\n",qPrintable(message));};
            if(!menuAudio->start(QDir(root).filePath("Sounds"),policy=="literal"?mnm::reconstruction::audio::NativeSourcePathPolicy::literal:mnm::reconstruction::audio::NativeSourcePathPolicy::dequoteMissingLeaf)){
                std::fprintf(stderr,"Menu audio startup failed: %s\n",qPrintable(menuAudio->lastError()));
                if(!menuAudio->canRecover())return 4;
            }
        }
        if(parser.isSet("menu-music")){
            menuMusic=std::make_unique<MenuMusicController>(makeQtMenuMusicOutput(),audioSettings.get());
            menuMusic->failed=[](const QString& message){std::fprintf(stderr,"Menu music: %s\n",qPrintable(message));};
            menuMusic->attach(preview);menuMusic->start(parser.value("menu-music"));
        }
        if(parser.isSet("mini-menu") && !preview.openMiniMenu(root,parser.value("mini-menu")=="campaign"?MiniMenuWidget::Mode::Campaign:MiniMenuWidget::Mode::Battle,&error)){
            std::fprintf(stderr,"Mini Menu assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("battle-results") && !preview.openBattleResults(root,parser.value("battle-results")=="victory"?BattleResultWidget::Outcome::Victory:BattleResultWidget::Outcome::Defeat,&error)){
            std::fprintf(stderr,"Battle result assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("quick-battle-results") && !preview.openQuickBattleResults(root,parser.value("quick-battle-results")=="spectate"?QuickBattleResultWidget::PrimaryAction::Spectate:QuickBattleResultWidget::PrimaryAction::Continue,&error)){
            std::fprintf(stderr,"Quick Battle result assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("map-selection") && !preview.openMapSelection(root,&error)){
            std::fprintf(stderr,"Map Selection assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("load-game") && !preview.openLoadGame(root,&error)){
            std::fprintf(stderr,"Load Game assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("save-game") && !preview.openSaveGame(root,&error)){
            std::fprintf(stderr,"Save Game assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("preferences") && !preview.openPreferences(root,&error)){
            std::fprintf(stderr,"Preferences assets failed: %s\n",qPrintable(error));return 9;
        }
        if((parser.isSet("join-multiplayer") || parser.isSet("create-multiplayer")) && !preview.openMultiplayer(root,parser.isSet("join-multiplayer")?MultiplayerSetupWidget::Mode::Join:MultiplayerSetupWidget::Mode::Create,&error)){
            std::fprintf(stderr,"Multiplayer assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("multiplayer-game-selection") && !preview.openMultiplayerGameSelection(root,&error)){
            std::fprintf(stderr,"Session selection assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("single-player-battle") && !preview.openSinglePlayerBattle(root,&error)){
            std::fprintf(stderr,"Single Player assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("multiplayer-lobby") && !preview.openMultiplayerLobby(root,parser.value("multiplayer-lobby")=="host"?MultiplayerLobbyWidget::Mode::Host:MultiplayerLobbyWidget::Mode::Join,&error)){
            std::fprintf(stderr,"Multiplayer lobby assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("region-entry") && !preview.openRegionEntry(root,&error)){
            std::fprintf(stderr,"Region Entry assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("character-screen") && !preview.openCharacterScreen(root,&error)){
            std::fprintf(stderr,"Character Screen assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("realm-viewer") && !preview.openRealmViewer(root,&error)){
            std::fprintf(stderr,"Realm Viewer assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("spell-research") && !preview.openSpellResearch(root,&error)){
            std::fprintf(stderr,"Spell Research assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("spellbox") && !preview.openSpellbox(root,&error)){
            std::fprintf(stderr,"Spellbox assets failed: %s\n",qPrintable(error));return 9;
        }
        if(parser.isSet("grimoire") && !preview.openGrimoire(root,&error)){
            std::fprintf(stderr,"Grimoire assets failed: %s\n",qPrintable(error));return 9;
        }
        preview.findChild<MainMenuWidget*>()->setCommandLineBattleVisible(parser.isSet("menu-command-line"));
        preview.show();
        if(parser.isSet("smoke-test"))QTimer::singleShot(100,&app,&QCoreApplication::quit);
        return app.exec();
    }
    if(parser.isSet("fixture-window")){
        QWidget fixture;fixture.setWindowTitle("MagicMayhem");fixture.resize(800,600);
        auto* layout=new QVBoxLayout(&fixture);layout->addWidget(new QLabel("External viewport fixture",&fixture));fixture.show();
        QTimer::singleShot(10000,&app,&QCoreApplication::quit);return app.exec();
    }
    if(parser.isSet("commands"))return runCommandReplay(parser.value("commands"),parser.isSet("smoke-test"),parser.isSet("command-checks"));
    if(parser.isSet("opengl-test") || parser.isSet("stream-test") || parser.isSet("surface-test") || parser.isSet("surface-demo")){
        FrameStream stream;QImage image;
        std::unique_ptr<mnm::render::GlBlitter> renderer;mnm::render::SurfaceId surface=0;
        if(parser.isSet("surface-test") || parser.isSet("surface-demo")){
            try {
                renderer=std::make_unique<mnm::render::GlBlitter>();
                const mnm::render::PixelFormat format{8,{}};
                surface=renderer->create({4,2,{3,3,3,3,3,3,3,3}},format);
                const auto sprite=renderer->create({2,1,{1,1}},format);
                renderer->update(surface,0,0,{2,1,{0,0}});renderer->update(surface,0,1,{2,1,{2,2}});
                renderer->copy(sprite,surface,{0,0,2,1},2,0);renderer->destroy(sprite);
                renderer->setPalette(surface,0,{{255,255,0},{0,255,0},{0,0,255},{255,255,255}});
                renderer->setPalette(surface,0,{{255,0,0}});image=renderer->present(surface);
            }catch(const std::exception& error){std::fprintf(stderr,"Renderer failed: %s\n",error.what());return 8;}
        }else if(parser.isSet("stream-test")){
            if(!stream.open(parser.value("stream-test")))return 4;
            image=stream.nextFrame();if(image.isNull())return 5;
        }else{
            image=QImage(4,2,QImage::Format_RGBA8888);
            for(int y=0;y<2;++y)for(int x=0;x<4;++x)
                image.setPixelColor(x,y,y?(x<2?Qt::blue:Qt::white):(x<2?Qt::red:Qt::green));
        }
        GlViewport viewport;viewport.resize(640,480);viewport.setFrame(image);viewport.show();
        QTimer paletteTimer;
        if(parser.isSet("surface-demo")){
            viewport.setWindowTitle("Magic & Mayhem — native surface palette demo");
            bool yellow=false;
            QObject::connect(&paletteTimer,&QTimer::timeout,&app,[&]{
                try {yellow=!yellow;renderer->setPalette(surface,0,{{255,std::uint8_t(yellow?255:0),0}});
                    viewport.setFrame(renderer->present(surface));}
                catch(const std::exception& error){std::fprintf(stderr,"Renderer failed: %s\n",error.what());app.exit(8);}
            });paletteTimer.start(500);return app.exec();
        }
        QTimer::singleShot(500,&app,[&]{
            if(!viewport.ready()){std::fprintf(stderr,"OpenGL failed: %s\n",qPrintable(viewport.error()));app.exit(6);return;}
            auto actual=viewport.grabFramebuffer();
            bool ok=true;
            const double scale=qMin(double(actual.width())/image.width(),double(actual.height())/image.height());
            const int width=qRound(image.width()*scale),height=qRound(image.height()*scale);
            const int left=(actual.width()-width)/2,top=(actual.height()-height+1)/2;
            const int columns=qMin(image.width(),32),rows=qMin(image.height(),32);
            for(int y=0;y<rows;++y)for(int x=0;x<columns;++x){
                const int sx=int((x+0.5)*image.width()/columns),sy=int((y+0.5)*image.height()/rows);
                const int dx=left+int((sx+0.5)*width/image.width()),dy=top+int((sy+0.5)*height/image.height());
                const int expectedX=qMin(image.width()-1,int((dx-left+0.5)*image.width()/width));
                const int expectedY=qMin(image.height()-1,int((dy-top+0.5)*image.height()/height));
                if(actual.pixelColor(dx,dy)!=image.pixelColor(expectedX,expectedY))ok=false;
            }
            if(top>1 && actual.pixelColor(actual.width()/2,top/2)!=QColor(Qt::black))ok=false;
            if(left>1 && actual.pixelColor(left/2,actual.height()/2)!=QColor(Qt::black))ok=false;
            app.exit(ok?0:7);
        });return app.exec();
    }
    const auto renderer=parser.isSet("live-menus")?QString("native"):parser.value("renderer");
    if(renderer!="opengl" && renderer!="native")parser.showHelp(2);
    if((parser.isSet("capture-draws") || parser.isSet("capture-history") || parser.isSet("skip-movies") || parser.isSet("no-readback") || parser.isSet("capture-locks") || parser.isSet("native-commands") || parser.isSet("native-media") || parser.isSet("native-voices")) && renderer!="opengl")parser.showHelp(2);
    Shell shell(QDir(parser.value("repo")).absolutePath(),renderer=="opengl" && !parser.isSet("embedding-test"),parser.isSet("capture-draws") || parser.isSet("capture-history"),parser.isSet("capture-history"),parser.isSet("skip-movies"),parser.isSet("no-readback"),parser.isSet("capture-locks")||parser.isSet("native-commands"),parser.isSet("native-media"),parser.isSet("native-voices"),parser.isSet("live-menus"),parser.isSet("native-commands"));shell.show();
    if(parser.isSet("live-menu-test"))installLiveMenuTest(app,shell,*shell.liveMenuSession(),parser.value("live-menu-test"));
    if(parser.isSet("smoke-test"))QTimer::singleShot(100,&app,&QCoreApplication::quit);
    if(parser.isSet("embedding-test")){
        if(!shell.host().available())return 2;
        QProcess fixture;fixture.start(QCoreApplication::applicationFilePath(),{"--fixture-window"});
        QTimer timer;QElapsedTimer elapsed;elapsed.start();
        QObject::connect(&timer,&QTimer::timeout,&app,[&]{
            const auto windows=shell.host().desktops({});
            if(windows.size()==1){
                const auto window=windows.front();
                const bool attached=shell.attach(window);
                app.processEvents();
                const bool embedded=shell.host().descendantOf(window,xcb_window_t(shell.winId()));
                shell.detach();app.processEvents();
                const bool detached=shell.host().exists(window) && !shell.host().descendantOf(window,xcb_window_t(shell.winId()));
                fixture.terminate();fixture.waitForFinished(1000);
                app.exit(attached && embedded && detached?0:1);
            }else if(elapsed.elapsed()>8000){fixture.kill();fixture.waitForFinished(1000);app.exit(3);}
        });
        timer.start(100);return app.exec();
    }
    return app.exec();
}
