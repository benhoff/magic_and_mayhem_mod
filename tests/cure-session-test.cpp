#include "../apps/cure-lab/session.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::cure;
namespace {
void require(bool ok, const char *why) { if (!ok) throw std::runtime_error(why); }
CureSessionState fixture(unsigned n = 0) {
  CureSessionState s; s.cost = 1 + n % 19; s.magnitude = 1 + n % 31;
  s.effects.resize(2); s.actors.resize(2);
  auto &source = s.actors[0]; source.position = {1,2,1}; source.resource.kind = 0;
  source.resource.value = (s.cost + 10) << 8; source.resource.maximum = 100;
  source.visible[0] = 1;
  auto &target = s.actors[1]; target.position = {4,5,1}; target.resource.kind = 1;
  target.health.health = 1 + n % 23; target.maximumHealth = 24;
  target.status.status = n % 3; target.ongoingDamage = n % 5;
  target.visible[0] = n % 7 ? 1 : 0; target.resource.blocked = n % 11 == 0 ? 1 : 0;
  if (n % 13 == 0) target.health.health = 0;
  if (n % 17 == 0) target.active = false;
  target.status.voice = 0;
  s.audio.enabled = n % 3 ? 1 : 0; s.audio.initialized = 1;
  s.audio.buffers = {1}; s.audio.outputs = {1};
  mnm::reconstruction::audio::Schedule32 node;
  node.voiceAddress = 1; node.outputAddress = 1;
  node.deadline = 300; node.x = 4; node.y = 5;
  s.audio.schedules.push_back(node);
  return s;
}
bool sameEvents(const std::vector<CureSessionEvent> &a, const std::vector<CureSessionEvent> &b) {
  if (a.size() != b.size()) return false;
  for (unsigned i = 0; i < a.size(); ++i)
    if (a[i].kind != b[i].kind || a[i].actor != b[i].actor || a[i].value != b[i].value ||
        a[i].position.x != b[i].position.x || a[i].position.y != b[i].position.y || a[i].position.z != b[i].position.z) return false;
  return true;
}
bool contains(const std::vector<CureSessionEvent> &events, CureSessionEvent::Kind k, unsigned value = UINT32_MAX) {
  return std::any_of(events.begin(), events.end(), [&](const auto &e) { return e.kind == k && (value == UINT32_MAX || e.value == value); });
}
template<class F> void refuses(CureSession &s, F f) {
  const auto before = s.checkpoint(); bool refused = false;
  try { f(); } catch (const std::invalid_argument &) { refused = true; }
  require(refused && before == s.checkpoint(), "refusal must preserve complete session");
}
void targeted() {
  auto state = fixture(1); state.actors[1].status.status = 7;
  CureSession s(state); auto cast = s.cast(0,1);
  require(contains(cast,CureSessionEvent::created,1) && s.state().actors[0].resource.value == 10 * 256,
          "admission must debit real owned resource and allocate effect");
  require(s.state().learning.known[41] && s.state().observed[0][0], "admission learning/observation ownership");
  auto restored = CureSession::restore(s.checkpoint());
  require(sameEvents(s.advance({}),restored.advance({})), "admitted checkpoint transition trace");
  require(s.state().effects[0].effect.type == 77 && s.state().actors[1].health.health == state.actors[1].health.health,
          "first update must defer without healing");
  const auto inFlight = s.checkpoint(); restored = CureSession::restore(inFlight);
  require(sameEvents(s.advance({{{0,1},2}}),restored.advance({{{0,1},2}})), "non-completion must wait");
  const std::vector<CureAudioInput> replies{{0,0,0,1}};
  const auto events = s.advance({{{0,1},1}},replies);
  require(sameEvents(events,restored.advance({{{0,1},1}},replies)) && s.checkpoint() == restored.checkpoint(),
          "mid-delivery checkpoint must preserve exact continuation");
  require(s.state().actors[1].health.health == 4 && s.state().actors[1].status.status == 0 &&
          !s.state().actors[1].status.voice && s.state().actors[1].ongoingDamage == 0 &&
          !s.state().effects[0].active && s.state().audio.outputs[0] == 0,
          "completion must heal, cleanse, clear ongoing damage/audio schedule and release");
  require(contains(events,CureSessionEvent::audioStop) && contains(events,CureSessionEvent::audioPosition) &&
          contains(events,CureSessionEvent::feedback), "native completion consequences");
  s.cast(0,1); s.advance({});
  require(s.state().effects[0].generation == 2, "pool reuse must advance generation");
  refuses(s,[&] { s.advance({{{0,1},1}},replies); });
  refuses(s,[&] { s.advance({{{0,2},1},{{0,2},1}},replies); });
  refuses(s,[&] { s.cast(0,99); }); refuses(s,[&] { s.cast(0,0); });
  refuses(s,[&] { s.cast(0,1,9); });
  auto poor = fixture(1); poor.actors[0].resource.value = 0;
  CureSession insufficient(poor);
  require(!contains(insufficient.cast(0,1),CureSessionEvent::created) && insufficient.state().actors[0].resource.value == 0,
          "insufficient resource must refuse effect without debit");
  CureSession free(poor); free.cast(0,1,4);
  require(free.state().effects[0].active && free.state().actors[0].resource.value == 0, "mode4 bypasses debit");
  CureSession full(fixture(1)); full.cast(0,1); full.cast(0,1);
  const auto before = full.checkpoint();
  require(full.cast(0,1).empty() && full.checkpoint() == before, "full pool refuses before debit/learning/cues");
  // Failing audio input also cannot publish partially healed/status-cleared state.
  CureSession missing(fixture(1)); missing.cast(0,1); missing.advance({});
  refuses(missing,[&] { missing.advance({{{0,1},1}}); });
  const auto bytes = missing.checkpoint();
  for (unsigned i = 0; i < bytes.size(); ++i) {
    auto corrupt = bytes; corrupt[i] ^= 0x80; bool rejected = false;
    try { (void)CureSession::restore(corrupt); } catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "checkpoint checksum must detect single-byte corruption");
  }
  for (auto length : {std::size_t(0),std::size_t(11),bytes.size()-1}) {
    bool rejected = false; try { (void)CureSession::restore({bytes.begin(),bytes.begin()+length}); }
    catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "truncated checkpoint refusal");
  }
  // Valid-checksum malformed payloads must reach semantic preflight rather
  // than relying on the checksum to reject every bad input. Offsets below are
  // independent version1 format fixtures (two212-byte actors, two60-byte slots).
  auto put = [](auto &v, unsigned offset, unsigned value) {
    for (unsigned i = 0; i < 4; ++i) v.at(offset+i) = std::uint8_t(value >> (i*8));
  };
  for (auto mutation : {std::pair<unsigned,unsigned>{0,0}, {4,2}, {24,8},
                        {44,4097}, {48,2}, {112,1}, {436,99}, {472,4097},
                        {484,0}, {496,99}, {500,99}, {636,99}}) {
    auto corrupt = bytes; put(corrupt,mutation.first,mutation.second);
    unsigned checksum = 2166136261u;
    for (unsigned i = 0; i < corrupt.size()-4; ++i) checksum = (checksum ^ corrupt[i]) * 16777619u;
    put(corrupt,unsigned(corrupt.size()-4),checksum);
    bool rejected = false;
    try { (void)CureSession::restore(corrupt); } catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "valid-checksum malformed continuation must refuse");
  }
}
} // namespace
int main() try {
  targeted(); std::uint64_t hash = 1469598103934665603ull;
  for (unsigned n = 0; n < 4096; ++n) {
    auto initial = fixture(n); CureSession uninterrupted(initial);
    uninterrupted.cast(0,1);
    auto resumed = CureSession::restore(uninterrupted.checkpoint());
    require(sameEvents(uninterrupted.advance({}),resumed.advance({})), "admitted resume");
    resumed = CureSession::restore(resumed.checkpoint());
    const bool alive = initial.actors[1].active && initial.actors[1].health.health;
    // A deferred dead target releases immediately, even without an animation reply.
    auto waiting = uninterrupted.advance({}); auto waitingResumed = resumed.advance({});
    require(sameEvents(waiting,waitingResumed), "waiting resume");
    resumed = CureSession::restore(resumed.checkpoint());
    const std::vector<CureAudioInput> audio{{n % 5 ? 0 : -1, n % 7 ? 0 : -2, n % 11 ? 0 : -3, n % 2}};
    std::vector<CureAnimationInput> signal;
    if (alive) signal.push_back({{0,1},1});
    const auto a = uninterrupted.advance(signal,audio), b = resumed.advance(signal,audio);
    require(sameEvents(a,b) && uninterrupted.checkpoint() == resumed.checkpoint(), "deferred resume final state/events");
    const auto &target = uninterrupted.state().actors[1];
    const bool heal = alive && !initial.actors[1].resource.blocked;
    require(target.health.health == (heal ? std::min(24u,initial.actors[1].health.health+initial.magnitude) : initial.actors[1].health.health), "bounded actual healing result");
    require(target.status.status == (heal ? 0 : initial.actors[1].status.status) &&
            target.ongoingDamage == (heal ? 0 : initial.actors[1].ongoingDamage) &&
            !uninterrupted.state().effects[0].active && uninterrupted.state().actors[0].resource.value == 10 * 256,
            "blocked/dead/success lifecycle and no deferred dead refund");
    for (auto byte : uninterrupted.checkpoint()) hash = (hash ^ byte) * 1099511628211ull;
  }
  std::cout << "{\"success\":true,\"cases\":4096,\"checkpoint_continuation\":true,\"atomic_refusals\":true,\"outcome_digest\":\"" << std::hex << hash << "\"}\n";
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
