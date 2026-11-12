#pragma once
#include "sprite_player.hpp"
#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
namespace mnm::ui {
class SpriteWindow final:public QWidget {
    sprites::SpritePlayer player_;
    render::GlBlitter renderer_;
    QLineEdit* path_;
    QComboBox *names_,*modes_;
    QPushButton *pause_,*reload_,*reverse_;
    QSlider* seek_;
    QLabel *image_,*status_;
    QTimer timer_;
    QElapsedTimer clock_;
    void attempt(const std::function<void()>&);
    void refresh();
    void choose();
    void names();
public:
    SpriteWindow(const std::filesystem::path& root,const QString& tag={},QWidget* parent=nullptr);
    const sprites::SpritePlayer& player() const {return player_;}
};
}
