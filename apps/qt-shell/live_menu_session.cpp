#include "live_menu_session.hpp"
#include "../../protocols/include/mnm/menu_v6.h"
#include <QDir>
#include <QProcessEnvironment>
LiveMenuSession::LiveMenuSession(QString repository,QObject* parent):QObject(parent),repo_(std::move(repository)){
    process_.setProcessChannelMode(QProcess::MergedChannels);
    connect(&process_,&QProcess::readyReadStandardOutput,this,[this]{
        const auto bytes=process_.readAllStandardOutput();if(preparing_)preparation_+=bytes;
        if(output)output(QString::fromLocal8Bit(bytes));
    });
    connect(&process_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){
        if(error==QProcess::FailedToStart){preparing_=false;fallback(process_.errorString());if(finished)finished();}
    });
    connect(&process_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){
        if(preparing_){
            preparation_+=process_.readAllStandardOutput();preparing_=false;
            for(const auto& line:preparation_.split('\n'))if(line.startsWith("Evidence directory: "))root_=QString::fromLocal8Bit(line.mid(20)).trimmed();
            if(code||status!=QProcess::NormalExit||root_.isEmpty()){
                fallback("Menu staging failed; see the launch log.");if(finished)finished();return;
            }
            channel_=QDir(root_).filePath("channel.bin");bridge_=std::make_unique<MenuBridge>();
            if(!bridge_->create(channel_,true,true,miniMenusEnabled(),resultMenusEnabled,preferencesMenusEnabled)){fallback("Cannot create menu channel.");if(finished)finished();return;}
            active_=!bypass_;clock_.restart();lastState_=0;
            if(active_)timer_.start();else bridge_->retire();
            QStringList arguments{root_,"--menu-channel",channel_,"--prefix",winePrefix.isEmpty()?QDir(repo_).filePath("working/tests/menu-live-wine"):winePrefix};
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
    preparation_.clear();root_.clear();sequence_=0;state_={};pending_=transition_=bypass_=exitRequested_=quitting_=inBattle_=false;
    auto env=QProcessEnvironment::systemEnvironment();env.remove("MNM_MENU_CHANNEL");env.remove("MNM_RUNNER");env.remove("MNM_MENU_OBSERVE");
    process_.setProcessEnvironment(env);process_.setWorkingDirectory(repo_);preparing_=true;
    QStringList arguments{"--actions"};if(miniMenusEnabled())arguments.append("--experimental-mini");
    process_.start(QDir(repo_).filePath("tools/prepare-menu-observer.py"),arguments);return true;
}
bool LiveMenuSession::running() const{return preparing_||process_.state()!=QProcess::NotRunning;}
bool LiveMenuSession::request(quint32 action,quint32 argument,const std::array<int,17>* rules){
    if(!active_||inBattle_||pending_||transition_||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->request(action,state_,argument,rules))return false;
    quitting_=action==MNM_MENU_QUIT;pending_=true;requestedAt_=clock_.elapsed();action_=action;
    switch(action){
    case MNM_MENU_OPEN_QUICK:case MNM_MENU_SETUP_CANCEL:target_=22;break;
    case MNM_MENU_OPEN_SINGLE:case MNM_MENU_MAP_OK:case MNM_MENU_MAP_CANCEL:case MNM_MENU_SETUP_PLAYER:case MNM_MENU_SETUP_APPLY:target_=14;break;
    case MNM_MENU_OPEN_PREFERENCES:target_=10;break;
    case MNM_MENU_SETUP_MAP:target_=25;break;
    default:target_=3;break;
    }
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestPreferences(quint32 action,const std::array<int,7>& values,quint32 slider){
    if(!preferencesMenusEnabled||!active_||inBattle_||pending_||transition_||state_.screen!=10||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->requestPreferences(action,state_,values,slider))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;target_=action==MNM_MENU_PREFERENCES_PREVIEW?10:3;
    if(action!=MNM_MENU_PREFERENCES_PREVIEW&&stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestResults(quint32 action){
    if(!active_||inBattle_||pending_||transition_||state_.screen!=MNM_MENU_RESULT_SCREEN||
       clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->request(action,state_))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestMini(quint32 action){
    if(!miniMenusEnabled()||!active_||inBattle_||pending_||transition_||state_.screen!=MNM_MENU_MINI_SCREEN||
       clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->request(action,state_))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=action;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
bool LiveMenuSession::requestExit(){
    if(!active_||inBattle_||!sequence_||(!pending_&&!transition_&&state_.screen!=3&&state_.screen!=22&&state_.screen!=14&&state_.screen!=25&&state_.screen!=MNM_MENU_RESULT_SCREEN&&state_.screen!=10))return false;
    exitRequested_=true;
    if(!pending_&&!transition_&&state_.ready){
        const bool accepted=state_.screen==10?requestPreferences(MNM_MENU_PREFERENCES_CANCEL,state_.preferences.values):state_.screen==MNM_MENU_RESULT_SCREEN?requestResults(MNM_MENU_RESULT_QUIT):request(state_.screen==25?MNM_MENU_MAP_CANCEL:state_.screen==14?MNM_MENU_SETUP_CANCEL:state_.screen==22?MNM_MENU_BACK:MNM_MENU_QUIT);
        if(!accepted){exitRequested_=false;return false;}
    }
    return true;
}
bool LiveMenuSession::finishSpells(const std::array<int,63>& assignments){
    if(!active_||inBattle_||pending_||transition_||clock_.elapsed()-lastState_>MNM_MENU_V1_LEASE_MS||!bridge_->finishSpells(state_,assignments))return false;
    pending_=true;requestedAt_=clock_.elapsed();action_=MNM_MENU_SPELL_FINISH;
    if(stateChanged){auto waiting=state_;waiting.ready=0;stateChanged(waiting);}return true;
}
void LiveMenuSession::fallback(const QString& reason){
    active_=false;bypass_=true;timer_.stop();pending_=transition_=inBattle_=false;if(bridge_)bridge_->retire();
    if(failed)failed(reason);
}
void LiveMenuSession::poll(){
    if(!active_||!bridge_)return;
    bridge_->heartbeat();MenuBridge::State next;
    if(bridge_->read(next)&&next.sequence!=sequence_){
        sequence_=next.sequence;lastState_=clock_.elapsed();
        if(next.status==MNM_MENU_RETIRED){fallback("Menu adapter retired; using original menus.");return;}
        if(pending_&&next.ack!=state_.ack){
            pending_=false;
            if(next.status!=MNM_MENU_OK){fallback(QString("Menu action rejected (%1); using original menus.").arg(next.status));return;}
            if(action_==MNM_MENU_SETUP_START){
                if(!next.handoff){fallback("Engine Start returned an unknown destination; using original viewport.");return;}
                if(next.handoff==1){transition_=true;target_=7;state_=next;return;}
                inBattle_=true;transition_=false;state_=next;
                if(battleStarted)battleStarted(next.handoff);
                return;
            }
            if(action_==MNM_MENU_SPELL_FINISH){inBattle_=true;transition_=false;state_=next;if(battleStarted)battleStarted(2);return;}
            if(action_==MNM_MENU_RESULT_CONTINUE||action_==MNM_MENU_RESULT_QUIT||action_==MNM_MENU_MINI_CANCEL||action_==MNM_MENU_MINI_PREFERENCES||action_==MNM_MENU_MINI_QUIT){
                // Original viewport owns gameplay, Preferences and Quit confirmation.
                // Confirmation Yes/No semantics remain entirely in the original game.
                inBattle_=true;transition_=false;state_=next;if(originalViewportRequested)originalViewportRequested();return;
            }
            if(quitting_){
                // Quit was accepted by the original callback. No further menu ticks
                // are required while the original engine shuts down.
                timer_.stop();active_=false;bridge_->retire();
                if(output)output("Original game Quit accepted; waiting for the launcher to finish.\n");
                return;
            }
            transition_=action_!=MNM_MENU_PREFERENCES_PREVIEW;
        }
        if(!inBattle_&&!pending_&&state_.screen==7&&next.screen==0&&next.handoff==2){inBattle_=true;transition_=false;state_=next;if(battleStarted)battleStarted(2);return;}
        if(inBattle_){
            // Only a fresh, engine-confirmed return to a supported root menu
            // restores command ownership. Setup ticks during Start are ignored.
            if(next.ready&&next.screen==MNM_MENU_RESULT_SCREEN&&next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(miniMenusEnabled()&&next.ready&&next.screen==MNM_MENU_MINI_SCREEN&&next.mini.battle&&!next.mini.confirmation&&
               next.thread==state_.thread&&next.generation!=state_.generation&&next.ack==state_.ack&&next.status==MNM_MENU_OK){
                inBattle_=false;state_=next;if(stateChanged)stateChanged(next);return;
            }
            if(!next.ready||(next.screen!=3&&next.screen!=22)||next.handoff||next.thread!=state_.thread||next.generation==state_.generation||next.ack!=state_.ack||next.status!=MNM_MENU_OK)return;
            inBattle_=false;
            if(output)output("Original battle returned; restoring native menus.\n");
        }
        state_=next;
        if(transition_&&next.screen==target_&&next.ready)transition_=false;
        if(stateChanged){if((pending_&&action_!=MNM_MENU_PREFERENCES_PREVIEW)||transition_)next.ready=0;stateChanged(next);}
        if(exitRequested_&&!pending_&&!transition_&&state_.ready){
            if(state_.screen==10)requestPreferences(MNM_MENU_PREFERENCES_CANCEL,state_.preferences.values);
            else if(state_.screen==MNM_MENU_RESULT_SCREEN)requestResults(MNM_MENU_RESULT_QUIT);
            else request(state_.screen==25?MNM_MENU_MAP_CANCEL:state_.screen==14?MNM_MENU_SETUP_CANCEL:state_.screen==22?MNM_MENU_BACK:MNM_MENU_QUIT);
        }
    }
    if(!inBattle_&&(clock_.elapsed()-lastState_>(sequence_?MNM_MENU_V1_LEASE_MS:120000)||((pending_||transition_)&&clock_.elapsed()-requestedAt_>10000)))
        fallback("Menu adapter timed out; using original menus. Requests will not be retried.");
}
