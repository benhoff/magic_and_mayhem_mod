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
    struct Defeat {quint32 actions=0,context=0,depth=0,outcome=0;QString title;std::array<QString,21> texts{};};
    struct Storage {quint32 screen=0,parent=0,depth=0,context=0,modal=0,pending=0,selected=0;QString fileName;QVector<QString> names;};
    struct Multiplayer {quint32 parent=0,depth=0,context=0,transport=0,available=0,actions=0,modal=0,pending=0,gameLimit=0,userLimit=0;QString gameName,userName;};
    struct Grimoire {quint32 chapter=0,section=0,cursor=0,available=0,parent=0,depth=0,context=0;};
    struct State { quint32 generation=0,screen=0,ready=0,ack=0,status=0,thread=0,sequence=0,handoff=0;Battle battle;Spells spells;Mini mini;Results results;Preferences preferences;RegionEntry region;Defeat defeat;Storage storage;Grimoire grimoire;Multiplayer multiplayer; };
    ~MenuBridge();
    bool create(const QString& path,bool battle=false,bool spells=false,bool mini=false,bool results=false,bool preferences=false,bool region=false,bool enter=false,bool campaignMini=false,bool campaignQuit=false,bool campaignDefeat=false,bool campaignPreferences=false,bool storage=false,bool grimoire=false,bool multiplayer=false);
    bool read(State& state) const;
    void heartbeat(bool alive=true);
    bool request(quint32 action,const State& state,quint32 argument=0,const std::array<int,17>* rules=nullptr);
    bool finishSpells(const State&,const std::array<int,63>& assignments);
    bool requestMultiplayer(quint32 action,const State&,quint32 transport=0,const QString& gameName={},const QString& userName={});
    bool requestStorage(quint32 action,const State&,quint32 row=0,const QString& name={});
    bool requestPreferences(quint32 action,const State&,const std::array<int,7>& values,quint32 slider=0);
    quint32 requestedSequence() const { return request_; }
    void retire();
private:
    QFile file_;
    uchar* mapping_=nullptr;
    quint32 heartbeat_=0,request_=0;
    bool retired_=false,battle_=false,spells_=false,mini_=false,results_=false,preferences_=false,region_=false,enter_=false,campaignMini_=false,campaignQuit_=false,campaignDefeat_=false,campaignPreferences_=false,storage_=false,grimoire_=false,multiplayer_=false;
    void publish(bool alive,quint32 action=0,quint32 generation=0,quint32 argument=0,const std::array<int,17>* rules=nullptr,const std::array<int,63>* assignments=nullptr,const std::array<int,7>* preferences=nullptr,const QByteArray* storageName=nullptr,const QByteArray* multiplayerNames=nullptr);
};
