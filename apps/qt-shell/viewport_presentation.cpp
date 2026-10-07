#include "viewport_presentation.hpp"
#include <QApplication>
#include <QComboBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSignalBlocker>
#include <QToolBar>
#include <QTimer>

ViewportPresentation::ViewportPresentation(QWidget& window,GlViewport& viewport,PresentationOptions options)
    :QObject(&window),window_(window),viewport_(viewport),startFullscreen_(options.fullscreen){
    viewport_.setScaling(options.scaling);
    // Application filters run before InputForwarder, including its ShortcutOverride handling.
    qApp->installEventFilter(this);
    connect(&viewport_,&QOpenGLWidget::frameSwapped,this,[this]{if(fullscreenActive_)scheduleChromeUpdate();});
}
ViewportPresentation::~ViewportPresentation(){qApp->removeEventFilter(this);}
void ViewportPresentation::addControls(QToolBar& toolbar){
    toolbar.addSeparator();
    fullscreenButton_=new QPushButton("Fullscreen",&toolbar);
    fullscreenButton_->setObjectName("fullscreenButton");fullscreenButton_->setCheckable(true);
    fullscreenButton_->setToolTip("Toggle fullscreen (F9)");
    connect(fullscreenButton_,&QPushButton::clicked,this,&ViewportPresentation::setFullscreen);
    toolbar.addWidget(fullscreenButton_);setFullscreen(window_.isFullScreen());
    toolbar.addWidget(new QLabel(" Scaling: ",&toolbar));
    auto* choices=new QComboBox(&toolbar);choices->setObjectName("viewportScaling");
    choices->addItem("Fit (sharp)",int(GlViewport::Scaling::Sharp));
    choices->addItem("Fit (smooth)",int(GlViewport::Scaling::Smooth));
    choices->addItem("Integer (sharp)",int(GlViewport::Scaling::Integer));
    choices->setCurrentIndex(int(viewport_.scaling()));
    choices->setToolTip("Scale the displayed image; game resolution stays unchanged.");
    connect(choices,&QComboBox::currentIndexChanged,this,[this,choices]{viewport_.setScaling(GlViewport::Scaling(choices->currentData().toInt()));});
    toolbar.addWidget(choices);
}
void ViewportPresentation::hideInFullscreen(QWidget& widget){chrome_.push_back({&widget,false});}
void ViewportPresentation::show(){window_.show();if(startFullscreen_)setFullscreen(true);}
void ViewportPresentation::updateChrome(){
    if(!fullscreenActive_)return;
    // Native commands show an empty GL widget to initialize its context before the first frame.
    const bool haveFrame=viewport_.isVisible() && viewport_.ready() && !viewport_.frameSize().isEmpty() && viewport_.error().isEmpty();
    bool changed=false;
    for(auto& item:chrome_)if(item.widget){const bool hidden=haveFrame || item.hidden;
        if(item.widget->isHidden()!=hidden){item.widget->setHidden(hidden);changed=true;}}
    if(changed)repaintQueued_=true;
}
void ViewportPresentation::scheduleChromeUpdate(bool repaint){
    repaintQueued_|=repaint;
    if(chromeUpdateQueued_)return;
    chromeUpdateQueued_=true;
    // Defer layout mutations until Qt finishes showing/resizing/compositing the GL widget.
    QTimer::singleShot(0,this,[this]{chromeUpdateQueued_=false;
        const bool repaint=repaintQueued_;repaintQueued_=false;updateChrome();
        if(repaint || repaintQueued_){repaintQueued_=false;
            // The new framebuffer size must follow the settled widget layout.
            QTimer::singleShot(0,this,[this]{
                if(window_.layout())window_.layout()->activate();
                auto* parent=viewport_.parentWidget();if(parent && parent->layout())parent->layout()->activate();
                viewport_.repaint();window_.update();});
        }});
}
void ViewportPresentation::setFullscreen(bool enabled){
    if(enabled!=window_.isFullScreen()){
        if(enabled){
            previousGeometry_=window_.geometry();previousState_=window_.windowState();
            for(auto& item:chrome_)if(item.widget)item.hidden=item.widget->isHidden();
            fullscreenActive_=true;window_.showFullScreen();
        }else{
            fullscreenActive_=false;window_.showNormal();
            for(auto& item:chrome_)if(item.widget)item.widget->setVisible(!item.hidden);
            window_.setGeometry(previousGeometry_);window_.setWindowState(previousState_);
        }
        if(viewport_.isVisible())viewport_.setFocus(Qt::OtherFocusReason);
        scheduleChromeUpdate(true);
    }
    if(fullscreenButton_){const QSignalBlocker blocker(fullscreenButton_);
        fullscreenButton_->setChecked(enabled);fullscreenButton_->setText(enabled?"Exit fullscreen":"Fullscreen");}
}
bool ViewportPresentation::eventFilter(QObject* object,QEvent* event){
    if(object==&viewport_ && (event->type()==QEvent::Show || event->type()==QEvent::Hide))scheduleChromeUpdate();
    if(object==&window_ && event->type()==QEvent::WindowStateChange)scheduleChromeUpdate(true);
    if(event->type()!=QEvent::ShortcutOverride && event->type()!=QEvent::KeyPress && event->type()!=QEvent::KeyRelease)return false;
    const auto* widget=qobject_cast<QWidget*>(object);
    if(!widget || widget->window()!=&window_)return false;
    const auto* key=static_cast<QKeyEvent*>(event);
    if(key->key()!=Qt::Key_F9 || key->modifiers()!=Qt::NoModifier)return false;
    if(event->type()==QEvent::KeyPress && !key->isAutoRepeat())setFullscreen(!window_.isFullScreen());
    event->accept();return true;
}
