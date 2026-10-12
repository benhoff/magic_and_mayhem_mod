#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include "../reconstruction/creature-ai/actor_route_request.hpp"
#include <map>
namespace {
struct RequestFixture {
  Fixture base;
  MotionActionState motion;
  SegmentCompletionState route;
  SegmentCompletionEnvironment environment;
  ActorRouteMetadata metadata;
  ActorRouteContext context;
  Cell target;
  unsigned mode = 0;
  bool regionRequest = false;
};
RequestFixture *request;
ActorRouteSnapshot generatedSnapshot(unsigned n, unsigned count) {
  ActorRouteSnapshot p;
  p.destination = {int(n % 16), int(n / 16 % 16), int(n % 4)};
  p.cursor = n % 3;
  p.count = count;
  for (unsigned i = 0; i < 16; ++i)
    p.points[i] = {{int((n + i) % 16), int((n / 7 + i) % 16), int((n + i) % 4)},
                   {int(i % 8), int(i % 3) - 1, int(i % 2), int(i % 5)}};
  for (unsigned i = 0; i < 14; ++i)
    p.motionWords[i] = 0x90000000u + n * 31u + i;
  return p;
}
unsigned producedCount() {
  constexpr unsigned counts[] = {0, 1, 4, 16};
  return counts[fixture->number % 4];
}
int regionResult() {
  return fixture->number % 11 == 0 ? -1 : int(fixture->number % 4);
}
void fillRegionPath(std::array<int, 50> &path) {
  path.fill(-1);
  unsigned length = 2 + fixture->number / 19 % 6;
  for (unsigned i = 0; i < length; ++i)
    path[i] = int((fixture->number + i) % 4);
}
int regionDistance(int from, int to) {
  if (request->route.regionCursor >= 48)
    return 200;
  return int((unsigned(from) * 71u + unsigned(to) * 17u + fixture->number) %
             250u);
}
void traceQuery(const ActorRegionQuery &q, Cell p, int dx, int dy) {
  record(600, q.actorSlot, unsigned(q.owner), p);
  record(601, q.actorSlot, q.command, {dx, dy, int(q.marker69c)});
}
ActorRouteRequestServices requestServices() {
  ActorRouteRequestServices s;
  s.regionAt = [](Cell p, const auto &q, int dx, int dy) {
    traceQuery(q, p, dx, dy);
    return regionResult();
  };
  s.regionRoute = [](int from, int to, const auto &q, auto &path,
                     unsigned limit, bool flag) {
    record(602, q.actorSlot, q.command, {from, to, int(q.marker69c)});
    if (limit != 700 || !flag)
      throw std::runtime_error("region search ABI");
    fillRegionPath(path);
    return fixture->number % 13 != 0;
  };
  s.regionDistance = [](int from, int to) {
    record(603, 0, unsigned(from), {to, 0, 0});
    return regionDistance(from, to);
  };
  s.regionCandidate = [](auto &a, Cell &p, unsigned parameter, Cell desired) {
    record(604, a.ai.slot, parameter, desired);
    if (fixture->number % 7 == 0)
      p = request->context.destination;
    else if (fixture->number % 7 == 1)
      p = {4, 5, 2};
  };
  s.legal = [](const auto &a, Cell p) {
    record(605, a.ai.slot, a.block723 || a.ai.behavior == 14, p);
    return fixture->number % 3 != 0;
  };
  s.adjustLegal = [](const auto &a, Cell &p) {
    record(606, a.ai.slot, 0, p);
    p.x = (p.x + 1) % 16;
    p.y = (p.y + 2) % 16;
  };
  s.coarse = [](auto &a, Cell p, auto &out) {
    record(607, a.ai.slot, 0, p);
    out = generatedSnapshot(fixture->number + 23, producedCount());
    return fixture->number % 5 == 0;
  };
  s.search = [](auto &a, unsigned mode, Cell p, auto &context, int &remaining) {
    record(608, a.ai.slot, mode, p);
    record(609, a.ai.slot, unsigned(remaining));
    remaining -= 1 + int(fixture->number % 80);
    context.scratch = generatedSnapshot(fixture->number + 47, producedCount());
  };
  return s;
}
RequestFixture generateRequest(unsigned n, unsigned op) {
  RequestFixture f;
  f.base = generate(n);
  f.base.world.dimensions = {16, 16, 4};
  auto &a = f.base.world.creatures[0];
  auto &e = f.base.execution[0];
  a.ai.position = {int(n % 16), int(n / 16 % 16), int(n % 4)};
  a.width = 1 + n / 7 % 2;
  a.height = 1 + n / 13 % 4;
  a.movementKind = n / 23 % 8;
  a.flag104 = n % 3;
  a.block723 = n / 17 % 2;
  a.ai.behavior = n / 19 % 2 ? 14 : 2;
  e.commandMode = n % 5;
  f.metadata.marker69c = 31;
  f.context.active = n / 7 % 3;
  f.context.displacementContinuation = n / 11 % 2;
  f.context.budget = int(n % 91) - 15;
  f.context.requestBudget = int(n % 177) - 20;
  f.context.destination = {3, 4, 2};
  f.context.origin = {7, 8, 1};
  f.context.mode = n % 3;
  f.context.scratch = generatedSnapshot(n + 13, 1 + n / 17 % 16);
  f.context.scratchDirty = n % 2;
  f.context.useRegions = op != 0;
  f.regionRequest = op != 0;
  f.mode = n / 3 % 3;
  f.target = {int(n / 11 % 48) - 16, int(n / 17 % 48) - 16, int(n / 23 % 4)};
  f.route.count = 1;
  f.route.cursor = 0;
  f.route.regionPending = op == 2;
  f.route.regionCursor = n / 29 % 48;
  f.route.regionPath.fill(-1);
  for (unsigned i = 0; i < 50; ++i)
    if (i < n % 50)
      f.route.regionPath[i] = int((n + i) % 4);
  f.metadata.regionDestination = f.target;
  f.environment.regions = {
      {{2, 3, 0}, 17}, {{5, 6, 1}, 23}, {{8, 9, 2}, 29}, {{11, 12, 3}, 31}};
  if (n % 5 == 0) {
    f.target = f.context.destination;
    f.mode = f.context.mode;
  }
  if (n % 7 == 0)
    f.context.origin = a.ai.position;
  if (n % 13 == 0)
    f.route.regionPending = false;
  if (op == 2 && n % 67 == 0) {
    f.route.regionCursor = 48 + (n / 67) % 2;
    f.route.regionPending = true;
    f.metadata.regionDestination = f.target;
    f.context.active = 0;
    f.route.regionPath[47] = f.route.regionPath[48] = f.route.regionPath[49] =
        0;
  }
  return f;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes requestBody;
std::array<unsigned, 131> snapshotWords(const ActorRouteSnapshot &s) {
  std::array<unsigned, 131> v{};
  v[0] = unsigned(s.destination.x);
  v[1] = unsigned(s.destination.y);
  v[2] = unsigned(s.destination.z);
  v[3] = s.cursor;
  v[4] = s.count;
  for (unsigned i = 0; i < 16; ++i) {
    unsigned o = 5 + i * 7;
    const auto &p = s.points[i];
    v[o] = unsigned(p.position.x);
    v[o + 1] = unsigned(p.position.y);
    v[o + 2] = unsigned(p.position.z);
    for (unsigned j = 0; j < 4; ++j)
      v[o + 3 + j] = unsigned(p.remainder[j]);
  }
  std::copy(s.motionWords.begin(), s.motionWords.end(), v.begin() + 117);
  return v;
}
ActorRouteSnapshot actorSnapshot(const AutonomousCreature &a,
                                 const CreatureExecution &e,
                                 const MotionActionState &m,
                                 const SegmentCompletionState &r) {
  ActorRouteSnapshot s;
  s.destination = a.destination;
  s.cursor = r.cursor;
  s.count = r.count;
  s.points = r.route;
  s.motionWords = {unsigned(m.progress),     unsigned(m.step),
                   unsigned(m.travelX),      unsigned(m.travelY),
                   unsigned(m.deltaX),       unsigned(m.deltaY),
                   unsigned(m.heightDelta),  unsigned(m.heightStep),
                   unsigned(m.heightOrigin), e.moveVariant,
                   unsigned(m.duration),     unsigned(m.accumulator),
                   e.routeChanged,           e.markerB73};
  return s;
}
void requestExtra(void *raw, const AutonomousCreature &a,
                  const CreatureExecution &e, const MotionActionState &m,
                  const SegmentCompletionState &r,
                  const ActorRouteMetadata &meta) {
  auto words = snapshotWords(actorSnapshot(a, e, m, r));
  std::memcpy(static_cast<unsigned char *>(raw) + 0x96b, words.data(), 0x20c);
  write32(raw, 0x69c, meta.marker69c);
  write32(raw, 0xd03, r.regionPending);
  write32(raw, 0xdcf, r.regionCursor);
  write32(raw, 0xdd3, unsigned(meta.regionDestination.x));
  write32(raw, 0xdd7, unsigned(meta.regionDestination.y));
  write32(raw, 0xddb, unsigned(meta.regionDestination.z));
  write32(raw, 0x104, a.flag104);
  for (unsigned i = 0; i < 50; ++i)
    write32(raw, 0xd07 + i * 4, unsigned(r.regionPath[i]));
}
struct RegionWire {
  unsigned char bytes[84];
};
ActorRegionQuery decodeQuery(const RegionWire &wire) {
  if (read32(wire.bytes, 0) ||
      read32(wire.bytes, 4) !=
          unsigned(reinterpret_cast<std::uintptr_t>(actors.data())))
    throw std::runtime_error("region query actor ABI");
  for (unsigned o :
       {12u, 16u, 20u, 24u, 28u, 32u, 36u, 41u, 46u, 50u, 54u, 58u})
    if (read32(wire.bytes, o))
      throw std::runtime_error("region query initialized zero ABI");
  if (wire.bytes[40] || wire.bytes[45])
    throw std::runtime_error("region query flag ABI");
  return {0, int(read32(wire.bytes, 70)), read32(wire.bytes, 74),
          read32(wire.bytes, 78)};
}
extern "C" int __attribute__((thiscall)) regionAtRequestStub(void *p, int x,
                                                             int y, int z,
                                                             RegionWire q,
                                                             int dx, int dy) {
  if (p != reinterpret_cast<void *>(0x6eaf08))
    throw std::runtime_error("region lookup this ABI");
  traceQuery(decodeQuery(q), {x, y, z}, dx, dy);
  return regionResult();
}
extern "C" unsigned __attribute__((thiscall))
regionRouteRequestStub(void *p, int from, int to, RegionWire q, int *path,
                       unsigned limit, unsigned flag) {
  if (p != reinterpret_cast<void *>(0x6eaf08) || limit != 700 || flag != 1 ||
      path != reinterpret_cast<int *>(actors.data() + 0xd07))
    throw std::runtime_error("region route ABI");
  auto query = decodeQuery(q);
  record(602, query.actorSlot, query.command, {from, to, int(query.marker69c)});
  std::array<int, 50> v;
  fillRegionPath(v);
  std::memcpy(path, v.data(), 200);
  return fixture->number % 13 != 0;
}
extern "C" int __attribute__((thiscall))
regionDistanceRequestStub(void *p, int from, int to) {
  if (p != reinterpret_cast<void *>(0x6eaf08))
    throw std::runtime_error("region distance ABI");
  record(603, 0, unsigned(from), {to, 0, 0});
  return regionDistance(from, to);
}
extern "C" void __attribute__((thiscall))
candidateRequestStub(void *p, int *x, int *y, int *z, unsigned parameter,
                     int tx, int ty, int tz) {
  record(604, slot(p), parameter, {tx, ty, tz});
  if (fixture->number % 7 == 0) {
    *x = request->context.destination.x;
    *y = request->context.destination.y;
    *z = request->context.destination.z;
  } else if (fixture->number % 7 == 1) {
    *x = 4;
    *y = 5;
    *z = 2;
  }
}
void descriptorRequest(const void *d) {
  const auto &a = fixture->world.creatures[0];
  if (read32(d, 0) != a.height || read32(d, 4) != a.width ||
      read32(d, 8) != a.flag104 || read32(d, 12) != a.movementKind ||
      read32(d, 17) != a.ai.type || static_cast<const unsigned char *>(d)[16] ||
      static_cast<const unsigned char *>(d)[21] !=
          unsigned(a.block723 || a.ai.behavior == 14))
    throw std::runtime_error("route22-byte descriptor ABI");
}
extern "C" unsigned __attribute__((thiscall))
legalRequestStub(void *, int x, int y, int z, void *d) {
  descriptorRequest(d);
  record(605, 0,
         fixture->world.creatures[0].block723 ||
             fixture->world.creatures[0].ai.behavior == 14,
         {x, y, z});
  return fixture->number % 3 != 0;
}
extern "C" void __attribute__((thiscall))
adjustRequestStub(void *, void *d, int *x, int *y, int *z) {
  descriptorRequest(d);
  record(606, 0, 0, {*x, *y, *z});
  *x = (*x + 1) % 16;
  *y = (*y + 2) % 16;
}
extern "C" unsigned __attribute__((thiscall))
coarseRequestStub(void *p, int x, int y, int z, void *out) {
  record(607, slot(p), 0, {x, y, z});
  auto words =
      snapshotWords(generatedSnapshot(fixture->number + 23, producedCount()));
  std::memcpy(out, words.data(), 0x20c);
  return fixture->number % 5 == 0;
}
extern "C" void __attribute__((thiscall))
searchRequestStub(void *p, void *actor, unsigned mode, int x, int y, int z,
                  int *remaining) {
  if (p != reinterpret_cast<void *>(0x6c4bd0))
    throw std::runtime_error("shared search this ABI");
  record(608, slot(actor), mode, {x, y, z});
  record(609, slot(actor), unsigned(*remaining));
  *remaining -= 1 + int(fixture->number % 80);
  auto words =
      snapshotWords(generatedSnapshot(fixture->number + 47, producedCount()));
  std::memcpy(p, words.data(), 0x20c);
}
void compareRequest(const RequestFixture &initial,
                    const RequestFixture &expected, unsigned result,
                    const std::vector<Event> &trace) {
  auto input = initial;
  request = &input;
  fixture = &input.base;
  events.clear();
  prepare();
  for (auto v :
       {std::pair<unsigned, void *>{
            0x53d7d0, reinterpret_cast<void *>(regionAtRequestStub)},
        {0x53e5e0, reinterpret_cast<void *>(regionRouteRequestStub)},
        {0x53ea50, reinterpret_cast<void *>(regionDistanceRequestStub)},
        {0x513cf0, reinterpret_cast<void *>(candidateRequestStub)},
        {0x4f4900, reinterpret_cast<void *>(legalRequestStub)},
        {0x4f4f90, reinterpret_cast<void *>(adjustRequestStub)},
        {0x513370, reinterpret_cast<void *>(coarseRequestStub)},
        {0x54b800, reinterpret_cast<void *>(searchRequestStub)}})
    redirect(v.first, v.second);
  for (unsigned i = 0; i < input.base.world.creatures.size(); ++i)
    requestExtra(actors.data() + i * 0xe4b, input.base.world.creatures[i],
                 input.base.execution[i], input.motion, input.route,
                 input.metadata);
  auto words = snapshotWords(input.context.scratch);
  std::memcpy(reinterpret_cast<void *>(0x6c4bd0), words.data(), 0x20c);
  *reinterpret_cast<unsigned char *>(0x6c4ddc) = input.context.scratchDirty;
  global(0x6df184, input.context.active);
  global(0x6df1a0, input.context.displacementContinuation);
  global(0x6df188, unsigned(input.context.budget));
  global(0x6df18c, unsigned(input.context.requestBudget));
  global(0x6df190, unsigned(input.context.destination.x));
  global(0x6df194, unsigned(input.context.destination.y));
  global(0x6df198, unsigned(input.context.destination.z));
  global(0x6df19c, input.context.mode);
  global(0x6df1a4, unsigned(input.context.origin.x));
  global(0x6df1a8, unsigned(input.context.origin.y));
  global(0x6df1ac, unsigned(input.context.origin.z));
  global(0x6e1f3c, input.context.useRegions);
  Bytes regions(input.environment.regions.size() * 660, 0);
  for (unsigned i = 0; i < input.environment.regions.size(); ++i) {
    auto *p = regions.data() + i * 660;
    const auto &r = input.environment.regions[i];
    write32(p, 4, unsigned(r.position.x));
    write32(p, 8, unsigned(r.position.y));
    write32(p, 12, unsigned(r.position.z));
    write32(p, 0x20c, r.parameter);
  }
  global(0x6eaf08, unsigned(reinterpret_cast<std::uintptr_t>(regions.data())));
  global(0x6eaf0c, input.environment.regions.size());
  auto regionBefore = regions;
  auto before = actors;
  using F = unsigned(__attribute__((thiscall)) *)(void *, int, int, int,
                                                  unsigned, unsigned);
  unsigned actual = reinterpret_cast<F>(0x5128a0)(
      actors.data(), input.target.x, input.target.y, input.target.z, input.mode,
      input.regionRequest);
  if (actual != result || events != trace) {
    std::cerr << "request n=" << input.base.number << " result=" << actual
              << " expected=" << result << '\n';
    for (auto &v : events)
      std::cerr << "A " << v.kind << "," << v.argument
                << " xyz=" << v.position.x << "," << v.position.y << ","
                << v.position.z << '\n';
    for (auto &v : trace)
      std::cerr << "E " << v.kind << "," << v.argument
                << " xyz=" << v.position.x << "," << v.position.y << ","
                << v.position.z << '\n';
    throw std::runtime_error("request effects");
  }
  for (unsigned i = 0; i < expected.base.world.creatures.size(); ++i) {
    auto *p = before.data() + i * 0xe4b;
    packActor(p, expected.base.world.creatures[i], expected.base.execution[i]);
    requestExtra(p, expected.base.world.creatures[i],
                 expected.base.execution[i],
                 i ? initial.motion : expected.motion,
                 i ? initial.route : expected.route,
                 i ? initial.metadata : expected.metadata);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "request actor n=" + std::to_string(input.base.number) +
        " offset=" + std::to_string(i % 0xe4b));
  }
  auto wanted = snapshotWords(expected.context.scratch);
  if (std::memcmp(reinterpret_cast<void *>(0x6c4bd0), wanted.data(), 0x20c) ||
      *reinterpret_cast<unsigned char *>(0x6c4ddc) !=
          unsigned(expected.context.scratchDirty))
    throw std::runtime_error("request scratch guard");
  for (auto v :
       {std::pair<unsigned, unsigned>{0x6df184, expected.context.active},
        {0x6df1a0, expected.context.displacementContinuation},
        {0x6df188, unsigned(expected.context.budget)},
        {0x6df18c, unsigned(expected.context.requestBudget)},
        {0x6df190, unsigned(expected.context.destination.x)},
        {0x6df194, unsigned(expected.context.destination.y)},
        {0x6df198, unsigned(expected.context.destination.z)},
        {0x6df19c, expected.context.mode},
        {0x6df1a4, unsigned(expected.context.origin.x)},
        {0x6df1a8, unsigned(expected.context.origin.y)},
        {0x6df1ac, unsigned(expected.context.origin.z)}})
    if (*reinterpret_cast<unsigned *>(v.first) != v.second)
      throw std::runtime_error(
          "request continuation state n=" + std::to_string(input.base.number) +
          " address=" + std::to_string(v.first));
  if (regions != regionBefore ||
      std::memcmp(reinterpret_cast<void *>(0x5128a0), requestBody.data(),
                  requestBody.size()))
    throw std::runtime_error("request region/body modified");
}
#endif
unsigned requestRefusals() {
  unsigned refused = 0;
  for (unsigned kind = 0; kind < 5; ++kind) {
    auto f = generateRequest(11, 2);
    request = &f;
    fixture = &f.base;
    events.clear();
    f.context.active = 0;
    f.context.budget = 1000;
    f.context.useRegions = f.regionRequest = f.route.regionPending = true;
    f.metadata.regionDestination = f.target;
    f.route.regionCursor = 1;
    f.route.regionPath.fill(0);
    auto services = requestServices();
    services.regionDistance = [](int, int) { return 200; };
    switch (kind) {
    case 0:
      f.base.world.dimensions.x = 0;
      break;
    case 1:
      f.context.useRegions = false;
      f.mode = 0;
      services.search = [](auto &, unsigned, Cell, auto &c, int &) {
        c.scratch.count = 17;
      };
      break;
    case 2:
      f.route.regionPath[1] = 4;
      break;
    case 3:
      f.route.regionCursor = 0xffffffffu;
      break;
    case 4:
      f.environment.regions.clear();
      break;
    }
    bool rejected = false;
    try {
      requestCreatureRoute(f.base.world.creatures[0], f.base.execution[0],
                           f.motion, f.route, f.metadata, f.context,
                           f.environment, f.base.world.dimensions, f.target,
                           f.mode, f.regionRequest, services);
    } catch (const std::invalid_argument &) {
      rejected = kind < 2;
    } catch (const std::out_of_range &) {
      rejected = kind >= 2;
    }
    if (!rejected)
      throw std::runtime_error("request admission failure absent");
    ++refused;
  }
  return refused;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE required");
  map_image(read(argv[1]));
  requestBody = Bytes(reinterpret_cast<unsigned char *>(0x5128a0),
                      reinterpret_cast<unsigned char *>(0x51336c));
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  std::map<std::string, unsigned> coverage;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 3; ++op) {
      auto input = generateRequest(n, op), expected = input;
      request = &expected;
      fixture = &expected.base;
      events.clear();
      const unsigned result = requestCreatureRoute(
          expected.base.world.creatures[0], expected.base.execution[0],
          expected.motion, expected.route, expected.metadata, expected.context,
          expected.environment, expected.base.world.dimensions, expected.target,
          expected.mode, expected.regionRequest, requestServices());
      auto trace = events;
      for (auto &v : trace)
        ++coverage["service_" + std::to_string(v.kind)];
      ++coverage["result_" + std::to_string(result)];
      if (expected.route.regionCursor != input.route.regionCursor)
        ++coverage["region_cursor"];
      if (expected.route.regionPending)
        ++coverage["region_pending"];
      if (input.route.regionCursor >= 48)
        ++coverage["region_alias_" + std::to_string(input.route.regionCursor)];
      mix(result);
      mix(expected.context.active);
      mix(unsigned(expected.context.budget));
      mix(unsigned(expected.context.requestBudget));
      mix(expected.context.scratchDirty);
      mix(expected.route.cursor);
      mix(expected.route.count);
      mix(expected.route.regionCursor);
      mix(expected.route.regionPending);
      mix(expected.metadata.marker69c);
      for (int v : expected.route.regionPath)
        mix(unsigned(v));
      for (auto &v : trace) {
        mix(v.kind);
        mix(v.slot);
        mix(v.argument);
        mix(unsigned(v.position.x));
        mix(unsigned(v.position.y));
        mix(unsigned(v.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareRequest(input, expected, result, trace);
#endif
      ++cases;
    }
  for (unsigned service = 600; service <= 609; ++service)
    if (!coverage["service_" + std::to_string(service)])
      throw std::runtime_error("missing request service " +
                               std::to_string(service));
  for (const char *branch :
       {"result_0", "result_1", "result_2", "region_cursor", "region_pending",
        "region_alias_48", "region_alias_49"})
    if (!coverage[branch])
      throw std::runtime_error(std::string("missing request branch ") + branch);
  const unsigned refused = requestRefusals();
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"native_refusal_cases\":" << refused
            << ",\"functions\":1,\"outcome_digest\":" << digest
            << ",\"coverage\":{";
  bool first = true;
  for (auto &v : coverage) {
    if (!first)
      std::cout << ',';
    first = false;
    std::cout << '\"' << v.first << "\":" << v.second;
  }
  std::cout << "}}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
