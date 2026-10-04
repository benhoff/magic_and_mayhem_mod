#pragma once
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <array>
namespace mnm::ui {
struct MenuSpriteFrame {QImage image;QPoint origin;};
using MenuSpriteSheet=QVector<MenuSpriteFrame>;
MenuSpriteSheet loadMenuSprites(const QString& root,const QString& path);
std::array<MenuSpriteFrame,3> spriteStates(const MenuSpriteSheet&,int first);
class SpriteButton final : public QPushButton {
public:
    explicit SpriteButton(const QString& text,QWidget* parent=nullptr);
    void setSprites(const std::array<MenuSpriteFrame,3>& frames,const QSize& canvas);
    void clearSprites();
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::array<MenuSpriteFrame,3> frames_{};
    QSize canvas_;
};
class TalismanBar final : public QLabel {
public:
    explicit TalismanBar(QWidget* parent=nullptr):QLabel(parent) {}
    void setSprites(const MenuSpriteFrame& active,const MenuSpriteFrame& inactive);
    void setCounts(int value,int maximum);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    MenuSpriteFrame active_,inactive_;
    int value_=0,maximum_=7;
};
}
