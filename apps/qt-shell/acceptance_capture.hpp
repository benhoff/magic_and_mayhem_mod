#pragma once
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <atomic>
#include <chrono>
#include <cstdint>
// Opt-in diagnostic interchange stream. The external collector retains its
// verified bytes in SQLite. No oracle, expected pixel or rendering input returns.
class AcceptanceCapture final {
public:
    explicit AcceptanceCapture(const QString& role):role_(role){
        const auto directory=qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_DIRECTORY");
        if(directory.isEmpty())return;
        enabled_=true;
        // Bounded normal sessions include many more paints than the short pilot.
        if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_NORMAL")==1)storedLimit_=1024ll*1024*1024;
        static std::atomic<unsigned> identity{0};
        const auto nonce=qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_ID");
        if(nonce.isEmpty() || nonce.size()>128 || !QDir::isAbsolutePath(directory)){error_="Invalid acceptance capture identity";return;}
        if(!QDir().mkpath(directory)){error_="Acceptance capture directory";return;}
        file_.setFileName(QDir(directory).filePath(role+"-"+QString::number(++identity)+".bin"));
        if(!file_.open(QIODevice::WriteOnly|QIODevice::NewOnly)){error_="Acceptance capture already exists or cannot open";return;}
        if(file_.write("MNMACE01",8)!=8){error_="Acceptance capture header";return;}
        append("open",QJsonObject{{"role",role},{"session",nonce},{"version",1},{"stored_limit_bytes",double(storedLimit_)}},{});
    }
    ~AcceptanceCapture(){finish();}
    bool enabled() const{return enabled_;}
    void append(const QString& kind,QJsonObject metadata,const QByteArray& bytes={}){
        if(!enabled_ || !error_.isEmpty() || closed_)return;
        if(rows_>=16384 || bytes.size()>32*1024*1024 || raw_+bytes.size()>8ll*1024*1024*1024){error_="Acceptance capture raw/row budget exceeded";return;}
        metadata["host_us"]=QString::number(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        metadata["kind"]=kind;metadata["index"]=int(rows_);
        const auto record=QJsonDocument(metadata).toJson(QJsonDocument::Compact);
        const auto packed=bytes.isEmpty()?QByteArray():qCompress(bytes,3);
        if(record.size()>65536 || stored_+record.size()+packed.size()+8>storedLimit_){error_="Acceptance capture stored budget exceeded";return;}
        char h[8];qToLittleEndian<quint32>(record.size(),h);qToLittleEndian<quint32>(packed.size(),h+4);
        if(file_.write(h,8)!=8 || file_.write(record)!=record.size() || file_.write(packed)!=packed.size()){error_="Acceptance capture write";return;}
        ++rows_;raw_+=bytes.size();stored_+=record.size()+packed.size()+8;
    }
    void fail(const QString& reason){if(error_.isEmpty())error_=reason;}
    void finish(){
        if(!enabled_ || closed_)return;
        const auto failure=error_;error_.clear();
        append("close",QJsonObject{{"complete",failure.isEmpty()},{"error",failure},{"raw_bytes",QString::number(raw_)},{"prior_rows",int(rows_)}},{});
        error_=failure;closed_=true;if(file_.isOpen()){file_.flush();file_.close();}
    }
private:
    QString role_,error_;QFile file_;bool enabled_=false,closed_=false;
    unsigned rows_=0;qint64 raw_=0,stored_=8,storedLimit_=256ll*1024*1024;
};
