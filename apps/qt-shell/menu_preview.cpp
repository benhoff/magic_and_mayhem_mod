#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QMetaEnum>
#include <QCloseEvent>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QStatusBar>

namespace {
const QVector<MapSelectionWidget::Map>& previewMaps() {
    static const QVector<MapSelectionWidget::Map> maps{{"sample-forest","Sample forest map"},{"sample-plains","Sample plains map"},{"sample-islands","Sample islands map"}};
    return maps;
}
}

MenuPreview::MenuPreview(QWidget* parent) : QMainWindow(parent) {
    screens_ = new QStackedWidget(this);
    main_ = new MainMenuWidget(screens_);
    quick_ = new QuickBattleMenuWidget(screens_);
    screens_->addWidget(main_); screens_->addWidget(quick_);
    setCentralWidget(screens_); resize(800, 630);
    connect(main_, &MainMenuWidget::actionRequested, this, [this](MainMenuWidget::Action action) {
        if (action == MainMenuWidget::Action::QuickBattle) { showQuickBattle(); return; }
        if (action == MainMenuWidget::Action::LoadGame) {
            QString error;
            if (!openLoadGame(assetRoot_,&error)) statusBar()->showMessage(QString("Load Game preview failed: %1").arg(error));
            return;
        }
        if (action == MainMenuWidget::Action::Preferences) {
            QString error;
            if (!openPreferences(assetRoot_,&error)) statusBar()->showMessage(QString("Preferences preview failed: %1").arg(error));
            return;
        }
        if (action == MainMenuWidget::Action::Quit) { close(); return; }
        const auto name = QMetaEnum::fromType<MainMenuWidget::Action>().valueToKey(int(action));
        statusBar()->showMessage(QString("Selected %1 — engine adapter pending.").arg(QString::fromLatin1(name)));
    });
    connect(quick_, &QuickBattleMenuWidget::actionRequested, this, [this](QuickBattleMenuWidget::Action action) {
        if (action == QuickBattleMenuWidget::Action::Cancel) { showMainMenu(); return; }
        if (action==QuickBattleMenuWidget::Action::JoinMultiplayer || action==QuickBattleMenuWidget::Action::CreateMultiplayer) {
            QString error;
            const auto mode=action==QuickBattleMenuWidget::Action::JoinMultiplayer?MultiplayerSetupWidget::Mode::Join:MultiplayerSetupWidget::Mode::Create;
            if (!openMultiplayer(assetRoot_,mode,&error)) statusBar()->showMessage(QString("Multiplayer preview failed: %1").arg(error));
            return;
        }
        if (action==QuickBattleMenuWidget::Action::CreateSinglePlayer) {
            QString error;
            if (!openSinglePlayerBattle(assetRoot_,&error)) statusBar()->showMessage(QString("Single Player preview failed: %1").arg(error));
            return;
        }
        const auto name = QMetaEnum::fromType<QuickBattleMenuWidget::Action>().valueToKey(int(action));
        statusBar()->showMessage(QString("Selected %1 — engine adapter pending.").arg(QString::fromLatin1(name)));
    });
}
bool MenuPreview::loadAssets(const QString& root, bool startQuickBattle, QString* error) {
    if (!main_->loadAssets(root, error) || !quick_->loadAssets(root, error)) return false;
    assetRoot_=root;
    if (startQuickBattle) showQuickBattle(); else showMainMenu();
    return true;
}
bool MenuPreview::openMiniMenu(const QString& root, MiniMenuWidget::Mode mode, QString* error) {
    if (!mini_) {
        mini_ = new MiniMenuWidget(screens_); screens_->addWidget(mini_);
        connect(mini_, &MiniMenuWidget::actionRequested, this, [this](MiniMenuWidget::Action action) {
            if (action == MiniMenuWidget::Action::SaveGame) {
                QString error;
                if (!openSaveGame(assetRoot_,&error)) statusBar()->showMessage(QString("Save Game preview failed: %1").arg(error));
                return;
            }
            if (action == MiniMenuWidget::Action::Preferences) {
                QString error;
                if (!openPreferences(assetRoot_,&error)) statusBar()->showMessage(QString("Preferences preview failed: %1").arg(error));
                return;
            }
            if (action == MiniMenuWidget::Action::Cancel) { showMainMenu(); return; }
            const auto name = QMetaEnum::fromType<MiniMenuWidget::Action>().valueToKey(int(action));
            statusBar()->showMessage(QString("Selected %1 — engine adapter pending.").arg(QString::fromLatin1(name)));
        });
    }
    if (!mini_->loadAssets(root, error)) return false;
    mini_->setMode(mode); activateScreen(mini_); mini_->focusFirstAction();
    setWindowTitle(mini_->windowTitle());
    statusBar()->showMessage("Mini Menu preview. Cancel or Escape returns to the main menu.");
    return true;
}
void MenuPreview::showMainMenu() {
    activateScreen(main_);
    setWindowTitle(main_->windowTitle());
    main_->findChild<QPushButton*>("mainMenuAction2")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Menu preview. Game actions are not connected.");
}
void MenuPreview::showQuickBattle() {
    activateScreen(quick_);
    setWindowTitle(quick_->windowTitle());
    quick_->focusFirstAction();
    statusBar()->showMessage("Quick Battle preview. Cancel or Escape returns to the main menu.");
}

bool MenuPreview::openBattleResults(const QString& root, BattleResultWidget::Outcome outcome, QString* error) {
    if (!results_) {
        results_=new BattleResultWidget(screens_); screens_->addWidget(results_);
        connect(results_,&BattleResultWidget::continueRequested,this,&MenuPreview::showMainMenu);
    }
    if (!results_->loadAssets(root,outcome,error)) return false;
    BattleResultWidget::Results sample;
    sample.title=outcome==BattleResultWidget::Outcome::Victory?"Victory!":"Defeat!";
    if (outcome==BattleResultWidget::Outcome::Victory) {
        sample.rewards[0]={"Sample achievement", "120"};
        sample.rewards[1]={"Sample bonus", "80"};
        sample.totalPoints="200"; sample.maximumPoints="300";
    } else {
        sample.summary="Sample defeat message";
        sample.rewards[0]={"Sample achievement", "40"};
        sample.rating="Sample rating";
    }
    results_->setResults(sample); activateScreen(results_); results_->focusContinue();
    setWindowTitle(results_->windowTitle());
    statusBar()->showMessage("Sample results. OK returns to the main menu preview.");
    return true;
}

bool MenuPreview::openQuickBattleResults(const QString& root, QuickBattleResultWidget::PrimaryAction action, QString* error) {
    if (!quickResults_) {
        quickResults_=new QuickBattleResultWidget(screens_); screens_->addWidget(quickResults_);
        connect(quickResults_,&QuickBattleResultWidget::actionRequested,this,[this](QuickBattleResultWidget::Action selected) {
            if (selected==QuickBattleResultWidget::Action::Quit) { showMainMenu(); return; }
            if (selected==QuickBattleResultWidget::Action::Continue) { showQuickBattle(); return; }
            statusBar()->showMessage("Spectate selected — engine adapter pending.");
        });
    }
    if (!quickResults_->loadAssets(root,error)) return false;
    QuickBattleResultWidget::Results sample;
    sample.primaryAction=action;
    for (int i=0;i<4;++i) {
        auto& player=sample.players[i]; player.active=true;
        player.name=QString("Sample player %1").arg(i+1); player.portraitText=QString("P%1").arg(i+1);
        player.kills=QString::number(12-i*3); player.deaths=QString::number(i+1);
        player.handicapBonus=QString::number(i*5); player.score=QString::number(120-i*20);
    }
    quickResults_->setResults(sample); activateScreen(quickResults_); quickResults_->focusFirstAction();
    setWindowTitle(quickResults_->windowTitle());
    statusBar()->showMessage("Sample results. Continue opens Quick Battle; Quit returns to the main menu preview.");
    return true;
}

bool MenuPreview::openMapSelection(const QString& root, QString* error) {
    if (!mapSelection_) {
        mapSelection_=new MapSelectionWidget(screens_); screens_->addWidget(mapSelection_);
        connect(mapSelection_,&MapSelectionWidget::cancelled,this,&MenuPreview::returnFromMapSelection);
        connect(mapSelection_,&MapSelectionWidget::mapSelected,this,[this](const QString& id) {
            if (mapReturnsToSinglePlayer_) {
                auto setup=singlePlayer_->setup();setup.mapId=id;
                for (const auto& map:previewMaps()) if (map.id==id) setup.mapName=map.name;
                singlePlayer_->setSetup(setup);
            }
            if (mapReturnsToLobby_) {
                auto lobby=mapReturnsToLobby_->lobby();lobby.mapId=id;
                for (const auto& map:previewMaps()) if (map.id==id) lobby.mapName=map.name;
                mapReturnsToLobby_->setLobby(lobby);
            }
            returnFromMapSelection();
            statusBar()->showMessage(QString("Selected sample map %1 — engine adapter pending.").arg(id));
        });
    }
    const bool fromSingle=singlePlayer_ && screens_->currentWidget()==singlePlayer_;
    auto* lobby=qobject_cast<MultiplayerLobbyWidget*>(screens_->currentWidget());
    if (lobby && lobby->mode()!=MultiplayerLobbyWidget::Mode::Host) lobby=nullptr;
    if (!mapSelection_->loadAssets(root,error)) return false;
    const auto selected=fromSingle?singlePlayer_->setup().mapId:lobby?lobby->lobby().mapId:"sample-forest";
    if (!mapSelection_->setMaps(previewMaps(),selected,error)) return false;
    mapReturnsToSinglePlayer_=fromSingle;mapReturnsToLobby_=lobby;
    activateScreen(mapSelection_); mapSelection_->focusSelection();
    setWindowTitle(mapSelection_->windowTitle());
    statusBar()->showMessage(lobby?"Sample maps. OK or Cancel returns to the host lobby.":fromSingle?"Sample maps. OK or Cancel returns to Single Player setup.":"Sample map list. OK or Cancel returns to the Quick Battle preview.");
    return true;
}

bool MenuPreview::openLoadGame(const QString& root, QString* error) {
    if (!loadGame_) {
        loadGame_=new LoadGameWidget(screens_); screens_->addWidget(loadGame_);
        connect(loadGame_,&LoadGameWidget::cancelled,this,&MenuPreview::showMainMenu);
        connect(loadGame_,&LoadGameWidget::loadRequested,this,[this](const QString& id) {
            statusBar()->showMessage(QString("Selected sample save %1 — engine adapter pending.").arg(id));
        });
    }
    if (!loadGame_->loadAssets(root,error)) return false;
    if (!loadGame_->setSaves({{"sample-autosave","Sample autosave"},{"sample-campaign","Sample campaign save"},{"sample-before-battle","Sample before battle"}},"sample-autosave",error)) return false;
    activateScreen(loadGame_); loadGame_->focusSelection(); setWindowTitle(loadGame_->windowTitle());
    statusBar()->showMessage("Sample saves. Load emits selection; Cancel returns to the main menu preview."); return true;
}

bool MenuPreview::openSaveGame(const QString& root, QString* error) {
    if (!saveGame_) {
        saveGame_=new SaveGameWidget(screens_); screens_->addWidget(saveGame_);
        connect(saveGame_,&SaveGameWidget::cancelled,this,&MenuPreview::returnFromSaveGame);
        connect(saveGame_,&SaveGameWidget::saveRequested,this,[this](const QString& name,const QString& existingId) {
            statusBar()->showMessage(QString("Requested sample %1: %2 — engine adapter pending.").arg(existingId.isEmpty()?"save":"overwrite",name));
        });
        connect(saveGame_,&SaveGameWidget::deleteRequested,this,[this](const QString& id) {
            statusBar()->showMessage(QString("Requested sample deletion: %1 — engine adapter pending.").arg(id));
        });
    }
    if (!saveGame_->loadAssets(root,error)) return false;
    if (!saveGame_->setSaves({{"sample-autosave","Sample autosave"},{"sample-campaign","Sample campaign save"},{"sample-before-battle","Sample before battle"}},"sample-campaign",error)) return false;
    saveReturnsToMini_=mini_ && screens_->currentWidget()==mini_;
    activateScreen(saveGame_); saveGame_->focusSelection(); setWindowTitle(saveGame_->windowTitle());
    statusBar()->showMessage("Sample saves. Save/Delete emit intent; Cancel returns to the previous preview."); return true;
}
void MenuPreview::returnFromSaveGame() {
    if (!saveReturnsToMini_) { showMainMenu(); return; }
    activateScreen(mini_); mini_->focusFirstAction(); setWindowTitle(mini_->windowTitle());
    statusBar()->showMessage("Mini Menu preview. Cancel or Escape returns to the main menu.");
}

bool MenuPreview::openPreferences(const QString& root, QString* error) {
    if (!preferences_) {
        preferences_=new PreferencesWidget(screens_); screens_->addWidget(preferences_);
        connect(preferences_,&PreferencesWidget::cancelled,this,&MenuPreview::returnFromPreferences);
        connect(preferences_,&PreferencesWidget::settingsApplied,this,[this](const PreferencesWidget::Settings&) {
            returnFromPreferences();
            statusBar()->showMessage("Preferences accepted locally — engine adapter and persistence pending.");
        });
    }
    if (!preferences_->loadAssets(root,error) || !preferences_->setSettings(preferences_->settings(),error)) return false;
    preferencesReturnToMini_=mini_ && screens_->currentWidget()==mini_;
    activateScreen(preferences_); preferences_->focusFirstControl(); setWindowTitle(preferences_->windowTitle());
    statusBar()->showMessage("Local sample settings. OK accepts changes; Cancel restores the snapshot."); return true;
}
void MenuPreview::returnFromPreferences() {
    if (!preferencesReturnToMini_) {
        showMainMenu(); main_->findChild<QPushButton*>("mainMenuAction3")->setFocus(Qt::OtherFocusReason); return;
    }
    activateScreen(mini_); setWindowTitle(mini_->windowTitle());
    mini_->findChild<QPushButton*>(mini_->mode()==MiniMenuWidget::Mode::Campaign?"miniMenuButton3":"miniMenuButton6")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Mini Menu preview. Cancel or Escape returns to the main menu.");
}

bool MenuPreview::openMultiplayer(const QString& root, MultiplayerSetupWidget::Mode mode, QString* error) {
    auto*& screen=mode==MultiplayerSetupWidget::Mode::Join?joinMultiplayer_:createMultiplayer_;
    if (!screen) {
        screen=new MultiplayerSetupWidget(mode,screens_); screens_->addWidget(screen);
        MultiplayerSetupWidget::Form sample; sample.userName="Sample player";
        if (mode==MultiplayerSetupWidget::Mode::Create) sample.gameName="Sample game";
        screen->setForm(sample);
        connect(screen,&MultiplayerSetupWidget::cancelled,this,[this,mode] {
            showQuickBattle();
            quick_->findChild<QPushButton*>(mode==MultiplayerSetupWidget::Mode::Join?"quickBattleAction1":"quickBattleAction0")->setFocus(Qt::OtherFocusReason);
        });
        connect(screen,&MultiplayerSetupWidget::requestSubmitted,this,[this](const MultiplayerSetupWidget::Request& request) {
            if (request.mode==MultiplayerSetupWidget::Mode::Join) {
                QString error;
                if (openMultiplayerGameSelection(assetRoot_,&error)) browsingRequest_=request;
                else statusBar()->showMessage(QString("Session selection preview failed: %1").arg(error));
                return;
            }
            QString error;
            if (!openMultiplayerLobby(assetRoot_,MultiplayerLobbyWidget::Mode::Host,&error))
                statusBar()->showMessage(QString("Host lobby preview failed: %1").arg(error));
        });
    }
    if (!screen->loadAssets(root,error)) return false;
    activateScreen(screen); screen->focusFirstField(); setWindowTitle(screen->windowTitle());
    statusBar()->showMessage("Sample multiplayer form. OK emits intent; Cancel returns to Quick Battle."); return true;
}

bool MenuPreview::openMultiplayerGameSelection(const QString& root, QString* error) {
    // A standalone invocation also has a Join form to return to on Cancel.
    if (!joinMultiplayer_ && !openMultiplayer(root,MultiplayerSetupWidget::Mode::Join,error)) return false;
    if (!multiplayerSelection_) {
        multiplayerSelection_=new MultiplayerGameSelectionWidget(screens_);
        screens_->addWidget(multiplayerSelection_);
        multiplayerSelection_->setSessions({{"sample-forest","Sample forest battle"},{"sample-island","Sample island battle"}});
        connect(multiplayerSelection_,&MultiplayerGameSelectionWidget::cancelled,this,[this] {
            activateScreen(joinMultiplayer_); setWindowTitle(joinMultiplayer_->windowTitle());
            joinMultiplayer_->findChild<QPushButton*>("multiplayerOk")->setFocus(Qt::OtherFocusReason);
            statusBar()->showMessage("Join preview. Local input retained; networking adapter pending.");
        });
        connect(multiplayerSelection_,&MultiplayerGameSelectionWidget::sessionSelected,this,[this](const QString& id) {
            Q_UNUSED(id);
            QString error;
            if (!openMultiplayerLobby(assetRoot_,MultiplayerLobbyWidget::Mode::Join,&error))
                statusBar()->showMessage(QString("Guest lobby preview failed: %1").arg(error));
        });
    }
    if (!multiplayerSelection_->loadAssets(root,error)) return false;
    browsingRequest_.mode=MultiplayerSetupWidget::Mode::Join;
    browsingRequest_.userName=joinMultiplayer_->form().userName.trimmed();
    browsingRequest_.transport=joinMultiplayer_->form().transport;
    browsingRequest_.gameName.clear();
    activateScreen(multiplayerSelection_); setWindowTitle(multiplayerSelection_->windowTitle());
    multiplayerSelection_->focusSelection();
    statusBar()->showMessage("Sample sessions; no discovery. Select a game and OK; Cancel returns to Join.");
    return true;
}

void MenuPreview::returnFromMapSelection() {
    if (mapReturnsToLobby_) {
        auto* lobby=mapReturnsToLobby_;mapReturnsToLobby_=nullptr;
        activateScreen(lobby);setWindowTitle(lobby->windowTitle());
        lobby->findChild<QPushButton*>("multiplayerLobbyMap")->setFocus(Qt::OtherFocusReason);
        statusBar()->showMessage("Sample host lobby. Networking and Start remain pending.");return;
    }
    if (!mapReturnsToSinglePlayer_) {showQuickBattle();return;}
    mapReturnsToSinglePlayer_=false;activateScreen(singlePlayer_);setWindowTitle(singlePlayer_->windowTitle());
    singlePlayer_->findChild<QPushButton*>("singlePlayerMap")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Single Player sample setup. Start emits intent; engine adapter pending.");
}
bool MenuPreview::openSinglePlayerBattle(const QString& root,QString* error) {
    if (!singlePlayer_) {
        singlePlayer_=new SinglePlayerBattleWidget(screens_);screens_->addWidget(singlePlayer_);
        SinglePlayerBattleWidget::Setup sample;sample.mapId=previewMaps()[0].id;sample.mapName=previewMaps()[0].name;
        const std::array<QString,4> colours{"Red","Blue","Green","Gold"};
        for (int i=0;i<4;++i) {
            auto& player=sample.players[i];player.active=true;player.name=i?QString("Sample AI %1").arg(i):"Sample player";
            player.portraitId="sample-wizard-1";player.portraitText="W1";player.portraitIndex=0;
            player.colourId=QString("sample-colour-%1").arg(i);player.colourText=colours[i];player.colourIndex=std::array<int,4>{1,2,0,6}[i];
        }
        singlePlayer_->setSetup(sample);
        connect(singlePlayer_,&SinglePlayerBattleWidget::cancelled,this,[this] {
            showQuickBattle();quick_->findChild<QPushButton*>("quickBattleAction2")->setFocus(Qt::OtherFocusReason);
        });
        connect(singlePlayer_,&SinglePlayerBattleWidget::mapRequested,this,[this] {
            QString error;if (!openMapSelection(assetRoot_,&error)) statusBar()->showMessage(QString("Map preview failed: %1").arg(error));
        });
        connect(singlePlayer_,&SinglePlayerBattleWidget::startRequested,this,[this](const auto& setup) {
            statusBar()->showMessage(QString("Start Single Player on %1 — engine command adapter pending.").arg(setup.mapId));
        });
        connect(singlePlayer_,&SinglePlayerBattleWidget::playerChangeRequested,this,[this](int slot) {
            auto setup=singlePlayer_->setup();auto& player=setup.players[slot];
            const int next=player.active?(player.portraitText=="W1"?2:player.portraitText=="W2"?3:1):1;
            player.active=true;player.portraitId=QString("sample-wizard-%1").arg(next);player.portraitText=QString("W%1").arg(next);player.portraitIndex=next-1;
            if (slot) player.name=QString("Sample AI %1 (W%2)").arg(slot).arg(next);
            singlePlayer_->setSetup(setup);
        });
        connect(singlePlayer_,&SinglePlayerBattleWidget::colourChangeRequested,this,[this](int slot) {
            auto setup=singlePlayer_->setup();auto& player=setup.players[slot];
            const std::array<QString,4> colours{"Red","Blue","Green","Gold"};
            int current=0;for (int i=0;i<4;++i) if (player.colourText==colours[i]) current=i;
            const int next=(current+1)%4;player.colourId=QString("sample-colour-%1").arg(next);player.colourText=colours[next];player.colourIndex=std::array<int,4>{1,2,0,6}[next];
            singlePlayer_->setSetup(setup);
        });
        connect(singlePlayer_,&SinglePlayerBattleWidget::playerRemovalRequested,this,[this](int slot) {
            auto setup=singlePlayer_->setup();setup.players[slot].active=false;singlePlayer_->setSetup(setup);
        });
    }
    if (!singlePlayer_->loadAssets(root,error)) return false;
    activateScreen(singlePlayer_);singlePlayer_->focusFirstControl();setWindowTitle(singlePlayer_->windowTitle());
    statusBar()->showMessage("Sample players and settings; portrait/colour buttons cycle samples. Start remains pending.");return true;
}

bool MenuPreview::openMultiplayerLobby(const QString& root,MultiplayerLobbyWidget::Mode mode,QString* error) {
    const bool host=mode==MultiplayerLobbyWidget::Mode::Host;
    // Standalone previews need a concrete caller for Cancel.
    if (host && !createMultiplayer_ && !openMultiplayer(root,MultiplayerSetupWidget::Mode::Create,error)) return false;
    if (!host && !multiplayerSelection_ && !openMultiplayerGameSelection(root,error)) return false;
    MultiplayerSetupWidget::Request request=browsingRequest_;
    QString sessionId,gameName;
    if (host) {
        const auto form=createMultiplayer_->form();request.mode=MultiplayerSetupWidget::Mode::Create;
        request.transport=form.transport;request.userName=form.userName.trimmed();request.gameName=form.gameName.trimmed();
        sessionId="sample-host";gameName=request.gameName;
    } else {
        sessionId=multiplayerSelection_->selectedSessionId();
        if (sessionId.isEmpty()) sessionId="sample-forest";
        const auto selected=multiplayerSelection_->findChild<QListWidget*>("multiplayerGameSelectionList")->selectedItems();
        gameName=selected.isEmpty()?"Sample forest battle":selected.front()->text();
    }
    auto*& screen=host?hostLobby_:joinLobby_;auto& context=host?hostLobbyContext_:joinLobbyContext_;
    const auto key=sessionId+QChar(0)+gameName+QChar(0)+request.userName+QChar(0)+QString::number(int(request.transport));
    if (!screen) {
        screen=new MultiplayerLobbyWidget(mode,screens_);screens_->addWidget(screen);
        connect(screen,&MultiplayerLobbyWidget::cancelled,this,[this,host] {
            auto* caller=host?static_cast<QWidget*>(createMultiplayer_):static_cast<QWidget*>(multiplayerSelection_);
            activateScreen(caller);setWindowTitle(caller->windowTitle());
            caller->findChild<QPushButton*>(host?"multiplayerOk":"multiplayerGameSelectionOk")->setFocus(Qt::OtherFocusReason);
            statusBar()->showMessage("Returned from sample lobby; no network connection was made.");
        });
        connect(screen,&MultiplayerLobbyWidget::mapRequested,this,[this] {
            QString error;if (!openMapSelection(assetRoot_,&error)) statusBar()->showMessage(QString("Map preview failed: %1").arg(error));
        });
        connect(screen,&MultiplayerLobbyWidget::startRequested,this,[this](const auto& lobby) {
            statusBar()->showMessage(QString("Start host session %1 on %2 — engine networking adapter pending.").arg(lobby.sessionId,lobby.mapId));
        });
        connect(screen,&MultiplayerLobbyWidget::readyRequested,this,[this](bool ready) {
            statusBar()->showMessage(ready?"Ready accepted locally — networking adapter pending.":"Ready cleared locally — networking adapter pending.");
        });
        connect(screen,&MultiplayerLobbyWidget::chatRequested,this,[this,screen](const QString& text) {
            const auto lobby=screen->lobby();screen->appendMessage({lobby.players[lobby.localSlot].name,text});
            statusBar()->showMessage("Local chat echo only — networking adapter pending.");
        });
        connect(screen,&MultiplayerLobbyWidget::playerChangeRequested,this,[screen](int slot) {
            auto lobby=screen->lobby();auto& player=lobby.players[slot];
            const int next=player.portraitText=="W1"?2:player.portraitText=="W2"?3:1;
            player.portraitId=QString("sample-wizard-%1").arg(next);player.portraitText=QString("W%1").arg(next);player.portraitIndex=next-1;screen->setLobby(lobby);
        });
        connect(screen,&MultiplayerLobbyWidget::colourChangeRequested,this,[screen](int slot) {
            auto lobby=screen->lobby();auto& player=lobby.players[slot];const std::array<QString,4> colours{"Red","Blue","Green","Gold"};
            int current=0;for (int i=0;i<4;++i) if (player.colourText==colours[i]) current=i;
            const int next=(current+1)%4;player.colourId=QString("sample-colour-%1").arg(next);player.colourText=colours[next];player.colourIndex=std::array<int,4>{1,2,0,6}[next];screen->setLobby(lobby);
        });
        connect(screen,&MultiplayerLobbyWidget::playerRemovalRequested,this,[screen](int slot) {
            auto lobby=screen->lobby();lobby.players[slot].active=false;screen->setLobby(lobby);
        });
    }
    if (!screen->loadAssets(root,error)) return false;
    if (context!=key) {
        MultiplayerLobbyWidget::Lobby sample;sample.sessionId=sessionId;sample.gameName=gameName;sample.localSlot=host?0:1;
        sample.mapId=previewMaps()[0].id;sample.mapName=previewMaps()[0].name;
        const std::array<QString,4> colours{"Red","Blue","Green","Gold"};
        for (int i=0;i<3;++i) {
            auto& player=sample.players[i];player.active=true;player.name=i==sample.localSlot?request.userName:i==0?"Sample host":QString("Sample guest %1").arg(i);
            player.portraitId="sample-wizard-1";player.portraitText="W1";player.portraitIndex=0;player.colourId=QString("sample-colour-%1").arg(i);player.colourText=colours[i];player.colourIndex=std::array<int,4>{1,2,0,6}[i];
        }
        if (!screen->setLobby(sample,error)) return false;
        screen->clearChat();screen->appendMessage({"Preview","Sample lobby — local chat only."});context=key;
    }
    (host?hostLobbyRequest_:joinLobbyRequest_)=request;
    activateScreen(screen);screen->focusFirstControl();setWindowTitle(host?"Magic & Mayhem — Host lobby preview":"Magic & Mayhem — Guest lobby preview");
    statusBar()->showMessage("Sample lobby; no network connection. Chat echoes locally; Start/Ready remain pending.");return true;
}

bool MenuPreview::openRegionEntry(const QString& root,QString* error) {
    if (!regionEntry_) {
        regionEntry_=new RegionEntryWidget(screens_);screens_->addWidget(regionEntry_);
        RegionEntryWidget::Region sample;sample.id="sample-celtic-region-1";sample.name="Sample Celtic region 1";
        regionEntry_->setRegion(sample);
        connect(regionEntry_,&RegionEntryWidget::cancelled,this,[this] {
            showMainMenu();main_->findChild<QPushButton*>("mainMenuAction0")->setFocus(Qt::OtherFocusReason);
        });
        connect(regionEntry_,&RegionEntryWidget::enterRequested,this,[this](const auto& request) {
            const auto name=QMetaEnum::fromType<RegionEntryWidget::Difficulty>().valueToKey(int(request.difficulty));
            statusBar()->showMessage(QString("Enter %1 at %2 — campaign engine adapter pending.").arg(request.regionId,QString::fromLatin1(name)));
        });
        connect(regionEntry_,&RegionEntryWidget::auxiliaryRequested,this,[this](auto action) {
            if (action==RegionEntryWidget::AuxiliaryAction::Grimoire) {
                QString error;
                if (!openGrimoire(assetRoot_,&error)) statusBar()->showMessage(QString("Grimoire preview failed: %1").arg(error));
                return;
            }
            if (action==RegionEntryWidget::AuxiliaryAction::Spellbox) {
                QString error;
                if (!openSpellbox(assetRoot_,&error)) statusBar()->showMessage(QString("Spellbox preview failed: %1").arg(error));
                return;
            }
            if (action==RegionEntryWidget::AuxiliaryAction::Character) {
                QString error;
                if (!openCharacterScreen(assetRoot_,&error)) statusBar()->showMessage(QString("Character preview failed: %1").arg(error));
                return;
            }
            const auto name=QMetaEnum::fromType<RegionEntryWidget::AuxiliaryAction>().valueToKey(int(action));
            statusBar()->showMessage(QString("%1 selected — campaign engine adapter pending.").arg(QString::fromLatin1(name)));
        });
    }
    if (!regionEntry_->loadAssets(root,error)) return false;
    activateScreen(regionEntry_);regionEntry_->focusFirstControl();setWindowTitle(regionEntry_->windowTitle());
    statusBar()->showMessage("Sample region; Enter and icon actions emit intent. Cancel returns to Main Menu.");return true;
}

bool MenuPreview::openCharacterScreen(const QString& root,QString* error) {
    if (!characterScreen_) {
        characterScreen_=new CharacterScreenWidget(screens_);screens_->addWidget(characterScreen_);
        CharacterScreenWidget::Character sample;sample.id="sample-character";sample.name="Sample character";
        sample.portraitText="Sample wizard portrait";sample.portraitIndex=0;sample.rating="Sample apprentice";sample.experiencePoints=100;
        const std::array<int,6> values{50,100,10,1,1,1};
        for (int i=0;i<6;++i) {sample.stats[i].value=values[i];sample.stats[i].upgradeCosts={5,10,15,20};}
        characterScreen_->setCharacter(sample);
        connect(characterScreen_,&CharacterScreenWidget::cancelled,this,&MenuPreview::returnFromCharacterScreen);
        connect(characterScreen_,&CharacterScreenWidget::characterAccepted,this,[this](const auto& request) {
            returnFromCharacterScreen();
            statusBar()->showMessage(QString("Character %1 accepted locally (%2 experience left) — progression adapter pending.").arg(request.characterId).arg(request.remainingExperience));
        });
    }
    if (!characterScreen_->loadAssets(root,error)) return false;
    characterReturnsToRegion_=regionEntry_ && screens_->currentWidget()==regionEntry_;
    activateScreen(characterScreen_);characterScreen_->focusFirstControl();setWindowTitle(characterScreen_->windowTitle());
    statusBar()->showMessage("Sample upgrade costs. + purchases; − undoes draft purchases. OK accepts locally; Cancel restores.");return true;
}
void MenuPreview::returnFromCharacterScreen() {
    if (!characterReturnsToRegion_) {
        showMainMenu();main_->findChild<QPushButton*>("mainMenuAction0")->setFocus(Qt::OtherFocusReason);return;
    }
    activateScreen(regionEntry_);setWindowTitle(regionEntry_->windowTitle());
    regionEntry_->findChild<QPushButton*>("regionEntryAuxiliary2")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Region Entry preview. Character edits are local; campaign engine adapter pending.");
}

bool MenuPreview::openGrimoire(const QString& root,QString* error) {
    if (!grimoire_) {
        grimoire_=new GrimoireWidget(screens_);screens_->addWidget(grimoire_);
        connect(grimoire_,&GrimoireWidget::closed,this,&MenuPreview::returnFromGrimoire);
        connect(grimoire_,&GrimoireWidget::navigationFailed,this,[this](const QString& error){statusBar()->showMessage(QString("Grimoire page failed: %1").arg(error));});
    }
    if (!grimoire_->loadAssets(root,error)) return false;
    grimoireReturnsToRegion_=regionEntry_ && screens_->currentWidget()==regionEntry_;
    activateScreen(grimoire_);setWindowTitle(grimoire_->windowTitle());grimoire_->focusFirstControl();
    statusBar()->showMessage("Offline Grimoire: chapter contents and installed entries. Campaign knowledge and dynamic stats remain pending.");return true;
}
void MenuPreview::returnFromGrimoire() {
    if (!grimoireReturnsToRegion_) {showMainMenu();main_->findChild<QPushButton*>("mainMenuAction0")->setFocus(Qt::OtherFocusReason);return;}
    activateScreen(regionEntry_);setWindowTitle(regionEntry_->windowTitle());regionEntry_->findChild<QPushButton*>("regionEntryAuxiliary0")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Region Entry preview. Campaign engine adapter pending.");
}


bool MenuPreview::openSpellbox(const QString& root,QString* error) {
    if (!spellbox_) {
        spellbox_=new SpellboxWidget(screens_);screens_->addWidget(spellbox_);
        SpellboxWidget::Inventory sample;sample.ownerId="sample-character";
        for(int i=0;i<6;++i){SpellboxWidget::Item item;item.id=QString("sample-item-%1").arg(i);item.name=QString("Sample item %1").arg(i+1);item.artworkIndex=i;item.quantity=2;
            for(int a=0;a<3;++a)item.spells[a]={QString("sample-spell-%1-%2").arg(i).arg(a),QString("Sample %1 spell %2").arg(a==0?"law":a==1?"neutral":"chaos").arg(i+1),a==0?3+i:a==1?std::array<int,6>{17,18,19,20,21,23}[i]:10+i};
            sample.items.push_back(item);
        }
        for(int a=0;a<3;++a)for(int t=0;t<3;++t)sample.talismans.push_back({QString("sample-talisman-%1-%2").arg(a).arg(t),SpellboxWidget::Alignment(a),{}});
        spellbox_->setInventory(sample);
        connect(spellbox_,&SpellboxWidget::cancelled,this,&MenuPreview::returnFromSpellbox);
        connect(spellbox_,&SpellboxWidget::loadoutAccepted,this,[this](const auto& request){returnFromSpellbox();statusBar()->showMessage(QString("Spellbox loadout for %1 accepted locally — campaign adapter pending.").arg(request.ownerId));});
        connect(spellbox_,&SpellboxWidget::spellPreviewRequested,this,[this](const auto& request){statusBar()->showMessage(QString("Preview %1 using %2 — supplied sample spell; engine adapter pending.").arg(request.spellId,request.itemId));});
    }
    if(!spellbox_->loadAssets(root,error))return false;
    spellboxReturnsToRegion_=regionEntry_ && screens_->currentWidget()==regionEntry_;
    activateScreen(spellbox_);setWindowTitle(spellbox_->windowTitle());spellbox_->focusFirstControl();
    statusBar()->showMessage("Sample inventory/spells. Drag or select and Assign; Remove restores a copy. OK accepts locally; Cancel restores.");return true;
}
void MenuPreview::returnFromSpellbox() {
    if(!spellboxReturnsToRegion_){showMainMenu();main_->findChild<QPushButton*>("mainMenuAction0")->setFocus(Qt::OtherFocusReason);return;}
    activateScreen(regionEntry_);setWindowTitle(regionEntry_->windowTitle());regionEntry_->findChild<QPushButton*>("regionEntryAuxiliary1")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Region Entry preview. Campaign engine adapter pending.");
}

void MenuPreview::activateScreen(QWidget* screen){
    // Bind application adapters after the widget's navigation handlers exist.
    emit screenReady(screen);
    if(screens_->currentWidget()!=screen){
        emit screenChanged();
        screens_->setCurrentWidget(screen);
    }
}
void MenuPreview::closeEvent(QCloseEvent* event){
    QMainWindow::closeEvent(event);
    if(event->isAccepted())emit closed();
}
