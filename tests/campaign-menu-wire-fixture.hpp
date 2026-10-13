#pragma once
#include "character-wire-fixture.hpp"
#include "../protocols/include/mnm/menu_v20.h"
// Synthetic engine peer only. No game process, progress grants or save writes.
class CampaignMenuWireFixture final {
public:
    CampaignMenuWireFixture(){
        bridge_=std::make_unique<MenuBridge>();
        check(directory_.isValid()&&bridge_->create(directory_.filePath("campaign.bin"),true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true),"V20 channel");
        file_.setFileName(directory_.filePath("campaign.bin"));check(file_.open(QIODevice::ReadWrite)&&file_.size()==MNM_MENU_V20_SIZE,"V20 peer");
        bytes_=file_.map(0,file_.size());check(bytes_,"V20 mapping");put(148,MNM_MENU_OK);put(152,9);region();
    }
    ~CampaignMenuWireFixture(){if(bytes_)file_.unmap(bytes_);}
    void put(int at,quint32 value){qToLittleEndian(value,bytes_+at);}
    quint32 word(int at)const{return qFromLittleEndian<quint32>(bytes_+at);}
    void text(int at,const QByteArray& value){check(value.size()<128,"fixture text bound");std::memset(bytes_+at,0,128);std::memcpy(bytes_+at,value.constData(),value.size());}
    void publish(){put(132,++generation_);put(128,word(128)+2);}
    void region(const QByteArray& realm="Greek",quint32 id=3,bool different=true,quint32 actions=31,quint32 handoff=4){
        std::memset(bytes_+MNM_MENU_V7_REGION,0,MNM_MENU_V7_REGION_SIZE);put(136,18);put(140,1);
        const int at=MNM_MENU_V7_REGION;const std::array<quint32,8> header{4,4,quint32(different),1,15,actions,id,(actions&2)?handoff:0};
        for(int i=0;i<8;++i)put(at+4*i,header[i]);
        text(at+32,realm);text(at+160,"Original supplied region");publish();
    }
    void realm(){
        put(136,4);put(140,1);put(156,0);const int at=MNM_MENU_V19_REALM;
        std::memset(bytes_+at,0,MNM_MENU_V19_REALM_SIZE);
        const std::array<quint32,10> header{3,3,5,2,3,0,1,UINT32_MAX,14,7};for(int i=0;i<10;++i)put(at+4*i,header[i]);
        for(int i=0;i<3;++i){const int row=at+48+i*24;put(row,quint32(i+1));put(row+4,i?1:0);put(row+8,i?0:1);put(row+12,i?1:0);put(row+16,100+100*i);put(row+20,100);}publish();
    }
    void original(quint32 handoff=0){put(136,0);put(140,0);put(156,handoff);publish();}
    void report(quint32 outcome){
        put(136,MNM_MENU_DEFEAT_SCREEN);put(140,1);put(156,0);
        const int at=MNM_MENU_V11_DEFEAT;std::memset(bytes_+at,0,MNM_MENU_V11_DEFEAT_SIZE);
        put(at,1);put(at+4,5);put(at+8,5);put(at+12,outcome);
        std::strcpy(reinterpret_cast<char*>(bytes_+at+16),outcome?"Original victory title":"Original defeat title");
        for(int i=0;i<21;++i)std::strcpy(reinterpret_cast<char*>(bytes_+at+272+i*256),i==19?"":"Original score text");
        publish();
    }
    void acknowledge(){put(144,word(28));put(148,MNM_MENU_OK);publish();}
    MenuBridge& bridge(){return *bridge_;}
    void attach(LiveMenuSession& session){MenuBridge::State state;check(bridge_->read(state),"V20 initial read");LiveMenuSessionTestAccess::attach(session,std::move(bridge_),state);}
private:
    static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
    QTemporaryDir directory_;QFile file_;uchar* bytes_=nullptr;std::unique_ptr<MenuBridge> bridge_;quint32 generation_=0;
};
