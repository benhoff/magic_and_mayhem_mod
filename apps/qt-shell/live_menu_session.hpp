#pragma once
#include "menu_bridge.hpp"
#include "engine_preferences_store.hpp"
#include "../../protocols/include/mnm/menu_v4.h"
#include <QProcess>
#include "game_process.hpp"
#include <QTimer>
#include <QElapsedTimer>
#include <functional>
#include <memory>
// Process staging, launch and wire protocol remain outside presentation widgets.
class LiveMenuSession final : public QObject {
public:
    explicit LiveMenuSession(QString repository,QObject* parent=nullptr);
    bool nativeCommandsEnabled=false;
    bool nativeOnlyEnabled=false;
    int launcherExitCode=-1;
    bool nativeVoicesEnabled=false;
    bool nativeMediaEnabled=false;
    bool nativeMusicEnabled=false;
    bool skipMoviesEnabled=false;
    QString musicAssetsRoot;
    std::function<bool(const QString&)> prepareNativeAudio;
    std::function<bool(const QString&)> prepareNativeMedia;
    std::function<bool(const QString&)> prepareNativeMusic;
    std::function<bool(const QString&)> prepareNativePresentation;
    std::function<bool(const QString&)> prepareMenuAssets;
    bool campaignBookEnabled=false; // Experimental V22 static knowledge pages.
    bool campaignSpellboxEnabled=false; // Experimental V21 campaign inventory/caller.
    bool progressedMenusEnabled=false; // Experimental loaded-Realm Region paths.
    bool realmMenusEnabled=false; // Experimental until genuine progressed-caller proof.
    bool characterMenusEnabled=false; // Experimental until genuine progressed-caller proof.
    bool hostLobbyEnabled=true;
    bool sessionMenusEnabled=true;
    bool multiplayerMenusEnabled=true;
    bool grimoireMenusEnabled=true;
    bool storageMenusEnabled=true;
    bool campaignPreferencesEnabled=true;
    bool campaignDefeatEnabled=true;
    bool resultMenusEnabled=true; // Compatibility harnesses may explicitly retain V3.
    bool preferencesMenusEnabled=true;
    bool regionMenusEnabled=true;
    bool regionEnterEnabled=true;
    bool campaignMiniEnabled=true;
    bool campaignQuitEnabled=true;
    QString preferencesStorePath; // User-scoped by default; tests isolate it.
    QString winePrefix; // Empty selects the normal menu prefix; tests isolate each run.
    int smokeSeconds=0; // Bounded live validation only; normal sessions have no limit.
    bool start();
    void forceStop(std::function<void(bool,const QString&)> completed);
    void setMediaActive(bool active);
    bool running() const;
    qint64 launcherProcessId() const {return process_.processId();}
    bool normalQuitAccepted();
    void renderingProgress(std::uint64_t cpuRead=0); // GUI-thread bounded native-reader progress; never resubmits.
    QString evidenceDirectory() const {return root_;}
    bool request(quint32 action,quint32 argument=0,const std::array<int,17>* rules=nullptr);
    bool finishSpells(const std::array<int,63>& assignments);
    bool miniMenusEnabled() const {return campaignMiniEnabled||MNM_MENU_MINI_EXPERIMENTAL!=0;}
    bool requestPreferences(quint32 action,const std::array<int,7>& values,quint32 slider=0);
    bool requestDefeatContinue();
    bool requestResults(quint32 action);
    bool requestMini(quint32 action);
    bool requestMultiplayer(quint32 action,quint32 transport=0,const QString& gameName={},const QString& userName={});
    bool requestStorage(quint32 action,quint32 row=0,const QString& name={});
    bool requestExit(); // Back from Quick Battle, then original Main Quit.
    void fallback(const QString& reason);
    std::function<void(const MenuBridge::State&)> stateChanged;
    // Bounded test/diagnostic witness; an accepted intent is not an engine ACK.
    std::function<void(quint32,bool,const QString&,const MenuBridge::State&)> miniRequestObserved;
    std::function<void(const QString&)> output;
    std::function<void(const QString&)> failed;
    std::function<void()> finished;
    std::function<void()> launched;
    std::function<void(quint32)> battleStarted;
    std::function<void()> originalViewportRequested;
private:
    friend struct LiveMenuSessionTestAccess; // Synthetic wire/lifecycle validation without a game process.
    void saveAcceptedPreferences();
    void poll();
    void submitResults();
    bool sameResults(const MenuBridge::State& state) const;
    bool restoreMiniStorage(const MenuBridge::State& state);
    QString repo_,root_,channel_;
    EnginePreferencesStore preferencesStore_;
    bool preferencesToSave_=false;
    std::array<int,7> acceptedPreferences_{};
    GameProcess process_;
    QTimer timer_;
    QElapsedTimer clock_;
    QByteArray preparation_;
    std::unique_ptr<MenuBridge> bridge_;
    MenuBridge::State state_;
    bool preparing_=false,active_=false,pending_=false,transition_=false,bypass_=false,exitRequested_=false,quitting_=false,inBattle_=false;
    bool normalQuitAccepted_=false;
    qint64 lastState_=0,requestedAt_=0;
    std::uint64_t cpuRead_=0;
    qint64 cpuReadStarted_=0,cpuReadGrace_=0;
    bool cpuReadWaiting(qint64 now) const;
    bool mediaActive_=false;
    qint64 mediaStartedAt_=0;
    quint32 sequence_=0,target_=0,action_=0;
    quint32 storageOpeningGeneration_=0;
    // One user intent; wire IDs remain strictly increasing and once-only.
    quint32 resultsAction_=0,resultsThread_=0,resultsDepth_=0;
    qint64 resultsRequestedAt_=0;
};
