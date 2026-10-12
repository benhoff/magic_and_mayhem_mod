#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include <memory>
namespace {
CreatureOrder command() {
  CreatureOrder o;
  o.kind = 2 + fixture->number % 3;
  o.destination = {int(fixture->number % 32), 7, int(fixture->number % 4)};
  if (o.kind == 3)
    o.target = int((fixture->number + 1) % fixture->world.creatures.size());
  return o;
}
ExecutionServices orderServices() {
  auto s = executionServices();
  s.performOrder = [](const auto &o) {
    record(48, 0, o.kind, o.destination);
    record(49, 0, unsigned(o.target));
  };
  s.adjustDuplicate = [](Cell &p) {
    record(50, 0, 0, p);
    p.y = (p.y + 1) % fixture->world.dimensions.y;
    return fixture->number % 3 != 0;
  };
  s.duplicateMove = [](auto &a, Cell p) {
    record(51, a.ai.slot, 1, p);
    return Cell{p.x + 1, p.y + 2, p.z + 1};
  };
  s.terrainHeight = [](Cell p) { return (p.x + p.y) % 16; };
  return s;
}
FollowServices followServices() {
  FollowServices s;
  s.adjust = [](const auto &, Cell &p) {
    record(52, 0, 0, p);
    p.x = (p.x + 1) % fixture->world.dimensions.x;
    return fixture->number % 3 != 0;
  };
  s.move = [](auto &a, Cell p) { record(53, a.ai.slot, 0, p); };
  return s;
}
void performDelivery(AutonomousCreature &a, const CreatureOrder &o,
                     AutonomousWorld &w) {
  deliverOrder(
      a, o, w, [](Cell p) { record(54, 0, 0, p); },
      [](auto &b) { record(55, 0, b.ai.slot); },
      [](Cell p) { record(56, 0, 0, p); });
}
#ifdef MNM_EXECUTION_REFERENCE
struct Allocation {
  std::unique_ptr<unsigned char[]> bytes;
  unsigned serial = 0, size = 0;
  bool freed = false;
  void *data() const { return bytes.get() + 8; }
};
std::vector<Allocation> allocations;
std::vector<void *> sentinels;
unsigned nextSerial = 1, freeCalls = 0;
void *allocate(unsigned size, unsigned serial) {
  Allocation a;
  a.bytes = std::make_unique<unsigned char[]>(size + 16);
  a.size = size;
  a.serial = serial;
  std::memset(a.bytes.get(), 0xa7, size + 16);
  std::memset(a.bytes.get() + 8, 0, size);
  void *p = a.data();
  allocations.push_back(std::move(a));
  return p;
}
extern "C" void *allocateStub(unsigned size) {
  if (size != 48)
    throw std::runtime_error("order allocation size");
  return allocate(size, nextSerial++);
}
extern "C" void freeStub(void *p) {
  for (auto &a : allocations)
    if (a.data() == p) {
      if (a.freed)
        throw std::runtime_error("order double free");
      a.freed = true;
      ++freeCalls;
      return;
    }
  throw std::runtime_error("order free outside owned fixture");
}
unsigned serialOf(void *p) {
  if (p == sentinels[0])
    return 0;
  for (auto &a : allocations)
    if (a.data() == p)
      return a.serial;
  throw std::runtime_error("unknown node pointer");
}
void restoreOrderBody(unsigned entry) {
  for (auto &g : originalBodies)
    if (g.first == entry) {
      std::memcpy(reinterpret_cast<void *>(entry), g.second.data(),
                  g.second.size());
      return;
    }
  throw std::runtime_error("missing order body");
}
void checkOrderBody(unsigned entry) {
  for (auto &g : originalBodies)
    if (g.first == entry) {
      if (std::memcmp(reinterpret_cast<void *>(entry), g.second.data(),
                      g.second.size()))
        throw std::runtime_error("order body changed");
      return;
    }
}
void node(void *p, const CreatureOrder &o) {
  for (auto v :
       {std::pair<unsigned, unsigned>{8, o.kind},
        {12, unsigned(o.destination.x)},
        {16, unsigned(o.destination.y)},
        {20, unsigned(o.destination.z)},
        {24, o.target == -1 ? 0u
                            : unsigned(reinterpret_cast<std::uintptr_t>(
                                  actors.data() + unsigned(o.target) * 0xe4b))},
        {28, unsigned(o.referenceDestination.x)},
        {32, unsigned(o.referenceDestination.y)},
        {36, unsigned(o.referenceDestination.z)},
        {40, o.effect.value_or(0)}})
    write32(p, v.first, v.second);
  static_cast<unsigned char *>(p)[44] = o.repeat;
}
void prepareOrders() {
  allocations.clear();
  sentinels.clear();
  freeCalls = 0;
  prepare();
  for (unsigned i = 0; i < fixture->world.creatures.size(); ++i) {
    auto &a = fixture->world.creatures[i];
    auto &e = fixture->execution[i];
    auto *p = actors.data() + i * 0xe4b;
    auto *list = lists.data() + i * 16;
    auto *head = allocate(48, 0);
    sentinels.push_back(head);
    write32(head, 0, reinterpret_cast<std::uintptr_t>(head));
    write32(head, 4, reinterpret_cast<std::uintptr_t>(head));
    write32(list, 4, reinterpret_cast<std::uintptr_t>(head));
    write32(list, 8, e.orders.size());
    void *previous = head;
    void *cursor = head;
    for (auto &o : e.orders) {
      void *n = allocate(48, o.serial);
      node(n, o);
      write32(n, 4, reinterpret_cast<std::uintptr_t>(previous));
      write32(n, 0, reinterpret_cast<std::uintptr_t>(head));
      write32(previous, 0, reinterpret_cast<std::uintptr_t>(n));
      write32(head, 4, reinterpret_cast<std::uintptr_t>(n));
      previous = n;
      if (e.orderCursor == o.serial)
        cursor = n;
    }
    write32(p, 0x8fd, reinterpret_cast<std::uintptr_t>(cursor));
    static_cast<unsigned char *>(p)[0x901] =
        static_cast<unsigned char>(e.queuedFlag);
    write32(p, 0x570, a.selectedSpell);
    write32(p, 0x61c, a.castSpell);
    write32(p, 0x638, unsigned(a.castSource));
    write32(p, 0x63c, unsigned(a.castTarget));
    write32(p, 0x65c, e.targetDistance);
    write32(p, 0x62c, e.castAim.x);
    write32(p, 0x630, e.castAim.y);
    write32(p, 0x634, e.castAim.z);
    write32(p, 0x7cd, a.animationKind);
    write32(p, 0x104, a.flag104);
    for (unsigned j = 0; j < 8; ++j)
      write32(p, 0x24 + j * 4, a.ai.visible[j]);
  }
  nextSerial = fixture->execution[0].nextOrderSerial;
}
extern "C" void __attribute__((thiscall))
performStub(void *, unsigned kind, int x, int y, int z, void *target) {
  record(48, 0, kind, {x, y, z});
  record(49, 0, target ? slot(target) : 0xffffffffu);
}
extern "C" unsigned __attribute__((thiscall))
duplicateAdjustStub(void *, int *x, int *y, int *z) {
  record(50, 0, 0, {*x, *y, *z});
  *y = (*y + 1) % fixture->world.dimensions.y;
  return fixture->number % 3 != 0;
}
extern "C" void __attribute__((thiscall))
duplicateMoveStub(void *p, int x, int y, int z, unsigned flag, int *u, int *v,
                  int *w) {
  record(51, slot(p), flag, {x, y, z});
  *u = x + 1;
  *v = y + 2;
  *w = z + 1;
}
extern "C" unsigned __attribute__((thiscall))
followAdjustStub(void *, unsigned actor, void *, int *x, int *y, int *z) {
  if (actor != 0xffffffffu)
    throw std::runtime_error("follow query actor");
  record(52, 0, 0, {*x, *y, *z});
  *x = (*x + 1) % fixture->world.dimensions.x;
  return fixture->number % 3 != 0;
}
extern "C" void __attribute__((thiscall)) followMoveStub(void *p, int x, int y,
                                                         int z) {
  record(53, slot(p), 0, {x, y, z});
}
extern "C" void __attribute__((thiscall)) deliverMoveStub(void *, int x, int y,
                                                          int z) {
  record(54, 0, 0, {x, y, z});
}
extern "C" unsigned __attribute__((thiscall)) deliverTargetStub(void *,
                                                                void *target) {
  record(55, 0, slot(target));
  return 1;
}
extern "C" void __attribute__((thiscall)) deliverCoordinateStub(void *, int x,
                                                                int y, int z) {
  record(56, 0, 0, {x, y, z});
}
extern "C" void __attribute__((thiscall)) queueDeliveryStub(void *p) {
  record(13, slot(p));
}
// Real coordinate-order code reads one map cell and its tile's fine-height
// field.
Bytes mapCells, mapTiles;
void coordinateTerrain(Cell p) {
  mapCells.assign(32 * 32 * 8 * 12, 0);
  mapTiles.assign(17 * 356, 0);
  for (unsigned z = 0; z < 8; ++z)
    global(0x6cb8c2 + 4 * z, z * 32 * 32);
  for (unsigned y = 0; y < 32; ++y)
    global(0x6cb942 + 4 * y, y * 32);
  global(0x6c54dc, reinterpret_cast<std::uintptr_t>(mapCells.data()));
  global(0x65660c, reinterpret_cast<std::uintptr_t>(mapTiles.data()));
  unsigned short type = static_cast<unsigned short>((p.x + p.y) % 16);
  std::memcpy(mapCells.data() + ((p.z * 32 + p.y) * 32 + p.x) * 12, &type, 2);
  write32(mapTiles.data() + type * 356, 0x94, type);
}
void compareOrders(const Fixture &input, const Fixture &expected, unsigned op,
                   unsigned returned, const std::vector<Event> &trace) {
  Fixture source = input;
  fixture = &source;
  events.clear();
  prepareOrders();
  auto before = actors;
  constexpr unsigned entries[] = {0x50ad10, 0x50ca30, 0x513850, 0x508b00,
                                  0x523be0, 0x524130, 0x5243b0, 0x520ad0};
  unsigned entry = entries[op];
  restoreOrderBody(entry);
  redirect(0x512220, reinterpret_cast<void *>(queueDeliveryStub));
  redirect(0x514630, reinterpret_cast<void *>(duplicateAdjustStub));
  redirect(0x517e80, reinterpret_cast<void *>(duplicateMoveStub));
  redirect(0x520ad0, reinterpret_cast<void *>(performStub));
  if (op == 2) {
    redirect(0x4f5230, reinterpret_cast<void *>(followAdjustStub));
    redirect(0x50fae0, reinterpret_cast<void *>(followMoveStub));
  }
  if (op == 7) {
    restoreOrderBody(entry);
    redirect(0x50fae0, reinterpret_cast<void *>(deliverMoveStub));
    redirect(0x50ad10, reinterpret_cast<void *>(deliverTargetStub));
    redirect(0x50ca30, reinterpret_cast<void *>(deliverCoordinateStub));
  }
  using Step = unsigned(__attribute__((thiscall)) *)(void *);
  using Ref = unsigned(__attribute__((thiscall)) *)(void *, void *);
  using Value = void(__attribute__((thiscall)) *)(void *, unsigned);
  using Move = void(__attribute__((thiscall)) *)(void *, int, int, int);
  unsigned result = 0;
  auto *a = actors.data();
  auto c = command();
  if (op == 0)
    result = reinterpret_cast<Ref>(entry)(
        a,
        actors.data() + unsigned(source.world.creatures[0].ai.target) * 0xe4b);
  else if (op == 1) {
    coordinateTerrain({5, 7, 2});
    reinterpret_cast<Move>(entry)(a, 5, 7, 2);
  } else if (op == 2 || op == 5)
    result = reinterpret_cast<Step>(entry)(a) & 255u;
  else if (op == 3)
    reinterpret_cast<Value>(entry)(
        a, unsigned(source.world.creatures[0].ai.target));
  else if (op == 4) {
    using Append =
        void(__attribute__((thiscall)) *)(void *, unsigned, int, int, int, int);
    reinterpret_cast<Append>(entry)(a, c.kind, c.destination.x, c.destination.y,
                                    c.destination.z, c.target);
  } else if (op == 6)
    reinterpret_cast<Step>(entry)(a);
  else {
    using Deliver = void(__attribute__((thiscall)) *)(void *, unsigned, int,
                                                      int, int, void *);
    reinterpret_cast<Deliver>(entry)(
        a, c.kind, c.destination.x, c.destination.y, c.destination.z,
        c.target == -1 ? nullptr : actors.data() + unsigned(c.target) * 0xe4b);
  }
  if ((op == 0 || op == 2 || op == 5) && result != returned)
    throw std::runtime_error(
        "order return mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "trace index=" << i << " actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual " << events[i].kind << "," << events[i].argument
                << " pos " << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected " << trace[i].kind << "," << trace[i].argument
                << " pos " << trace[i].position.x << "," << trace[i].position.y
                << "," << trace[i].position.z << '\n';
    throw std::runtime_error(
        "order trace mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i) {
    auto &b = expected.world.creatures[i];
    auto &e = expected.execution[i];
    auto *p = before.data() + i * 0xe4b;
    packActor(p, b, e);
    static_cast<unsigned char *>(p)[0x901] =
        static_cast<unsigned char>(e.queuedFlag);
    for (auto f : {std::pair<unsigned, unsigned>{0x61c, b.castSpell},
                   {0x638, unsigned(b.castSource)},
                   {0x63c, unsigned(b.castTarget)},
                   {0x65c, e.targetDistance},
                   {0x62c, unsigned(e.castAim.x)},
                   {0x630, unsigned(e.castAim.y)},
                   {0x634, unsigned(e.castAim.z)}})
      write32(p, f.first, f.second);
  }
  for (auto e : trace)
    if (e.kind == 6) {
      write32(before.data() + e.slot * 0xe4b, 0x5f4,
              behaviorAddresses[e.argument - 1]);
      write32(before.data() + e.slot * 0xe4b, 0x5f8, 0);
    }
  // Order cursor and owned nodes are compared as logical identities below.
  write32(before.data(), 0x8fd, read32(actors.data(), 0x8fd));
  if (actors != before) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "order write mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  auto &e = expected.execution[0];
  auto *list = lists.data();
  if (read32(list, 8) != e.orders.size())
    throw std::runtime_error("order node count mismatch");
  void *cursor = reinterpret_cast<void *>(read32(actors.data(), 0x8fd));
  if (serialOf(cursor) != e.orderCursor.value_or(0))
    throw std::runtime_error(
        "order cursor mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  auto *head = sentinels[0];
  void *n = reinterpret_cast<void *>(read32(head, 0));
  void *previous = head;
  for (auto &o : e.orders) {
    if (n == head ||
        read32(n, 4) != reinterpret_cast<std::uintptr_t>(previous) ||
        serialOf(n) != o.serial)
      throw std::runtime_error("order links mismatch");
    Bytes b(48, 0);
    node(b.data(), o);
    if (std::memcmp(static_cast<unsigned char *>(n) + 8, b.data() + 8, 37)) {
      for(unsigned field=8;field<44;field+=4)std::cerr<<"node "<<o.serial<<" field "<<field<<" actual="<<read32(n,field)<<" expected="<<read32(b.data(),field)<<'\n';
      std::cerr<<"repeat actual="<<unsigned(static_cast<unsigned char*>(n)[44])<<" expected="<<unsigned(b[44])<<'\n';
      throw std::runtime_error("order payload mismatch n=" + std::to_string(input.number)+" op="+std::to_string(op));
    }
    previous = n;
    n = reinterpret_cast<void *>(read32(n, 0));
  }
  if (n != head ||
      read32(head, 4) != reinterpret_cast<std::uintptr_t>(previous))
    throw std::runtime_error("order sentinel mismatch");
  for (auto &m : allocations)
    for (unsigned j = 0; j < 8; ++j)
      if (m.bytes[j] != 0xa7 || m.bytes[m.size + 8 + j] != 0xa7)
        throw std::runtime_error("order allocation extent overwritten");
  checkOrderBody(entry);
  restoreOrderBody(0x50ad10);
  restoreOrderBody(0x50ca30);
  restoreOrderBody(0x50fae0);
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("expected pinned PE");
  install(argv[1]);
  redirect(0x597850, reinterpret_cast<void *>(allocateStub));
  redirect(0x597870, reinterpret_cast<void *>(freeStub));
#endif
  (void)argc;
  (void)argv;
  unsigned cases = 0;
  for (unsigned n = 0; n < 4096; ++n)
    for (unsigned op = 0; op < 8; ++op) {
      auto input = generate(n);
      auto &a = input.world.creatures[0];
      auto &e = input.execution[0];
      a.selectedSpell = n % 104;
      a.flag104 = n % 2;
      a.animationKind = n % 4;
      a.formCapability = n % 2;
      a.ai.flying = n % 3;
      a.form = n % 13 == 0;
      a.ai.target = int((n + 1) % input.world.creatures.size());
      e.follow = n % 3 == 0 ? -1 : a.ai.target;
      if (op >= 4) {
        a.ai.behavior = n % 2 ? 2 : 1;
        unsigned count = n % 5;
        e.nextOrderSerial = count + 1;
        e.queuedCount = count;
        e.queuedFlag = n % 3 == 0;
        for (unsigned i = 0; i < count; ++i) {
          CreatureOrder o;
          o.serial = i + 1;
          o.kind = i % 3 == 0 ? 3 : 2;
          o.destination = {int((n + i) % 32), 7, int(n % 4)};
          o.referenceDestination = o.destination;
          o.target = o.kind == 3 ? a.ai.target : -1;
          o.repeat = n % 7 == 0;
          o.effect = i % 2 ? std::optional<unsigned>(0x1000 + i) : std::nullopt;
          e.orders.push_back(o);
        }
        if (!e.orders.empty()) {
          e.orderCursor = e.orders[n % count].serial;
          if (n % 11 == 0) {
            e.orders.back().kind = 2 + n % 3;
            e.orders.back().destination = {int(n % 32), 7, int(n % 4)};
            e.orders.back().referenceDestination = e.orders.back().destination;
            e.orders.back().target =
                e.orders.back().kind == 3 ? a.ai.target : -1;
          }
          e.orders[0].kind = 2;
          e.orders[0].target = -1;
        }
      }
      auto expected = input;
      fixture = &expected;
      events.clear();
      auto &v = expected.world.creatures[0];
      auto &x = expected.execution[0];
      auto s = orderServices();
      unsigned returned = 0;
      if (op == 0)
        returned = orderTarget(v, x, expected.world, v.ai.target, s);
      else if (op == 1)
        orderCoordinateAttack(v, x, expected.world, {5, 7, 2}, s);
      else if (op == 2)
        returned = followReference(v, x, expected.world, followServices());
      else if (op == 3)
        repairCreatureReference(v, x, expected.world, v.ai.target, s);
      else if (op == 4)
        appendOrder(v, x, expected.world, command(), s);
      else if (op == 5)
        returned = advanceOrders(v, x, s);
      else if (op == 6)
        clearOrders(x, s);
      else
        performDelivery(v, command(), expected.world);
      auto trace = events;
      mix(returned);
      mix(v.ai.behavior);
      mix(unsigned(v.ai.target));
      mix(v.ai.pending);
      mix(x.queuedCount);
      mix(x.queuedFlag);
      mix(x.orderCursor.value_or(0));
      for (auto &o : x.orders) {
        mix(o.serial);
        mix(o.kind);
        mix(o.repeat);
        mix(o.effect.value_or(0));
        mix(unsigned(o.destination.x));
        mix(unsigned(o.destination.y));
        mix(unsigned(o.destination.z));
      }
      for (auto z : trace) {
        mix(z.kind);
        mix(z.argument);
        mix(unsigned(z.position.x));
        mix(unsigned(z.position.y));
        mix(unsigned(z.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareOrders(input, expected, op, returned, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":8,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
