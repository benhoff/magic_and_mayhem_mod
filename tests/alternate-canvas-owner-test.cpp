#include "../compat/legacy/alternate_canvas_dispatch.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <type_traits>

using mnm::render::AlternateCanvas;
namespace re = mnm::reconstruction;
using mnm::compat::dispatchOwnedAlternateCanvas;
namespace {
unsigned checks = 0, frames = 0, refusals = 0;
void check(bool ok) {
  ++checks;
  if (!ok) throw std::runtime_error("Alternate canvas ownership check " + std::to_string(checks));
}
template<class F> void refuse(F f) {
  bool caught = false;
  try { f(); } catch (const std::logic_error&) { caught = true; }
  check(caught); ++refusals;
}
std::uint16_t pixel(unsigned x, unsigned y) { return std::uint16_t(1 + (x * 13 + y * 37) % 65534); }
void layouts() {
  for (bool high : {false, true}) for (unsigned xp : {0u, 4u, 96u})
    for (unsigned yp : {0u, 3u}) for (bool locked : {false, true}) {
      AlternateCanvas owner;
      const AlternateCanvas::Layout layout{high, xp, yp};
      const auto handle = owner.publish(layout);
      const unsigned w = layout.visibleWidth(), h = layout.visibleHeight();
      std::vector<std::uint16_t> ordinary(std::size_t(w) * h, 0xaaaa);
      const re::DispatchBinding binding{{&ordinary, 0}, w, h};
      re::DispatchState state{binding, {2, 3, w - 2, h - 3}};
      const auto clip = state.clip;
      std::string events;
      bool pinObserved = false;
      check(dispatchOwnedAlternateCanvas(owner, handle, binding, clip, 2, state,
        [&] { events += 'L'; return locked; },
        [&](re::DispatchState& s) {
          events += 'W'; check(s.binding.stride == w + 2 * xp && s.binding.height == h + 2 * yp);
          check(s.clip.right == clip.right + 2 * xp && s.clip.bottom == clip.bottom + 2 * yp);
          auto& words = *s.binding.span.words;
          check(std::all_of(words.begin(), words.end(), [](auto p) { return p == 0; }));
          for (unsigned y = 0; y < h + 2 * yp; ++y)
            for (unsigned x = 0; x < w + 2 * xp; ++x) words[std::size_t(y) * (w + 2 * xp) + x] = pixel(x, y);
          refuse([&] { owner.publish(layout); });
          refuse([&] { owner.acquire(handle); });
          pinObserved = !handle.expired();
        },
        [&](re::DispatchState& s, unsigned i) {
          events += char('0' + i); check(s.binding.span.words == &ordinary);
          check(s.clip.right == clip.right && s.clip.bottom == clip.bottom);
          (*s.binding.span.words)[i] = std::uint16_t(0xf000 + i);
        }, [&] { events += 'U'; }) == locked);
      check(pinObserved); check(events == (locked ? "WL01U" : "WL"));
      check(state.binding.span.words == &ordinary);
      for (unsigned y = 0; y < h; ++y) for (unsigned x = 0; x < w; ++x) {
        const auto at = std::size_t(y) * w + x;
        const auto expected = !locked ? 0xaaaau : at < 2 ? 0xf000u + unsigned(at) : pixel(x + xp, y + yp);
        check(ordinary[at] == expected);
      }
      { // A failed Lock still leaves World output in its owned generation.
        auto lease = owner.acquire(handle);
        for (unsigned y = 0; y < h + 2 * yp; ++y)
          for (unsigned x = 0; x < w + 2 * xp; ++x)
            check(lease.words()[std::size_t(y) * (w + 2 * xp) + x] == pixel(x, y));
      }
      { auto lease = owner.acquire(handle); check(lease.words().back() == pixel(w + 2 * xp - 1, h + 2 * yp - 1)); }
      owner.retire(); check(handle.expired()); ++frames;
    }
}
void lifetime() {
  static_assert(!std::is_copy_constructible_v<AlternateCanvas>);
  static_assert(!std::is_copy_constructible_v<AlternateCanvas::Lease>);
  AlternateCanvas owner, other;
  auto handle = owner.publish({});
  refuse([&] { other.acquire(handle); });
  refuse([&] { owner.acquire({}); });
  {
    auto lease = owner.acquire(handle);
    lease.words()[17] = 99;
    auto moved = std::move(lease);
    refuse([&] { lease.words(); });
    check(moved.words()[17] == 99);
    owner.retire(); check(!owner.published() && !handle.expired());
    refuse([&] { owner.acquire(handle); });
    refuse([&] { owner.publish({}); });
    check(moved.words()[17] == 99);
  }
  check(handle.expired());
  refuse([&] { owner.acquire(handle); });
  auto fresh = owner.publish({});
  { auto lease = owner.acquire(fresh); check(lease.words()[17] == 0); }
  for (auto bad : {AlternateCanvas::Layout{false, 0xffffffff, 0},
                  AlternateCanvas::Layout{true, 0, 0xffffffff},
                  AlternateCanvas::Layout{false, 1729, 0},
                  AlternateCanvas::Layout{true, 0, 1749}}) {
    refuse([&] { owner.publish(bad); });
    auto lease = owner.acquire(fresh); check(lease.words()[17] == 0);
  }
  auto replaced = owner.publish({true, 96, 3});
  check(fresh.expired()); refuse([&] { owner.acquire(fresh); });
  { auto lease = owner.acquire(replaced); check(lease.words().size() == 992u * 606); }
  // A lease survives its owner, including move assignment over another lease.
  AlternateCanvas::Lease survivor;
  AlternateCanvas::Handle departed;
  {
    AlternateCanvas temporary;
    departed = temporary.publish({});
    survivor = temporary.acquire(departed); survivor.words()[9] = 77;
  }
  check(!departed.expired() && survivor.words()[9] == 77);
  survivor = {}; check(departed.expired());
  owner.retire(); owner.retire(); check(replaced.expired());
  for (bool high : {false, true}) {
    auto maximum = owner.publish({high, high ? 1648u : 1728u, high ? 1748u : 1808u});
    { auto lease = owner.acquire(maximum);
      check(lease.words().size() == 4096u * 4096);
      check(std::all_of(lease.words().begin(), lease.words().end(), [](auto p) { return p == 0; })); }
    owner.retire(); check(maximum.expired());
  }
}
void dispatchRetirement() {
  for (bool locked : {false, true}) {
    AlternateCanvas owner;
    auto handle = owner.publish({});
    std::vector<std::uint16_t> ordinary(640 * 480, 11);
    re::DispatchBinding binding{{&ordinary, 0}, 640, 480};
    re::DispatchState state{binding, {0, 0, 640, 480}};
    check(dispatchOwnedAlternateCanvas(owner, handle, binding, state.clip, 0, state,
      [&] { check(!handle.expired()); return locked; },
      [&](auto& s) {
        std::fill(s.binding.span.words->begin(), s.binding.span.words->end(), 37);
        owner.retire(); check(!handle.expired());
      }, [](auto&, unsigned) {}, [] {}) == locked);
    check(handle.expired() && state.binding.span.words == &ordinary);
    check(ordinary[0] == (locked ? 37 : 11));
  }
  AlternateCanvas owner;
  auto handle = owner.publish({});
  std::vector<std::uint16_t> ordinary(640 * 480, 11);
  re::DispatchBinding binding{{&ordinary, 0}, 640, 480};
  re::DispatchState state{binding, {0, 0, 640, 480}};
  unsigned calls = 0;
  refuse([&] { dispatchOwnedAlternateCanvas(owner, handle, {{&ordinary, 0}, 636, 480}, state.clip, 0, state,
    [&] { ++calls; return true; }, [&](auto&) { ++calls; }, [&](auto&, unsigned) { ++calls; }, [&] { ++calls; }); });
  check(calls == 0 && ordinary[0] == 11 && state.binding.span.words == &ordinary);
  refuse([&] { dispatchOwnedAlternateCanvas(owner, handle, binding, state.clip, 257, state,
    [&] { ++calls; return true; }, [&](auto&) { ++calls; }, [&](auto&, unsigned) { ++calls; }, [&] { ++calls; }); });
  check(calls == 0 && ordinary[0] == 11);
  // Native cleanup on an early World exception: no borrowed alternate binding
  // may escape. This does not claim original callback exception equivalence.
  try {
    dispatchOwnedAlternateCanvas(owner, handle, binding, state.clip, 0, state,
      [] { return true; }, [&](auto&) { owner.retire(); throw 7; }, [](auto&, unsigned) {}, [] {});
    check(false);
  } catch (int value) { check(value == 7); }
  check(handle.expired() && state.binding.span.words == &ordinary);
}
}
int main() {
  layouts(); lifetime(); dispatchRetirement();
  std::cout << "{\"success\":true,\"frames\":" << frames << ",\"checks\":" << checks
            << ",\"refusals\":" << refusals << "}\n";
}
