#include "ai.hpp"
#include "control.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_CONTROL_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#endif
using namespace mnm::reconstruction::combat;
namespace {
void check(bool b, const char *s) {
  if (!b)
    throw std::runtime_error(s);
}
unsigned reachCases = 0, directions = 0, stunCases = 0, readyCases = 0,
         decisions = 0, knockbacks = 0, selections = 0;
#ifdef MNM_CONTROL_REFERENCE
std::vector<ControlEvent> trace;
std::vector<AiEvent> aiTrace;
const DecisionInputs *decisionInputs = nullptr;
unsigned aiDraws = 0;
std::uint32_t suppliedWord = 0;
bool testingAi = false;
bool lineClear = true;
unsigned lines = 0;
std::vector<std::pair<unsigned, Bytes>> code;
extern "C" unsigned __attribute__((thiscall)) line(void *, int, int, int, int,
                                                   int, int, void *) {
  ++lines;
  return lineClear;
}
extern "C" int __attribute__((thiscall)) terrain(void *, void *) { return 0; }
extern "C" void __attribute__((thiscall)) behavior(void *, int n) {
  if (testingAi)
    aiTrace.push_back({AiEffect::behavior, n});
  else
    trace.push_back({ControlEffect::behavior, n});
}
extern "C" void __attribute__((thiscall)) reset(void *) {
  if (testingAi)
    aiTrace.push_back({AiEffect::reset, 0});
  else
    trace.push_back({ControlEffect::reset, 0});
}
extern "C" void __attribute__((thiscall)) action(void *, int n) {
  trace.push_back({ControlEffect::action, n});
}
extern "C" void __attribute__((thiscall)) surface(void *) {
  trace.push_back({ControlEffect::surfaceRepair, 0});
}
extern "C" void __attribute__((thiscall)) colorSet(void *, int n) {
  trace.push_back({ControlEffect::colorSet, n});
}
extern "C" void __attribute__((thiscall)) colorClear(void *, int n) {
  trace.push_back({ControlEffect::colorClear, n});
}
extern "C" void __attribute__((thiscall)) stunSound(void *, void *) {
  trace.push_back({ControlEffect::stunAudio, 0});
}
extern "C" void __attribute__((thiscall)) form(void *, int n) {
  trace.push_back({ControlEffect::formRestore, n});
}
int suppliedAnimation = 0;
void *primaryAnimation = nullptr;
extern "C" int __attribute__((thiscall)) animationEvent(void *object) {
  trace.push_back(
      {ControlEffect::animation, object == primaryAnimation ? 0 : 1});
  return suppliedAnimation;
}
unsigned readyCalls = 0;
int readySet = -999;
bool idleRestart = false;
extern "C" void __attribute__((thiscall)) readyBehavior(void *object) {
  ++readyCalls;
  if (readySet != -999)
    std::memcpy(static_cast<unsigned char *>(object) + 0x66c, &readySet, 4);
}
extern "C" void __attribute__((thiscall)) idle(void *, int n, int zero) {
  check(n == 288 && zero == 0, "idle arguments");
  idleRestart = true;
}
extern "C" unsigned __attribute__((thiscall)) aiImmediate(void *) {
  aiTrace.push_back({AiEffect::immediate, 0});
  return decisionInputs->immediate;
}
extern "C" void __attribute__((thiscall)) aiEscape(void *) {
  aiTrace.push_back({AiEffect::escapePreparation, 0});
}
extern "C" void __attribute__((thiscall)) aiPrepare(void *) {
  aiTrace.push_back({AiEffect::prepareInteraction, 0});
}
extern "C" void __attribute__((thiscall)) aiTail(void *) {
  aiTrace.push_back({AiEffect::tailUpdate, 0});
}
extern "C" void __attribute__((thiscall)) aiAttack(void *) {
  aiTrace.push_back({AiEffect::attackResponse, 0});
}
extern "C" void __attribute__((thiscall)) aiSub(void *, int n) {
  aiTrace.push_back({AiEffect::substate, n});
}
extern "C" int __attribute__((thiscall)) aiHazard(void *, int, int, int) {
  aiTrace.push_back({AiEffect::hazard, 0});
  return decisionInputs->hazard;
}
extern "C" unsigned __attribute__((thiscall)) randomWord(void *) {
  ++aiDraws;
  return suppliedWord;
}
#define SELECTOR(name, enumerator, field)                                      \
  extern "C" unsigned __attribute__((thiscall)) name(void *) {                 \
    aiTrace.push_back({AiEffect::selector, int(Selector::enumerator)});        \
    return decisionInputs->field;                                              \
  }
SELECTOR(aiPriority, priority, priority)
SELECTOR(aiCorpse21, corpse21, corpse21)
SELECTOR(aiCorpse10, corpse10, corpse10)
SELECTOR(aiAssist, assist, assist)
SELECTOR(aiScore, score, score)
SELECTOR(aiHome, home, home)
SELECTOR(aiInteraction, interaction, interaction)
SELECTOR(aiSpell, spell, spell)
#undef SELECTOR
std::vector<Cell> probes;
std::optional<Cell> requested;
int legalAt = -1;
bool moveOk = false;
extern "C" unsigned __attribute__((thiscall))
legalProbe(void *, int, int, int, int x, int y, int z, void *, unsigned zero) {
  check(zero == 0, "knockback legality flag");
  probes.push_back({x, y, z});
  return int(probes.size()) == legalAt;
}
extern "C" unsigned __attribute__((thiscall)) moveProbe(void *, int x, int y,
                                                        int z, unsigned zero) {
  check(zero == 0, "knockback request flag");
  requested = Cell{x, y, z};
  return moveOk;
}
std::vector<std::uint32_t> admissions;
unsigned admitMask = 0, maximum = 360;
extern "C" unsigned __attribute__((thiscall)) admission(void *, void *b) {
  unsigned slot;
  std::memcpy(&slot, b, 4);
  admissions.push_back(slot);
  return (admitMask >> slot) & 1;
}
extern "C" unsigned __attribute__((thiscall)) assistAdmission(void *, void *,
                                                              void *) {
  return 1;
}
extern "C" unsigned __attribute__((thiscall)) maxHealth(void *) {
  return maximum;
}
void redirect(unsigned at, std::uintptr_t function) {
  auto *p = reinterpret_cast<unsigned char *>(at);
  p[0] = 0xe9;
  auto delta = std::uint32_t(function) - at - 5;
  std::memcpy(p + 1, &delta, 4);
}
void initialize(const char *path) {
  map_image(read(path));
  for (auto range :
       std::vector<std::pair<unsigned, unsigned>>{{0x50bd60, 0x50c0c3},
                                                  {0x50c930, 0x50ca27},
                                                  {0x50d060, 0x50d2dd},
                                                  {0x50f590, 0x50f7bb},
                                                  {0x50f7c0, 0x50f84f},
                                                  {0x518420, 0x5186b3},
                                                  {0x521160, 0x52141f},
                                                  {0x521420, 0x521573},
                                                  {0x5189e0, 0x518c49},
                                                  {0x518c50, 0x518f77},
                                                  {0x518750, 0x5189df},
                                                  {0x518f80, 0x5190fb},
                                                  {0x519100, 0x5192c9},
                                                  {0x519330, 0x51983e}}) {
    auto *p = reinterpret_cast<unsigned char *>(range.first);
    code.push_back({range.first, Bytes(p, p + range.second - range.first)});
  }
  redirect(0x4f4ba0, reinterpret_cast<std::uintptr_t>(&line));
  redirect(0x4f3260, reinterpret_cast<std::uintptr_t>(&terrain));
  redirect(0x50aa30, reinterpret_cast<std::uintptr_t>(&behavior));
  redirect(0x50ab30, reinterpret_cast<std::uintptr_t>(&reset));
  redirect(0x50db60, reinterpret_cast<std::uintptr_t>(&action));
  redirect(0x5120c0, reinterpret_cast<std::uintptr_t>(&surface));
  redirect(0x54dcb0, reinterpret_cast<std::uintptr_t>(&colorSet));
  redirect(0x54dce0, reinterpret_cast<std::uintptr_t>(&colorClear));
  redirect(0x5708e0, reinterpret_cast<std::uintptr_t>(&stunSound));
  redirect(0x520c70, reinterpret_cast<std::uintptr_t>(&form));
}
template <class T> void put(Bytes &b, unsigned off, T n) {
  std::memcpy(b.data() + off, &n, sizeof(n));
}
void guards() {
  for (const auto &r : code)
    check(std::memcmp(reinterpret_cast<void *>(r.first), r.second.data(),
                      r.second.size()) == 0,
          "original control body changed");
}
void reachObject(Bytes &object, Bytes &config, const ReachActor &a) {
  put(config, 8, unsigned(a.width));
  put(config, 12, unsigned(a.height));
  put(object, 0xac,
      std::uint32_t(reinterpret_cast<std::uintptr_t>(config.data())));
  put(object, 8, a.position.x);
  put(object, 12, a.position.y);
  put(object, 16, a.position.z);
  put(object, 28, a.fineHeight);
  if (a.endpoint) {
    put(object, 0x618, 1U);
    put(object, 0x97f, a.endpoint->x);
    put(object, 0x983, a.endpoint->y);
    put(object, 0x987, a.endpoint->z);
  }
}
#endif
void reach(const ReachActor &a, const ReachActor &b, bool clear) {
  auto n = meleeReach(a, b, {32, 32, 8}, [&](Cell, Cell) { return clear; });
  check(!n.diagonalChecked || (n.admitted == clear),
        "native reach diagonal contract");
#ifdef MNM_CONTROL_REFERENCE
  Bytes source(0xe4b), target(0xe4b), sc(0x5c9), tc(0x5c9);
  reachObject(source, sc, a);
  reachObject(target, tc, b);
  Bytes cells(32 * 32 * 8 * 12, 0);
  global(0x6c54dc,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(cells.data())));
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  for (unsigned i = 0; i < 32; ++i)
    global(0x6cb942 + 4 * i, i * 32);
  for (unsigned i = 0; i < 8; ++i)
    global(0x6cb8c2 + 4 * i, i * 1024);
  auto sourceBefore = source, targetBefore = target;
  lineClear = clear;
  lines = 0;
  using Fn = int(__attribute__((thiscall)) *)(void *, void *, int, int, int);
  auto o = reinterpret_cast<Fn>(0x50bd60)(
      source.data(), target.data(), a.position.x, a.position.y, a.position.z);
  check(bool(o) == n.admitted && bool(lines) == n.diagonalChecked,
        "original reach/diagonal mismatch");
  check(source == sourceBefore && target == targetBefore, "reach wrote actor");
  guards();
#endif
  ++reachCases;
}
#ifdef MNM_CONTROL_REFERENCE
Bytes stunObject(const StunState &s, Bytes &config, Bytes &target,
                 Bytes &queue) {
  Bytes b(0xe4b);
  put(b, 0xa8, s.type);
  put(b, 0xe4, s.health);
  put(b, 0xa0, s.mode);
  put(b, 0x9c, std::uint32_t(s.timer));
  put(b, 0x5ec, s.behavior);
  put(b, 0x5f0, s.pending);
  put(b, 0x5fc, s.action);
  put(b, 0x664, s.engaged);
  put(target, 0x668, s.incoming);
  put(b, 0xb8b, s.marker);
  put(b, 0x164, s.readiness);
  put(b, 0xd0, s.animationDirty);
  put(b, 0x7cd, s.animationKind);
  put(config, 0x3c, s.configFlag);
  put(b, 0xac, std::uint32_t(reinterpret_cast<std::uintptr_t>(config.data())));
  put(b, 0x798, s.form);
  put(b, 0x108, s.flight);
  put(b, 0xa4, s.savedHeight);
  put(b, 0xde3, s.blockDE3);
  put(b, 0x962, std::uint8_t(s.block962));
  put(b, 0x963, s.block963);
  put(b, 0x71c, std::uint8_t(s.block71c));
  put(b, 0x723, std::uint8_t(s.block723));
  put(b, 8, s.position.x);
  put(b, 12, s.position.y);
  put(b, 16, s.position.z);
  check(!s.attachment && !s.queueEffects,
        "test scope expects empty effect ownership");
  put(b, 0x8f9, std::uint32_t(reinterpret_cast<std::uintptr_t>(queue.data())));
  put(b, 0x68, s.surface ? 1U : 0U);
  put(b, 0x64c, s.target);
  put(b, 0x6a8, 0U);
  global(0x6def5c, 1);
  global(0x6def58,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(target.data())));
  return b;
}
void writeStunFields(Bytes &b, Bytes &target, const StunState &s) {
  put(b, 0xa0, s.mode);
  put(b, 0x9c, std::uint32_t(s.timer));
  put(b, 0x5ec, s.behavior);
  put(b, 0x5f0, s.pending);
  put(b, 0x664, s.engaged);
  put(target, 0x668, s.incoming);
  put(b, 0xb8b, s.marker);
  put(b, 0x164, s.readiness);
  put(b, 0xd0, s.animationDirty);
  put(b, 0x798, s.form);
  put(b, 0xa4, s.savedHeight);
  put(b, 0xde3, s.blockDE3);
  put(b, 0x962, std::uint8_t(s.block962));
  put(b, 0x963, s.block963);
}
#endif
void stun(StunState s, int duration, unsigned mode, bool forced, bool tick,
          int event) {
  auto n = tick ? tickStun(s, event) : beginStun(s, duration, mode, forced);
#ifdef MNM_CONTROL_REFERENCE
  Bytes config(0x5c9), target(0xe4b), queue(12);
  auto b = stunObject(s, config, target, queue);
  auto before = b, targetBefore = target;
  trace.clear();
  suppliedAnimation = event;
  primaryAnimation = b.data() + 0xb0;
  redirect(0x464ec0, reinterpret_cast<std::uintptr_t>(&animationEvent));
  if (tick) {
    using Fn = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Fn>(0x50f590)(b.data());
  } else {
    using Fn =
        void(__attribute__((thiscall)) *)(void *, int, unsigned, unsigned);
    reinterpret_cast<Fn>(0x50d060)(b.data(), duration, mode, forced);
  }
  writeStunFields(before, targetBefore, n.state);
  if (before != b || targetBefore != target || trace != n.events) {
    std::cerr << "stun " << stunCases << " timer " << s.timer << " mode "
              << s.mode << " behavior " << s.behavior << " tick " << tick
              << " event " << event << '\n';
    for (auto e : n.events)
      std::cerr << "native " << int(e.kind) << ' ' << e.argument << '\n';
    for (auto e : trace)
      std::cerr << "original " << int(e.kind) << ' ' << e.argument << '\n';
    for (unsigned i = 0; i < b.size(); ++i)
      if (before[i] != b[i])
        std::cerr << "field " << std::hex << i << ' ' << unsigned(before[i])
                  << '/' << unsigned(b[i]) << std::dec << '\n';
    throw std::runtime_error("original stun state/events mismatch");
  }
  guards();
#endif
  ++stunCases;
}
void knockback(KnockbackInput p, std::uint32_t word, int allow, bool accepted) {
  unsigned calls = 0;
  auto n = admitKnockback(
      p, word, [&](Cell) { return int(++calls) == allow; },
      [&](Cell) { return accepted; });
#ifdef MNM_CONTROL_REFERENCE
  Bytes cfg(0x5c9), source(0xe4b), queue(12);
  auto b = stunObject(p.actor, cfg, source, queue);
  put(b, 0x618, unsigned(p.motion));
  put(b, 0x608, p.facing);
  // Use the real unchanged direction routine to produce the requested response.
  int sx = 0, sy = 0;
  bool found = false;
  for (int x = -2; x <= 2 && !found; ++x)
    for (int y = -2; y <= 2 && !found; ++y)
      if (relativeDirection({x, y, 0}, {0, 0, 0}, p.facing, p.dimensions) ==
          int(p.response)) {
        sx = x;
        sy = y;
        found = true;
      }
  check(found, "response fixture missing");
  put(source, 8, p.actor.position.x + sx);
  put(source, 12, p.actor.position.y + sy);
  auto before = b, sourceBefore = source;
  probes.clear();
  requested.reset();
  legalAt = allow;
  moveOk = accepted;
  aiDraws = 0;
  suppliedWord = word;
  global(0x6c5494, p.dimensions.x);
  global(0x6c5498, p.dimensions.y);
  global(0x6c549c, p.dimensions.z);
  global(0x6e1f30, 0);
  for (unsigned i = 0; i < 8; ++i) {
    check(*reinterpret_cast<int *>(0x5e178c + 12 * i) == p.offsets[i].x &&
              *reinterpret_cast<int *>(0x5e1790 + 12 * i) == p.offsets[i].y,
          "neighbor offsets differ");
  }
  redirect(0x54e200, reinterpret_cast<std::uintptr_t>(&randomWord));
  redirect(0x514360, reinterpret_cast<std::uintptr_t>(&legalProbe));
  redirect(0x512800, reinterpret_cast<std::uintptr_t>(&moveProbe));
  using Fn = unsigned(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
  auto original = reinterpret_cast<Fn>(0x521160)(b.data(), 7, 0);
  if (bool(original) != n.admitted || probes != n.checks ||
      !(requested == n.requested) || aiDraws != 1) {
    std::cerr << "knockback " << knockbacks << " facing " << p.facing
              << " response " << p.response << " word " << word << " allow "
              << allow << " accepted " << accepted << " result " << original
              << "/" << n.admitted << " draws " << aiDraws << '\n';
    for (auto q : n.checks)
      std::cerr << "native probe " << q.x << ' ' << q.y << ' ' << q.z << '\n';
    for (auto q : probes)
      std::cerr << "original probe " << q.x << ' ' << q.y << ' ' << q.z << '\n';
    if (requested)
      std::cerr << "original request " << requested->x << ' ' << requested->y
                << ' ' << requested->z << '\n';
    if (n.requested)
      std::cerr << "native request " << n.requested->x << ' ' << n.requested->y
                << ' ' << n.requested->z << '\n';
    throw std::runtime_error("knockback result/queries mismatch");
  }
  check(b == before && source == sourceBefore,
        "knockback actor writes outside helpers");
  guards();
#endif
  ++knockbacks;
}
void response(StunState s) {
  auto n = knockbackResponse(s);
#ifdef MNM_CONTROL_REFERENCE
  Bytes cfg(0x5c9), target(0xe4b), queue(12);
  auto b = stunObject(s, cfg, target, queue), expected = b,
       expectedTarget = target;
  trace.clear();
  using Fn = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Fn>(0x521420)(b.data());
  writeStunFields(expected, expectedTarget, n.state);
  check(b == expected && target == expectedTarget && trace == n.events,
        "knockback response mismatch");
  guards();
#endif
  ++stunCases;
}
void selection(AiWorld w, Selector selector, unsigned mask) {
  std::vector<unsigned> queries;
  const auto &a = w.creatures[0];
  auto admit = [&](unsigned slot) {
    queries.push_back(slot);
    return bool((mask >> slot) & 1);
  };
  Selection n;
  const bool home = selector == Selector::home;
  if (selector == Selector::home || selector == Selector::score)
    n = selectEnemy(a, w, home, admit);
  else if (selector == Selector::corpse21 || selector == Selector::corpse10)
    n = selectCorpse(a, w, selector == Selector::corpse21, admit);
  else if (selector == Selector::assist)
    n = selectAssist(a, w, [](unsigned, unsigned) { return true; }, admit);
  else
    n = selectRetaliation(a, w, admit);
  check(!n.target || *n.target < w.creatures.size(), "native selection bound");
#ifdef MNM_CONTROL_REFERENCE
  Bytes actors(0xe4b * w.creatures.size());
  std::vector<Bytes> cfg(w.creatures.size(), Bytes(0x5c9));
  for (unsigned i = 0; i < w.creatures.size(); ++i) {
    auto &v = w.creatures[i];
    Bytes b(0xe4b);
    put(b, 0, v.slot);
    put(b, 4, unsigned(v.active));
    put(b, 8, v.position.x);
    put(b, 12, v.position.y);
    put(b, 16, v.position.z);
    put(b, 0xbc3, v.home.x);
    put(b, 0xbc7, v.home.y);
    put(b, 0xbcb, v.home.z);
    put(b, 0x174, v.owner);
    put(b, 0xe4, v.health);
    put(b, 0xa8, v.type);
    put(b, 0x680, v.history);
    put(b, 0x64c, v.target);
    put(b, 0x108, v.flying);
    put(b, 0x618, unsigned(v.motion));
    put(b, 0x5ec, v.behavior);
    put(b, 0x94, unsigned(v.bloodLust));
    put(b, 0xf8, v.physical);
    put(b, 0xf0, v.corpseA);
    put(b, 0xf4, v.corpseB);
    put(b, 0x690, v.corpseClaim);
    put(b, 0x664, v.engaged);
    put(b, 0x668, v.incoming);
    put(b, 0x80d, v.recent);
    put(b, 0x811, v.age);
    put(cfg[i], 0, v.kind);
    put(cfg[i], 0x30, v.physical);
    put(cfg[i], 0x34, v.ranged);
    for (unsigned j = 0; j < 28; ++j) {
      put(cfg[i], 0x1a0 + j * 4, v.hth[j]);
      put(cfg[i], 0x3d0 + j * 4, v.enemyRadius[j]);
      put(cfg[i], 0x360 + j * 4, v.allyRadius[j]);
      put(cfg[i], 0x440 + j * 4, v.retaliateThreshold[j]);
    }
    for (unsigned j = 0; j < 8; ++j)
      put(b, 0x24 + j * 4, v.visible[j]);
    put(b, 0xac,
        std::uint32_t(reinterpret_cast<std::uintptr_t>(cfg[i].data())));
    std::copy(b.begin(), b.end(), actors.begin() + i * 0xe4b);
  }
  for (unsigned i = 0; i < 8; ++i) {
    global(0x643a80 + i * 340 + 4, w.human[i] ? 1 : 0);
    for (unsigned j = 0; j < 8; ++j)
      *reinterpret_cast<unsigned char *>(0x643a80 + i * 340 + 0x18 + j) =
          w.relation[i][j];
  }
  global(0x6c5494, w.dimensions.x);
  global(0x6c5498, w.dimensions.y);
  global(0x6df180, w.creatures.size());
  global(0x6def5c, w.creatures.size());
  global(0x6def58,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(actors.data())));
  auto expected = actors;
  admissions.clear();
  admitMask = mask;
  maximum = a.maximumHealth;
  redirect(0x524910, reinterpret_cast<std::uintptr_t>(&admission));
  redirect(0x5075f0, reinterpret_cast<std::uintptr_t>(&maxHealth));
  using Fn = unsigned(__attribute__((thiscall)) *)(void *);
  unsigned entry = selector == Selector::home       ? 0x5189e0
                   : selector == Selector::score    ? 0x518c50
                   : selector == Selector::corpse21 ? 0x518f80
                   : selector == Selector::corpse10 ? 0x519100
                   : selector == Selector::assist   ? 0x518750
                                                    : 0x519330;
  aiTrace.clear();
  redirect(0x51c3a0, reinterpret_cast<std::uintptr_t>(&aiSub));
  redirect(0x51e090, reinterpret_cast<std::uintptr_t>(&assistAdmission));
  auto o = reinterpret_cast<Fn>(entry)(actors.data());
  if (selector == Selector::priority) {
    std::vector<AiEvent> expectedTrace;
    if (n.target)
      expectedTrace.push_back({AiEffect::substate, int(n.substate)});
    check(aiTrace == expectedTrace, "retaliation substate mismatch");
  }
  if (n.target) {
    put(expected, 0x64c, *n.target);
    if (selector != Selector::corpse21 && selector != Selector::corpse10)
      put(expected, 0x68c, n.range);
  }
  if (bool(o) != bool(n.target) || expected != actors ||
      queries != admissions) {
    std::cerr << "selection " << selections << " home " << home << " target "
              << (n.target ? int(*n.target) : -1) << " original " << o << '\n';
    throw std::runtime_error("enemy selector mismatch");
  }
  guards();
#endif
  ++selections;
}
void direction(Cell a, Cell b, unsigned facing) {
  auto n = relativeDirection(a, b, facing, {32, 32, 8});
  check(n >= 0 && n < 8, "direction domain");
#ifdef MNM_CONTROL_REFERENCE
  Bytes source(0xe4b), target(0xe4b);
  put(source, 8, a.x);
  put(source, 12, a.y);
  put(target, 8, b.x);
  put(target, 12, b.y);
  put(target, 0x608, facing);
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  using Fn = int(__attribute__((thiscall)) *)(void *, void *);
  check(reinterpret_cast<Fn>(0x50c930)(source.data(), target.data()) == n,
        "direction mismatch");
  guards();
#endif
  ++directions;
}
void ready(int cooldown, int event, int set) {
  auto n = readyStep(cooldown, event, [&](int &v) {
    if (set != -999)
      v = set;
  });
  check(n.behaviorCalls <= 2, "ready callback count");
#ifdef MNM_CONTROL_REFERENCE
  Bytes b(0xe4b);
  put(b, 0x66c, cooldown);
  put(b, 0x5f4,
      std::uint32_t(reinterpret_cast<std::uintptr_t>(&readyBehavior)));
  auto expected = b;
  put(expected, 0x66c, n.cooldown);
  if (n.restartIdle)
    put(expected, 0xe0, 288U);
  readyCalls = 0;
  readySet = set;
  idleRestart = false;
  suppliedAnimation = event;
  primaryAnimation = b.data() + 0xb0;
  redirect(0x464ec0, reinterpret_cast<std::uintptr_t>(&animationEvent));
  redirect(0x464cb0, reinterpret_cast<std::uintptr_t>(&idle));
  using Fn = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Fn>(0x50f7c0)(b.data());
  check(b == expected && readyCalls == n.behaviorCalls &&
            idleRestart == n.restartIdle,
        "ready state mismatch");
  guards();
#endif
  ++readyCases;
}
void decision(AiCreature a, DecisionInputs p, std::uint32_t word) {
  auto n = decide(a, p, [&] { return word; });
  check(n.draws.size() <= 1, "AI draw count");
#ifdef MNM_CONTROL_REFERENCE
  testingAi = true;
  decisionInputs = &p;
  suppliedWord = word;
  aiDraws = 0;
  aiTrace.clear();
  Bytes b(0xe4b), cells(12);
  put(b, 0x5ec, a.behavior);
  put(b, 0x5f0, a.pending);
  put(b, 0xa8, a.type);
  put(b, 0xfc, a.gateFC);
  put(b, 0x174, 0U);
  put(b, 0x704, a.flag704);
  put(b, 0x680, a.history);
  put(b, 0x88, a.bravery);
  put(b, 0x811, a.age);
  put(b, 0x80d, a.recent);
  put(b, 0x815, a.flag815);
  put(b, 0x684, a.countdown);
  put(b, 0x164, a.readiness);
  put(b, 0xb8b, a.marker);
  put(cells, 10, p.cellFlags);
  global(0x6c54dc,
         std::uint32_t(reinterpret_cast<std::uintptr_t>(cells.data())));
  global(0x6cb942, 0);
  global(0x6cb8c2, 0);
  global(0x643a80 + 12, p.ownerEligible ? 0 : -1);
  global(0x643a80 + 4, p.humanOwner ? 1 : 0);
  global(0x6e1f30, 0);
  auto expected = b;
  put(expected, 0x680, n.actor.history);
  put(expected, 0x811, n.actor.age);
  put(expected, 0x80d, n.actor.recent);
  put(expected, 0x815, n.actor.flag815);
  put(expected, 0x932, n.actor.dirty932);
  put(expected, 0x164, n.actor.readiness);
  put(expected, 0xb8b, n.actor.marker);
  for (auto endpoint : std::vector<std::pair<unsigned, std::uintptr_t>>{
           {0x516220, reinterpret_cast<std::uintptr_t>(&aiImmediate)},
           {0x51d3a0, reinterpret_cast<std::uintptr_t>(&aiEscape)},
           {0x517820, reinterpret_cast<std::uintptr_t>(&aiPrepare)},
           {0x51d660, reinterpret_cast<std::uintptr_t>(&aiTail)},
           {0x5186c0, reinterpret_cast<std::uintptr_t>(&aiAttack)},
           {0x51c3a0, reinterpret_cast<std::uintptr_t>(&aiSub)},
           {0x5198b0, reinterpret_cast<std::uintptr_t>(&aiHazard)},
           {0x54e200, reinterpret_cast<std::uintptr_t>(&randomWord)},
           {0x519330, reinterpret_cast<std::uintptr_t>(&aiPriority)},
           {0x518f80, reinterpret_cast<std::uintptr_t>(&aiCorpse21)},
           {0x519100, reinterpret_cast<std::uintptr_t>(&aiCorpse10)},
           {0x518750, reinterpret_cast<std::uintptr_t>(&aiAssist)},
           {0x518c50, reinterpret_cast<std::uintptr_t>(&aiScore)},
           {0x5189e0, reinterpret_cast<std::uintptr_t>(&aiHome)},
           {0x51d7b0, reinterpret_cast<std::uintptr_t>(&aiInteraction)},
           {0x51db30, reinterpret_cast<std::uintptr_t>(&aiSpell)}})
    redirect(endpoint.first, endpoint.second);
  using Fn = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Fn>(0x518420)(b.data());
  check(b == expected && aiTrace == n.events && aiDraws == n.draws.size(),
        "AI precedence/state mismatch");
  for (const auto &r : code)
    if (r.first == 0x5189e0 || r.first == 0x518c50 || r.first == 0x518750 ||
        r.first == 0x518f80 || r.first == 0x519100 || r.first == 0x519330)
      std::memcpy(reinterpret_cast<void *>(r.first), r.second.data(),
                  r.second.size());
  guards();
  testingAi = false;
#endif
  ++decisions;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_CONTROL_REFERENCE
  check(argc == 2, "expected pinned original PE");
  initialize(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  for (int aw : {1, 2})
    for (int bw : {1, 2})
      for (int ah : {1, 2, 4})
        for (int bh : {1, 2, 4})
          for (int dx = -4; dx <= 4; ++dx)
            for (int dy = -4; dy <= 4; ++dy)
              for (int dz = -2; dz <= 2; ++dz)
                for (bool clear : {false, true}) {
                  ReachActor a{{15, 15, 3}, 48, aw, ah, {}, 0},
                      b{{15 + dx, 15 + dy, 3 + dz},
                        (3 + dz) * 16,
                        bw,
                        bh,
                        {},
                        0};
                  reach(a, b, clear);
                }
  for (int x : {0, 1, 31})
    for (int y : {0, 1, 31}) {
      ReachActor a{{x, y, 1}, 16, 2, 2, {}, 0},
          b{{31 - x, 31 - y, 1}, 16, 2, 2, {}, 0};
      reach(a, b, true);
      b.endpoint = Cell{1, 1, 1};
      reach(a, b, true);
    }
  for (int x = -31; x <= 31; ++x)
    for (int y = -31; y <= 31; ++y)
      for (unsigned f = 0; f < 8; ++f)
        direction({x + 31, y + 31, 0}, {31, 31, 0}, f);
  for (int cool : {-2, 0, 1, 2, 34})
    for (int ev : {0, 1, 3})
      for (int set : {-999, 0, 1, 7})
        ready(cool, ev, set);
  for (unsigned f = 0; f < 8; ++f)
    for (unsigned resp = 0; resp < 8; ++resp)
      for (unsigned word : {0U, 50U, 51U, 99U, 0x80000033U})
        for (int allow : {-1, 1, 2, 3, 5, 9})
          for (bool ok : {false, true}) {
            KnockbackInput p;
            p.actor.position = {1, 1, 1};
            p.facing = f;
            p.response = resp;
            p.offsets = {{{0, -1, 0},
                          {1, -1, 0},
                          {1, 0, 0},
                          {1, 1, 0},
                          {0, 1, 0},
                          {-1, 1, 0},
                          {-1, 0, 0},
                          {-1, -1, 0}}};
            knockback(p, word, allow, ok);
          }
  for (unsigned type : {17U, 22U, 27U})
    for (unsigned behavior : {1U, 3U, 6U, 8U, 14U, 15U, 16U})
      for (unsigned mode : {0U, 1U, 3U})
        for (unsigned engaged : {0U, 1U}) {
          StunState s;
          s.type = type;
          s.behavior = behavior;
          s.mode = mode;
          s.engaged = engaged;
          s.incoming = 1;
          response(s);
        }
  for (unsigned mask = 0; mask < 4096; ++mask)
    for (bool home : {false, true}) {
      AiWorld w;
      w.creatures.resize(4);
      w.relation[0][0] = 1;
      for (unsigned i = 0; i < 4; ++i) {
        auto &v = w.creatures[i];
        v.slot = i;
        v.owner = i ? 1 : 0;
        v.position = {int(2 + i * 2), 2, 1};
        v.home = {2, 2, 1};
        v.enemyRadius.fill((mask & 1) ? 8 : -8);
        v.hth.fill(18);
        v.health = (mask & 2) ? 180 : 360;
        v.target = 0;
        v.ranged = mask & 4;
        v.behavior = (mask & 8) ? 3 : 10;
        v.motion = mask & 16;
        v.active = !(mask & (32U << i));
        v.visible[0] = !(mask & 512);
      }
      w.creatures[0].history = (mask & 1024) ? 3 : 0;
      w.creatures[0].bloodLust = mask & 2048;
      selection(w, home ? Selector::home : Selector::score, mask >> 3);
    }
  for (unsigned mask = 0; mask < 256; ++mask)
    for (bool home : {false, true}) {
      AiWorld w;
      w.creatures.resize(4);
      w.relation[0][0] = 1;
      for (unsigned i = 0; i < 4; ++i) {
        auto &v = w.creatures[i];
        v.slot = i;
        v.owner = i ? 1 : 0;
        v.position = {int(2 + i * 3), int(2 + i), 1};
        v.home = {2, 2, 1};
        v.health = (mask & (1U << i)) ? 120 + i * 37 : 360;
        v.physical = (mask & (16U << i)) ? 1 : 0;
        v.hth.fill(int(3 + i * 7));
        v.enemyRadius.fill((mask & 1) ? 9 : -9);
      }
      w.creatures[0].kind = (mask % 7 == 0) ? 0 : 1;
      w.human[0] = mask % 3 == 0;
      selection(w, home ? Selector::home : Selector::score, mask >> 3);
    }
  for (unsigned mask = 0; mask < 1024; ++mask)
    for (auto selector : {Selector::assist, Selector::corpse21,
                          Selector::corpse10, Selector::priority}) {
      AiWorld w;
      w.creatures.resize(4);
      w.relation[0][0] = 1;
      for (unsigned i = 0; i < 4; ++i) {
        auto &v = w.creatures[i];
        v.slot = i;
        v.owner = (i == 0 || i == 2) ? 0 : 1;
        v.position = {int(2 + i), 2, 1};
        v.home = {2, 2, 1};
        v.enemyRadius.fill(8);
        v.allyRadius.fill(8);
        v.retaliateThreshold.fill((mask & 1) ? -10 : 10);
        v.hth.fill(18);
        v.active = !(mask & (2U << i));
        v.physical = (mask & 32) ? 0 : 1;
        v.health = (mask & 64) ? 180 : 360;
        v.visible[0] = !(mask & 128);
        v.target = 1;
        v.engaged = mask & 256;
        v.recent = 2;
        v.age = (mask & 512) ? 100 : 80;
      }
      if (selector == Selector::corpse21 || selector == Selector::corpse10) {
        w.creatures[0].type = selector == Selector::corpse21 ? 21 : 10;
        for (unsigned i = 1; i < 4; ++i) {
          w.creatures[i].health = 0;
          w.creatures[i].corpseA = mask & 256;
          w.creatures[i].corpseB = mask & 512;
          w.creatures[i].corpseClaim = mask & 1;
        }
      }
      selection(w, selector, mask >> 4);
    }
  for (unsigned mode : {0U, 1U, 2U, 3U})
    for (unsigned behavior : {1U, 2U, 3U, 6U, 8U, 15U, 16U})
      for (int duration : {-1, 0, 1, 20, 39})
        for (bool engaged : {false, true})
          for (bool forced : {false, true}) {
            StunState s;
            s.mode = mode;
            s.behavior = behavior;
            s.pending = 3;
            s.engaged = engaged;
            s.incoming = 1;
            s.timer = 10;
            stun(s, duration, 0, forced, false, 0);
          }
  for (unsigned mode : {0U, 1U, 2U, 3U})
    for (int timer : {-1, 0, 1, 2, 39})
      for (unsigned pending : {0U, 2U, 3U, 6U})
        for (int event : {0, 1, 3, 100}) {
          StunState s;
          s.mode = mode;
          s.behavior = 8;
          s.pending = pending;
          s.timer = timer;
          stun(s, 0, 0, false, true, event);
        }
  for (unsigned type : {17U, 27U})
    for (unsigned health : {0U, 360U})
      for (unsigned mode : {0U, 1U, 2U, 3U})
        for (unsigned incomingMode : {0U, 1U, 2U, 3U})
          for (unsigned form : {0U, 2U})
            for (unsigned action : {10U, 20U, 15U}) {
              StunState s;
              s.type = type;
              s.health = health;
              s.mode = mode;
              s.form = form;
              s.action = action;
              s.pending = 3;
              s.engaged = 1;
              s.incoming = 1;
              stun(s, 20, incomingMode, true, false, 0);
            }
  for (int target : {-1, 0})
    for (unsigned secondary : {0U, 1U})
      for (unsigned mode : {0U, 1U, 2U, 3U})
        for (int event : {0, 1, 3, 100}) {
          StunState s;
          s.behavior = 8;
          s.pending = 3;
          s.target = target;
          s.animationKind = secondary;
          s.mode = mode;
          s.timer = 0;
          stun(s, 0, 0, false, true, event);
        }
  for (unsigned behavior : {1U, 2U, 3U, 8U, 10U, 11U})
    for (unsigned type : {17U, 22U, 27U})
      for (unsigned mask = 0; mask < 1024; ++mask) {
        AiCreature a;
        a.behavior = behavior;
        a.type = type;
        a.age = (mask & 1) ? 80 : 0;
        a.history = 5;
        a.recent = 3;
        a.flag815 = 9;
        a.countdown = (mask & 2) ? 5 : 0;
        a.pending = (mask & 4) ? 0 : 3;
        DecisionInputs p;
        p.immediate = mask & 1;
        p.priority = mask & 2;
        p.corpse21 = mask & 4;
        p.corpse10 = mask & 8;
        p.assist = mask & 16;
        p.score = mask & 32;
        p.home = mask & 64;
        p.interaction = mask & 128;
        p.spell = mask & 256;
        p.hazard = (mask & 512) ? 25 : 0;
        p.cellFlags = (mask % 7 == 0) ? 32 : 0;
        decision(a, p, mask);
      }
  std::cout << "{\"success\":true,\"reach_cases\":" << reachCases
            << ",\"stun_cases\":" << stunCases
            << ",\"ready_cases\":" << readyCases
            << ",\"decisions\":" << decisions
            << ",\"directions\":" << directions
            << ",\"knockbacks\":" << knockbacks
            << ",\"selections\":" << selections << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << "reach " << reachCases << " stun " << stunCases << ": "
            << e.what() << '\n';
  return 1;
}
