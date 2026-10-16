#pragma once
#include "controller.hpp"
#include <QWidget>
namespace mnm::skirmish {
class ArenaCanvas final : public QWidget {
    ArenaView view_;

  protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

  public:
    explicit ArenaCanvas(QWidget * = nullptr);
    void present(ArenaView v) {
        view_ = std::move(v);
        update();
    }
    static QPoint cellCenter(game::Point);
    QPoint presentedCenter(game::Point) const;
    std::function<void(game::Point, bool)> onCell;
};
class Window final : public QWidget {
  public:
    explicit Window(Controller &);
};
} // namespace mnm::skirmish
