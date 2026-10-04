#pragma once
#include <QMainWindow>
#include "mini_menu_widget.hpp"
#include "battle_result_widget.hpp"
#include "quick_battle_result_widget.hpp"
#include "map_selection_widget.hpp"
#include "load_game_widget.hpp"
#include "save_game_widget.hpp"
#include "preferences_widget.hpp"
class MainMenuWidget;
class QuickBattleMenuWidget;
class QStackedWidget;

// Standalone native navigation; no game process or legacy adapter.
class MenuPreview final : public QMainWindow {
public:
    explicit MenuPreview(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, bool startQuickBattle, QString* error = nullptr);
    bool openMiniMenu(const QString& root, MiniMenuWidget::Mode mode, QString* error = nullptr);
    bool openBattleResults(const QString& root, BattleResultWidget::Outcome outcome, QString* error = nullptr);
    bool openQuickBattleResults(const QString& root, QuickBattleResultWidget::PrimaryAction action, QString* error = nullptr);
    bool openMapSelection(const QString& root, QString* error = nullptr);
    bool openLoadGame(const QString& root, QString* error = nullptr);
    bool openSaveGame(const QString& root, QString* error = nullptr);
    bool openPreferences(const QString& root, QString* error = nullptr);
private:
    void returnFromPreferences();
    void returnFromSaveGame();
    void showMainMenu();
    void showQuickBattle();
    QStackedWidget* screens_ = nullptr;
    MainMenuWidget* main_ = nullptr;
    QuickBattleMenuWidget* quick_ = nullptr;
    MiniMenuWidget* mini_ = nullptr;
    BattleResultWidget* results_ = nullptr;
    QuickBattleResultWidget* quickResults_ = nullptr;
    MapSelectionWidget* mapSelection_ = nullptr;
    LoadGameWidget* loadGame_ = nullptr;
    SaveGameWidget* saveGame_ = nullptr;
    bool saveReturnsToMini_ = false;
    PreferencesWidget* preferences_ = nullptr;
    bool preferencesReturnToMini_ = false;
    QString assetRoot_;
};
