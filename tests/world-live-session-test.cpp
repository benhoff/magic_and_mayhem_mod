#include "live_world_session.hpp"
#include "resource-fixtures.hpp"
#include <QApplication>
#include <QSurfaceFormat>
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
        {LiveWorldSession session(viewport,path,directory.path());publish(1,5);require(session.poll()&&session.presentations()==0,"Partial World was presented");require(session.poll()&&session.presentations()==1,"Live complete World presentation failed");
            app.processEvents();require(viewport.frameSize()==QSize(5,3)&&viewport.imageUploads()==0,"GPU-only presentation differs");
            publish(2,7);require(session.poll()&&session.poll()&&session.presentations()==2&&viewport.frameSize()==QSize(7,3),"World resize ownership failed");app.processEvents();
            if(verify){publish(3,7,true);require(session.poll()&&!session.poll()&&!session.error().isEmpty()&&viewport.frameSize().isEmpty(),"Mismatch was presented or stale lease retained");}
            else{publish(3,7);reinterpret_cast<char*>(slot)[16+84]=6;require(!session.poll()&&viewport.frameSize().isEmpty(),"Unknown operation was presented");}
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
                publish(2,0);require(session.poll()&&session.poll()&&session.presentations()==1&&viewport.frameSize()==QSize(5,3),"Native presentation did not recover after initial refusal");app.processEvents();
                publish(3,MNM_WORLD_UNSUPPORTED_WAVE);require(session.poll()&&viewport.frameSize().isEmpty(),"Wave refusal retained a stale viewport lease");app.processEvents();
                for(quint32 i=4;i<=103;++i){publish(i,MNM_WORLD_UNSUPPORTED_KIND);require(session.poll(),"Repeated capability refusal ended the session");}
                auto report=session.report();require(report["capture_refusals"].toInteger()==102&&report["refusals"].toArray().size()==64,"Refusal diagnostics are unbounded or lost counts");
                const auto counts=report["refusal_counts"].toObject();require(counts["7"].toInteger()==1&&counts["8"].toInteger()==101,"Refusal reason counts differ");
                require(report["native_readbacks"].toInteger()==0&&viewport.imageUploads()==0,"Normal refusal/recovery read back or uploaded original pixels");
                publish(104,0);require(session.poll()&&session.poll()&&session.presentations()==2,"Native presentation did not recover after repeated refusals");
                publish(105,MNM_WORLD_UNSUPPORTED_KIND);reinterpret_cast<quint32*>(reinterpret_cast<char*>(slot)+16)[9]=1;
                require(!session.poll()&&!session.error().isEmpty()&&viewport.frameSize().isEmpty(),"Partial refusal packet was accepted or left a stale lease");
            }
            session.close();const auto report=session.report();require(report["remaining_surfaces"].toInteger()==0&&report["native_readbacks"].toInteger()==0,"Refusal session leaked surfaces or read pixels");
        }
        file.unmap(mapping);
    }
    std::cout<<"Continuous Qt GPU lease, no-readback, resize, mismatch, normal capability refusal/recovery, strict verification, bounded diagnostics, malformed refusal and cleanup pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
