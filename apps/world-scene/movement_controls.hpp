#pragma once
#include "orders.hpp"
#include <QWidget>
#include <functional>
class QComboBox;
class QSpinBox;
class QPushButton;
namespace mnm::scene {
class MovementControls final:public QWidget {
    QComboBox* creatures_;
    QSpinBox *x_,*y_,*z_;
    QPushButton* move_;
    std::vector<CreatureChoice> choices_;
    std::optional<game::Handle> selected_;
public:
    explicit MovementControls(QWidget* parent=nullptr);
    void updateChoices(std::vector<CreatureChoice>,std::optional<game::Handle>,game::Point dimensions);
    void setTarget(game::Point);
    std::function<void(std::optional<game::Handle>)> onSelect;
    std::function<void(game::Point)> onMove;
};
}
