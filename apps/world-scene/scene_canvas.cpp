#include "scene_canvas.hpp"
#include <QMouseEvent>
#include <QPixmap>
#include <cmath>
#include <stdexcept>
namespace mnm::scene {
SceneCanvas::SceneCanvas(QWidget* parent):QLabel(parent) {
    setFixedSize(512,256);setAlignment(Qt::AlignLeft|Qt::AlignTop);setMargin(0);setFrameStyle(QFrame::NoFrame);
    setToolTip("Left-click a creature to select it. Right-click terrain to queue a move. Play or Step applies the order.");
}
void SceneCanvas::present(QPixmap pixels) {
    if(pixels.width()!=512 || pixels.height()!=256) throw std::invalid_argument("Scene canvas must be 512x256");
    pixels.setDevicePixelRatio(1);setPixmap(pixels);
}
void SceneCanvas::mousePressEvent(QMouseEvent* event) {
    const auto p=event->position();
    if(!(p.x()>=0 && p.y()>=0 && p.x()<512 && p.y()<256)) {event->ignore();return;}
    const auto x=int(std::floor(p.x())),y=int(std::floor(p.y()));
    if(event->button()==Qt::LeftButton && onSelectAt) {onSelectAt(x,y);event->accept();}
    else if(event->button()==Qt::RightButton && onMoveAt) {onMoveAt(x,y);event->accept();}
    else event->ignore();
}
}
