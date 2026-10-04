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
