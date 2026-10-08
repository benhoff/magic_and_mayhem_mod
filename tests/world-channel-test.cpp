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
    file.unmap(mapping);std::cout<<"World ownership, superseding, concurrent publication, malformed extent, identity, failure and cancellation pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
