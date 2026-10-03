#pragma once
#include "native_playback.hpp"
#include "gl_viewport.hpp"
#include <QFile>
#include <QJsonArray>
#include <QTimer>
#include <memory>

class MediaBroker final:public QObject {
public:
    static constexpr int Size=2048;
    MediaBroker(GlViewport& viewport,QString assetRoot,bool audible=true);
    ~MediaBroker() override;
    bool create(const QString& path);
    bool movieActive() const{return movie_ && movie_->active();}
    void skipMovie();
    void stop();
    QJsonArray history() const{return history_;}
    std::function<void(QImage)> frame;
    std::function<void(bool)> movieChanged;
    std::function<void()> recorded;
    bool skipFixture=false;
protected:
    bool eventFilter(QObject* object,QEvent* event) override;
private:
    void poll();
    QString asset(const QByteArray& path,unsigned operation) const;
    void respond(unsigned id,unsigned status);
    void record(unsigned id,unsigned operation,unsigned status,const QString& path,QJsonObject report={});
    GlViewport& viewport_;QString root_;bool audible_;QFile file_;uchar* map_=nullptr;
    QTimer timer_;unsigned last_=0,movieId_=0,soundId_=0,soundCancelled_=0;
    std::unique_ptr<NativePlayback> movie_,sound_;QJsonArray history_;
};
