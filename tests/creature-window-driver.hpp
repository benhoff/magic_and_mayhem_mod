#pragma once
#include "../apps/creature-lab/window.hpp"
#include <QApplication>
namespace mnm::creature_lab::test {
void start(QApplication &, Window &, Controller &,
           const std::filesystem::path &);
}
