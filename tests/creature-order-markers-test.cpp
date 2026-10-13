#include "../reconstruction/creature-ai/order_markers.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_MARKERS_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousCreature actor;
  CreatureExecution execution;
  int localOwner = 0;
  unsigned number = 0;
};
Fixture *current;
unsigned allocation = 0;
std::vector<unsigned> trace;
void event(unsigned kind, std::initializer_list<unsigned> args = {}) {
  trace.push_back(kind);
  trace.push_back(unsigned(args.size()));
  trace.insert(trace.end(), args.begin(), args.end());
}
std::optional<unsigned> allocate() {
  unsigned draw = allocation++;
  std::optional<unsigned> result;
  if ((current->number + draw) % 4)
    result = draw ? 100 + draw : 0;
  event(0, {draw, result ? *result : UINT32_MAX});
  return result;
}
void markerEvent(unsigned token, unsigned node,
                 const QueuedMarkerParameters &p) {
  event(1, {token, node, 70});
  trace.insert(trace.end(), p.begin(), p.end());
}
QueuedMarkerServices services() {
  return {allocate, [](unsigned token, const auto &p) {
            auto &orders = current->execution.orders;
            auto it =
                std::find_if(orders.begin(), orders.end(), [&](const auto &o) {
                  return o.effect && *o.effect == token;
                });
            if (it == orders.end())
              throw std::runtime_error(
                  "marker token not stored before initialization");
            markerEvent(token, unsigned(it - orders.begin()), p);
          }};
}
Fixture generate(unsigned n) {
  Fixture f;
  f.number = n;
  f.actor.ai.slot = n % 128;
  f.actor.ai.owner = int(n % 10) - 1;
  f.localOwner = n % 3 ? f.actor.ai.owner : int((n + 3) % 10) - 1;
  f.actor.ai.health = n % 2 ? 360 : 0;
  f.execution.queuedCount = 73;
  f.execution.queuedFlag = 17;
  f.execution.nextOrderSerial = 12345;
  constexpr int extremes[] = {0, 1, -1, INT32_MIN, INT32_MAX, 15, 16, 32};
  for (unsigned i = 0; i < n % 16; ++i) {
    CreatureOrder o;
    o.kind = (n + i) % 5;
    o.serial = i + 1;
    o.target = -1;
    o.repeat = (n + i) % 2;
    o.destination = {extremes[(n + i) % 8], extremes[(n / 8 + i) % 8],
                     extremes[(n / 64 + i) % 8]};
    o.referenceDestination = o.destination;
    if ((n + i) % 4 == 1)
      o.referenceDestination.x = int(unsigned(o.referenceDestination.x) ^ 1u);
    if ((n + i) % 4 == 2)
      o.referenceDestination.y = int(unsigned(o.referenceDestination.y) ^ 1u);
    if ((n + i) % 4 == 3)
      o.referenceDestination.z = int(unsigned(o.referenceDestination.z) ^ 1u);
    if ((n + i) % 7 == 0)
      o.effect = 1000 + i;
    f.execution.orders.push_back(o);
  }
  return f;
}
#ifdef MNM_MARKERS_REFERENCE
std::array<unsigned char, 0xe4b> actor{};
std::array<unsigned char, 16> container{};
std::array<unsigned char, 44> sentinel{};
Bytes nodes, tokens;
std::array<unsigned char, 277> body{};
unsigned get(const void *p, unsigned offset) {
  unsigned n;
  std::memcpy(&n, static_cast<const unsigned char *>(p) + offset, 4);
  return n;
}
void put(void *p, unsigned offset, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + offset, &n, 4);
}
unsigned pointer(const void *p) { return reinterpret_cast<std::uintptr_t>(p); }
unsigned effectPointer(std::optional<unsigned> id) {
  return id ? pointer(tokens.data() + *id * 4) : 0;
}
extern "C" void *__attribute__((thiscall)) pool(void *p) {
  if (p != reinterpret_cast<void *>(0x689498))
    throw std::runtime_error("marker pool ABI");
  auto id = allocate();
  return id ? tokens.data() + *id * 4 : nullptr;
}
extern "C" unsigned __attribute__((thiscall)) initialize(void *p, unsigned kind,
                                                         void *record) {
  if (kind != 70)
    throw std::runtime_error("marker init kind");
  auto offset = static_cast<unsigned char *>(p) - tokens.data();
  if (offset < 0 || unsigned(offset) >= tokens.size() || offset % 4)
    throw std::runtime_error("marker token ABI");
  unsigned node = 0;
  while (node < current->execution.orders.size() &&
         get(nodes.data() + node * 44, 0x28) != pointer(p))
    ++node;
  if (node == current->execution.orders.size())
    throw std::runtime_error(
        "original marker not stored before initialization");
  QueuedMarkerParameters parameters{};
  std::memcpy(parameters.data(), record, 252);
  markerEvent(unsigned(offset) / 4, node, parameters);
  return current->number % 3 ? 0xabc001 : 0;
}
void redirect(unsigned address, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  put(p, 1, pointer(target) - address - 5);
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(body.data(), reinterpret_cast<void *>(0x512220), body.size());
  redirect(0x49cec0, reinterpret_cast<void *>(pool));
  redirect(0x493f20, reinterpret_cast<void *>(initialize));
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &events) {
  current = &input;
  trace.clear();
  allocation = 0;
  actor.fill(0x9d);
  container.fill(0xb1);
  sentinel.fill(0xc7);
  tokens.assign(2048 * 4, 0xe3);
  nodes.assign(input.execution.orders.size() * 44, 0xd9);
  put(actor.data(), 0, input.actor.ai.slot);
  put(actor.data(), 0x174, unsigned(input.actor.ai.owner));
  put(actor.data(), 0xe4, input.actor.ai.health);
  put(actor.data(), 0x8f9, pointer(container.data()));
  put(container.data(), 4, pointer(sentinel.data()));
  put(container.data(), 8, input.execution.orders.size());
  put(sentinel.data(), 0,
      input.execution.orders.empty() ? pointer(sentinel.data())
                                     : pointer(nodes.data()));
  for (unsigned i = 0; i < input.execution.orders.size(); ++i) {
    auto *p = nodes.data() + i * 44;
    const auto &o = input.execution.orders[i];
    put(p, 0,
        i + 1 < input.execution.orders.size()
            ? pointer(nodes.data() + (i + 1) * 44)
            : pointer(sentinel.data()));
    put(p, 4,
        i ? pointer(nodes.data() + (i - 1) * 44) : pointer(sentinel.data()));
    for (auto f : {std::pair<unsigned, unsigned>{8, o.kind},
                   {12, unsigned(o.destination.x)},
                   {16, unsigned(o.destination.y)},
                   {20, unsigned(o.destination.z)},
                   {24, unsigned(o.target)},
                   {28, unsigned(o.referenceDestination.x)},
                   {32, unsigned(o.referenceDestination.y)},
                   {36, unsigned(o.referenceDestination.z)},
                   {40, effectPointer(o.effect)}})
      put(p, f.first, f.second);
  }
  const auto beforeActor = actor;
  const auto beforeContainer = container;
  const auto beforeSentinel = sentinel;
  const auto beforeTokens = tokens;
  auto beforeNodes = nodes;
  global(0x644520, unsigned(input.localOwner));
  using Update = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Update>(0x512220)(actor.data());
  if (trace != events)
    throw std::runtime_error("marker ordered calls/parameters mismatch case=" +
                             std::to_string(input.number));
  for (unsigned i = 0; i < expected.execution.orders.size(); ++i)
    put(beforeNodes.data() + i * 44, 40,
        effectPointer(expected.execution.orders[i].effect));
  if (actor != beforeActor || container != beforeContainer ||
      sentinel != beforeSentinel || nodes != beforeNodes ||
      tokens != beforeTokens ||
      std::memcmp(body.data(), reinterpret_cast<void *>(0x512220), body.size()))
    throw std::runtime_error("marker actor/list/pool/body guard mismatch");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_MARKERS_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  unsigned calls = 0, initialized = 0, zeroToken = 0;
  for (unsigned n = 0; n < 16384; ++n) {
    auto input = generate(n);
    auto expected = input;
    current = &expected;
    trace.clear();
    allocation = 0;
    updateQueuedMarkers(expected.actor, expected.execution, expected.localOwner,
                        services());
    const auto events = trace;
    calls += allocation;
    for (auto v : trace)
      mix(v);
    for (unsigned i = 0; i < expected.execution.orders.size(); ++i) {
      const auto &a = input.execution.orders[i];
      const auto &b = expected.execution.orders[i];
      mix(b.effect ? *b.effect : UINT32_MAX);
      if (!a.effect && b.effect) {
        ++initialized;
        if (*b.effect == 0)
          ++zeroToken;
      }
    }
    if (expected.execution.queuedCount != 73 ||
        expected.execution.queuedFlag != 17 ||
        expected.execution.nextOrderSerial != 12345)
      throw std::runtime_error("marker changed queue bookkeeping");
#ifdef MNM_MARKERS_REFERENCE
    compare(input, expected, events);
#endif
  }
  if (!calls || !initialized || !zeroToken)
    throw std::runtime_error("missing marker branch");
  std::cout
      << "{\"success\":true,\"cases\":16384,\"functions\":1,\"outcome_digest\":"
      << digest << ",\"allocation_calls\":" << calls
      << ",\"initialized_markers\":" << initialized
      << ",\"zero_token_markers\":" << zeroToken << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
