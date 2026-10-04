#pragma once
#include <QObject>
#include <QPointer>
#include <QSet>
#include <functional>
#include <memory>
class QSettings;
class QWidget;
class MenuPreview;

// Backend callbacks run on the owning Qt thread. The controller defers delivery
// so stop/restart cannot destroy a decoder on its notification call stack.
class MenuMusicOutput {
public:
    virtual ~MenuMusicOutput()=default;
    virtual bool start(const QString& file,bool loop,QString& error)=0;
    virtual void stop()=0;
    virtual void setVolume(float linear)=0;
    std::function<void()> ready;
    std::function<void(const QString&)> failed;
    std::function<void()> available;
};
std::unique_ptr<MenuMusicOutput> makeQtMenuMusicOutput();
enum class MenuMusicState {stopped,loading,playing,failed,recovering};

class MenuMusicController final:public QObject {
public:
    explicit MenuMusicController(std::unique_ptr<MenuMusicOutput>,QSettings* settings=nullptr,QObject* parent=nullptr);
    ~MenuMusicController() override;
    void attach(MenuPreview&);
    bool start(const QString& file);
    bool recover();
    bool canRecover() const{return state_==MenuMusicState::failed && !failurePending_ && !track_.isEmpty();}
    void stop();
    MenuMusicState state() const{return state_;}
    int musicLevel() const{return musicLevel_;}
    QString lastError() const{return error_;}
    std::function<void(MenuMusicState,const QString&)> changed;
    std::function<void(const QString&)> failed;
private:
    void bind(QWidget*);
    void applyMusicLevel(int);
    void publish(MenuMusicState,const QString& message={});
    void fault(const QString&);
    bool launch(const QString& file,bool recovery);
    void watchAvailability();
    void outputAvailable();
    void outputFailed(const QString&);
    void checkThread() const;
    std::unique_ptr<MenuMusicOutput> output_;
    QPointer<QSettings> settings_;
    QPointer<MenuPreview> preview_;
    QSet<QWidget*> bound_;
    MenuMusicState state_=MenuMusicState::stopped;
    quint64 generation_=0;
    bool failurePending_=false,availabilityPending_=false;
    int musicLevel_=-1500;
    QString error_,track_;
};
