#define main unused_replacement_brain_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_task_lifecycle.hpp"
namespace {
std::vector<float> scores;
#ifdef MNM_BRAIN_REFERENCE
unsigned cursor = 0, savedCursor = 0;
bool installed = false;
void *previousAddress = nullptr;
void __attribute__((thiscall)) prepareTask(void *, void *, void *, int) {
  cursor = 0;
}
unsigned __attribute__((thiscall)) nextTask(void *, float *v, int *budget,
                                            int) {
  if (cursor == scores.size() || budget[2] < 1)
    return 0;
  --budget[2];
  *v = scores[cursor++];
  return 1;
}
void __attribute__((thiscall)) saveTask(void *) { savedCursor = cursor; }
void __attribute__((thiscall)) installTask(void *, void *) { installed = true; }
void __attribute__((thiscall)) destroyTask(void *) {}
void __attribute__((cdecl)) releaseTask(void *p) {
  if (p != previousAddress)
    release(p);
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = 1;
  segment.limit_in_pages = 1;
  segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "FS");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&releaseTask));
  redirect(0x4289e0, reinterpret_cast<void *>(&installTask));
  redirect(0x418050, reinterpret_cast<void *>(&destroyTask));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, replaced = 0, retained = 0, progress = 0, improved = 0,
           completed = 0;
  for (unsigned n = 0; n < 2048; ++n) {
    scores.clear();
    for (unsigned i = 0; i < 1 + n % 5; ++i)
      scores.push_back(std::array<float, 9>{-1, -.9f, -.5f, 0, .125f, .5f, .75f,
                                            .95f, 1}[(n / 5 + i) % 9]);
    m::OwnerPreviousTask old;
    old.started = 10;
    old.expires = 20 + int(n % 90);
    old.completed = n % 7 == 0;
    old.weighted = n % 3 == 0;
    old.value = std::array<float, 5>{-1, -.5f, 0, .5f, 1}[n % 5];
    old.baseline = float(n % 11) / 8;
    old.successes = int(n % 3);
    old.attempts = int(n % 13);
    m::OwnerReplacementPolicy policy;
    // Force an expired, low-progress positive proposal whose score is worse:
    // only the original pow(progress, elapsed) fallback can admit it.
    if (n == 16) {
      scores = {.25f};
      old.expires = 20;
      old.completed = old.weighted = false;
      old.value = .5f;
      old.baseline = 0;
      old.successes = 0;
      old.attempts = 999;
    }
    const int tick = n == 16 ? 110 : 10 + int(n % 240);
    const bool hasOld = n % 11 != 0;
    m::OwnerTaskSelection selection;
    selection.weighted = n % 2;
    auto random = m::seedRandom(n * 137 + 17);
    unsigned nativeCursor = 0, nativeSaved = 0;
    m::OwnerTaskSelectionServices services{
        1,
        true,
        [&](unsigned) { nativeCursor = 0; },
        [&](unsigned, int &b) -> std::optional<float> {
          if (nativeCursor == scores.size() || b < 1)
            return {};
          --b;
          return scores[nativeCursor++];
        },
        [&](unsigned) { nativeSaved = nativeCursor; },
        []() {}};
#ifdef MNM_BRAIN_REFERENCE
    Bytes brain(0x12100), group(0x8d), actor(0xe4b), previous(0x100),
        evaluator(0x160), queueHead(32), pendingHead(32), queueNil(32);
    std::array<unsigned, 4> vt{address(reinterpret_cast<void *>(&prepareTask)),
                               0, address(reinterpret_cast<void *>(&nextTask)),
                               address(reinterpret_cast<void *>(&saveTask))};
    unsigned list[1] = {address(evaluator.data())};
    put(queueNil.data(), 0, address(queueNil.data()));
    put(queueNil.data(), 4, address(queueNil.data()));
    put(queueNil.data(), 8, address(queueNil.data()));
    queueNil[16] = 1;
    global(0x5faf50, address(queueNil.data()));
    for (auto *head : {queueHead.data(), pendingHead.data()}) {
      put(head, 0, address(head));
      put(head, 4, address(queueNil.data()));
      put(head, 8, address(head));
      head[16] = 1;
    }
    put(brain.data(), 0xbe, address(queueHead.data()));
    put(brain.data(), 0x11dc7, address(pendingHead.data()));
    put(evaluator.data(), 0, address(vt.data()));
    put(brain.data(), 8, address(list));
    put(brain.data(), 12, address(list + 1));
    put(brain.data(), 0x14, tick);
    brain[0x19] = 1;
    brain[0x11dd3] = 1;
    brain[0x11fe0] = 1;
    put(group.data(), 0x35, address(actor.data()));
    put(group.data(), 0x89, 3);
    for (unsigned i = 0; i < 8; ++i)
      group[0x41 + i] = 1;
    put(previous.data(), 0xc, old.started);
    put(previous.data(), 0x10, old.expires);
    put(previous.data(), 8, address(group.data()));
    put(previous.data(), 0x23, unsigned(-1));
    previous[0x15] = old.completed;
    previous[0x1a] = old.weighted;
    put(previous.data(), 0x1b, bits(old.value));
    put(previous.data(), 0x2b, bits(old.baseline));
    put(previous.data(), 0x2f, old.successes);
    put(previous.data(), 0x33, old.attempts);
    put(group.data(), 5, hasOld ? address(previous.data()) : 0);
    previousAddress = previous.data();
    cursor = savedCursor = 0;
    installed = false;
    global(0x643a80, 1);
    global(0x643a80 + 12, unsigned(-1));
    global(0x6def5c, 1);
    global(0x5d854c, 2);
    global(0x6e1f64, 0);
    global(0x5d8650, selection.weighted ? 2 : 0);
    global(0x5d8654, bits(policy.improvement));
    global(0x5d8658, bits(policy.expiry));
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
#endif
    do {
      ++slices;
      int budget = 1 + int((n + slices) % 4);
      const int before = budget;
      m::advanceOwnerTaskSelection(selection, budget, random, services);
      const bool expected =
          selection.reset && nativeSaved &&
          m::replaceOwnerTask(selection.value, hasOld ? &old : nullptr, tick,
                              policy);
#ifdef MNM_BRAIN_REFERENCE
      int rawBudget[3] = {0, 0, before};
      using Plan = void(__attribute__((thiscall)) *)(void *, void *, int *);
      reinterpret_cast<Plan>(0x423590)(brain.data(), group.data(), rawBudget);
      record(expected, installed, "direct-root replacement");
      record(unsigned(budget), unsigned(rawBudget[2]), "driver budget");
      record(nativeCursor, cursor, "driver candidate cursor");
      record(nativeSaved, savedCursor, "driver saved candidate");
      record(bits(selection.value),
             *reinterpret_cast<unsigned *>(brain.data() + 0x11ddd),
             "driver selected value");
      record(random.left, *reinterpret_cast<unsigned *>(0x6a29d8),
             "driver RNG left");
      record(random.right, *reinterpret_cast<unsigned *>(0x6a29dc),
             "driver RNG right");
#else
      (void)before;
      for (unsigned value :
           {unsigned(expected), unsigned(budget), nativeCursor, nativeSaved,
            bits(selection.value), random.left, random.right})
        record(value, value, "native driver");
#endif
      if (selection.reset) {
        replaced += expected;
        retained += !expected;
        if (expected && hasOld) {
          const long double denominator =
              1.L - static_cast<long double>(selection.value) * old.value;
          if (old.completed)
            ++completed;
          else if (denominator > 0 &&
                   (static_cast<long double>(selection.value) - old.value) /
                           denominator >
                       policy.improvement)
            ++improved;
          else
            ++progress;
        }
      }
      need(slices < 100000, "driver did not finish");
    } while (!selection.reset);
#ifdef MNM_BRAIN_REFERENCE
    // Scratch vector owners are empty in this one-source direct-root domain.
    need(allocated.empty(), "driver allocation leaked");
#endif
  }
  need(replaced && retained && progress && improved && completed,
       "driver decision witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":2048,\"checks\":" << checks
            << ",\"slices\":" << slices << ",\"replaced\":" << replaced
            << ",\"retained\":" << retained
            << ",\"progress_replacements\":" << progress
            << ",\"improved_replacements\":" << improved
            << ",\"completed_replacements\":" << completed << ",\"digest\":\""
            << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
