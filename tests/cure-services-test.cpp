#include "../reconstruction/casting/cure_services.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_CURE_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::casting;
namespace {
void require(bool ok, const char *why) { if (!ok) throw std::runtime_error(why); }
std::uint32_t rng(std::uint32_t &x) { x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x; }
std::vector<std::uint32_t> trace;
unsigned number = 0;
std::uint64_t digest = 1469598103934665603ull;
void hash(unsigned x) { digest = (digest ^ x) * 1099511628211ull; }
void hashTrace() { for (auto x : trace) hash(x); }
struct Backend : mnm::reconstruction::audio::VoiceBackend {
  mnm::reconstruction::audio::Status getStatus(unsigned b, unsigned &flags) override {
    trace.insert(trace.end(), {1, b});
    if (number % 17) flags = number % 4;
    return number % 3 == 0 ? -1 : 0;
  }
  mnm::reconstruction::audio::Status stop(unsigned b) override {
    trace.insert(trace.end(), {2, b}); return number % 11 == 0 ? -2 : 0;
  }
  mnm::reconstruction::audio::Status position(unsigned b, unsigned p) override {
    trace.insert(trace.end(), {3, b, p}); return number % 19 == 0 ? -3 : 0;
  }
  mnm::reconstruction::audio::Status volume(unsigned, int) override { throw std::runtime_error("unexpected volume"); }
  mnm::reconstruction::audio::Status pan(unsigned, int) override { throw std::runtime_error("unexpected pan"); }
  mnm::reconstruction::audio::Status play(unsigned, unsigned, unsigned, unsigned) override { throw std::runtime_error("unexpected play"); }
  void volumeRecord(mnm::reconstruction::audio::VoiceContract &, int) override { throw std::runtime_error("unexpected volume record"); }
} backend;
struct StatusFixture { CureStatus actor; CureStatusAudio audio; };
StatusFixture statusFixture(unsigned n) {
  StatusFixture f;
  auto &a = f.audio;
  f.actor.status = n % 3 ? 0xfefefefe : 0;
  if (n % 7) f.actor.voice = n % 3;
  a.enabled = n % 5 ? 9 : 0;
  a.initialized = n % 13 ? 3 : 0;
  a.buffers = {101, n % 17 ? 102u : 0u, 103};
  a.outputs = {11, 12, 13, 14};
  a.statusScratch = 0xabcdef01;
  a.resultScratch = 0xfedcba98;
  const auto size = 1 + n % 4;
  a.head = n / 4 % size;
  for (unsigned i = 0; i < size; ++i) {
    mnm::reconstruction::audio::Schedule32 s;
    s.next = (i + 1) % size; s.previous = (i + size - 1) % size;
    s.deadline = n * 19 + i; s.volume = int(i) * -100;
    s.voiceAddress = 0; s.outputAddress = (n + i) % 5;
    s.x = int(i) - 100; s.y = int(i) + 100;
    a.schedules.push_back(s);
  }
  if (f.actor.voice) {
    const auto match = (a.head + n / 16 % size) % size;
    a.schedules[match].voiceAddress = *f.actor.voice + 1;
    // Duplicate matches establish traversal from head, not vector order.
    if (n % 19 == 0) a.schedules[(match + 1) % size].voiceAddress = *f.actor.voice + 1;
  }
  return f;
}
CastResource resourceFixture(unsigned n) {
  std::uint32_t seed = (n + 1) * 0x9e3779b9u;
  constexpr unsigned edge[] = {0, 1, 255, 256, 0x7fffffff, 0x80000000, 0xffffffff, 0xffffff00};
  constexpr unsigned kinds[] = {0, 1, 17, 23, 24, 25, 26, 27, 0xffffffff};
  CastResource a;
  a.kind = kinds[n % 9];
  a.value = n % 3 ? rng(seed) : edge[n / 3 % 8];
  a.maximum = n % 5 ? rng(seed) : edge[n / 5 % 8];
  a.blocked = n % 7 == 0 ? 91 : 0;
  a.owner = n % 3;
  a.dirty = rng(seed);
  return a;
}
unsigned deltaFor(unsigned n) {
  constexpr unsigned edge[] = {0, 1, 255, 256, 0xffffffff, 0xffffff00, 0x7fffffff, 0x80000000};
  std::uint32_t seed = (n + 9) * 0x9234011u;
  return n % 2 ? rng(seed) : edge[n / 2 % 8];
}
void notify(CastResource &a, unsigned k, unsigned v) {
  trace.insert(trace.end(), {k, v, a.dirty});
  if (k == 4 && number % 2) a.value ^= 0xdeadbeefu;
}

#ifdef MNM_CURE_REFERENCE
void put(void *p, unsigned offset, unsigned x) { std::memcpy(static_cast<unsigned char *>(p) + offset, &x, 4); }
unsigned get(const void *p, unsigned offset) { unsigned x; std::memcpy(&x, static_cast<const unsigned char *>(p) + offset, 4); return x; }
unsigned address(const void *p) { return unsigned(reinterpret_cast<std::uintptr_t>(p)); }
Bytes rawActor, rawManager, rawNodes, rawOutputs, rawWrappers, rawBuffers;
unsigned char *actor = nullptr;
std::array<unsigned char, 0x9c> resourceBody{};
std::array<unsigned char, 0xc5> statusBody{};
std::array<unsigned char, 0x5b> retireBody{};
std::array<unsigned char, 0x1e> predicateBody{};
extern "C" void __attribute__((thiscall)) uiFirst(void *p, unsigned v) {
  require(address(p) == 0x6a2dd0, "resource UI first receiver");
  trace.insert(trace.end(), {4, v, get(actor, 0x92e)});
  if (number % 2) put(actor, 0xe8, get(actor, 0xe8) ^ 0xdeadbeefu);
}
extern "C" void __attribute__((thiscall)) uiSecond(void *p, unsigned v) {
  require(address(p) == 0x6c4e40, "resource UI second receiver");
  trace.insert(trace.end(), {5, v, get(actor, 0x92e)});
}
unsigned bufferId(void *p) {
  const unsigned delta = address(p) - address(rawBuffers.data() + 32);
  require(delta % 16 == 0 && delta / 16 < 3, "buffer identity");
  return 101 + delta / 16;
}
extern "C" int __attribute__((stdcall)) getStatus(void *p, unsigned *flags) { return backend.getStatus(bufferId(p), *flags); }
extern "C" int __attribute__((stdcall)) stop(void *p) { return backend.stop(bufferId(p)); }
extern "C" int __attribute__((stdcall)) position(void *p, unsigned byte) { return backend.position(bufferId(p), byte); }
void redirect(unsigned a, void *p) {
  auto *target = reinterpret_cast<unsigned char *>(a); target[0] = 0xe9;
  put(target, 1, address(p) - a - 5);
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(resourceBody.data(), reinterpret_cast<void *>(0x507f00), resourceBody.size());
  std::memcpy(statusBody.data(), reinterpret_cast<void *>(0x516150), statusBody.size());
  std::memcpy(retireBody.data(), reinterpret_cast<void *>(0x571d20), retireBody.size());
  std::memcpy(predicateBody.data(), reinterpret_cast<void *>(0x411e50), predicateBody.size());
  redirect(0x4d6f30, (void *)uiFirst); redirect(0x4d3a10, (void *)uiSecond);
}
void guardBodies() {
  require(!std::memcmp(resourceBody.data(), reinterpret_cast<void *>(0x507f00), resourceBody.size()) &&
          !std::memcmp(statusBody.data(), reinterpret_cast<void *>(0x516150), statusBody.size()) &&
          !std::memcmp(retireBody.data(), reinterpret_cast<void *>(0x571d20), retireBody.size()) &&
          !std::memcmp(predicateBody.data(), reinterpret_cast<void *>(0x411e50), predicateBody.size()), "tested original bodies changed");
}
void resourceCompare(const CastResource &input, const CastResource &expected,
                     unsigned delta, const std::vector<unsigned> &expectedTrace) {
  trace.clear(); rawActor.assign(0xe4b + 64, 0x91); actor = rawActor.data() + 32;
  Bytes config(0x5c9 + 64, 0x97);
  put(config.data() + 32, 0, input.kind);
  put(actor, 0xac, address(config.data() + 32));
  for (auto pair : {std::pair<unsigned, unsigned>{0xe8, input.value}, {0xec, input.maximum},
                   {0x7c, input.blocked}, {0x174, input.owner}, {0x92e, input.dirty}})
    put(actor, pair.first, pair.second);
  const auto savedConfig = config;
  auto wanted = rawActor;
  put(wanted.data() + 32, 0xe8, expected.value); put(wanted.data() + 32, 0x92e, expected.dirty);
  global(0x644520, number % 2);
  using Resource = void(__attribute__((thiscall)) *)(void *, unsigned);
  using Predicate = unsigned(__attribute__((thiscall)) *)(void *);
  require(reinterpret_cast<Predicate>(0x411e50)(config.data() + 32) == chargeableKind(input.kind), "chargeable predicate");
  reinterpret_cast<Resource>(0x507f00)(actor, delta);
  require(trace == expectedTrace && wanted == rawActor && config == savedConfig, "resource state/trace/guards");
  guardBodies();
}
void statusCompare(const StatusFixture &input, const StatusFixture &expected,
                   const std::vector<unsigned> &expectedTrace) {
  trace.clear();
  const auto &a = input.audio;
  rawActor.assign(0xe4b + 64, 0x91); actor = rawActor.data() + 32;
  rawManager.assign(0x264 + 64, 0xa3); auto *manager = rawManager.data() + 32;
  rawNodes.assign(a.schedules.size() * 32 + 64, 0xc1);
  rawOutputs.assign(a.outputs.size() * 4 + 64, 0xc9);
  rawWrappers.assign(3 * 40 + 64, 0xd1);
  rawBuffers.assign(3 * 16 + 64, 0xd9);
  std::array<unsigned, 32> vtable{};
  vtable[0x24/4] = address((void *)getStatus); vtable[0x48/4] = address((void *)stop); vtable[0x34/4] = address((void *)position);
  auto nodePtr = [&](unsigned i) { return address(rawNodes.data() + 32 + i * 32); };
  auto wrapperPtr = [&](unsigned i) { return address(rawWrappers.data() + 32 + i * 40); };
  for (unsigned i = 0; i < 3; ++i) {
    put(rawBuffers.data() + 32 + i * 16, 0, address(vtable.data()));
    put(rawWrappers.data() + 32 + i * 40, 0, a.buffers[i] ? address(rawBuffers.data() + 32 + i * 16) : 0);
  }
  auto packNodes = [&](Bytes &bytes, const CureStatusAudio &s) {
    for (unsigned i = 0; i < s.schedules.size(); ++i) {
      auto r = s.schedules[i];
      r.next = nodePtr(r.next); r.previous = nodePtr(r.previous);
      r.voiceAddress = r.voiceAddress ? wrapperPtr(r.voiceAddress - 1) : 0;
      r.outputAddress = r.outputAddress ? address(rawOutputs.data() + 32 + (r.outputAddress - 1) * 4) : 0;
      std::memcpy(bytes.data() + 32 + i * 32, &r, 32);
    }
  };
  packNodes(rawNodes, a);
  for (unsigned i = 0; i < a.outputs.size(); ++i) put(rawOutputs.data() + 32, i * 4, a.outputs[i]);
  put(actor, 0x114, input.actor.status); put(actor, 0x120, input.actor.voice ? wrapperPtr(*input.actor.voice) : 0);
  put(manager, 0x18, a.initialized); put(manager, 0x1c, a.enabled); put(manager, 0x260, nodePtr(a.head));
  auto wantedActor = rawActor, wantedManager = rawManager, wantedNodes = rawNodes, wantedOutputs = rawOutputs;
  const auto savedWrappers = rawWrappers, savedBuffers = rawBuffers;
  const auto savedVtable = vtable;
  put(wantedActor.data() + 32, 0x114, expected.actor.status); put(wantedActor.data() + 32, 0x120, 0);
  put(wantedManager.data() + 32, 0x260, nodePtr(expected.audio.head));
  packNodes(wantedNodes, expected.audio);
  for (unsigned i = 0; i < a.outputs.size(); ++i) put(wantedOutputs.data() + 32, i * 4, expected.audio.outputs[i]);
  global(0x6f437c, address(manager)); global(0x6fa794, a.statusScratch); global(0x6fa798, a.resultScratch);
  using Cleanse = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Cleanse>(0x516150)(actor);
  require(trace == expectedTrace && rawActor == wantedActor && rawManager == wantedManager &&
          rawNodes == wantedNodes && rawOutputs == wantedOutputs && rawWrappers == savedWrappers &&
          rawBuffers == savedBuffers && vtable == savedVtable &&
          get(reinterpret_cast<void *>(0x6fa794), 0) == expected.audio.statusScratch &&
          get(reinterpret_cast<void *>(0x6fa798), 0) == expected.audio.resultScratch,
          "status/audio/scheduler state/trace/guards");
  guardBodies();
}
#endif
unsigned refusals() {
  for (unsigned n = 0; n < 8; ++n) {
    auto f = statusFixture(1);
    if (n == 0) f.actor.voice = 99;
    if (n == 1) f.audio.head = 99;
    if (n == 2) f.audio.schedules[0].next = 99;
    if (n == 3) f.audio.schedules[0].previous = 99;
    if (n == 4) for (auto &s : f.audio.schedules) s.voiceAddress = 0;
    if (n == 5) f.audio.schedules[0].outputAddress = 99;
    if (n == 6) f.audio.schedules[0].voiceAddress = 99;
    if (n == 7) f.audio.schedules.resize(65537);
    trace.clear(); bool refused = false;
    const auto oldStatus = f.actor.status;
    try { clearCureStatus(f.actor, f.audio, backend); } catch (const std::invalid_argument &) { refused = true; }
    require(refused && trace.empty() && f.actor.status == oldStatus && f.actor.voice, "status preflight refusal");
  }
  return 8;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_CURE_REFERENCE
  require(argc == 2, "expected pinned executable"); install(argv[1]);
#else
  (void)argv; require(argc == 1, "unexpected argument");
#endif
  const auto refused = refusals();
  std::array<unsigned, 6> callCounts{};
  for (number = 0; number < 32768; ++number) {
    const auto input = resourceFixture(number); auto expected = input;
    trace.clear();
    changeCastResource(expected, deltaFor(number), number % 2,
      [&](unsigned v) { notify(expected, 4, v); }, [&](unsigned v) { notify(expected, 5, v); });
    hash(expected.value); hash(expected.dirty); hashTrace();
    if (!trace.empty()) { ++callCounts[4]; ++callCounts[5]; }
#ifdef MNM_CURE_REFERENCE
    const auto expectedTrace = trace; resourceCompare(input, expected, deltaFor(number), expectedTrace);
#endif
  }
  for (number = 0; number < 8192; ++number) {
    auto input = statusFixture(number), expected = input;
    trace.clear(); clearCureStatus(expected.actor, expected.audio, backend);
    hash(expected.actor.status); hash(expected.actor.voice.value_or(UINT32_MAX));
    const auto &a = expected.audio;
    hash(a.head); hash(a.statusScratch); hash(a.resultScratch);
    for (const auto &s : a.schedules) { hash(s.next); hash(s.previous); hash(s.deadline); hash(unsigned(s.volume)); hash(s.voiceAddress); hash(s.outputAddress); hash(unsigned(s.x)); hash(unsigned(s.y)); }
    for (auto o : a.outputs) hash(o);
    hashTrace();
    for (std::size_t i = 0; i < trace.size();) { const auto k = trace[i]; ++callCounts[k]; i += k == 3 ? 3 : 2; }
#ifdef MNM_CURE_REFERENCE
    const auto expectedTrace = trace; statusCompare(input, expected, expectedTrace);
#endif
  }
  for (unsigned i = 1; i < 6; ++i) require(callCounts[i], "backend branch unexercised");
  require(callCounts[1] > callCounts[2] && callCounts[2] > callCounts[3], "idle status and failed Stop branches must execute");
  std::cout << "{\"success\":true,\"cases\":40960,\"resource_cases\":32768,\"status_cases\":8192,\"functions\":4,\"native_refusal_cases\":" << refused
            << ",\"outcome_digest\":\"" << std::hex << digest << std::dec << "\",\"service_calls\":[";
  for (unsigned i = 1; i < 6; ++i) std::cout << (i == 1 ? "" : ",") << callCounts[i];
  std::cout << "]}\n"; return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
