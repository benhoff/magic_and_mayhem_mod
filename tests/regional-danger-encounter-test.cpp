#include "../game/persistence/snapshot.hpp"
#include "regional-danger-encounter-state.hpp"
#ifdef MNM_OWNER_KNOWLEDGE
#include "owner-knowledge-encounter-state.hpp"
#endif
#ifdef MNM_PAIRED_REFERENCE
#include "regional-danger-original.hpp"
#endif
#include <fstream>
#include <iostream>
namespace {
using namespace mnm;
namespace reg = encounter::regional_danger;
namespace cs = command_session;
#ifdef MNM_PAIRED_REFERENCE
m::RegionalDangerActor observedActor(const Original &o, unsigned i,
                                    const unsigned char *snapshot = nullptr) {
  const auto *raw = snapshot ? snapshot : o.actors.data() + i * 0xe4b;
  auto *cfg = reinterpret_cast<void *>(get(raw, 0xac));
  m::RegionalDangerActor a;
  a.slot = int(get(raw, 0));
  a.owner = int(get(raw, 0x174));
  a.region = int(get(raw, 0xde7));
  a.position = {int(get(raw, 8)), int(get(raw, 12)), int(get(raw, 16))};
  a.type = get(raw, 0xa8);
  a.typeClass = get(cfg, 0);
  a.height = get(cfg, 12);
  a.width = get(cfg, 8);
  a.movementKind = get(cfg, 0x44);
  a.health = get(raw, 0xe4);
  a.resource = get(raw, 0xe8);
  a.maximumResource = get(raw, 0xec);
  a.threat = int(get(raw, 0x91e));
  using Aging = int(__attribute__((thiscall)) *)(const void *, int);
  a.aging = reinterpret_cast<Aging>(0x520620)(raw, int(get(cfg, 0x589)));
  using Power = unsigned(__attribute__((thiscall)) *)(const void *);
  a.inventoryResource = reinterpret_cast<Power>(0x51e500)(raw) - a.resource;
  a.attackRange = int(get(cfg, 0x34));
  a.animationKind = get(raw, 0x7cd);
  a.active = get(raw, 4) != 0;
  a.ranged = get(cfg, 0x34) != 0;
  a.flying = get(raw, 0x108) != 0;
  a.flag104 = get(raw, 0x104) != 0;
  a.status114 = get(raw, 0x114) != 0;
  a.block7c = get(raw, 0x7c) != 0;
  a.block723 = get(raw, 0x723, 1) != 0;
  a.behavior14 = get(raw, 0x5ec) == 14;
  for (unsigned j = 0; j < 8; ++j)
    a.estimatedResource[j] = get(raw, 0xe2b + 4 * j);
  return a;
}
[[maybe_unused]] void originalObserve(const Original &o, m::RegionalDangerInputs &in) {
  in.tick = int(o.tick) + 1;
  in.actors.clear();
  for (unsigned i = 0; i < o.profile.initial.world.creatures.size(); ++i)
    in.actors.push_back(observedActor(o, i));
  for (unsigned owner = 0; owner < 8; ++owner) {
    auto &mem = in.memories[owner];
    for (auto &a : in.actors) {
      const auto *raw = o.actors.data() + unsigned(a.slot) * 0xe4b;
      if (!get(raw, 0x24 + 4 * owner))
        continue;
      auto it = std::lower_bound(mem.begin(), mem.end(), a.slot,
                                 [](auto &v, int key) { return v.key < key; });
      if (a.owner == int(owner) || in.allied[owner][unsigned(a.owner)]) {
        if (it != mem.end() && it->key == a.slot)
          mem.erase(it);
        continue;
      }
      m::RegionalDangerMemory record{a.slot, in.tick, a};
      if (it != mem.end() && it->key == a.slot)
        *it = record;
      else
        mem.insert(it, record);
    }
  }
}
void originalPolicy(Original &o, m::RegionalDangerInputs &in,
                    unsigned scenario) {
  if (reg::visibilityEvent(scenario, o.tick))
    put(o.actors.data() + 0xe4b, 0x24, o.tick != 64);
  if (reg::diplomacyEvent(scenario, o.tick))
    in.allied[0][1] = in.allied[1][0] = 1;
  if ((scenario != 3 && o.tick == 32) || (scenario == 1 && o.tick == 96)) {
    const unsigned slot = o.tick == 96 ? 1 : 0;
    const auto q = slot ? o.profile.initial.execution[0].roamHome
                        : o.profile.initial.world.creatures[0].destination;
    using Move = void(__attribute__((thiscall)) *)(void *, int, int, int);
    reinterpret_cast<Move>(0x50fae0)(o.actors.data() + slot * 0xe4b, q.x, q.y,
                                     q.z);
  }
  if (scenario == 1 && o.tick ==
#ifdef MNM_OWNER_KNOWLEDGE
      1792
#else
      384
#endif
  ) {
    using Health =
        void(__attribute__((thiscall)) *)(void *, int, int, void *, int);
    reinterpret_cast<Health>(0x5076f0)(o.actors.data() + 0xe4b, -10000, -1,
                                       nullptr, 1);
  }
}
encounter::paired::Projection originalDanger(const Original &o,
                                             const DangerOriginal &d,
                                             const m::RegionalDangerInputs &in,
                                             unsigned count) {
  std::vector<m::NavigationRegion> g(count);
  m::RegionalDangerState s;
  const auto *ctx = d.context.data();
  s.resetDanger = get(ctx, 0x12073, 1);
  s.dangerOwner = int(get(ctx, 0x12074));
  s.memoryKey = int(get(ctx, 0x12078));
  s.dangerRegion = int(get(ctx, 0x1207c));
  s.dangerTotal = int(get(ctx, 0x12084));
  const auto commander = get(ctx, 0x12080);
  s.commander =
      commander ? int((commander - address(o.actors.data())) / 0xe4b) : -1;
  s.resetDeduction = get(ctx, 0x12098, 1);
  s.deductionOwner = int(get(ctx, 0x12099));
  s.actorCursor = int(get(ctx, 0x1209d));
  s.deductionRegion = int(get(ctx, 0x120a1));
  s.deductionTotal = int(get(ctx, 0x120a9));
  s.pendingDanger.resize(count);
  s.pendingDeduction.resize(count);
  s.pendingBase.resize(count);
  for (unsigned i = 0; i < count; ++i) {
    const auto *raw = reinterpret_cast<void *>(get(regionManager) + 660 * i);
    for (unsigned owner = 0; owner < 8; ++owner)
      for (unsigned j = 0; j < 7; ++j) {
        g[i].danger[owner][j] = get(raw, 532 + 7 * owner + j, 1);
        g[i].deductedDanger[owner][j] = get(raw, 588 + 7 * owner + j, 1);
      }
    for (unsigned j = 0; j < 7; ++j) {
      s.pendingDanger[i][j] = get(raw, 645 + j, 1);
      s.pendingDeduction[i][j] = get(raw, 652 + j, 1);
    }
    g[i].dangerBase = get(raw, 644, 1);
    s.pendingBase[i] = get(raw, 659, 1);
  }
  for (unsigned i = 0; i < 8; ++i)
    if (in.enabled[i]) {
      const auto *raw = o.actors.data() + unsigned(in.commander[i]) * 0xe4b;
      s.dangerTotals[i] = int(get(raw, 0x922));
      s.deductionTotals[i] = int(get(raw, 0x926));
    }
  for (unsigned i = 0; i < in.actors.size(); ++i)
    s.actorSupport.push_back(int(get(o.actors.data() + i * 0xe4b, 0x92a)));
  return reg::dangerProjection(g, s, in);
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_PAIRED_REFERENCE
  if (argc != 7)
    throw std::runtime_error("PE INSTALL SCENARIO SEED TICKS NEW_OUTPUT");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  for (unsigned at : {0x411e50u, 0x466870u}) {
    auto prior = redirects.at(at);
    std::memcpy(reinterpret_cast<void *>(at), prior.data(), prior.size());
    redirects.erase(at);
  }
  redirect(0x4dc5e0, reinterpret_cast<std::uintptr_t>(&noArg));
  sealText(pe);
  initializeSearch();
  using Constructor = void *(__attribute__((thiscall)) *)(void *, unsigned);
  reinterpret_cast<Constructor>(0x53a660)(regionManager, 512);
  const unsigned scenario = std::stoul(argv[3]), seed = std::stoul(argv[4]),
                 ticks = std::stoul(argv[5]);
  auto p = reg::profile(argv[2], scenario);
  game::CreatureRuleSession session(reg::runtime(p, scenario), p.bytes, seed,
                                    unsigned(p.initial.world.creatures.size()));
  auto initial = cs::decode(session.world().state().systems);
  Original o(p);
  o.actors.resize(125 * 0xe4b);
  setup(o, initial, seed);
  for (unsigned i = 0; i < initial.execution.size(); ++i)
    for (unsigned j = 0; j < 6; ++j)
      put(o.actors.data() + i * 0xe4b, 0x90a + 4 * j,
          initial.execution[i].combatValue[j]);
  const auto shapes = encounter::navigationShapes(p);
  for (unsigned i = 0; i < 28; ++i) {
    auto *cfg = reinterpret_cast<void *>(0x6a5f80 + i * 0x5c9);
    put(cfg, 8, shapes[i].width);
    put(cfg, 12, shapes[i].height);
    put(cfg, 0x44, shapes[i].movementKind);
  }
  using Build = void(__attribute__((thiscall)) *)(void *, int);
  reinterpret_cast<Build>(0x53ab20)(regionManager, 1);
  call(0x53d710, regionManager);
  using Connect = void(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
  reinterpret_cast<Connect>(0x53bb40)(regionManager, 1, 0);
  if (get(regionManager, 8) != initial.native.regions.size())
    throw std::runtime_error("independent danger graph count");
  for (unsigned i = 0; i < initial.native.regions.size(); ++i) {
    auto *raw = reinterpret_cast<void *>(get(regionManager) + 660 * i);
    const auto &v = initial.native.regions[i];
    if (get(raw, 4) != unsigned(v.position.x) ||
        get(raw, 8) != unsigned(v.position.y) ||
        get(raw, 12) != unsigned(v.position.z))
      throw std::runtime_error("independent danger node");
    for (unsigned j = 0; j < 18; ++j) {
      const auto &edge = v.connections[j];
      if (get(raw, 16 + 28 * j) != unsigned(edge.cost) ||
          get(raw, 20 + 28 * j) != unsigned(edge.destination) ||
          std::memcmp(static_cast<unsigned char *>(raw) + 24 + 28 * j,
                      edge.clearance.data(), 20))
        throw std::runtime_error("independent danger edge");
    }
  }
  global(0x6e1f3c, 1);
  DangerOriginal danger;
  auto observed = initial.native.regionalDanger->inputs;
  global(0x6def58, address(o.actors.data()));
  global(0x6def5c, 125);
  danger.inputs(observed, false);
  danger.memoryInputs(observed);
  danger.effectInputs(observed);
#ifdef MNM_OWNER_KNOWLEDGE
  put(danger.context.data(), 0x11fed, 1, 1);
  m::OwnerKnowledge knowledge;
  auto readKnowledge = [&] {
    auto *ctx=danger.context.data();
    knowledge.reset=get(ctx,0x11fed,1);knowledge.owner=int(get(ctx,0x11fee));
    knowledge.previousSweep=int(get(ctx,0x11da2));knowledge.currentSweep=int(get(ctx,0x11da6));
    for(unsigned owner=0;owner<8;++owner){
      auto &records=knowledge.records[owner]; records.clear(); observed.memories[owner].clear();
      knowledge.nextKey[owner]=int(get(ctx,0x11d82+4*owner));
      const unsigned head=get(ctx,0x11c06+48*owner);
      auto cursor=get(reinterpret_cast<void *>(head));
      while(cursor!=head){
        auto *raw=reinterpret_cast<unsigned char *>(cursor);
        m::OwnerMemory r;
        r.memory={int(get(raw,12)),int(get(raw,16)),observedActor(o,0,raw+24)};
        r.markedTick=int(get(raw,20));r.finePosition={int(get(raw,44)),int(get(raw,48)),int(get(raw,52))};
        records.push_back(r);observed.memories[owner].push_back(r.memory);
        call(0x42d0a0,&cursor);
      }
    }
    if(!knowledge.reset){
      auto cursor=get(ctx,0x11ff2),head=get(ctx,0x11c06+48*unsigned(knowledge.owner));
      knowledge.cursor=cursor==head?-1:int(get(reinterpret_cast<void *>(cursor),12));
    }
    for(unsigned i=0;i<125;++i)knowledge.matched[i]=get(ctx,0x11ff6+i,1)!=0;
  };
#endif
  const std::filesystem::path out = argv[6];
  if (!std::filesystem::create_directory(out))
    throw std::runtime_error("existing danger output");
  game::writeSnapshot(out / "initial.snapshot", session.world().state());
  const auto final = out / "final.snapshot", tracePath = out / "trace.txt";
#else
  if (argc != 7)
    throw std::runtime_error(
        "SNAPSHOT SCENARIO TICKS NEW_SNAPSHOT NEW_TRACE CHECKPOINT_INTERVAL");
  auto saved = game::readSnapshot(argv[1]);
  const unsigned scenario = std::stoul(argv[2]), ticks = std::stoul(argv[3]),
                 interval = std::stoul(argv[6]);
  auto p = reg::profile(saved.campaign, scenario);
  game::World world(0);
  world.restore(std::move(saved));
  game::CreatureRuleSession session(std::move(world),
                                    reg::runtime(p, scenario));
  auto initial = cs::decode(session.world().state().systems);
  const std::filesystem::path final = argv[4], tracePath = argv[5];
  if (std::filesystem::exists(tracePath))
    throw std::runtime_error("existing danger trace");
#endif
  if (scenario > 3 || ticks > 8192)
    throw std::runtime_error("danger fixture bound");
  std::ofstream trace(tracePath);
  std::uint64_t digest = 1469598103934665603ull;
  unsigned restores = 0, dangerFrames = 0, regionalTicks = 0, arrivals = 0,
           hiddenFrames = 0, rememberedFrames = 0, rowChanges = 0;
  std::uint64_t priorRows = 0;
  for (unsigned tick = 0; tick < ticks; ++tick) {
#ifdef MNM_PAIRED_REFERENCE
    originalPolicy(o, observed, scenario);
#ifdef MNM_OWNER_KNOWLEDGE
    observed.tick=int(o.tick)+1;
    observed.actors.clear();
    for(unsigned i=0;i<o.profile.initial.world.creatures.size();++i)
      observed.actors.push_back(observedActor(o,i));
#else
    originalObserve(o, observed);
#endif
    danger.inputs(observed, false);
#ifdef MNM_OWNER_KNOWLEDGE
    reinterpret_cast<Value>(0x4181d0)(danger.context.data(),1);
#else
    danger.memoryInputs(observed);
#endif
    danger.effectInputs(observed);
    call(0x41a060, danger.context.data());
    call(0x41d270, danger.context.data());
#ifdef MNM_OWNER_KNOWLEDGE
    reinterpret_cast<Value>(0x4181d0)(danger.context.data(),2);
    readKnowledge();
#endif
    // Declared danger input lists are isolated from unrelated World effects.
    for (unsigned at : {0x6898e4u, 0x689908u, 0x6898fcu, 0x6898f0u})
      global(at, address(danger.empty.data()));
    step(o);
    call(0x5404a0, regionManager);
#endif
    session.step();
    auto c = cs::decode(session.world().state().systems);
    auto projected = reg::projection(c, p);
#ifdef MNM_OWNER_KNOWLEDGE
    auto nativeKnowledge=reg::knowledgeProjection(*c.native.ownerKnowledge);
    projected.insert(projected.end(),nativeKnowledge.begin(),nativeKnowledge.end());
#endif
#ifdef MNM_PAIRED_REFERENCE
    auto expected = projectOriginal(o, false);
    auto add = [&](std::string name, unsigned v) {
      expected.push_back({std::move(name), v});
    };
    for (unsigned slot = 0; slot < c.world.creatures.size(); ++slot) {
      auto *raw = o.actors.data() + slot * 0xe4b;
      const auto prefix = "regional.actor[" + std::to_string(slot) + "].";
      add(prefix + "pending", get(raw, 0xd03));
      add(prefix + "cursor", get(raw, 0xdcf));
      for (unsigned j = 0; j < 50; ++j)
        add(prefix + "path[" + std::to_string(j) + "]",
            get(raw, 0xd07 + 4 * j));
      add(prefix + "destination.x", get(raw, 0xdd3));
      add(prefix + "destination.y", get(raw, 0xdd7));
      add(prefix + "destination.z", get(raw, 0xddb));
    }
    for (unsigned i = 0; i < c.native.regions.size(); ++i)
      add("regional.traffic[" + std::to_string(i) + "]",
          get(reinterpret_cast<void *>(get(regionManager) + 660 * i), 528));
    std::uint64_t grid = 1469598103934665603ull;
    for (unsigned i = 0; i < c.native.cells.size(); ++i) {
      auto *raw = o.terrain.data() + 12 * i;
      auto &v = c.native.cells[i];
      unsigned occupant = get(raw, 4, 2);
      if (occupant != 0xffff)
        ++occupant;
      const unsigned actual[]{get(raw, 0, 2), occupant, get(raw, 10, 2),
                              get(raw, 8, 2)},
          want[]{v.kind, v.occupant, v.flags, v.links};
      for (unsigned j = 0; j < 4; ++j) {
        if (actual[j] != want[j])
          throw std::runtime_error("danger grid differs tick=" +
                                   std::to_string(c.native.tick));
        grid = (grid ^ actual[j]) * 1099511628211ull;
      }
    }
    add("regional.grid.low", unsigned(grid));
    add("regional.grid.high", unsigned(grid >> 32));
    for (unsigned i = 0; i < c.execution.size(); ++i)
      for (unsigned j = 0; j < 6; ++j)
        add("value[" + std::to_string(i) + "][" + std::to_string(j) + "]",
            get(o.actors.data() + i * 0xe4b, 0x90a + 4 * j));
    auto values = originalDanger(o, danger, observed, c.native.regions.size());
    expected.insert(expected.end(), values.begin(), values.end());
#ifdef MNM_OWNER_KNOWLEDGE
    auto originalKnowledge=reg::knowledgeProjection(knowledge);
    expected.insert(expected.end(),originalKnowledge.begin(),originalKnowledge.end());
#endif
    auto difference = encounter::paired::difference(projected, expected);
    if (!difference.empty())
      throw std::runtime_error("danger tick=" + std::to_string(c.native.tick) +
                               "\n" + difference);
#endif
#ifndef MNM_PAIRED_REFERENCE
    if (interval && (tick + 1) % interval == 0) {
      game::World restored(0);
      restored.restore(
          game::decodeSnapshot(game::encodeSnapshot(session.world().state())));
      session = game::CreatureRuleSession(std::move(restored),
                                          reg::runtime(p, scenario));
      ++restores;
    }
#endif
    auto &d = *c.native.regionalDanger;
    std::uint64_t rows = 0;
    for (auto &r : c.native.regions)
      for (auto &row : r.danger)
        for (auto v : row)
          rows = rows * 131 + v;
    dangerFrames += rows != 0;
    rowChanges += rows != priorRows;
    priorRows = rows;
    regionalTicks += c.actions.segments[0].regionPending;
    arrivals += c.world.creatures[0].ai.position ==
                p.initial.world.creatures[0].destination;
    hiddenFrames += c.world.creatures[1].ai.visible[0] == 0;
    if (!c.world.creatures[1].ai.visible[0] && !d.inputs.memories[0].empty())
      rememberedFrames +=
          d.inputs.memories[0][0].observedTick < int(c.native.tick);
    digest = encounter::paired::digest(digest, projected);
    trace << c.native.tick << ' ' << digest << '\n';
  }
  if (!trace)
    throw std::runtime_error("danger trace write");
  game::writeSnapshot(final, session.world().state());
  auto finalState = cs::decode(session.world().state().systems);
  const auto stats = encounter::statistics(finalState);
#ifdef MNM_PAIRED_REFERENCE
  using Erase =
      void(__attribute__((thiscall)) *)(void *, unsigned *, unsigned, unsigned);
  unsigned result = 0;
  reinterpret_cast<Erase>(0x42acc0)(danger.context.data() + 0x12088, &result,
                                    get(danger.header.data()),
                                    address(danger.header.data()));
  checkText();
#ifdef MNM_OWNER_KNOWLEDGE
  for(unsigned owner=0;owner<8;++owner)
    call(0x42f2e0,danger.context.data()+0x11c02+48*owner);
#endif
  call(0x53a840, regionManager);
  destroySearch();
#endif
  std::cout << "{\"success\":true,\"ticks\":" << ticks
            << ",\"digest\":" << digest << ",\"restores\":" << restores
            << ",\"danger_frames\":" << dangerFrames
            << ",\"row_changes\":" << rowChanges
            << ",\"regional_ticks\":" << regionalTicks
            << ",\"arrivals\":" << arrivals
            << ",\"hidden_frames\":" << hiddenFrames
            << ",\"remembered_frames\":" << rememberedFrames
            << ",\"hits\":" << stats.hits << ",\"deaths\":" << stats.deaths
            << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
