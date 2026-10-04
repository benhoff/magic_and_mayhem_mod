#pragma once
#include "audio_session.hpp"
#include <QPointer>
#include <QSet>
class MenuPreview;
class QWidget;

// Native preview policy, not recovered original menu-to-sound mappings.
struct MenuAudioCues {std::int32_t activate=822,pageTurn=830;};
class MenuAudioController final:public QObject {
public:
    explicit MenuAudioController(std::unique_ptr<AudioSessionOutput>,MenuAudioCues cues={},QObject* parent=nullptr);
    ~MenuAudioController() override;
    void attach(MenuPreview&);
    bool start(const QString& soundsRoot,mnm::reconstruction::audio::NativeSourcePathPolicy);
    bool recover();
    bool canRecover() const{return session_.canRecover();}
    void stop();
    bool running() const{return session_.running();}
    int soundLevel() const{return soundLevel_;}
    QString lastError() const{return error_.isEmpty()?session_.lastError():error_;}
    std::function<void(const QString&)> failed;
private:
    void bind(QWidget*);
    void activate();
    void pageTurn();
    void play(std::int32_t);
    void clear();
    void applySoundLevel(int);
    AudioSession session_;MenuAudioCues cues_;
    int soundLevel_=-1000;
    QString error_;QPointer<MenuPreview> preview_;QSet<QWidget*> bound_;
};
