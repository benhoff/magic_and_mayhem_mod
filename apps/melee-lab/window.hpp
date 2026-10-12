#pragma once
#include "controller.hpp"
#include <QWidget>
namespace mnm::melee_lab {
class Window final : public QWidget {
public:
  explicit Window(Controller &);
};
} // namespace mnm::melee_lab
