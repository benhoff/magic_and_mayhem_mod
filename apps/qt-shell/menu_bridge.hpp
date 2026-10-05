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
    struct State { quint32 generation=0,screen=0,ready=0,ack=0,status=0,thread=0,sequence=0,handoff=0;Battle battle; };
    ~MenuBridge();
    bool create(const QString& path,bool battle=false);
    bool read(State& state) const;
    void heartbeat(bool alive=true);
    bool request(quint32 action,const State& state,quint32 argument=0,const std::array<int,17>* rules=nullptr);
    void retire();
private:
    QFile file_;
    uchar* mapping_=nullptr;
    quint32 heartbeat_=0,request_=0;
    bool retired_=false,battle_=false;
    void publish(bool alive,quint32 action=0,quint32 generation=0,quint32 argument=0,const std::array<int,17>* rules=nullptr);
};
