#pragma once
#include <QString>
#include <memory>
namespace mnm::skirmish {
class Window;
class Controller;
namespace test {
class Driver;
std::shared_ptr<Driver> start(const QString &, const QString &, Window &, Controller &);
} // namespace test
} // namespace mnm::skirmish
