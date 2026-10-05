#pragma once
#include "menu_bridge.hpp"
#include "engine_preferences_store.hpp"
#include "../../protocols/include/mnm/menu_v4.h"
#include <QProcess>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>
#include <memory>
// Process staging, launch and wire protocol remain outside presentation widgets.
class LiveMenuSession final : public QObject {
public:
    explicit LiveMenuSession(QString repository,QObject* parent=nullptr);
    bool resultMenusEnabled=true; // Compatibility harnesses may explicitly retain V3.
    bool preferencesMenusEnabled=true;
    bool regionMenusEnabled=true;
    QString preferencesStorePath; // User-scoped by default; tests isolate it.
    QString winePrefix; // Empty selects the normal menu prefix; tests isolate each run.
    int smokeSeconds=0; // Bounded live validation only; normal sessions have no limit.
    bool start();
    bool running() const;
    QString evidenceDirectory() const {return root_;}
    bool request(quint32 action,quint32 argument=0,const std::array<int,17>* rules=nullptr);
    bool finishSpells(const std::array<int,63>& assignments);
    bool miniMenusEnabled() const {return MNM_MENU_MINI_EXPERIMENTAL!=0;}
    bool requestPreferences(quint32 action,const std::array<int,7>& values,quint32 slider=0);
    bool requestResults(quint32 action);
    bool requestMini(quint32 action);
    bool requestExit(); // Back from Quick Battle, then original Main Quit.
    void fallback(const QString& reason);
    std::function<void(const MenuBridge::State&)> stateChanged;
    std::function<void(const QString&)> output;
    std::function<void(const QString&)> failed;
    std::function<void()> finished;
    std::function<void()> launched;
    std::function<void(quint32)> battleStarted;
    std::function<void()> originalViewportRequested;
private:
    void poll();
    QString repo_,root_,channel_;
    EnginePreferencesStore preferencesStore_;
    bool preferencesToSave_=false;
    std::array<int,7> acceptedPreferences_{};
    QProcess process_;
    QTimer timer_;
    QElapsedTimer clock_;
    QByteArray preparation_;
    std::unique_ptr<MenuBridge> bridge_;
    MenuBridge::State state_;
    bool preparing_=false,active_=false,pending_=false,transition_=false,bypass_=false,exitRequested_=false,quitting_=false,inBattle_=false;
    qint64 lastState_=0,requestedAt_=0;
    quint32 sequence_=0,target_=0,action_=0;
};
