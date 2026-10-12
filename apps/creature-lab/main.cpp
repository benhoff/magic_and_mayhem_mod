#include "window.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <iostream>
#ifdef MNM_CREATURE_WINDOW_TEST
#include "../../tests/creature-window-driver.hpp"
#endif
int main(int argc, char **argv) try {
  QApplication app(argc, argv);
  QCommandLineParser p;
  p.addHelpOption();
  p.addOption(
      {"installation",
       "Read Crocodile configuration and animation from this installation",
       "directory", "working/game-clean"});
  p.addOption(
      {"seed", "Deterministic native encounter RNG seed", "integer", "1"});
  p.addOption({"load", "Resume a self-contained native encounter", "file"});
#ifdef MNM_CREATURE_WINDOW_TEST
  p.addOption({"test-output", "Bounded window test artifacts", "directory"});
#endif
  p.process(app);
  bool ok = false;
  const auto seed = p.value("seed").toUInt(&ok);
  if (!ok)
    throw std::invalid_argument("invalid seed");
  auto s = p.isSet("load")
               ? mnm::creature_lab::load(p.value("load").toStdString())
               : mnm::creature_lab::create(
                     mnm::creature_lab::loadProfile(
                         p.value("installation").toStdString()),
                     seed);
  mnm::creature_lab::Controller c(std::move(s), seed);
  mnm::creature_lab::Window window(c);
  window.show();
#ifdef MNM_CREATURE_WINDOW_TEST
  mnm::creature_lab::test::start(app, window, c,
                                 p.value("test-output").toStdString());
#endif
  return app.exec();
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
