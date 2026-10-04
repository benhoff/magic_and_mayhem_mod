#pragma once
#include <QMainWindow>
#include "mini_menu_widget.hpp"
#include "battle_result_widget.hpp"
#include "quick_battle_result_widget.hpp"
#include "map_selection_widget.hpp"
#include "load_game_widget.hpp"
#include "save_game_widget.hpp"
#include "preferences_widget.hpp"
#include "multiplayer_setup_widget.hpp"
#include "multiplayer_game_selection_widget.hpp"
#include "single_player_battle_widget.hpp"
#include "multiplayer_lobby_widget.hpp"
#include "region_entry_widget.hpp"
#include "character_screen_widget.hpp"
#include "grimoire_widget.hpp"
#include "spellbox_widget.hpp"
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
    bool openMultiplayer(const QString& root, MultiplayerSetupWidget::Mode mode, QString* error = nullptr);
    bool openMultiplayerGameSelection(const QString& root, QString* error = nullptr);
    bool openSinglePlayerBattle(const QString& root, QString* error = nullptr);
    bool openMultiplayerLobby(const QString& root, MultiplayerLobbyWidget::Mode mode, QString* error = nullptr);
    bool openRegionEntry(const QString& root, QString* error = nullptr);
    bool openCharacterScreen(const QString& root, QString* error = nullptr);
    bool openSpellbox(const QString& root, QString* error = nullptr);
    bool openGrimoire(const QString& root, QString* error = nullptr);
private:
    void returnFromCharacterScreen();
    void returnFromGrimoire();
    void returnFromSpellbox();
    void returnFromMapSelection();
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
    bool mapReturnsToSinglePlayer_ = false;
    MultiplayerLobbyWidget* mapReturnsToLobby_ = nullptr;
    SinglePlayerBattleWidget* singlePlayer_ = nullptr;
    LoadGameWidget* loadGame_ = nullptr;
    SaveGameWidget* saveGame_ = nullptr;
    bool saveReturnsToMini_ = false;
    PreferencesWidget* preferences_ = nullptr;
    bool preferencesReturnToMini_ = false;
    MultiplayerSetupWidget* joinMultiplayer_ = nullptr;
    MultiplayerSetupWidget* createMultiplayer_ = nullptr;
    MultiplayerGameSelectionWidget* multiplayerSelection_ = nullptr;
    MultiplayerSetupWidget::Request browsingRequest_;
    MultiplayerLobbyWidget* hostLobby_ = nullptr;
    MultiplayerLobbyWidget* joinLobby_ = nullptr;
    QString hostLobbyContext_, joinLobbyContext_;
    MultiplayerSetupWidget::Request hostLobbyRequest_, joinLobbyRequest_;
    RegionEntryWidget* regionEntry_ = nullptr;
    CharacterScreenWidget* characterScreen_ = nullptr;
    bool characterReturnsToRegion_ = false;
    GrimoireWidget* grimoire_ = nullptr;
    bool grimoireReturnsToRegion_ = false;
    SpellboxWidget* spellbox_ = nullptr;
    bool spellboxReturnsToRegion_ = false;
    QString assetRoot_;
};
