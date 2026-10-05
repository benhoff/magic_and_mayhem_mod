#pragma once
#include <QLabel>
#include <functional>
namespace mnm::scene {
class SceneCanvas final:public QLabel {
public:
    explicit SceneCanvas(QWidget* parent=nullptr);
    void present(QPixmap);
    std::function<void(int,int)> onSelectAt,onMoveAt;
protected:
    void mousePressEvent(QMouseEvent*) override;
};
}
