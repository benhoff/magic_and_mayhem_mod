#pragma once
#include "../protocols/include/mnm/music_v1.h"
#include <QObject>
#include <QThread>
#include <QString>
#include <QJsonObject>
#include <memory>
namespace mnm::audio {
// Owner-thread facade. Music/decoder/channel work share an owned Qt worker;
// the GUI receives copied status only, never service or mapping pointers.
class MusicBroker final:public QObject {
public:
    explicit MusicBroker(QObject* parent=nullptr);
    ~MusicBroker()override;
    bool create(const QString& channel,const QString& root,bool audible=true);
    void stop();
    QString lastError()const{return error_;}
    QJsonObject activity()const;
private:
    struct State;
    QThread thread_;QObject* worker_=nullptr;std::unique_ptr<State> state_;
    QString error_;QJsonObject lastActivity_;
};
}
