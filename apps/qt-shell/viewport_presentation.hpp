#pragma once
#include "gl_viewport.hpp"
#include <QObject>
#include <QPointer>
#include <vector>

class QPushButton;
class QToolBar;
struct PresentationOptions {
    GlViewport::Scaling scaling=GlViewport::Scaling::Sharp;
    bool fullscreen=false;
};

// Host display policy only: never rebuilds game surfaces or changes engine preferences.
class ViewportPresentation final:public QObject {
public:
    ViewportPresentation(QWidget& window,GlViewport& viewport,PresentationOptions options={});
    ~ViewportPresentation() override;
    void addControls(QToolBar& toolbar);
    void hideInFullscreen(QWidget& widget);
    void setFullscreen(bool enabled);
    void show();
protected:
    bool eventFilter(QObject* object,QEvent* event) override;
private:
    QWidget& window_;
    GlViewport& viewport_;
    QPointer<QPushButton> fullscreenButton_;
    bool startFullscreen_=false;
    Qt::WindowStates previousState_;
    QRect previousGeometry_;
    bool fullscreenActive_=false;
    bool chromeUpdateQueued_=false,repaintQueued_=false;
    void updateChrome();
    void scheduleChromeUpdate(bool repaint=false);
    struct Chrome {QPointer<QWidget> widget;bool hidden=false;};
    std::vector<Chrome> chrome_;
};
