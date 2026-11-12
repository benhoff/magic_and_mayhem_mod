#define main unused_screen_provider_main
#include "screen-provider-reference.cpp"
#undef main
struct ExceptionMemory : Memory {
  std::vector<U> constructed, destroyed;
  U invoke(U f, U p, std::initializer_list<U> args = {}) {
    if (f == 0x7f001000) {
      invoke(0x4d27b0, p);
      invoke(0x4d28c0, p, {Text});
      constructed.push_back(p);
      return p;
    }
    if (f == 0x7f001040) {
      destroyed.push_back(p);
      invoke(0x4d27d0, p);
      return 0;
    }
    if (auto value =
            mnm::reconstruction::ScreenResourceProviders<ExceptionMemory>(*this)
                .tryRun(f, p, args))
      return *value;
    return endpoint(f, p, args);
  }
};
int main() try {
  U cases = 0;
  for (U count = 0; count <= 24; ++count)
    for (U fail = 1; fail <= count + 1; ++fail) {
      ExceptionMemory m;
      m.initialize();
      m.fail = fail;
      bool thrown = false;
      try {
        m.invoke(0x59c441, 0,
                 {Base + 0x1000, 0x29, count, 0x7f001000, 0x7f001040});
      } catch (const WireFault &) {
        thrown = true;
      }
      check(thrown == (fail <= count), "constructor failure reachability");
      if (thrown) {
        check(m.constructed.size() == fail - 1, "completed constructor prefix");
        check(m.destroyed.size() == m.constructed.size(), "incomplete unwind");
        for (U i = 0; i < m.destroyed.size(); ++i)
          check(m.destroyed[i] == m.constructed[m.constructed.size() - 1 - i],
                "unwind order");
        check(m.alive.empty(), "unwind leaked text");
      } else {
        check(m.destroyed.empty(), "success unwound constructors");
        for (auto p : m.constructed)
          m.invoke(0x7f001040, p);
        check(m.alive.empty(), "normal array teardown leaked");
      }
      ++cases;
    }
  std::cout << "{\"constructor_failure_cases\":" << cases
            << ",\"completed_prefix_reverse_cleanup\":true}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
