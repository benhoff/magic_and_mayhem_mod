#include "live_menu_session.hpp"
#include "../../protocols/include/mnm/menu_v20.h"
#include "../../protocols/include/mnm/menu_v9.h"
#include <QDir>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
LiveMenuSession::LiveMenuSession(QString repository,QObject* parent):QObject(parent),repo_(std::move(repository)){
    progressedMenusEnabled=qEnvironmentVariable("MNM_EXPERIMENTAL_CAMPAIGN_MENUS")=="1";
    campaignBookEnabled=progressedMenusEnabled;
    campaignSpellboxEnabled=progressedMenusEnabled;
    realmMenusEnabled=progressedMenusEnabled||qEnvironmentVariable("MNM_EXPERIMENTAL_CAMPAIGN_REALM")=="1";
    characterMenusEnabled=realmMenusEnabled||qEnvironmentVariable("MNM_EXPERIMENTAL_CAMPAIGN_CHARACTER")=="1";
    preferencesStorePath=QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath("engine-preferences.json");
    process_.setProcessChannelMode(QProcess::MergedChannels);
    connect(&process_,&QProcess::readyReadStandardOutput,this,[this]{
        const auto bytes=process_.readAllStandardOutput();if(preparing_)preparation_+=bytes;
        if(output)output(QString::fromLocal8Bit(bytes));
    });
    connect(&process_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){
        if(process_.hardStopping())return;
        if(error==QProcess::FailedToStart){preparing_=false;fallback(process_.errorString());if(finished)finished();}
    });
    connect(&process_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){
        if(process_.hardStopping())return;
        if(preparing_){
            preparation_+=process_.readAllStandardOutput();preparing_=false;
            for(const auto& line:preparation_.split('\n'))if(line.startsWith("Evidence directory: "))root_=QString::fromLocal8Bit(line.mid(20)).trimmed();
            if(code||status!=QProcess::NormalExit||root_.isEmpty()){
                fallback("Menu staging failed; see the launch log.");if(finished)finished();return;
            }
            if(preferencesMenusEnabled){
                QFile manifest(QDir(root_).filePath("manifest.json"));
                if(manifest.open(QIODevice::ReadOnly)){
                    const auto snapshot=QJsonDocument::fromJson(manifest.readAll()).object().value("preference_store").toObject();
                    preferencesStore_.begin(preferencesStorePath,snapshot.value("revision").toString().toLatin1());
                }
            }
            if(nativeCommandsEnabled&&(!prepareNativePresentation||!prepareNativePresentation(root_))){fallback("Cannot create native presentation channels.");if(finished)finished();return;}
            if(nativeVoicesEnabled&&(!prepareNativeAudio||!prepareNativeAudio(QDir(root_).filePath("native-voices.bin")))){fallback("Cannot create native voice channel; see the launch log.");if(finished)finished();return;}
            if(nativeMusicEnabled&&(!prepareNativeMusic||!prepareNativeMusic(QDir(root_).filePath("native-music.bin")))){fallback("Cannot create native music channel; see the launch log.");if(finished)finished();return;}
            if(nativeMediaEnabled&&(!prepareNativeMedia||!prepareNativeMedia(QDir(root_).filePath("native-media.bin")))){fallback("Cannot create native media channel; see the launch log.");if(finished)finished();return;}
            channel_=QDir(root_).filePath("channel.bin");bridge_=std::make_unique<MenuBridge>();
            if(!bridge_->create(channel_,true,true,miniMenusEnabled(),resultMenusEnabled,preferencesMenusEnabled,regionMenusEnabled&&preferencesMenusEnabled,regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,campaignPreferencesEnabled&&campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,storageMenusEnabled&&campaignPreferencesEnabled&&campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,grimoireMenusEnabled&&storageMenusEnabled&&campaignPreferencesEnabled&&campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,multiplayerMenusEnabled&&grimoireMenusEnabled&&storageMenusEnabled&&campaignPreferencesEnabled&&campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,sessionMenusEnabled&&multiplayerMenusEnabled&&grimoireMenusEnabled&&storageMenusEnabled&&campaignPreferencesEnabled&&campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,hostLobbyEnabled&&sessionMenusEnabled&&multiplayerMenusEnabled&&grimoireMenusEnabled&&storageMenusEnabled&&campaignPreferencesEnabled&&campaignDefeatEnabled&&campaignQuitEnabled&&campaignMiniEnabled&&regionEnterEnabled&&regionMenusEnabled&&preferencesMenusEnabled,characterMenusEnabled,realmMenusEnabled,progressedMenusEnabled,campaignSpellboxEnabled,campaignBookEnabled)){fallback("Cannot create menu channel.");if(finished)finished();return;}
            active_=!bypass_;clock_.restart();lastState_=0;
            if(active_)timer_.start();else bridge_->retire();
            QStringList arguments{root_,"--menu-channel",channel_,"--prefix",winePrefix.isEmpty()?QDir(repo_).filePath(hostLobbyEnabled?"working/tests/menu-host-dx6-wine":"working/tests/menu-live-wine"):winePrefix};
            if(nativeCommandsEnabled)arguments.append("--native-render");
            if(smokeSeconds)arguments.append({"--seconds",QString::number(smokeSeconds)});
            process_.start(QDir(repo_).filePath("tools/run-menu-observer.py"),arguments);
            if(launched)launched();
        }else{
            timer_.stop();active_=false;if(bridge_)bridge_->retire();bridge_.reset();
            if(output)output(QString("Menu launcher exited with status %1. Evidence: %2\n").arg(code).arg(root_));
            if(finished)finished();
        }
    });
    timer_.setInterval(50);connect(&timer_,&QTimer::timeout,this,[this]{poll();});
}
bool LiveMenuSession::start(){
    if(running())return false;
    preferencesStore_.begin(preferencesStorePath,{});
    preferencesToSave_=false;preparation_.clear();root_.clear();sequence_=0;state_={};pending_=transition_=bypass_=exitRequested_=quitting_=inBattle_=normalQuitAccepted_=false;
    mediaActive_=false;mediaStartedAt_=0;
    auto env=QProcessEnvironment::systemEnvironment();env.remove("MNM_MENU_CHANNEL");env.remove("MNM_RUNNER");env.remove("MNM_MENU_OBSERVE");env.remove("MNM_AUDIO_CHANNEL");env.remove("MNM_RENDER_MEDIA");
    if(!env.contains("MNM_MENU_DIRECTPLAY_DX6"))env.insert("MNM_MENU_DIRECTPLAY_DX6",hostLobbyEnabled?"1":"0");
    env.remove("MNM_MUSIC_CHANNEL");process_.setProcessEnvironment(env);process_.setWorkingDirectory(repo_);preparing_=true;
    QStringList arguments{"--actions"};if(env.value("MNM_GRIMOIRE_TEST_OBSERVE")=="1")arguments.append("--grimoire-observe");if(nativeCommandsEnabled)arguments.append("--native-render");if(preferencesMenusEnabled)arguments.append({"--preferences-store",preferencesStorePath});if(MNM_MENU_MINI_EXPERIMENTAL)arguments.append("--experimental-mini");
    if(nativeCommandsEnabled&&env.value("MNM_NATIVE_DRAW",QStringLiteral("1"))!=QStringLiteral("0"))arguments.append("--native-draw");
    if(nativeCommandsEnabled&&env.value("MNM_PUBLIC_RESIDENT_WORLD")=="1")arguments.append("--resident-world");
    if(nativeVoicesEnabled)arguments.append("--native-voices");
    if(nativeMediaEnabled)arguments.append("--native-media");
    if(skipMoviesEnabled)arguments.append("--skip-movies");
    if(nativeMusicEnabled)arguments.append({"--native-music","--music-assets",musicAssetsRoot.isEmpty()?QDir(repo_).filePath("working/assets/native-cd-audio-20261012"):musicAssetsRoot});
    if(!env.value("MNM_MENU_GAME_SOURCE").isEmpty())arguments.append({"--source-game",env.value("MNM_MENU_GAME_SOURCE")});
    process_.start(QDir(repo_).filePath("tools/prepare-menu-observer.py"),arguments);return true;
}
void LiveMenuSession::setMediaActive(bool active){
    if(mediaActive_==active)return;
    const auto now=clock_.isValid()?clock_.elapsed():0;
    if(active)mediaStartedAt_=now;
    else{const auto paused=now-mediaStartedAt_;lastState_+=paused;requestedAt_+=paused;}
    mediaActive_=active;
}
bool LiveMenuSession::normalQuitAccepted(){
    // The resident timer can observe producer cancellation before the slower
    // menu timer consumes the already-published final Main Quit callback ACK.
    if(quitting_ && pending_)poll();
    return normalQuitAccepted_;
}
bool LiveMenuSession::running() const{return preparing_||process_.hardStopping()||process_.state()!=QProcess::NotRunning;}
void LiveMenuSession::forceStop(std::function<void(bool,const QString&)> completed){
    const bool staged=preparing_;
    timer_.stop();active_=false;preferencesToSave_=false;pending_=transition_=false;
    if(bridge_)bridge_->retire();
    process_.forceStop(staged?QString{}:winePrefix.isEmpty()?QDir(repo_).filePath(hostLobbyEnabled?"working/tests/menu-host-dx6-wine":"working/tests/menu-live-wine"):winePrefix,
        [this,completed=std::move(completed)](bool ok,const QString& error){
            preparing_=false;bridge_.reset();completed(ok,error);
        });
}
bool LiveMenuSession::request(quint32 action,quint32 argument,const std::array<int,17>* rules){
    if(mediaActive_||!active_||inBattle_||pending_||transition_||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->request(action,state_,argument,rules))return false;
    quitting_=action==MNM_MENU_QUIT;pending_=true;requestedAt_=clock_.elapsed();action_=action;
    switch(action){
    case MNM_MENU_REALM_ACTIVATE:target_=(state_.realm.rows[int(argument)-1].owned?4:18);break;
    case MNM_MENU_REALM_CHARACTER:target_=20;break;
    case MNM_MENU_REALM_GRIMOIRE:case MNM_MENU_REALM_SPELLBOX:target_=0;break;
    case MNM_MENU_REGION_CANCEL:target_=progressedMenusEnabled&&state_.region.different?4:3;break;
    case MNM_MENU_REGION_CHARACTER:target_=20;break;
    case MNM_MENU_REGION_SPELLBOX:target_=0;break;
    case MNM_MENU_CHARACTER_ADJUST:target_=20;break;
    case MNM_MENU_LOBBY_CANCEL:case MNM_MENU_SESSION_CANCEL:case MNM_MENU_MULTIPLAYER_CANCEL:case MNM_MENU_OPEN_QUICK:case MNM_MENU_SETUP_CANCEL:target_=22;break;
    case MNM_MENU_OPEN_CREATE:target_=15;break;
    case MNM_MENU_OPEN_JOIN:target_=16;break;
    case MNM_MENU_OPEN_SINGLE:case MNM_MENU_MAP_OK:case MNM_MENU_MAP_CANCEL:case MNM_MENU_SETUP_PLAYER:case MNM_MENU_SETUP_APPLY:target_=14;break;
    case MNM_MENU_GRIMOIRE_CLOSE:case MNM_MENU_NEW_GAME:case MNM_MENU_REGION_DIFFICULTY:target_=18;break;
    case MNM_MENU_REGION_GRIMOIRE:case MNM_MENU_GRIMOIRE_SELECT:target_=MNM_MENU_GRIMOIRE_SCREEN;break;
    case MNM_MENU_OPEN_LOAD:target_=MNM_MENU_LOAD_SCREEN;break;
    case MNM_MENU_OPEN_PREFERENCES:target_=10;break;
    case MNM_MENU_SETUP_MAP:target_=25;break;
    default:target_=3;break;
    }
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestPreferences(quint32 action,const std::array<int,7>& values,quint32 slider){
    if(mediaActive_||!preferencesMenusEnabled||!active_||inBattle_||pending_||transition_||state_.screen!=10||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->requestPreferences(action,state_,values,slider))return false;
    if(action==MNM_MENU_PREFERENCES_OK){preferencesToSave_=true;acceptedPreferences_=values;}
    pending_=true;requestedAt_=clock_.elapsed();action_=action;target_=action==MNM_MENU_PREFERENCES_PREVIEW?10:3;
    if(action!=MNM_MENU_PREFERENCES_PREVIEW&&stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestDefeatContinue(){
    if(!campaignDefeatEnabled||state_.screen!=MNM_MENU_DEFEAT_SCREEN)return false;
    return request(MNM_MENU_DEFEAT_OK);
}
bool LiveMenuSession::requestResults(quint32 action){
    if(mediaActive_||!active_||inBattle_||pending_||transition_||state_.screen!=MNM_MENU_RESULT_SCREEN||
       clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->request(action,state_))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestMini(quint32 action){
    if(mediaActive_||!miniMenusEnabled()||!active_||inBattle_||pending_||transition_||state_.screen!=MNM_MENU_MINI_SCREEN||
       clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->request(action,state_))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestMultiplayer(quint32 action,quint32 transport,const QString& gameName,const QString& userName){
    if(!multiplayerMenusEnabled||!active_||inBattle_||pending_||transition_||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->requestMultiplayer(action,state_,transport,gameName,userName))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;target_=22;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestStorage(quint32 action,quint32 row,const QString& name){
    if(!storageMenusEnabled||!active_||inBattle_||pending_||transition_||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->requestStorage(action,state_,row,name))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;target_=state_.screen;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestExit(){
    if(mediaActive_||!active_||inBattle_||!sequence_||(!pending_&&!transition_&&state_.screen!=3&&state_.screen!=22&&state_.screen!=14&&state_.screen!=25&&state_.screen!=MNM_MENU_RESULT_SCREEN&&state_.screen!=10&&state_.screen!=18&&state_.screen!=MNM_MENU_DEFEAT_SCREEN&&state_.screen!=MNM_MENU_LOAD_SCREEN&&state_.screen!=13&&state_.screen!=MNM_MENU_GRIMOIRE_SCREEN&&state_.screen!=15&&state_.screen!=16&&state_.screen!=21&&state_.screen!=24))return false;
    exitRequested_=true;
    if(!pending_&&!transition_&&state_.ready){
        const bool accepted=state_.screen==24?request(MNM_MENU_LOBBY_CANCEL):state_.screen==21?request(MNM_MENU_SESSION_CANCEL):(state_.screen==15||state_.screen==16)?requestMultiplayer(MNM_MENU_MULTIPLAYER_CANCEL):state_.screen==MNM_MENU_GRIMOIRE_SCREEN?request(MNM_MENU_GRIMOIRE_CLOSE):state_.screen==MNM_MENU_DEFEAT_SCREEN?requestDefeatContinue():state_.screen==18?request(MNM_MENU_REGION_CANCEL):state_.screen==10?requestPreferences(MNM_MENU_PREFERENCES_CANCEL,state_.preferences.values):state_.screen==MNM_MENU_RESULT_SCREEN?requestResults(MNM_MENU_RESULT_QUIT):request(state_.screen==25?MNM_MENU_MAP_CANCEL:state_.screen==14?MNM_MENU_SETUP_CANCEL:state_.screen==22?MNM_MENU_BACK:MNM_MENU_QUIT);
        if(!accepted){exitRequested_=false;return false;}
    }
    return true;
}
bool LiveMenuSession::finishSpells(const std::array<int,63>& assignments){
    if(mediaActive_||!active_||inBattle_||pending_||transition_||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->finishSpells(state_,assignments))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=MNM_MENU_SPELL_FINISH;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
void LiveMenuSession::fallback(const QString& reason){
    preferencesToSave_=false;active_=false;bypass_=true;timer_.stop();pending_=transition_=inBattle_=false;if(bridge_)bridge_->retire();
    if(failed)failed(reason);
}
void LiveMenuSession::saveAcceptedPreferences(){
    if(!preferencesToSave_)return;
    preferencesToSave_=false;QString error;
    const bool saved=preferencesStore_.accept(QDir(root_).filePath("game/CFG/prefs.cfg"),acceptedPreferences_,&error);
    if(output)output(saved?"Preferences saved for future launches.\n":"Preferences remain available in this session; persistence failed: "+error+"\n");
}
void LiveMenuSession::renderingProgress(){if(active_&&bridge_)bridge_->heartbeat();}
void LiveMenuSession::poll(){
    if(!active_||!bridge_)return;
    bridge_->heartbeat();MenuBridge::State next;
    if(bridge_->read(next)&&next.sequence!=sequence_){
        sequence_=next.sequence;lastState_=clock_.elapsed();
        if(next.status==MNM_MENU_RETIRED){fallback("Menu adapter retired; using original menus.");return;}
        if(pending_&&next.ack!=state_.ack){
            if(((progressedMenusEnabled&&(state_.screen==18||action_==MNM_MENU_DEFEAT_OK||action_==MNM_MENU_SPELL_FINISH||(campaignBookEnabled&&(action_==MNM_MENU_GRIMOIRE_SELECT||action_==MNM_MENU_GRIMOIRE_CLOSE))))||(action_>=MNM_MENU_REALM_ACTIVATE&&action_<=MNM_MENU_REGION_SPELLBOX)||action_==MNM_MENU_CHARACTER_ADJUST||action_==MNM_MENU_CHARACTER_OK||action_==MNM_MENU_CHARACTER_CANCEL)&&
               (next.ack!=bridge_->requestedSequence()||next.thread!=state_.thread)){
                fallback("Menu acknowledgement identity differs; using original menus.");return;
            }
            pending_=false;
            if(next.status!=MNM_MENU_OK){fallback(QString("Menu action rejected (%1); using original menus.").arg(next.status));return;}
            if(campaignBookEnabled&&action_==MNM_MENU_GRIMOIRE_CLOSE){inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;}
            if(action_==MNM_MENU_CHARACTER_OK||action_==MNM_MENU_CHARACTER_CANCEL){
                inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
            }
            if(action_==MNM_MENU_REGION_ENTER){
                if(progressedMenusEnabled&&next.handoff==MNM_MENU_HANDOFF_CAMPAIGN_RETURN){inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;}
                if(next.handoff!=3){fallback("Original campaign Enter returned an unknown destination; using original controls.");return;}
                inBattle_=true;transition_=false;state_=next;if(battleStarted)battleStarted(3);return;
            }
            if(action_==MNM_MENU_SETUP_START){
                if(!next.handoff){fallback("Engine Start returned an unknown destination; using original viewport.");return;}
                if(next.handoff==1){transition_=true;target_=7;state_=next;return;}
                inBattle_=true;transition_=false;state_=next;
                if(battleStarted)battleStarted(next.handoff);
                return;
            }
            if(action_==MNM_MENU_SPELL_FINISH){
                const bool campaign=campaignSpellboxEnabled&&!state_.spells.mode;
                if(next.handoff!=(campaign?MNM_MENU_HANDOFF_CAMPAIGN_RETURN:2u)){fallback("Original spell selection returned an unknown destination.");return;}
                inBattle_=true;transition_=false;state_=next;if(campaign){if(originalViewportRequested)originalViewportRequested();}else if(battleStarted)battleStarted(2);return;
            }
            if(progressedMenusEnabled&&state_.screen==18&&(action_==MNM_MENU_REGION_CHARACTER||action_==MNM_MENU_REGION_SPELLBOX||action_==MNM_MENU_REGION_GRIMOIRE)){
                inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
            }
            if(realmMenusEnabled&&state_.screen==4&&(action_==MNM_MENU_REALM_CHARACTER||action_==MNM_MENU_REALM_GRIMOIRE||action_==MNM_MENU_REALM_SPELLBOX)){
                inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
            }
            if(state_.screen==10&&state_.preferences.parentScreen==17&&(action_==MNM_MENU_PREFERENCES_OK||action_==MNM_MENU_PREFERENCES_CANCEL)){
                if(action_==MNM_MENU_PREFERENCES_OK)saveAcceptedPreferences();
                inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
            }
            if(action_==MNM_MENU_SESSION_JOIN||action_==MNM_MENU_MULTIPLAYER_SUBMIT||action_==MNM_MENU_STORAGE_ACCEPT||action_==MNM_MENU_STORAGE_CANCEL||action_==MNM_MENU_MINI_LOAD||action_==MNM_MENU_MINI_SAVE||action_==MNM_MENU_DEFEAT_OK||action_==MNM_MENU_RESULT_CONTINUE||action_==MNM_MENU_RESULT_QUIT||action_==MNM_MENU_MINI_CANCEL||action_==MNM_MENU_MINI_PREFERENCES||action_==MNM_MENU_MINI_QUIT){
                // Original viewport owns gameplay, Preferences and Quit confirmation.
                // Confirmation Yes/No semantics remain entirely in the original game.
                inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
            }
            if(quitting_){
                if(next.ack!=bridge_->requestedSequence() || next.thread!=state_.thread){
                    fallback("Original Main Quit acknowledgement identity differs.");return;
                }
                normalQuitAccepted_=true;
                // Quit was accepted by the original callback. No further menu ticks
                // are required while the original engine shuts down.
                timer_.stop();active_=false;bridge_->retire();
                if(output)output("Original game Quit accepted; waiting for the launcher to finish.\n");
                return;
            }
            transition_=action_!=MNM_MENU_PREFERENCES_PREVIEW&&action_!=MNM_MENU_REGION_DIFFICULTY&&action_!=MNM_MENU_CHARACTER_ADJUST;
        }
        if(!inBattle_&&!pending_&&state_.screen==7&&next.screen==0&&next.handoff==(campaignSpellboxEnabled&&!state_.spells.mode?4u:2u)){inBattle_=true;transition_=false;const bool campaign=campaignSpellboxEnabled&&!state_.spells.mode;state_=next;if(campaign){if(originalViewportRequested)originalViewportRequested();}else if(battleStarted)battleStarted(2);return;}
        if(hostLobbyEnabled&&!inBattle_&&!pending_&&!transition_&&state_.screen==24&&next.screen==0&&next.thread==state_.thread&&next.ack==state_.ack){
            inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
        }
        if(storageMenusEnabled&&!inBattle_&&(next.screen==MNM_MENU_LOAD_SCREEN||next.screen==13)&&next.storage.modal&&next.thread==state_.thread&&next.ack==state_.ack){
            inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
        }
        if(inBattle_){
            if(progressedMenusEnabled&&next.ready&&(next.screen==18||next.screen==MNM_MENU_GRIMOIRE_SCREEN||(campaignSpellboxEnabled&&next.screen==7))&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(realmMenusEnabled&&next.ready&&next.screen==4&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(characterMenusEnabled&&next.ready&&next.screen==20&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            // Only a fresh, engine-confirmed return to a supported root menu
            // restores command ownership. Setup ticks during Start are ignored.
            if(next.ready&&((campaignDefeatEnabled&&next.screen==MNM_MENU_DEFEAT_SCREEN)||next.screen==MNM_MENU_RESULT_SCREEN)&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(hostLobbyEnabled&&next.ready&&next.screen==24&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(sessionMenusEnabled&&next.ready&&next.screen==21&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(multiplayerMenusEnabled&&next.ready&&(next.screen==15||next.screen==16)&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(storageMenusEnabled&&next.ready&&(next.screen==MNM_MENU_LOAD_SCREEN||next.screen==13)&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(campaignPreferencesEnabled&&next.ready&&next.screen==10&&next.preferences.parentScreen==17&&next.preferences.depth==6&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(miniMenusEnabled()&&next.ready&&next.screen==MNM_MENU_MINI_SCREEN&&(next.mini.battle||(campaignMiniEnabled&&next.mini.context==5&&next.mini.mode==2))&&!next.mini.confirmation&&
               next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(!next.ready||(next.screen!=3&&next.screen!=22)||next.handoff||next.thread!=state_.thread||next.generation==state_.generation||next.ack!=state_.ack||next.status!=MNM_MENU_OK)return;
            inBattle_=false;
            if(output)output("Original battle returned; restoring native menus.\n");
        }
        if(preferencesToSave_&&!pending_&&next.ready&&next.screen==3){
            saveAcceptedPreferences();
        }
        state_=next;
        if(transition_&&next.screen==target_&&next.ready)transition_=false;
        if(stateChanged){if((pending_&&action_!=MNM_MENU_PREFERENCES_PREVIEW)||transition_)next.ready=0;stateChanged(next);}
        if(exitRequested_&&!pending_&&!transition_&&state_.ready){
            if(state_.screen==24)request(MNM_MENU_LOBBY_CANCEL);
            else if(state_.screen==21)request(MNM_MENU_SESSION_CANCEL);
            else if(state_.screen==15||state_.screen==16)requestMultiplayer(MNM_MENU_MULTIPLAYER_CANCEL);
            else if(state_.screen==MNM_MENU_GRIMOIRE_SCREEN)request(MNM_MENU_GRIMOIRE_CLOSE);
            else if(state_.screen==MNM_MENU_LOAD_SCREEN||state_.screen==13)requestStorage(MNM_MENU_STORAGE_CANCEL);
            else if(state_.screen==MNM_MENU_DEFEAT_SCREEN)requestDefeatContinue();
            else if(state_.screen==18)request(MNM_MENU_REGION_CANCEL);
            else if(state_.screen==10)requestPreferences(MNM_MENU_PREFERENCES_CANCEL,state_.preferences.values);
            else if(state_.screen==MNM_MENU_RESULT_SCREEN)requestResults(MNM_MENU_RESULT_QUIT);
            else request(state_.screen==25?MNM_MENU_MAP_CANCEL:state_.screen==14?MNM_MENU_SETUP_CANCEL:state_.screen==22?MNM_MENU_BACK:MNM_MENU_QUIT);
        }
    }
    if(!mediaActive_&&!inBattle_&&(clock_.elapsed()-lastState_>(sequence_?MNM_MENU_V1_LEASE_MS:120000)||((pending_||transition_)&&clock_.elapsed()-requestedAt_>10000)))
        fallback("Menu adapter timed out; using original menus. Requests will not be retried.");
}
