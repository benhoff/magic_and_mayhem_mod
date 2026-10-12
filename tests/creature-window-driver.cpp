#include "creature-window-driver.hpp"
#include "persistence/snapshot.hpp"
#include <QPushButton>
#include <QTimer>
#include <fstream>
#include <iostream>
namespace mnm::creature_lab::test {
void start(QApplication &app, Window &window, Controller &c,
           const std::filesystem::path &out) {
  if (out.empty() || std::filesystem::exists(out))
    throw std::invalid_argument("new window test output required");
  std::filesystem::create_directories(out);
  auto button = [&window](const char *name) {
    auto b = window.findChild<QPushButton *>(QString("creature") + name);
    if (!b)
      throw std::runtime_error("missing creature control");
    b->click();
  };
  QTimer::singleShot(0, &app, [&app, &window, &c, out, button] {
    try {
      for (unsigned i = 0; i < 13; ++i)
        button("Step");
      c.save(out / "mid-attack.mnm");
      auto before = game::encodeSnapshot(c.session().world().state());
      window.grab().save(
          QString::fromStdString((out / "mid-attack.png").string()));
      try {
        c.load(out / "missing.mnm");
        throw std::runtime_error("missing checkpoint accepted");
      } catch (const std::invalid_argument &) {
      } catch (const std::runtime_error &e) {
        if (std::string(e.what()) == "missing checkpoint accepted")
          throw;
      }
      if (game::encodeSnapshot(c.session().world().state()) != before)
        throw std::runtime_error("failed load changed encounter");
      button("Play");
      QTimer::singleShot(170, &app, [&app, &window, &c, out, button] {
        try {
          button("Pause");
          const auto paused = c.session().world().state().tick;
          if (paused <= 13)
            throw std::runtime_error("production timer did not advance");
          QTimer::singleShot(
              170, &app, [&app, &window, &c, out, button, paused] {
                try {
                  if (c.session().world().state().tick != paused)
                    throw std::runtime_error("paused timer advanced");
                  c.load(out / "mid-attack.mnm");
                  for (unsigned i = 0; i < 8192 && !c.session().finished(); ++i)
                    button("Step");
                  if (!c.session().finished())
                    throw std::runtime_error(
                        "window encounter failed to finish");
                  c.save(out / "final.mnm");
                  window.grab().save(
                      QString::fromStdString((out / "final.png").string()));
                  const auto s = c.session().state();
                  std::ofstream(out / "result.json")
                      << "{\"success\":true,\"timer_advanced\":true,\"pause_"
                         "preserved\":true,\"load_failure_atomic\":true,"
                         "\"tick\":"
                      << c.session().world().state().tick << ",\"health\":["
                      << s.actors[0].health << ',' << s.actors[1].health
                      << "]}\n";
                  app.exit(0);
                } catch (const std::exception &e) {
                  std::cerr << e.what() << '\n';
                  app.exit(1);
                }
              });
        } catch (const std::exception &e) {
          std::cerr << e.what() << '\n';
          app.exit(1);
        }
      });
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
      app.exit(1);
    }
  });
}
} // namespace mnm::creature_lab::test
