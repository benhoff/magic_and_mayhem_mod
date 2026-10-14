#pragma once
#include "live_menu_session.hpp"
#include "../protocols/include/mnm/menu_v18.h"
#include <QTemporaryDir>
#include <QtEndian>
#include <cstring>
#include <stdexcept>

// Synthetic peer: prices/resources/progress are test inputs, separate from the
// private original PE oracle. The actual production bridge/session consumes it.
struct LiveMenuSessionTestAccess {
    static void attach(LiveMenuSession& s,std::unique_ptr<MenuBridge> bridge,const MenuBridge::State& state){
        s.bridge_=std::move(bridge);s.active_=true;s.clock_.start();
        s.state_=state;s.sequence_=state.sequence;s.lastState_=0;
    }
    static void poll(LiveMenuSession& s){s.poll();}
};
class CharacterWireFixture final {
public:
    static constexpr std::array<quint32,6> initial{40,160,4,2,2,2};
    static constexpr std::array<quint32,6> maximum{200,800,40,7,7,7};
    static constexpr std::array<quint32,6> increment{5,20,1,1,1,1};
    CharacterWireFixture(){
        bridge_=std::make_unique<MenuBridge>();
        check(directory_.isValid()&&bridge_->create(directory_.filePath("character.bin"),true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true),"fixture channel");
        file_.setFileName(directory_.filePath("character.bin"));
        check(file_.open(QIODevice::ReadWrite)&&file_.size()==MNM_MENU_V18_SIZE,"fixture mapping");
        bytes_=file_.map(0,file_.size());check(bytes_,"fixture map");begin();
    }
    ~CharacterWireFixture(){if(bytes_)file_.unmap(bytes_);}
    void attach(LiveMenuSession& session){
        MenuBridge::State state;check(bridge_->read(state),"initial state");
        LiveMenuSessionTestAccess::attach(session,std::move(bridge_),state);
    }
    void begin(quint32 parent=4){
        parent_=parent;values_=committed_;experience_=committedExperience_;++generation_;
        put(136,20);put(140,1);publish();
    }
    bool acknowledge(){
        const auto request=word(28);if(request==word(144))return false;
        check(word(136)==20&&word(36)==generation_,"fixture request generation");
        const auto action=word(32),arg=word(40);
        if(action==MNM_MENU_CHARACTER_ADJUST){
            check(arg<12,"fixture adjustment index");const int row=int(arg/2);
            check(mask()&(1u<<arg),"fixture permitted adjustment");
            if(arg&1){values_[row]+=increment[row];experience_-=11;}
            else {values_[row]-=increment[row];experience_+=11;}
        }else{
            check(!arg&&(action==MNM_MENU_CHARACTER_OK||action==MNM_MENU_CHARACTER_CANCEL),"fixture return action");
            if(action==MNM_MENU_CHARACTER_OK){committed_=values_;committedExperience_=experience_;}
            put(136,0);put(140,0);
        }
        put(144,request);++generation_;publish();return true;
    }
    quint32 word(int offset) const {return qFromLittleEndian<quint32>(bytes_+offset);}
    void put(int offset,quint32 value){qToLittleEndian(value,bytes_+offset);}
    void publish(){
        const auto sequence=word(128);put(128,sequence+1);put(132,generation_);put(148,MNM_MENU_OK);put(152,42);
        const int at=MNM_MENU_V18_CHARACTER;std::memset(bytes_+at,0,MNM_MENU_V18_CHARACTER_SIZE);
        const quint32 fields[]{parent_,parent_==4?4u:5u,5,0,experience_,3,mask(),0};
        for(int i=0;i<8;++i)put(at+i*4,fields[i]);
        for(int i=0;i<6;++i){const quint32 row[]{committed_[i],values_[i],maximum[i],increment[i],11,11};
            for(int j=0;j<6;++j)put(at+32+i*24+j*4,row[j]);}
        std::memcpy(bytes_+at+176,"Fixture wizard",15);put(128,sequence+2);
    }
    static void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
private:
    quint32 mask() const {
        quint32 result=0;for(int i=0;i<6;++i){
            if(values_[i]>committed_[i])result|=1u<<(2*i);
            if(values_[i]+increment[i]<=maximum[i]&&experience_>=11)result|=1u<<(2*i+1);
        }return result;
    }
    QTemporaryDir directory_;
    QFile file_;
    uchar* bytes_=nullptr;
    std::unique_ptr<MenuBridge> bridge_;
    quint32 generation_=0,parent_=4,experience_=50,committedExperience_=50;
    std::array<quint32,6> values_=initial,committed_=initial;
};
