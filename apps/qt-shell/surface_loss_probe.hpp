#pragma once
// Explicit experiment adapter. It owns no game bytes, wire cursors or widgets.
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QThread>
#include <chrono>
#include <stdexcept>

class SurfaceLossProbe final {
public:
    SurfaceLossProbe(const QString& channel,quint32 producer,quint32 version)
        :directory_(QFileInfo(channel).dir()),producer_(producer),version_(version){}
    static qint64 now(){return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
    bool requested(){
        if(consumed_)return false;
        QFile f(directory_.filePath("surface-loss-request-v1.json"));
        if(!f.exists())return false;
        consumed_=true;
        require(f.size()>0 && f.size()<=4096 && f.open(QIODevice::ReadOnly),"Surface-loss request unavailable or oversize");
        const auto raw=f.readAll();QJsonParseError error;const auto doc=QJsonDocument::fromJson(raw,&error);
        require(error.error==QJsonParseError::NoError && doc.isObject(),"Malformed surface-loss request");
        const auto request=doc.object();const auto keys=request.keys();
        require(QSet<QString>(keys.begin(),keys.end())==QSet<QString>{"schema","producer","transport_version","request_id","action"},"Unknown surface-loss request fields");
        require(request["schema"].toInt()==1 && request["producer"].toDouble()==producer_ &&
            request["transport_version"].toInt()==int(version_) && version_==3 &&
            request["action"].toString()=="destroy-render-target", "Surface-loss request identity/action mismatch");
        const auto id=request["request_id"].toString().toLatin1();
        require(id.size()==32 && QByteArray::fromHex(id).size()==16 && QByteArray::fromHex(id).toHex()==id,"Invalid surface-loss request nonce");
        observation={{"schema",1},{"kind","native_offscreen_target_loss"},{"producer",qint64(producer_)},
            {"transport_version",int(version_)},{"request_id",QString::fromLatin1(id)},
            {"request_sha256",QString::fromLatin1(QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex())},
            {"begin_host_us",QString::number(now())},{"injected_diagnostic",true},{"physical_hardware_reset",false},
            {"old_generation_continuation",false}};
        return true;
    }
    void loss(const QString& error,bool graphics){
        observation["end_host_us"]=QString::number(now());
        observation["error"]=error;observation["graphics_failure"]=graphics;
        write("surface-loss-observation-v1.json",observation);observed_=true;
    }
    void retired(bool restored,quint64 surfaces,quint64 storage,bool commandOwner,bool worldOwner,bool remainingFrames){
        if(!observed_)return;
        auto final=observation;final["retirement_host_us"]=QString::number(now());
        final["target_restored_for_deletion"]=restored;
        final["command_surfaces_after_cleanup"]=QString::number(surfaces);
        final["renderer_surfaces_after_cleanup"]=QString::number(storage);
        final["command_owner_retired"]=!commandOwner;final["world_owner_retired"]=!worldOwner;
        final["frame_leases_retired"]=!remainingFrames;
        final["worker_teardown_complete"]=true;final["normal_completion_claimed"]=false;
        write("surface-loss-retirement-v1.json",final);
    }
    QJsonObject observation;
private:
    static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
    void write(const char* name,const QJsonObject& value){
        QFile f(directory_.filePath(QString::fromLatin1(name)));const auto bytes=QJsonDocument(value).toJson();
        require(f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(bytes)==bytes.size()&&f.flush(),"Cannot retain unique surface-loss observation");
    }
    QDir directory_;quint32 producer_,version_;bool consumed_=false,observed_=false;
};
