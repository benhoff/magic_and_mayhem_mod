#pragma once
#include "controller.hpp"
#include <QWidget>
namespace mnm::creature_lab {
class Window final : public QWidget {
public:
  explicit Window(Controller &);
};
} // namespace mnm::creature_lab
