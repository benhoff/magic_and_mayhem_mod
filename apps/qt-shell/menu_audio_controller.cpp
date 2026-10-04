#include "menu_audio_controller.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QStackedWidget>
#include "menu_audio_preferences.hpp"
#include <stdexcept>
MenuAudioController::MenuAudioController(std::unique_ptr<AudioSessionOutput> output,MenuAudioCues cues,QObject* parent,QSettings* settings)
    :QObject(parent),session_(std::move(output)),cues_(cues),settings_(settings){
    if(cues.activate<=0 || cues.pageTurn<=0)throw std::invalid_argument("Menu cues require positive catalog IDs");
    soundLevel_=menuAudioPreferences::read(settings,"audio/v1/effectsLevel",-1000);
    session_.changed=[this](AudioSessionState state,const QString& message){
        if(state==AudioSessionState::running){
            error_.clear();
            if(!session_.preflight().playable(cues_.activate) || !session_.preflight().playable(cues_.pageTurn)){
                error_="Selected menu cue is unavailable in catalog preflight";session_.stop();return;
            }
        }
        if(preview_){
            const auto text=state==AudioSessionState::running?QString("Audio ready"):
                state==AudioSessionState::recovering?QString("Restarting audio…"):
                state==AudioSessionState::failed?QString("Audio paused: %1").arg(message):QString("Audio stopped");
            preview_->setAudioStatus(text,state==AudioSessionState::failed && session_.canRecover());
        }
        if(state==AudioSessionState::failed && failed)failed(message);
    };
}
MenuAudioController::~MenuAudioController(){session_.changed={};stop();}
void MenuAudioController::attach(MenuPreview& preview){
    if(preview_)throw std::logic_error("Menu audio controller already attached");
    preview_=&preview;
    QString error;
    if(settings_ && !preview.setEffectsVolume(soundLevel_,&error) && failed)failed(error);
    connect(&preview,&MenuPreview::audioRetryRequested,this,[this]{recover();});
    connect(&preview,&MenuPreview::screenReady,this,[this](QWidget* screen){bind(screen);});
    connect(&preview,&MenuPreview::screenChanged,this,[this]{clear();});
    connect(&preview,&MenuPreview::closed,this,[this]{stop();});
    connect(&preview,&QObject::destroyed,this,[this]{preview_=nullptr;bound_.clear();stop();});
    if(auto* stack=preview.findChild<QStackedWidget*>())for(int i=0;i<stack->count();++i)bind(stack->widget(i));
}
bool MenuAudioController::start(const QString& root,mnm::reconstruction::audio::NativeSourcePathPolicy policy){
    error_.clear();
    session_.setMasterVolume(soundLevel_);
    if(!session_.start(root,1,policy))return false;
    if(!session_.preflight().playable(cues_.activate) || !session_.preflight().playable(cues_.pageTurn)){
        error_="Selected menu cue is unavailable in catalog preflight";session_.stop();return false;
    }
    if(!session_.setMasterVolume(soundLevel_)){error_=session_.lastError();session_.stop();return false;}
    return true;
}
bool MenuAudioController::recover(){
    error_.clear();session_.setMasterVolume(soundLevel_);return session_.recover();
}
void MenuAudioController::stop(){session_.stop();}
void MenuAudioController::play(std::int32_t id){
    if(!running())return;
    // Native minimum is an explicit preview mute gate, not an original inference.
    if(soundLevel_== -10000)return;
    if(!session_.play(id) && failed)failed(session_.lastError());
}
void MenuAudioController::activate(){play(cues_.activate);}
void MenuAudioController::pageTurn(){play(cues_.pageTurn);}
void MenuAudioController::clear(){if(running())session_.clearVoices();}
void MenuAudioController::applySoundLevel(int level){
    if(level< -10000 || level>0){if(failed)failed("Menu sound level outside native range");return;}
    soundLevel_=level;
    clear(); // Accepted volume cannot leave queued PCM at the old gain.
    if(!session_.setMasterVolume(level) && failed)failed(session_.lastError());
}
void MenuAudioController::bind(QWidget* screen){
    if(bound_.contains(screen))return;
    bound_.insert(screen);
    connect(screen,&QObject::destroyed,this,[this,screen]{bound_.remove(screen);});
    // These connections are installed after navigation. Transitions discard old
    // cues first, then the accepted action sounds once in the destination screen.
    const auto cue=[this](auto...){activate();};
    if(auto* w=qobject_cast<MainMenuWidget*>(screen))connect(w,&MainMenuWidget::actionRequested,this,cue);
    else if(auto* w=qobject_cast<QuickBattleMenuWidget*>(screen))connect(w,&QuickBattleMenuWidget::actionRequested,this,cue);
    else if(auto* w=qobject_cast<MiniMenuWidget*>(screen))connect(w,&MiniMenuWidget::actionRequested,this,cue);
    else if(auto* w=qobject_cast<BattleResultWidget*>(screen))connect(w,&BattleResultWidget::continueRequested,this,cue);
    else if(auto* w=qobject_cast<QuickBattleResultWidget*>(screen))connect(w,&QuickBattleResultWidget::actionRequested,this,cue);
    else if(auto* w=qobject_cast<MapSelectionWidget*>(screen)){connect(w,&MapSelectionWidget::mapSelected,this,cue);connect(w,&MapSelectionWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<LoadGameWidget*>(screen)){connect(w,&LoadGameWidget::loadRequested,this,cue);connect(w,&LoadGameWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<SaveGameWidget*>(screen)){connect(w,&SaveGameWidget::saveRequested,this,cue);connect(w,&SaveGameWidget::deleteRequested,this,cue);connect(w,&SaveGameWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<PreferencesWidget*>(screen)){
        const auto level=w->settings().soundLevel;
        if(level!=soundLevel_)applySoundLevel(level);
        connect(w,&PreferencesWidget::settingsApplied,this,[this](const auto& settings){
            applySoundLevel(settings.soundLevel);
            if(!menuAudioPreferences::save(settings_,"audio/v1/effectsLevel",soundLevel_) && failed)
                failed("Could not save effects volume; accepted value remains active for this session");
            activate();
        });
        connect(w,&PreferencesWidget::cancelled,this,cue);
    }
    else if(auto* w=qobject_cast<MultiplayerSetupWidget*>(screen)){connect(w,&MultiplayerSetupWidget::requestSubmitted,this,cue);connect(w,&MultiplayerSetupWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<MultiplayerGameSelectionWidget*>(screen)){connect(w,&MultiplayerGameSelectionWidget::sessionSelected,this,cue);connect(w,&MultiplayerGameSelectionWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<SinglePlayerBattleWidget*>(screen)){
        connect(w,&SinglePlayerBattleWidget::startRequested,this,cue);connect(w,&SinglePlayerBattleWidget::mapRequested,this,cue);
        connect(w,&SinglePlayerBattleWidget::playerChangeRequested,this,cue);connect(w,&SinglePlayerBattleWidget::colourChangeRequested,this,cue);
        connect(w,&SinglePlayerBattleWidget::playerRemovalRequested,this,cue);connect(w,&SinglePlayerBattleWidget::cancelled,this,cue);
    }
    else if(auto* w=qobject_cast<MultiplayerLobbyWidget*>(screen)){
        connect(w,&MultiplayerLobbyWidget::startRequested,this,cue);connect(w,&MultiplayerLobbyWidget::readyRequested,this,cue);
        connect(w,&MultiplayerLobbyWidget::chatRequested,this,cue);connect(w,&MultiplayerLobbyWidget::mapRequested,this,cue);
        connect(w,&MultiplayerLobbyWidget::playerChangeRequested,this,cue);connect(w,&MultiplayerLobbyWidget::colourChangeRequested,this,cue);
        connect(w,&MultiplayerLobbyWidget::playerRemovalRequested,this,cue);connect(w,&MultiplayerLobbyWidget::cancelled,this,cue);
    }
    else if(auto* w=qobject_cast<RealmViewerWidget*>(screen)){connect(w,&RealmViewerWidget::regionRequested,this,cue);connect(w,&RealmViewerWidget::auxiliaryRequested,this,cue);connect(w,&RealmViewerWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<RegionEntryWidget*>(screen)){connect(w,&RegionEntryWidget::enterRequested,this,cue);connect(w,&RegionEntryWidget::auxiliaryRequested,this,cue);connect(w,&RegionEntryWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<CharacterScreenWidget*>(screen)){connect(w,&CharacterScreenWidget::characterAccepted,this,cue);connect(w,&CharacterScreenWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<SpellboxWidget*>(screen)){connect(w,&SpellboxWidget::loadoutAccepted,this,cue);connect(w,&SpellboxWidget::spellPreviewRequested,this,cue);connect(w,&SpellboxWidget::cancelled,this,cue);}
    else if(auto* w=qobject_cast<GrimoireWidget*>(screen)){
        connect(w,&GrimoireWidget::closed,this,cue);
        connect(w,&GrimoireWidget::pageChanged,this,[this,w](const auto&){
            if(preview_ && preview_->findChild<QStackedWidget*>()->currentWidget()==w)pageTurn();
        });
    }
}
