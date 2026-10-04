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
        if (action == MainMenuWidget::Action::Quit) { close(); return; }
        const auto name = QMetaEnum::fromType<MainMenuWidget::Action>().valueToKey(int(action));
        statusBar()->showMessage(QString("Selected %1 — engine adapter pending.").arg(QString::fromLatin1(name)));
    });
    connect(quick_, &QuickBattleMenuWidget::actionRequested, this, [this](QuickBattleMenuWidget::Action action) {
        if (action == QuickBattleMenuWidget::Action::Cancel) { showMainMenu(); return; }
        const auto name = QMetaEnum::fromType<QuickBattleMenuWidget::Action>().valueToKey(int(action));
        statusBar()->showMessage(QString("Selected %1 — engine adapter pending.").arg(QString::fromLatin1(name)));
    });
}
bool MenuPreview::loadAssets(const QString& root, bool startQuickBattle, QString* error) {
    if (!main_->loadAssets(root, error) || !quick_->loadAssets(root, error)) return false;
    if (startQuickBattle) showQuickBattle(); else showMainMenu();
    return true;
}
bool MenuPreview::openMiniMenu(const QString& root, MiniMenuWidget::Mode mode, QString* error) {
    if (!mini_) {
        mini_ = new MiniMenuWidget(screens_); screens_->addWidget(mini_);
        connect(mini_, &MiniMenuWidget::actionRequested, this, [this](MiniMenuWidget::Action action) {
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
