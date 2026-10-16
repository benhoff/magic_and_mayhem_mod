#include "trace.hpp"
#include "cure_runtime.hpp"
#include <algorithm>
namespace mnm::skirmish {
void trace(std::ostream &out, const game::SkirmishSession &session,
           const game::SkirmishReport &report) {
  const auto s = session.state();
  const auto c = session.combat().combat();
  out << "{\"tick\":" << session.world().state().tick
      << ",\"healthRules\":" << c.healthRulesIdentity
      << ",\"outcome\":" << static_cast<unsigned>(s.outcome)
      << ",\"wizards\":[";
  for (unsigned i = 0; i < 2; ++i) {
    if (i)
      out << ',';
    const auto &w = s.wizards[i];
    out << "{\"slot\":" << w.actor.slot << ",\"mana\":" << w.mana
        << ",\"regen\":" << w.regeneration
        << ",\"spell\":" << static_cast<unsigned>(w.spell)
        << ",\"remaining\":" << w.remaining << '}';
  }
  out << "],\"units\":[";
  bool comma = false;
  for (const auto &u : s.units) {
    if (comma)
      out << ',';
    comma = true;
    const auto &e = *session.world().find(u.actor);
    const auto &f = *std::find_if(
        c.fighters.begin(), c.fighters.end(),
        [&](const game::Fighter &f) { return f.actor == u.actor; });
    out << "{\"slot\":" << u.actor.slot
        << ",\"generation\":" << u.actor.generation
        << ",\"role\":" << static_cast<unsigned>(u.role)
        << ",\"owner\":" << e.owner << ",\"health\":" << f.health
        << ",\"position\":[" << e.x << ',' << e.y << ',' << e.z
        << "],\"action\":" << static_cast<unsigned>(f.action)
        << ",\"remaining\":" << f.remaining
        << ",\"control\":" << static_cast<unsigned>(f.control) << '}';
  }
  out << "],\"projectiles\":[";
  comma = false;
  for (const auto &p : s.projectiles) {
    if (comma)
      out << ',';
    comma = true;
    out << "{\"id\":" << p.id << ",\"owner\":" << p.owner
        << ",\"target\":" << p.target.slot << ",\"remaining\":" << p.remaining
        << '}';
  }
  out << "],\"cures\":[";
  comma = false;
  for (const auto &effect : cureViews(session.combat())) {
    if (comma)
      out << ',';
    comma = true;
    out << "{\"slot\":" << effect.effect.slot
        << ",\"generation\":" << effect.effect.generation
        << ",\"target\":" << effect.target.slot << ",\"position\":["
        << effect.position.x << ',' << effect.position.y << ','
        << effect.position.z << "]}";
  }
  out << "],\"pending\":" << s.pending.size() << ",\"events\":[";
  comma = false;
  for (const auto &e : report.events) {
    if (comma)
      out << ',';
    comma = true;
    out << "{\"kind\":" << static_cast<unsigned>(e.kind)
        << ",\"actor\":" << e.actor.slot
        << ",\"refusal\":" << static_cast<unsigned>(e.refusal) << '}';
  }
  out << "]}\n";
}
} // namespace mnm::skirmish
