#include "live_world_session.hpp"
#include "resource-fixtures.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <QElapsedTimer>
#include <QTimer>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstring>
#include <iostream>
using namespace resource_test;
static void put(QByteArray& b,qsizetype at,quint32 v){for(unsigned i=0;i<4;++i)b[at+i]=char(v>>(8*i));}
static QByteArray wire(const Bytes& sprite,quint32 sequence,unsigned width=5){
    QByteArray encoded(reinterpret_cast<const char*>(sprite.data()+804),53);put(encoded,28,0);
    QByteArray b(144,0);std::memcpy(b.data(),"MNMWRLD1",8);put(b,8,1);put(b,12,80);put(b,20,sequence);put(b,24,width);put(b,28,3);put(b,32,width);put(b,36,1);put(b,44,MNM_WORLD_BUILD);
    put(b,80,629);put(b,88,2);put(b,104,width);put(b,108,3);put(b,112,53);put(b,116,1);put(b,120,512);
    b.append(encoded);for(unsigned i=0;i<256;++i){b.append(char(i));b.append(char(i>>8));}put(b,16,quint32(b.size()));return b;
}
static bool advance(LiveWorldSession& session,unsigned presentations){
    QElapsedTimer timer;timer.start();
    while(timer.elapsed()<10000){
        if(!session.poll())return false;
        if(session.presentations()>=presentations)return true;
        QApplication::processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    throw std::runtime_error("World preparation timed out");
}
int main(int argc,char** argv)try{
    QApplication app(argc,argv);QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QTemporaryDir directory;const auto root=std::filesystem::path(directory.path().toStdString());const auto indexed=sprite(true);write(root,"body.spr",indexed);
    GlViewport viewport;viewport.resize(100,100);viewport.show();for(unsigned i=0;i<1000&&!viewport.ready();++i)app.processEvents();require(viewport.ready(),"Shared viewport did not initialize");
    for(bool verify:{false,true}){
        const auto path=directory.filePath(verify?"verify":"normal");mnm::legacy::WorldChannel::create(path,1,verify);
        QFile file(path);require(file.open(QIODevice::ReadWrite),"World test map");auto* mapping=file.map(0,MNM_WCH_SIZE);auto* slot=reinterpret_cast<quint32*>(mapping+MNM_WCH_HEADER);
        auto publish=[&](quint32 sequence,unsigned width,bool mismatch=false){auto b=wire(indexed,sequence,width);require(mnm_wch_cas(slot,0,1),"Test publication ownership");slot[1]=sequence;slot[2]=quint32(b.size());slot[3]=verify?width*3*2:0;
            std::memcpy(reinterpret_cast<char*>(slot)+16,b.constData(),std::size_t(b.size()));
            if(verify){auto* oracle=reinterpret_cast<char*>(slot)+16+MNM_WORLD_MAX_BYTES;std::memset(oracle,0,width*3*2);oracle[(width+2)*2]=1;if(mismatch)oracle[0]=1;}mnm_wch_store(slot,2);};
        {LiveWorldSession session(viewport,path,directory.path());publish(1,5);require(session.poll()&&session.hasPendingFrame()&&session.presentations()==0,"Partial World was presented");require(advance(session,1)&&!session.hasPendingFrame()&&session.presentations()==1,"Live complete World presentation failed");
            app.processEvents();require(viewport.frameSize()==QSize(5,3)&&viewport.imageUploads()==0,"GPU-only presentation differs");
            publish(2,7);require(advance(session,2)&&session.presentations()==2&&viewport.frameSize()==QSize(7,3),"World resize ownership failed");app.processEvents();
            if(verify){publish(3,7,true);require(session.poll()&&!advance(session,3)&&!session.error().isEmpty()&&viewport.frameSize().isEmpty(),"Mismatch was presented or stale lease retained");}
            else{publish(3,7);reinterpret_cast<char*>(slot)[16+84]=6;require(!advance(session,3)&&viewport.frameSize().isEmpty(),"Unknown operation was presented");}
            session.close();const auto report=session.report();require(report["remaining_surfaces"].toInt()==0&&report["native_readbacks"].toInt()==(verify?3:0),"Session readback/lifetime boundary differs");
            require(*reinterpret_cast<quint32*>(mapping+24)==1,"Session failure did not cancel producer");}
        file.unmap(mapping);
    }
    for(bool verify:{false,true}){
        const auto path=directory.filePath(verify?"verify-refusal":"normal-refusal");mnm::legacy::WorldChannel::create(path,2,verify);
        QFile file(path);require(file.open(QIODevice::ReadWrite),"Refusal test map");auto* mapping=file.map(0,MNM_WCH_SIZE);
        auto* slot=reinterpret_cast<quint32*>(mapping+MNM_WCH_HEADER);mnm_wch_store(reinterpret_cast<quint32*>(mapping+20),MNM_WCH_ACTIVE);
        auto publish=[&](quint32 sequence,quint32 failure){
            auto b=wire(indexed,sequence);
            if(failure){b.resize(MNM_WORLD_HEADER);put(b,16,MNM_WORLD_HEADER);put(b,36,0);put(b,40,failure);}
            require(mnm_wch_cas(slot,0,1),"Refusal publication ownership");slot[1]=sequence;slot[2]=quint32(b.size());slot[3]=0;
            std::memcpy(reinterpret_cast<char*>(slot)+16,b.constData(),std::size_t(b.size()));mnm_wch_store(slot,2);
        };
        {LiveWorldSession session(viewport,path,directory.path());const auto initialStatus=session.status();publish(1,MNM_WORLD_UNSUPPORTED_KIND);
            if(verify){require(!session.poll()&&!session.error().isEmpty()&&session.presentations()==0&&viewport.frameSize().isEmpty(),"Verification accepted a capability refusal");}
            else{
                require(session.poll()&&session.presentations()==0&&viewport.frameSize().isEmpty(),"Normal initial refusal ended the session or was presented");
                require(*reinterpret_cast<quint32*>(mapping+24)==0&&session.report()["capture_refusals"].toInteger()==1,"Normal refusal cancelled the producer or was not counted");
                require(!session.status().isEmpty()&&session.status()!=initialStatus,"Refusal has no waiting status");
                publish(2,0);require(advance(session,1)&&session.presentations()==1&&viewport.frameSize()==QSize(5,3),"Native presentation did not recover after initial refusal");app.processEvents();
                publish(3,MNM_WORLD_UNSUPPORTED_WAVE);require(session.poll()&&viewport.frameSize().isEmpty(),"Wave refusal retained a stale viewport lease");app.processEvents();
                for(quint32 i=4;i<=103;++i){publish(i,MNM_WORLD_UNSUPPORTED_KIND);require(session.poll(),"Repeated capability refusal ended the session");}
                auto report=session.report();require(report["capture_refusals"].toInteger()==102&&report["refusals"].toArray().size()==64,"Refusal diagnostics are unbounded or lost counts");
                const auto counts=report["refusal_counts"].toObject();require(counts["7"].toInteger()==1&&counts["8"].toInteger()==101,"Refusal reason counts differ");
                require(report["native_readbacks"].toInteger()==0&&viewport.imageUploads()==0,"Normal refusal/recovery read back or uploaded original pixels");
                publish(104,0);require(advance(session,2)&&session.presentations()==2,"Native presentation did not recover after repeated refusals");
                publish(105,MNM_WORLD_UNSUPPORTED_KIND);reinterpret_cast<quint32*>(reinterpret_cast<char*>(slot)+16)[9]=1;
                require(!session.poll()&&!session.error().isEmpty()&&viewport.frameSize().isEmpty(),"Partial refusal packet was accepted or left a stale lease");
            }
            session.close();const auto report=session.report();require(report["remaining_surfaces"].toInteger()==0&&report["native_readbacks"].toInteger()==0,"Refusal session leaked surfaces or read pixels");
        }
        file.unmap(mapping);
    }
    for(bool verify:{false,true}){
        const auto path=directory.filePath(verify?"history-verify":"history-normal");mnm::legacy::WorldChannel::createHistory(path,21,verify,4);
        QFile file(path);require(file.open(QIODevice::ReadWrite),"History session map");auto* mapping=file.map(0,MNM_WCH_HISTORY_SIZE(4));
        for(unsigned i=0;i<4;++i){const unsigned width=i==3?7:5;auto b=wire(indexed,i+1,width);if(i)put(b,88,100);
            auto* slot=reinterpret_cast<quint32*>(mapping+MNM_WCH_HEADER+i*MNM_WCH_HISTORY_SLOT);slot[1]=i+1;slot[2]=b.size();slot[3]=verify?width*3*2:0;slot[4]=i<2?1:i;slot[5]=i+1;slot[6]=i==1?0:1;
            std::memcpy(reinterpret_cast<char*>(slot)+32,b.constData(),std::size_t(b.size()));
            if(verify){auto* oracle=reinterpret_cast<char*>(slot)+32+MNM_WORLD_MAX_BYTES;std::memset(oracle,0,width*3*2);if(i<2)oracle[(width+2)*2]=1;}mnm_wch_store(slot,2);}
        mnm_wch_store(reinterpret_cast<quint32*>(mapping+20),MNM_WCH_ENDED);
        {LiveWorldSession session(viewport,path,directory.path());require(!session.ended(),"Ended producer discarded journal");
            require(advance(session,1)&&!session.ended(),"First history frame did not drain progressively");require(advance(session,4)&&session.ended(),"History final packet failed to drain");
            const auto report=session.report();const auto records=report["frames"].toArray();require(report["history"].toBool()&&report["superseded"].toInteger()==0&&records.size()==4,"History diagnostics lost frames");
            require(!records[1].toObject()["native_zero_reset"].toBool()&&records[2].toObject()["native_zero_reset"].toBool()&&viewport.frameSize()==QSize(7,3),"Retain/reset/resize metadata differs");
            require(report["mismatches"].toInteger()==0&&report["native_readbacks"].toInteger()==(verify?4:0),"History pixels or readback boundary differs");session.close();require(session.report()["remaining_surfaces"].toInteger()==0,"History cleanup leaked surfaces");}
        file.unmap(mapping);
    }
    // Hold actual cold resource preparation on the worker while GUI timers and
    // session polls continue. Cancellation must not join this blocked work.
    struct Gate {std::atomic<unsigned> calls{0};std::atomic<bool> entered{false},release{false};};
    auto gate=std::make_shared<Gate>();
    const auto slowPath=directory.filePath("slow-preparation");mnm::legacy::WorldChannel::create(slowPath,3,false);
    QFile slowFile(slowPath);require(slowFile.open(QIODevice::ReadWrite),"Slow preparation map");auto* slowMap=slowFile.map(0,MNM_WCH_SIZE);
    auto* slowSlot=reinterpret_cast<quint32*>(slowMap+MNM_WCH_HEADER);const auto slowWire=wire(indexed,1);
    require(mnm_wch_cas(slowSlot,0,1),"Slow preparation ownership");slowSlot[1]=1;slowSlot[2]=quint32(slowWire.size());slowSlot[3]=0;
    std::memcpy(reinterpret_cast<char*>(slowSlot)+16,slowWire.constData(),std::size_t(slowWire.size()));mnm_wch_store(slowSlot,2);
    {LiveWorldSession session(viewport,slowPath,directory.path(),{},[gate](const std::atomic<bool>& stop){
        if(gate->calls.fetch_add(1)==0)return;
        gate->entered=true;while(!gate->release&&!stop)std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
        QElapsedTimer deadline;deadline.start();
        while(!gate->entered&&deadline.elapsed()<10000){require(session.poll(),"Cold preparation failed");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        require(gate->entered&&session.presentations()==0,"Cold preparation did not reach the worker gate");
        unsigned heartbeats=0;QTimer heartbeat;heartbeat.setInterval(1);QObject::connect(&heartbeat,&QTimer::timeout,[&]{++heartbeats;});heartbeat.start();
        deadline.restart();while(deadline.elapsed()<50){require(session.poll(),"Waiting for slow preparation failed");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}heartbeat.stop();
        require(heartbeats>=5&&session.presentations()==0,"Slow asset preparation blocked GUI events or presented partial work");
        deadline.restart();session.close();require(deadline.elapsed()<100,"Close joined slow asset preparation");gate->release=true;
        for(unsigned i=0;i<10;++i)app.processEvents();
        require(!session.poll()&&viewport.frameSize().isEmpty()&&session.report()["remaining_surfaces"].toInteger()==0,"Late preparation revived a closed session");
        std::cout<<"Slow preparation heartbeat: "<<heartbeats<<"; close_ms: "<<deadline.elapsed()<<'\n';
    }
    slowFile.unmap(slowMap);
    // Remove the input after worker preparation, before GUI adoption. Rendering
    // must consume the exact owned decode and never reopen the file.
    const auto ownedPath=directory.filePath("owned-preparation");mnm::legacy::WorldChannel::create(ownedPath,4,false);
    QFile ownedFile(ownedPath);require(ownedFile.open(QIODevice::ReadWrite),"Owned preparation map");auto* ownedMap=ownedFile.map(0,MNM_WCH_SIZE);auto* ownedSlot=reinterpret_cast<quint32*>(ownedMap+MNM_WCH_HEADER);
    auto ownedGate=std::make_shared<Gate>();
    {LiveWorldSession session(viewport,ownedPath,directory.path(),{},[ownedGate](const std::atomic<bool>& stop){
        if(ownedGate->calls.fetch_add(1)==0)return;
        ownedGate->entered=true;while(!ownedGate->release&&!stop)std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
        require(mnm_wch_cas(ownedSlot,0,1),"Owned preparation ownership");ownedSlot[1]=1;ownedSlot[2]=quint32(slowWire.size());ownedSlot[3]=0;
        std::memcpy(reinterpret_cast<char*>(ownedSlot)+16,slowWire.constData(),std::size_t(slowWire.size()));mnm_wch_store(ownedSlot,2);
        QElapsedTimer deadline;deadline.start();while(!ownedGate->entered&&deadline.elapsed()<10000){
            require(session.poll(),"Owned resource preparation failed");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        require(ownedGate->entered,"Owned resource did not reach preparation");
        mnm_wch_store(reinterpret_cast<quint32*>(ownedMap+20),MNM_WCH_ENDED);
        require(!session.ended(),"Ended producer discarded pending CPU preparation");ownedGate->release=true;
        while(session.report()["resources_prepared"].toInteger()==0&&deadline.elapsed()<10000){app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        require(session.report()["resources_prepared"].toInteger()==1&&session.presentations()==0,"Prepared resource completed without an adoption boundary");
        std::filesystem::remove(root/"body.spr");require(advance(session,1),"GUI adoption or rendering reopened prepared input");
        require(session.ended(),"Completed final packet did not retire ended producer");
        require(session.report()["resources_prepared"].toInteger()==1&&session.report()["native_readbacks"].toInteger()==0,"Owned preparation repeated decoding or read back pixels");session.close();
    }
    ownedFile.unmap(ownedMap);
    write(root,"body.spr",indexed);
    // Hold a new sprite frame's plane preparation while the previous complete
    // GPU lease stays displayed, then close without joining the worker.
    const auto uploadPath=directory.filePath("slow-upload");mnm::legacy::WorldChannel::create(uploadPath,5,false);
    QFile uploadFile(uploadPath);require(uploadFile.open(QIODevice::ReadWrite),"Upload map");auto* uploadMap=uploadFile.map(0,MNM_WCH_SIZE);auto* uploadSlot=reinterpret_cast<quint32*>(uploadMap+MNM_WCH_HEADER);
    auto uploadGate=std::make_shared<Gate>();
    {LiveWorldSession session(viewport,uploadPath,directory.path(),{},[uploadGate](const std::atomic<bool>& stop){
        if(uploadGate->calls.fetch_add(1)<3)return;
        uploadGate->entered=true;while(!uploadGate->release&&!stop)std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
        auto publish=[&](unsigned sequence,bool change){auto b=wire(indexed,sequence);if(change)b[144+52]=char(2);
            require(mnm_wch_cas(uploadSlot,0,1),"Upload ownership");uploadSlot[1]=sequence;uploadSlot[2]=quint32(b.size());uploadSlot[3]=0;
            std::memcpy(reinterpret_cast<char*>(uploadSlot)+16,b.constData(),std::size_t(b.size()));mnm_wch_store(uploadSlot,2);};
        publish(1,false);require(advance(session,1),"Initial upload did not complete");publish(2,true);
        QElapsedTimer deadline;deadline.start();while(!uploadGate->entered&&deadline.elapsed()<10000){require(session.poll(),"Warm upload failed");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        require(uploadGate->entered,"Plane preparation checkpoint absent");unsigned heartbeats=0;QTimer heartbeat;QObject::connect(&heartbeat,&QTimer::timeout,[&]{++heartbeats;});heartbeat.start(1);deadline.restart();
        while(deadline.elapsed()<50){require(session.poll(),"Blocked plane poll failed");app.processEvents();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        heartbeat.stop();require(heartbeats>=5&&session.presentations()==1&&viewport.frameSize()==QSize(5,3),"Plane preparation blocked GUI or replaced previous complete lease");
        require(session.report()["max_tick_upload_bytes"].toInteger()<=256*1024,"Live upload exceeded byte budget");deadline.restart();session.close();require(deadline.elapsed()<100&&viewport.frameSize().isEmpty(),"Upload cancellation joined worker or retained lease");
        require(session.report()["remaining_surfaces"].toInteger()==0,"Upload cancellation leaked GPU surfaces");uploadGate->release=true;
        std::cout<<"Held upload preparation: "<<heartbeats<<" GUI heartbeats/50ms; close "<<deadline.elapsed()<<"ms\n";
    }
    uploadFile.unmap(uploadMap);
    std::cout<<"Continuous Qt GPU lease, no-readback, resize, mismatch, normal capability refusal/recovery, strict verification, bounded diagnostics, malformed refusal, async preparation and cleanup pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
