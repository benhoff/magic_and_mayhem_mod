#pragma once
#include <QApplication>
#include <QKeyEvent>
#include <QPushButton>
#include <functional>

// Catch the semantic shell shortcut before the viewport's game-input filter.
class GameRestartAction final : public QObject {
public:
    GameRestartAction(QWidget& window,QPushButton& button,std::function<void()> restart)
        :QObject(&window),window_(window),button_(button){
        button_.setObjectName("hardRestartGame");button_.setEnabled(false);
        button_.setToolTip("Force-stop the game and relaunch (Ctrl+Shift+R). Unsaved progress is lost. Stops Wine processes in this game's prefix.");
        connect(&button_,&QPushButton::clicked,this,[restart=std::move(restart)]{restart();});
        qApp->installEventFilter(this);
    }
    ~GameRestartAction() override{qApp->removeEventFilter(this);}
protected:
    bool eventFilter(QObject* object,QEvent* event) override{
        if(event->type()!=QEvent::ShortcutOverride&&event->type()!=QEvent::KeyPress&&event->type()!=QEvent::KeyRelease)return false;
        const auto* widget=qobject_cast<QWidget*>(object);
        if(!widget||widget->window()!=&window_)return false;
        const auto* key=static_cast<QKeyEvent*>(event);
        if(key->key()!=Qt::Key_R||key->modifiers()!=(Qt::ControlModifier|Qt::ShiftModifier))return false;
        if(event->type()==QEvent::KeyPress&&!key->isAutoRepeat())button_.click();
        event->accept();return true;
    }
private:
    QWidget& window_;QPushButton& button_;
};
