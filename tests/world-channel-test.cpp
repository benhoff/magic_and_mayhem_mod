#include "world_channel.hpp"
#include "resource-fixtures.hpp"
#include <QCoreApplication>
#include <atomic>
#include <thread>
#include <cstring>
#include <iostream>
using namespace mnm::legacy;
using resource_test::require;
using resource_test::rejected;
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);QTemporaryDir dir;require(dir.isValid(),"Channel fixture root");
    const auto path=dir.filePath("channel");WorldChannel::create(path,7,false);rejected([&]{WorldChannel::create(path,7,false);});
    QFile file(path);require(file.open(QIODevice::ReadWrite),"Channel fixture map");auto* mapping=file.map(0,MNM_WCH_SIZE);require(mapping,"Channel fixture mapping");
    auto word=[&](quint32 at){return reinterpret_cast<quint32*>(mapping+at);};
    auto publish=[&](unsigned slot,quint32 sequence){auto* p=word(MNM_WCH_HEADER+slot*MNM_WCH_SLOT);
        require(mnm_wch_cas(p,0,1),"Producer overwrote owned slot");p[1]=sequence;p[2]=80;p[3]=0;
        std::memset(reinterpret_cast<char*>(p)+16,int(sequence&255),80);mnm_wch_store(p,2);};
    {
        WorldChannel channel(path);require(!channel.poll(),"Unpublished frame admitted");
        auto* first=word(MNM_WCH_HEADER);mnm_wch_store(first,1);require(!channel.poll(),"WRITING frame admitted");mnm_wch_store(first,0);
        publish(0,1);publish(1,2);auto packet=channel.poll();require(packet&&packet->sequence==2&&packet->inputs==QByteArray(80,2),"Latest closed publication differs");
        require(channel.superseded()==1&&mnm_wch_load(first)==0,"Older READY slot not released");channel.presented(2);require(*word(48)==2,"Presentation acknowledgement missing");rejected([&]{channel.presented(1);});
        std::atomic<bool> done{false};std::thread producer([&]{for(quint32 sequence=3;sequence<2003;++sequence){
            for(;;){bool written=false;for(unsigned slot=0;slot<2;++slot){auto* p=word(MNM_WCH_HEADER+slot*MNM_WCH_SLOT);
                if(mnm_wch_cas(p,0,1)){p[1]=sequence;p[2]=80;p[3]=0;std::memset(reinterpret_cast<char*>(p)+16,int(sequence&255),80);mnm_wch_store(p,2);written=true;break;}}
                if(written)break;
                std::this_thread::yield();}}
            done=true;});
        unsigned consumed=0;quint32 previous=2;
        while(!done||mnm_wch_load(first)==2||mnm_wch_load(word(MNM_WCH_HEADER+MNM_WCH_SLOT))==2){
            if(auto p=channel.poll()){require(p->sequence>previous&&p->inputs==QByteArray(80,char(p->sequence&255)),"Torn/regressing concurrent frame");previous=p->sequence;++consumed;}
            else std::this_thread::yield();}
        producer.join();require(consumed&&previous==2002,"Concurrent stream did not drain");
        publish(0,2003);first[2]=MNM_WORLD_MAX_BYTES+1;rejected([&]{channel.poll();});require(*word(24)==1&&mnm_wch_load(first)==0,"Malformed packet did not cancel/release");
    }
    *word(24)=0;*word(20)=MNM_WCH_FAILED;
    {WorldChannel channel(path);rejected([&]{channel.poll();});}
    *word(20)=0;*word(24)=0;
    {WorldChannel channel(path);*word(16)=8;rejected([&]{channel.poll();});*word(16)=7;}
    // Ordered journal must drain queued packets after producer ENDED.
    for(unsigned scenario=0;scenario<7;++scenario){
        const auto historyPath=dir.filePath(QString("history-%1").arg(scenario));WorldChannel::createHistory(historyPath,9,false,3);
        QFile journal(historyPath);require(journal.open(QIODevice::ReadWrite),"History fixture map");auto* bytes=journal.map(0,MNM_WCH_HISTORY_SIZE(3));
        auto header=reinterpret_cast<quint32*>(bytes);
        auto publishHistory=[&](unsigned index,unsigned sequence,unsigned canvas,unsigned source,unsigned reset){auto* p=reinterpret_cast<quint32*>(bytes+MNM_WCH_HEADER+index*MNM_WCH_HISTORY_SLOT);
            require(mnm_wch_cas(p,0,1),"History slot overwrite");p[1]=sequence;p[2]=80;p[3]=0;p[4]=canvas;p[5]=source;p[6]=reset;p[7]=0;std::memset(reinterpret_cast<char*>(p)+32,0,80);mnm_wch_store(p,2);};
        {WorldChannel channel(historyPath);require(channel.history()&&channel.targetFrames()==3&&!channel.drained(),"History mode identity differs");
            if(scenario==0){publishHistory(0,1,1,1,1);publishHistory(1,2,1,2,0);publishHistory(2,3,2,3,1);mnm_wch_store(header+5,MNM_WCH_ENDED);
                for(unsigned i=1;i<=3;++i){auto packet=channel.poll();require(packet&&packet->sequence==i&&packet->sourceSequence==i&&packet->reset==(i!=2),"History superseded or lost reset metadata");channel.presented(i);require(channel.drained()==(i==3),"History ended before draining journal");}
                require(channel.superseded()==0&&!channel.poll(),"History used latest-frame policy");
            }else{
                if(scenario==1)publishHistory(0,1,1,2,1);
                if(scenario==2)publishHistory(1,2,1,2,1);
                if(scenario==3)publishHistory(0,1,1,1,0);
                if(scenario==4){publishHistory(0,1,1,1,1);require(channel.poll().has_value(),"Initial history packet absent");publishHistory(1,2,2,2,0);}
                if(scenario==5)mnm_wch_store(header+5,MNM_WCH_ENDED);
                if(scenario==6){publishHistory(0,1,1,1,1);reinterpret_cast<quint32*>(bytes+MNM_WCH_HEADER)[7]=1;}
                rejected([&]{channel.poll();});
            }
        }
        journal.unmap(bytes);
    }
    file.unmap(mapping);std::cout<<"World ownership, superseding, concurrent publication, malformed extent, identity, failure and cancellation pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
