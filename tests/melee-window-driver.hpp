#pragma once
#include "../apps/melee-lab/window.hpp"
#include <QApplication>
namespace mnm::melee_lab::test {
void start(QApplication &, Window &, Controller &,
           const std::filesystem::path &);
}
