#pragma once
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QString>

// Session policy/accounting, independent of widgets, Wine and game addresses.
// Failures are sticky; a failed owner cannot become a completed generation.
class NativeOnlySession {
public:
    static constexpr const char* policy="native-only-rendering-v1";
    bool begin(const QString& directory){
        if((!state_.isEmpty() && state_!="restarted") || directory.isEmpty())return false;
        const auto root=QFileInfo(directory).absoluteFilePath();
        const auto path=QDir(root).filePath("native-only-session.json");
        if(QFileInfo::exists(path))return false;
        root_=root;path_=path;
        error_.clear();normalQuit_=fallback_=restartRequested_=stopVerified_=rendererRetired_=false;launcherExit_=-1;
        state_="starting";return save();
    }
    bool fail(const QString& reason){
        // A subsequent staging failure must not rewrite the retired generation.
        if(state_=="restarted"){path_.clear();root_.clear();}
        if(state_!="failed"){state_="failed";error_=reason.isEmpty()?QString("Native-only session refused"):reason;}
        return path_.isEmpty() || save();
    }
    bool finish(bool normalQuit,int launcherExit,bool rendererFinished,bool fallback){
        normalQuit_=normalQuit;launcherExit_=launcherExit;fallback_=fallback;
        if(state_=="failed")return save();
        if(!normalQuit || launcherExit || !rendererFinished || fallback)
            return fail("Native-only shutdown requires normal Quit, renderer retirement and zero fallback");
        state_="completed";return save();
    }
    bool requestRestart(){
        if(path_.isEmpty())return true; // Staging has not created a generation.
        if(state_=="completed" || state_=="restarted" || restartRequested_)return false;
        restartRequested_=true;return save();
    }
    bool retireForRestart(bool stopped,bool rendererRetired){
        if(path_.isEmpty()){
            if(!stopped||!rendererRetired)return fail("Hard restart did not retire staging");
            state_="restarted";return true;
        }
        if(!restartRequested_ || !stopped || !rendererRetired)return fail("Hard restart did not retire its process and renderer");
        stopVerified_=true;rendererRetired_=true;state_="restarted";return save();
    }
    bool completed() const{return state_=="completed";}
    QString error() const{return error_;}
private:
    bool save(){
        QSaveFile file(path_);const auto bytes=QJsonDocument(QJsonObject{
            {"schema",1},{"policy",policy},{"session",root_},{"state",state_},
            {"normal_quit",normalQuit_},{"launcher_exit",launcherExit_},
            {"fallback",fallback_},{"error",error_},{"complete_replacement",false},
            {"restart_requested",restartRequested_},{"force_stop_verified",stopVerified_},
            {"renderer_retired",rendererRetired_}}).toJson();
        return file.open(QIODevice::WriteOnly) && file.write(bytes)==bytes.size() && file.commit();
    }
    QString root_,path_,state_,error_;
    bool normalQuit_=false,fallback_=false,restartRequested_=false,stopVerified_=false,rendererRetired_=false;int launcherExit_=-1;
};
