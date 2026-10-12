#pragma once
#include "profile.hpp"
#include <QTimer>
#include <functional>
namespace mnm::melee_lab {
class Controller {
  game::MeleeExchangeSession session_;
  std::uint32_t seed_;
  QTimer timer_;
  std::vector<game::ExchangeEvent> last_;

public:
  explicit Controller(game::MeleeExchangeSession, std::uint32_t seed = 1);
  const game::MeleeExchangeSession &session() const { return session_; }
  const std::vector<game::ExchangeEvent> &last() const { return last_; }
  bool playing() const { return timer_.isActive(); }
  void play();
  void pause();
  void step();
  void restart();
  void save(const std::filesystem::path &, bool overwrite = false);
  void load(const std::filesystem::path &);
  std::function<void()> changed;
  std::function<void(const std::string &)> error;
};
} // namespace mnm::melee_lab
