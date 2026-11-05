#pragma once
// Opt-in test adapter. No renderer policy or original byte/lease authority.
#include "../../renderer/commands.hpp"
#include "../../protocols/include/mnm/render_command_ring.h"
#include "../../protocols/include/mnm/native_cpu_reader_journal_v1.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSaveFile>
#include <QThread>
#include <QtEndian>
#include <cstring>
#include <atomic>
#include <stdexcept>

class PendingCpuFaultTest final {
public:
    PendingCpuFaultTest(QString path,quint32 identity):path_(std::move(path)),identity_(identity),guiThread_(QCoreApplication::instance()->thread()){}
    bool arm(){bool expected=false;if(!armedOnce_.compare_exchange_strong(expected,true))return false;armed_.store(true,std::memory_order_release);return true;}
    void observe(const mnm::render::SurfaceCommand& command,const mnm::render::CommandResult& result){
        if(!armed_.exchange(false,std::memory_order_acq_rel))return;
        require(QThread::currentThread()!=guiThread_,"Pending read fault must run on the worker");
        const auto& w=command.words;
        require(command.version==5&&command.operation==21&&w[1]&&!w[2]&&w[3]==identity_,"Pending read command identity");
        QFile journal(QFileInfo(path_).dir().filePath("lock-capture/cpu-reader-journal-v1.bin"));
        require(journal.open(QIODevice::ReadOnly)&&journal.size()==MNM_CRJ_BYTES,"Original request journal unavailable");
        auto* journalMap=journal.map(0,journal.size());require(journalMap,"Original request journal mapping");
        // Keep the map scoped through all refusal paths, including mismatched rows.
        struct Unmap {QFile& f;uchar* p;~Unmap(){f.unmap(p);}} unmap{journal,journalMap};
        auto* h=reinterpret_cast<uint32_t*>(journalMap);
        require(!memcmp(journalMap,"MNMCRJ01",8)&&h[2]==1&&h[3]==128&&h[4]==128&&h[5]==MNM_CRJ_CAPACITY&&h[11]==identity_,"Original request journal identity");
        QJsonArray original;QElapsedTimer clock;clock.start();uint32_t next=0;
        while(original.isEmpty()&&clock.elapsed()<1000){
            const auto count=mnm_ring_load(h+7);require(count<=MNM_CRJ_CAPACITY&&!mnm_ring_load(h+8)&&!mnm_ring_load(h+9),"Original request journal loss");
            while(next<count){
                auto* r=h+32+(next++)*32;require(mnm_ring_load(r)==next,"Original request row not committed");
                if(r[1]!=MNM_CRJ_REQUEST||r[18]!=w[1])continue;
                const uint32_t fields[]={r[15],r[18],0,r[4],r[19],r[20],r[21],r[22],r[23],r[24]};
                for(unsigned i=0;i<10;++i)require(fields[i]==w[i],"Original and decoded CPU requests differ");
                for(unsigned i=0;i<32;++i)original.append(qint64(r[i]));
            }
            if(original.isEmpty())QThread::msleep(1);
        }
        require(!original.isEmpty(),"Original pending CPU request not witnessed");
        QFile channel(path_);require(channel.open(QIODevice::ReadWrite)&&channel.size()==MNM_RENDER_COMMANDS_V2_SIZE,"Pending read channel unavailable");
        auto* map=channel.map(0,channel.size());require(map,"Pending read channel mapping");Unmap channelUnmap{channel,map};
        auto* words=reinterpret_cast<uint32_t*>(map);
        require(mnm_ring_identity(words,identity_)&&words[2]==3&&mnm_ring_load(words+6)==1&&!mnm_ring_load(words+8),"Pending read producer is not active V3");
        const auto oldReply=replyHash();
        QJsonArray request;for(unsigned i=0;i<10;++i)request.append(qint64(w[i]));
        QJsonObject observation{{"schema",1},{"command_stream_version",5},{"transport_version",3},{"worker_thread",true},
            {"sequence",qint64(command.sequence)},{"request",request},{"original_request",original},
            {"readbacks_before",QString::number(result.stats.nativeReadbacks)},{"materializations_before",QString::number(result.cpuMaterializations)},
            {"reply_sha256_before",oldReply},{"journal_wait_ms",clock.elapsed()},{"reason",int(MNM_RENDER_COMMANDS_V2_REASON_INVALID)}};
        const auto waitPath=QFileInfo(path_).dir().filePath("game/native-cpu-reader-wait.bin");
        const auto refusalPath=QFileInfo(path_).dir().filePath("game/native-cpu-transfer-refusal.bin");
        require(!QFileInfo::exists(waitPath)&&!QFileInfo::exists(refusalPath),"Old original failure diagnostic present");
        mnm_ring_writer fault{};fault.map=words;fault.session=identity_;fault.version=3;
        mnm_ring_fail(&fault,MNM_RENDER_COMMANDS_V2_REASON_INVALID);
        require(mnm_ring_load(words+6)==3,"Pending read fault injection refused");
        observation["injected"]=true;
        // The original records its failed wait before returning to the host's
        // terminal path. This bounded worker-only dwell cannot publish a reply.
        QJsonArray originalWait,originalRefusal;clock.restart();
        while(originalRefusal.isEmpty()&&clock.elapsed()<1000){
            QFile f(waitPath);if(originalWait.isEmpty()&&f.open(QIODevice::ReadOnly)){const auto bytes=f.readAll();if(bytes.size()==64){
                auto* d=reinterpret_cast<const uchar*>(bytes.constData());
                for(unsigned i=0;i<16;++i)originalWait.append(qint64(qFromLittleEndian<quint32>(d+i*4)));
            }}
            QFile refused(refusalPath);if(refused.open(QIODevice::ReadOnly)){const auto bytes=refused.readAll();if(bytes.size()==32){
                for(unsigned i=0;i<8;++i)originalRefusal.append(qint64(qFromLittleEndian<quint32>(bytes.constData()+i*4)));
            }}
            if(originalRefusal.isEmpty())QThread::msleep(1);
        }
        observation["original_refusal"]=originalRefusal;observation["original_wait"]=originalWait;observation["original_wait_ms"]=clock.elapsed();
        save(observation);observed_.store(true,std::memory_order_release);
    }
    void finish(const mnm::render::CommandResult* result,std::uint64_t pending){
        if(!observed_.load(std::memory_order_acquire))return;
        QFile f(recordPath());require(f.open(QIODevice::ReadOnly),"Pending read fault record unavailable");auto value=QJsonDocument::fromJson(f.readAll()).object();f.close();
        require(result&&result->liveSurfaces==0&&!pending,"Pending CPU wait/surfaces did not retire");
        require(QString::number(result->stats.nativeReadbacks)==value["readbacks_before"].toString()&&QString::number(result->cpuMaterializations)==value["materializations_before"].toString()&&replyHash()==value["reply_sha256_before"].toString(),"Terminal CPU read published or read back");
        value["no_new_reply"]=true;value["no_new_readback"]=true;value["pending_cleared"]=true;value["remaining_surfaces"]=0;value["finalized"]=true;save(value);
    }
    QString recordPath() const{return QFileInfo(path_).dir().filePath("pending-cpu-fault.json");}
private:
    static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
    QString replyHash() const{QFile f(path_+".cpu-reply");if(!f.exists())return {};require(f.size()<=64+2048*2048*2&&f.open(QIODevice::ReadOnly),"CPU reply unavailable or oversized");return QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex();}
    void save(const QJsonObject& value) const{QSaveFile f(recordPath());const auto bytes=QJsonDocument(value).toJson();require(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size()&&f.commit(),"Pending CPU fault record publication");}
    const QString path_;const quint32 identity_;QThread* const guiThread_;
    std::atomic<bool> armedOnce_{false},armed_{false},observed_{false};
};
