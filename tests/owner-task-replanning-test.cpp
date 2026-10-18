#include "../reconstruction/creature-ai/owner_task_lifecycle.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_BRAIN_REFERENCE
#define main unused_planner_mapper_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
namespace m = mnm::reconstruction::combat;
namespace {
void need(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(why);
}
unsigned bits(float f) {
  unsigned v;
  std::memcpy(&v, &f, 4);
  return v;
}
struct Evaluator {
  std::vector<float> values;
  unsigned cursor = 0, current = 0;
};
std::vector<Evaluator> evaluators;
std::vector<unsigned> events;
void prepare(unsigned i) {
  evaluators[i].cursor = 0;
  events.insert(events.end(), {0, i});
}
std::optional<float> next(unsigned i, int &budget) {
  auto &e = evaluators[i];
  events.insert(events.end(), {1, i, unsigned(budget), e.cursor});
  if (e.cursor == e.values.size() || budget < 1)
    return {};
  --budget;
  e.current = e.cursor++;
  return e.values[e.current];
}
void saved(unsigned i) {
  events.insert(events.end(), {2, i, evaluators[i].current});
}
std::uint64_t digest = 1469598103934665603ull;
unsigned checks = 0, fixture = 0;
void check(unsigned a, unsigned b, const char *why) {
  if (a != b)
    throw std::runtime_error(
        std::string(why) + " fixture=" + std::to_string(fixture) +
        " native=" + std::to_string(a) + " original=" + std::to_string(b));
  ++checks;
  digest = (digest ^ a) * 1099511628211ull;
}
#ifdef MNM_BRAIN_REFERENCE
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<char *>(p) + off, &v, 4);
}
unsigned get(const void *p, unsigned off = 0) {
  unsigned v;
  std::memcpy(&v, static_cast<const char *>(p) + off, 4);
  return v;
}
unsigned address(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
struct RawEvaluator {
  std::array<unsigned char, 0x160> bytes{};
};
void __attribute__((thiscall)) oldPrepare(void *p, void *, void *, int) {
  prepare(get(p, 0x140));
}
unsigned __attribute__((thiscall)) oldNext(void *p, float *score, int *budget,
                                           int) {
  auto v = next(get(p, 0x140), budget[2]);
  if (!v)
    return 0;
  *score = *v;
  return 1;
}
void __attribute__((thiscall)) oldSave(void *p) { saved(get(p, 0x140)); }
using Advance = unsigned(__attribute__((thiscall)) *)(void *, void *, int *,
                                                      float, void *, unsigned,
                                                      unsigned);
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  const auto pe = read(argv[1]);
  map_image(pe);
  const unsigned textEnd = 0x5c0000;
  Bytes text(reinterpret_cast<unsigned char *>(0x401000),
             reinterpret_cast<unsigned char *>(textEnd));
  need(mprotect(reinterpret_cast<void *>(0x401000), textEnd - 0x401000,
                PROT_READ | PROT_EXEC) == 0,
       "text seal");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, yields = 0, selections = 0, weighted = 0;
  // Defined native refusal for the original exponential zero-draw/ftol edge.
  auto zeroRandom = m::seedRandom(17);
  zeroRandom.words.fill(0);
  bool refused = false;
  try {
    (void)m::ownerTaskDuration(zeroRandom);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  need(refused, "zero lifetime draw admitted");
  for (unsigned n = 0; n < 8192; ++n) {
    fixture = n;
    const unsigned count = 1 + n % 7;
    evaluators.assign(count, {});
    for (unsigned i = 0; i < count; ++i)
      for (unsigned j = 0; j < (n + i * 3) % 9; ++j)
        evaluators[i].values.push_back(
            std::array<float, 9>{-1, -.75f, -.5f, 0, .25f, .5f, .75f, 1,
                                 1}[(n / 7 + i + j * 5) % 9]);
    m::OwnerPreviousTask old;
    old.started = 10;
    old.expires = 110;
    old.completed = n % 7 == 0;
    old.weighted = n % 3 == 0;
    old.value = std::array<float, 5>{-1, -.5f, 0, .5f, 1}[n % 5];
    old.baseline = float(n % 13) / 4;
    old.successes = int(n % 5);
    old.attempts = int(n % 9);
    const int tick = 10 + int(n % 151);
    const bool hasOld = n % 11 != 0;
    m::OwnerReplacementPolicy policy;
    policy.ownerAvailable = n % 4 != 0;
    policy.ownerNumerator = int(n % 19);
    policy.ownerDenominator = int(n % 7);
    if (n % 17 == 0)
      policy.improvement = 0;
    if (n % 19 == 0)
      policy.expiry = .5f;
    m::OwnerTaskSelection state;
    state.weighted = n % 2;
    m::RandomState random = m::seedRandom(n * 137u + 17);
    m::OwnerTaskSelectionServices services{count, n % 3 != 0, prepare,
                                           next,  saved,      [] {}};
    const float bias = n % 13 == 0 ? .125f : 0.f;
    const bool save = n % 11 != 0, reject = n % 5 == 0;
#ifdef MNM_BRAIN_REFERENCE
    Bytes brain(0x12100), group(0x8d), actor(0xe4b), previousTask(0x100);
    std::array<unsigned, 4> vtable{
        address(reinterpret_cast<void *>(&oldPrepare)), 0,
        address(reinterpret_cast<void *>(&oldNext)),
        address(reinterpret_cast<void *>(&oldSave))};
    std::vector<RawEvaluator> objects(count);
    std::vector<unsigned> list;
    for (unsigned i = 0; i < count; ++i) {
      put(objects[i].bytes.data(), 0, address(vtable.data()));
      put(objects[i].bytes.data(), 0x140, i);
      list.push_back(address(objects[i].bytes.data()));
    }
    put(brain.data(), 8, address(list.data()));
    put(brain.data(), 12, address(list.data() + count));
    put(brain.data(), 0x11fe0, 1);
    put(brain.data(), 0x11ddc, state.weighted);
    put(brain.data(), 0x11ddd, bits(state.value));
    put(group.data(), 0x35, address(actor.data()));
    put(group.data(), 0x89, 3);
    if (!services.root)
      put(group.data(), 0x39, address(group.data()));
    for (unsigned i = 0; i < 8; ++i)
      group[0x41 + i] = 1;
    put(previousTask.data(), 0xc, old.started);
    put(previousTask.data(), 0x10, old.expires);
    previousTask[0x15] = old.completed;
    previousTask[0x1a] = old.weighted;
    put(previousTask.data(), 0x1b, bits(old.value));
    put(previousTask.data(), 0x2b, bits(old.baseline));
    put(previousTask.data(), 0x2f, old.successes);
    put(previousTask.data(), 0x33, old.attempts);
    global(0x643a80, policy.ownerAvailable);
    global(0x643a80 + 12, 0);
    global(0x6def58, address(actor.data()));
    global(0x6def5c, 1);
    put(actor.data(), 0x922, policy.ownerNumerator);
    put(actor.data(), 0x926, policy.ownerDenominator);
    global(0x5d8654, bits(policy.improvement));
    global(0x5d8658, bits(policy.expiry));
    put(brain.data(), 0x14, tick);
    global(0x6e1f64, 0);
    global(0x5d854c, 3);
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
#endif
    auto durationRandom = m::seedRandom(n * 31 + 7);
    const auto beforeDuration = durationRandom;
    const int duration = m::ownerTaskDuration(durationRandom);
#ifdef MNM_BRAIN_REFERENCE
    global(0x6a29d8, beforeDuration.left);
    global(0x6a29dc, beforeDuration.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), beforeDuration.words.data(),
                1000);
    using Duration = long double(__attribute__((thiscall)) *)(void *, float);
    check(unsigned(duration),
          unsigned(int(reinterpret_cast<Duration>(0x40b3f0)(nullptr, 100))),
          "task duration");
    check(durationRandom.left, get(reinterpret_cast<void *>(0x6a29d8)),
          "duration RNG left");
    check(durationRandom.right, get(reinterpret_cast<void *>(0x6a29dc)),
          "duration RNG right");
    for (unsigned word = 0; word < 250; ++word)
      check(durationRandom.words[word],
            get(reinterpret_cast<void *>(0x6a29e0), 4 * word),
            "duration RNG words");
    using Progress = long double(__attribute__((thiscall)) *)(void *);
    const auto actualProgress =
        reinterpret_cast<Progress>(0x40a3b0)(previousTask.data());
    need(std::abs(actualProgress - m::ownerTaskProgress(old, policy)) < 1e-12L,
         "task progress");
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
#else
    (void)beforeDuration;
    check(unsigned(duration), unsigned(duration), "task duration");
    check(durationRandom.left, durationRandom.left, "duration RNG left");
    check(durationRandom.right, durationRandom.right, "duration RNG right");
    for (unsigned word : durationRandom.words)
      check(word, word, "duration RNG words");
#endif
    do {
      ++slices;
      int budget = 1 + int((n + slices) % 5);
      const auto previous = evaluators;
      events.clear();
      const bool changed = m::advanceOwnerTaskReplanning(
          state, budget, random, services, hasOld ? &old : nullptr, tick,
          policy, bias, save, reject);
      const auto expectedEvents = events;
      const auto expected = evaluators;
#ifdef MNM_BRAIN_REFERENCE
      evaluators = previous;
      events.clear();
      int rawBudget[3] = {0, 0, 1 + int((n + slices) % 5)};
      const auto result = reinterpret_cast<Advance>(0x4284e0)(
          brain.data(), group.data(), rawBudget, bias,
          hasOld ? previousTask.data() : nullptr, save, reject);
      check(changed, result & 255, "slice result");
      check(unsigned(budget), unsigned(rawBudget[2]), "budget");
      check(state.reset, get(brain.data(), 0x11fe0) & 255, "reset");
      const auto cursor = get(brain.data(), 0x11fe1);
      check(state.reset ? 0 : state.evaluator,
            cursor ? unsigned((cursor - address(list.data())) / 4) : 0,
            "evaluator cursor");
      check(bits(state.value), get(brain.data(), 0x11ddd), "best score");
      check(state.ties, get(brain.data(), 0x11de1), "tie count");
      check(bits(state.weight), get(brain.data(), 0x11fe9), "weight");
      const auto selected = get(brain.data(), 0x11fe5);
      check(unsigned(state.selected),
            selected ? unsigned((selected - address(list.data())) / 4)
                     : unsigned(-1),
            "selected evaluator");
      check(random.left, get(reinterpret_cast<void *>(0x6a29d8)),
            "random left");
      check(random.right, get(reinterpret_cast<void *>(0x6a29dc)),
            "random right");
      for (unsigned i = 0; i < 250; ++i)
        check(random.words[i], get(reinterpret_cast<void *>(0x6a29e0), i * 4),
              "random state");
      need(events == expectedEvents, "ordered evaluator consequences");
      for (unsigned i = 0; i < count; ++i)
        need(evaluators[i].cursor == expected[i].cursor, "candidate cursor");
      evaluators = expected;
#else
      (void)previous;
      (void)expected;
      for (unsigned v :
           {unsigned(changed), unsigned(budget), unsigned(state.reset),
            state.reset ? 0 : state.evaluator, bits(state.value), state.ties,
            bits(state.weight), unsigned(state.selected), random.left,
            random.right})
        check(v, v, "native");
      for (unsigned v : random.words)
        check(v, v, "native random");
#endif
      for (unsigned v : expectedEvents)
        check(v, v, "events");
      yields += !state.reset;
      selections += changed;
      weighted += changed && state.weighted;
      need(slices < 200000, "selection failed to finish");
    } while (!state.reset);
  }
  need(yields && selections && weighted, "missing selection witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "original text modified");
#endif
  std::cout << "{\"success\":true,\"cases\":8192,\"checks\":" << checks
            << ",\"slices\":" << slices << ",\"yields\":" << yields
            << ",\"selections\":" << selections << ",\"weighted\":" << weighted
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
