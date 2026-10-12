#include "controller.hpp"
#include "persistence/snapshot.hpp"
#include <stdexcept>
namespace mnm::creature_lab {
Controller::Controller(game::CreatureEncounterSession s, std::uint32_t seed)
    : session_(std::move(s)), seed_(seed) {
  timer_.setInterval(50);
  QObject::connect(&timer_, &QTimer::timeout, [this] {
    try {
      step();
    } catch (const std::exception &e) {
      pause();
      if (error)
        error(e.what());
    }
  });
}
void Controller::play() {
  if (!session_.finished())
    timer_.start();
  if (changed)
    changed();
}
void Controller::pause() {
  timer_.stop();
  if (changed)
    changed();
}
void Controller::step() {
  last_ = session_.step();
  if (session_.finished())
    timer_.stop();
  if (changed)
    changed();
}
void Controller::restart() {
  pause();
  auto s = create(decodeProfile(session_.world().state().campaign), seed_);
  session_ = std::move(s);
  last_.clear();
  if (changed)
    changed();
}
void Controller::save(const std::filesystem::path &p, bool overwrite) {
  pause();
  auto r = game::writeSnapshot(p, session_.world().state(), overwrite);
  if (!r.durable)
    throw std::runtime_error(r.detail);
}
void Controller::load(const std::filesystem::path &p) {
  pause();
  auto s = creature_lab::load(p);
  session_ = std::move(s);
  last_.clear();
  if (changed)
    changed();
}
} // namespace mnm::creature_lab
