#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QMetaEnum>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>

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
    mini_->setMode(mode); screens_->setCurrentWidget(mini_); mini_->focusFirstAction();
    setWindowTitle(mini_->windowTitle());
    statusBar()->showMessage("Mini Menu preview. Cancel or Escape returns to the main menu.");
    return true;
}
void MenuPreview::showMainMenu() {
    screens_->setCurrentWidget(main_);
    setWindowTitle(main_->windowTitle());
    main_->findChild<QPushButton*>("mainMenuAction2")->setFocus(Qt::OtherFocusReason);
    statusBar()->showMessage("Menu preview. Game actions are not connected.");
}
void MenuPreview::showQuickBattle() {
    screens_->setCurrentWidget(quick_);
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
    results_->setResults(sample); screens_->setCurrentWidget(results_); results_->focusContinue();
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
    quickResults_->setResults(sample); screens_->setCurrentWidget(quickResults_); quickResults_->focusFirstAction();
    setWindowTitle(quickResults_->windowTitle());
    statusBar()->showMessage("Sample results. Continue opens Quick Battle; Quit returns to the main menu preview.");
    return true;
}

bool MenuPreview::openMapSelection(const QString& root, QString* error) {
    if (!mapSelection_) {
        mapSelection_=new MapSelectionWidget(screens_); screens_->addWidget(mapSelection_);
        connect(mapSelection_,&MapSelectionWidget::cancelled,this,&MenuPreview::showQuickBattle);
        connect(mapSelection_,&MapSelectionWidget::mapSelected,this,[this](const QString& id) {
            showQuickBattle();
            statusBar()->showMessage(QString("Selected sample map %1 — engine adapter pending.").arg(id));
        });
    }
    if (!mapSelection_->loadAssets(root,error)) return false;
    if (!mapSelection_->setMaps({{"sample-forest","Sample forest map"},{"sample-plains","Sample plains map"},{"sample-islands","Sample islands map"}},"sample-forest",error)) return false;
    screens_->setCurrentWidget(mapSelection_); mapSelection_->focusSelection();
    setWindowTitle(mapSelection_->windowTitle());
    statusBar()->showMessage("Sample map list. OK or Cancel returns to the Quick Battle preview.");
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
    screens_->setCurrentWidget(loadGame_); loadGame_->focusSelection(); setWindowTitle(loadGame_->windowTitle());
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
    screens_->setCurrentWidget(saveGame_); saveGame_->focusSelection(); setWindowTitle(saveGame_->windowTitle());
    statusBar()->showMessage("Sample saves. Save/Delete emit intent; Cancel returns to the previous preview."); return true;
}
void MenuPreview::returnFromSaveGame() {
    if (!saveReturnsToMini_) { showMainMenu(); return; }
    screens_->setCurrentWidget(mini_); mini_->focusFirstAction(); setWindowTitle(mini_->windowTitle());
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
    screens_->setCurrentWidget(preferences_); preferences_->focusFirstControl(); setWindowTitle(preferences_->windowTitle());
    statusBar()->showMessage("Local sample settings. OK accepts changes; Cancel restores the snapshot."); return true;
}
void MenuPreview::returnFromPreferences() {
    if (!preferencesReturnToMini_) {
        showMainMenu(); main_->findChild<QPushButton*>("mainMenuAction3")->setFocus(Qt::OtherFocusReason); return;
    }
    screens_->setCurrentWidget(mini_); setWindowTitle(mini_->windowTitle());
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
            const QString action=request.mode==MultiplayerSetupWidget::Mode::Join?"Join":"Create";
            statusBar()->showMessage(QString("%1 request for %2 — engine networking adapter pending.").arg(action,request.userName));
        });
    }
    if (!screen->loadAssets(root,error)) return false;
    screens_->setCurrentWidget(screen); screen->focusFirstField(); setWindowTitle(screen->windowTitle());
    statusBar()->showMessage("Sample multiplayer form. OK emits intent; Cancel returns to Quick Battle."); return true;
}
