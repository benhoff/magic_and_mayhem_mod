#pragma once
#include <QMainWindow>
class MainMenuWidget;
class QuickBattleMenuWidget;
class QStackedWidget;

// Standalone native navigation; no game process or legacy adapter.
class MenuPreview final : public QMainWindow {
public:
    explicit MenuPreview(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, bool startQuickBattle, QString* error = nullptr);
private:
    void showMainMenu();
    void showQuickBattle();
    QStackedWidget* screens_ = nullptr;
    MainMenuWidget* main_ = nullptr;
    QuickBattleMenuWidget* quick_ = nullptr;
};
