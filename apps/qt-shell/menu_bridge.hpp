#pragma once
#include <QFile>
#include <QString>
#include <QVector>
#include <array>
#include <cstdint>
class MenuBridge final {
public:
    struct Player {bool active=false;int portrait=-1,colour=-1,handicap=0;QString name;};
    struct Battle {quint32 map=0;QString mapName;std::array<int,13> rules{};std::array<Player,4> players{};QVector<QString> maps;};
    struct Spells {int owner=0,seconds=-1;quint32 offered=0;std::array<int,3> counts{};std::array<int,63> assignments{},recipes{};std::array<int,25> shelves{};std::array<QString,21> items{};std::array<QString,63> names{};};
    struct Mini {quint32 battle=0,confirmation=0,depth=0,parentScreen=0,actions=0,context=0,mode=0;};
    struct ResultPlayer {bool active=false;QString name,kills,deaths,handicap,score;};
    struct Results {quint32 actions=0,context=0,depth=0;std::array<ResultPlayer,4> players{};};
    struct Preferences {quint32 actions=0,available=0,parentScreen=0,depth=0;std::array<int,7> values{};};
    struct RegionEntry {quint32 caller=0,depth=0,different=0,difficulty=0,available=0,actions=0,region=0;QString realm,name;};
    struct State { quint32 generation=0,screen=0,ready=0,ack=0,status=0,thread=0,sequence=0,handoff=0;Battle battle;Spells spells;Mini mini;Results results;Preferences preferences;RegionEntry region; };
    ~MenuBridge();
    bool create(const QString& path,bool battle=false,bool spells=false,bool mini=false,bool results=false,bool preferences=false,bool region=false,bool enter=false,bool campaignMini=false,bool campaignQuit=false);
    bool read(State& state) const;
    void heartbeat(bool alive=true);
    bool request(quint32 action,const State& state,quint32 argument=0,const std::array<int,17>* rules=nullptr);
    bool finishSpells(const State&,const std::array<int,63>& assignments);
    bool requestPreferences(quint32 action,const State&,const std::array<int,7>& values,quint32 slider=0);
    void retire();
private:
    QFile file_;
    uchar* mapping_=nullptr;
    quint32 heartbeat_=0,request_=0;
    bool retired_=false,battle_=false,spells_=false,mini_=false,results_=false,preferences_=false,region_=false,enter_=false,campaignMini_=false,campaignQuit_=false;
    void publish(bool alive,quint32 action=0,quint32 generation=0,quint32 argument=0,const std::array<int,17>* rules=nullptr,const std::array<int,63>* assignments=nullptr,const std::array<int,7>* preferences=nullptr);
};
