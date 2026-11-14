#pragma once
#include "surface_loss_probe.hpp"
#include "live_command_session.hpp"

// Same private request/receipt contract for the ordinary public GUI owner.
// Menu actions and game addresses remain outside this provider adapter.
class GuiSurfaceLossProbe final {
public:
    GuiSurfaceLossProbe(const QString& channel,quint32 producer,quint32 version):probe_(channel,producer,version){}
    QString poll(LiveCommandSession& commands,quint64 worlds,bool pendingWorld){
        const auto* result=commands.result();
        // An externally drained producer ring does not prove that this GUI
        // poll has retired its buffered commands or borrowed World turn.
        // Leave the atomic request untouched until the owner is quiescent.
        if(!result || !result->liveSurfaces || !result->presents || pendingWorld || commands.hasPendingCommands())return {};
        if(!probe_.requested())return {};
        auto& observation=probe_.observation;
        observation["provider_owner"]="gui-resident";
        observation["worker_thread"]=false;
        observation["render_owner_gui_thread"]=QThread::currentThread()==QCoreApplication::instance()->thread();
        require(observation["render_owner_gui_thread"].toBool(),"GUI surface-loss probe on foreign thread");
        observation["worlds_before"]=QString::number(worlds);
        observation["live_surfaces_before"]=qint64(result->liveSurfaces);
        observation["pending_cpu_read"]="0";
        observation["destroy_begin_host_us"]=QString::number(SurfaceLossProbe::now());
        const auto target=commands.loseGraphicsTargetForTest();
        observation["destroy_end_host_us"]=QString::number(SurfaceLossProbe::now());
        observation["surface_valid_before"]=target[0];observation["surface_valid_after"]=target[1];
        observation["gui_destroyed_target"]=target[0]&&!target[1];
        QString error;bool graphics=false;
        try{commands.checkpointGraphics();error="Native provider accepted a destroyed target";}
        catch(const mnm::render::GraphicsFailure& e){error=QString::fromUtf8(e.what());graphics=true;}
        catch(const std::exception& e){error=QString::fromUtf8(e.what());}
        probe_.loss(error,graphics);observed_=true;
        // Recreate only a deletion target. The caller must terminally retire
        // this session before returning to command polling or presentation.
        restored_=commands.restoreGraphicsTargetForDeletion();
        require(restored_,"GUI platform target could not be restored for deletion");
        return error;
    }
    void commandCleanup(const mnm::render::CommandResult* result){
        if(!observed_)return;
        require(result,"Missing GUI command cleanup result");surfaces_=result->liveSurfaces;storage_=result->stats.surfaces;
    }
    void retired(bool commandOwner,bool worldOwner,bool frame){probe_.retired(restored_,surfaces_,storage_,commandOwner,worldOwner,frame);}
private:
    static void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
    SurfaceLossProbe probe_;bool observed_=false,restored_=false;quint64 surfaces_=UINT64_MAX,storage_=UINT64_MAX;
};
