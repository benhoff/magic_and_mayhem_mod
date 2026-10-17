#include "creature_runtime.hpp"
#include "map_campaign.hpp"
#include "persistence/snapshot.hpp"
#include "trace.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace g = mnm::game;
namespace app = mnm::skirmish;
void need(bool ok, const char *why) {
  if (!ok)
    throw std::runtime_error(why);
}
g::Bytes bytes(const g::CampaignSession &s) {
  return g::encodeCampaignSnapshot(s.checkpoint(), s.progress().identity);
}
g::Entity actor(g::Point p, unsigned owner) {
  g::Entity e;
  e.type = 17;
  e.owner = owner;
  e.x = p.x;
  e.y = p.y;
  e.z = p.z;
  return e;
}
g::CampaignSession fresh(const std::filesystem::path &root) {
  auto s = app::newMapCampaign(
      app::mapCampaignResources(root, true, true, true, true, true));
  s.startMission();
  return s;
}
void weaken(g::CampaignSession &s, const std::vector<g::Handle> &group) {
  auto c = s.battle()->combat();
  const auto f = c.combat().fighters;
  for (auto h : group)
    for (const auto &a : f)
      if (a.actor == h && a.health > 1)
        c.damage(h, a.health - 1, s.battle()->state().wizards[0].actor);
  auto checkpoint = s.checkpoint();
  checkpoint.world = c.world().state();
  s.restore(std::move(checkpoint));
}
g::CampaignSession seed(const std::filesystem::path &root) {
  auto s = fresh(root);
  auto c = s.battle()->combat();
  auto meta = s.battle()->state();
  auto ally = c.spawn(actor({8, 2, 7}, 0), c.driver()->creatureStats());
  meta.units.push_back({ally, g::UnitRole::sentinel});
  auto checkpoint = s.checkpoint();
  checkpoint.world = c.world().state();
  checkpoint.world.campaign = g::encodeSkirmish(meta);
  s.restore(std::move(checkpoint));
  weaken(s, s.progress().mission->initial);
  return s;
}
void pilot(g::CampaignSession &s) {
  if (!s.battle())
    return;
  const auto &b = *s.battle();
  const auto meta = b.state();
  const auto combat = b.combat().combat();
  const auto wizard = meta.wizards[0];
  if (wizard.spell != g::Spell::none)
    return;
  unsigned allies = 0;
  for (const auto &f : combat.fighters)
    if (f.health && b.world().find(f.actor)->owner == 0 &&
        !(f.actor == wizard.actor))
      ++allies;
  if (wizard.mana >= g::SkirmishRules::summonCost && allies < 3) {
    const auto *w = b.world().find(wizard.actor);
    for (auto p : b.mission().standing)
      if (std::max(std::abs(p.x - w->x), std::abs(p.y - w->y)) <= 2 &&
          b.combat().driver()->canSpawn(b.combat(), p)) {
        s.enqueue({g::SkirmishOrderKind::summon, wizard.actor, {}, p});
        return;
      }
  }
  if (wizard.mana >= g::SkirmishRules::boltCost) {
    auto target = meta.wizards[1].actor;
    if (s.progress().mission) {
      const auto &m = *s.progress().mission;
      for (auto h : m.stage == 0 ? m.initial : m.wave)
        for (const auto &f : combat.fighters)
          if (f.actor == h && f.health) {
            target = h;
            break;
          }
    }
    s.enqueue({g::SkirmishOrderKind::bolt, wizard.actor, target, {}});
  }
}
int main(int argc, char **argv) try {
  need(argc >= 2,
       "usage: staged-mission-test INSTALLATION [seed OUTPUT | wave OUTPUT]");
  const std::filesystem::path root = argv[1];
  if (argc == 4) {
    auto s = seed(root);
    if (std::string(argv[2]) == "wave") {
      for (unsigned i = 0;
           i < 100 && s.battle() && s.progress().mission->stage == 0; ++i)
        s.step();
      need(s.battle() && s.progress().mission->stage == 1,
           "initial stage failed");
      s.step();
      need(s.progress().mission->reinforcement, "wave failed");
      weaken(s, s.progress().mission->wave);
    } else
      need(std::string(argv[2]) == "seed", "unknown fixture mode");
    need(s.save(argv[3]).durable, "fixture save");
    std::cout << "{\"success\":true}\n";
    return 0;
  }
  auto s = fresh(root);
  need(s.progress().mission->stage == 0 && !s.spellAvailable(g::Spell::cure),
       "initial objective/loadout");
  const auto original = bytes(s);
  bool rejected = false;
  try {
    s.enqueue({g::SkirmishOrderKind::cure,
               s.battle()->state().wizards[0].actor,
               s.progress().mission->initial[0],
               {}});
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected && bytes(s) == original, "locked spell mutated campaign");
  auto forged = s.checkpoint();
  forged.campaign.wizards[0].spells.push_back("cure");
  rejected = false;
  try {
    s.restore(forged);
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected && bytes(s) == original, "unearned reward accepted");
  forged = s.checkpoint();
  forged.campaign.mission->initial[0].generation++;
  rejected = false;
  try {
    s.restore(forged);
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected && bytes(s) == original, "stale objective handle accepted");
  forged = s.checkpoint();
  auto pending = g::decodeSkirmish(forged.world.campaign);
  pending.pending.push_back({g::SkirmishOrderKind::cure,
                             pending.wizards[0].actor,
                             forged.campaign.mission->initial[0],
                             {}});
  forged.world.campaign = g::encodeSkirmish(pending);
  rejected = false;
  try {
    s.restore(forged);
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected && bytes(s) == original, "queued locked spell accepted");
  need(!s.step({false, false, false}).battle && bytes(s) == original,
       "unadmitted mission advanced");
  auto malformed = s.progress();
  malformed.battle->attempt = 0;
  rejected = false;
  try {
    g::encodeCampaign(malformed);
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected, "objective continuation accepted an unbound attempt");
  auto truncated = g::encodeCampaign(s.progress());
  truncated.pop_back();
  rejected = false;
  try {
    g::decodeCampaign(truncated);
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected, "truncated objective continuation accepted");
  auto plan = *s.missionPlan();
  plan.version = 2;
  rejected = false;
  try {
    g::validateMissionPlan(plan, s.battle()->mission());
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  need(rejected, "unsupported definition accepted");
  auto controlled = *s.battle();
  const auto beforeGroup = g::encodeSnapshot(controlled.world().state());
  auto refusedGroup = controlled.spawnMissionGroup({{8, 6, 7}, {8, 4, 7}});
  need(!refusedGroup &&
           g::encodeSnapshot(controlled.world().state()) == beforeGroup,
       "reinforcement partial publication");
  auto early = fresh(root);
  auto ec = early.battle()->combat();
  const auto em = early.battle()->state();
  ec.damage(em.wizards[1].actor, g::SkirmishRules::wizardHealth - 1,
            em.wizards[0].actor);
  auto earlyCheckpoint = early.checkpoint();
  earlyCheckpoint.world = ec.world().state();
  early.restore(earlyCheckpoint);
  early.enqueue({g::SkirmishOrderKind::bolt,
                 em.wizards[0].actor,
                 em.wizards[1].actor,
                 {}});
  for (unsigned i = 0; i < 7; ++i)
    early.step();
  need(early.battle() && early.progress().mission->stage == 0 &&
           !early.spellAvailable(g::Spell::cure),
       "early wizard kill skipped objectives");
  auto defeat = fresh(root);
  auto dc = defeat.battle()->combat();
  const auto dm = defeat.battle()->state();
  dc.damage(dm.wizards[0].actor, g::SkirmishRules::wizardHealth - 1,
            dm.wizards[1].actor);
  auto defeatCheckpoint = defeat.checkpoint();
  defeatCheckpoint.world = dc.world().state();
  defeat.restore(defeatCheckpoint);
  for (unsigned i = 0; i < 120 && defeat.battle(); ++i)
    defeat.step();
  need(!defeat.battle() &&
           defeat.progress().lastResult->outcome ==
               g::CampaignOutcome::defeat &&
           !defeat.spellAvailable(g::Spell::cure),
       "defeat earned reward");
  defeat.startMission();
  need(defeat.progress().battle->attempt == 3 &&
           defeat.progress().mission->stage == 0 &&
           !defeat.progress().mission->reinforcement,
       "retry retained mission events");
  // Normal player orders and full installed health/damage: no fixture
  // reductions.
  auto natural = fresh(root);
  unsigned naturalTicks = 0, naturalHits = 0;
  for (; natural.battle() && naturalTicks < 768; ++naturalTicks) {
    pilot(natural);
    auto r = natural.step();
    for (const auto &e : r.battle->combat.events)
      if (e.kind == g::CombatEventKind::hit && e.actor.slot >= 2 &&
          e.target.slot < 2)
        ++naturalHits;
  }
  if (natural.battle()) {
    std::cerr << "natural stage " << natural.progress().mission->stage
              << " wave " << natural.progress().mission->reinforcement
              << " ticks " << naturalTicks << "\n";
    app::trace(std::cerr, *natural.battle());
  } else
    std::cerr << "natural outcome "
              << unsigned(natural.progress().lastResult->outcome) << " ticks "
              << naturalTicks << "\n";
  need(!natural.battle() && natural.progress().lastResult->outcome ==
                                g::CampaignOutcome::victory,
       "normal mission player strategy failed");
  need(naturalHits > 0, "normal mission omitted wizard melee");
  const auto resources =
      app::mapCampaignResources(root, true, true, true, true, true);
  s = seed(root);
  unsigned continuations = 0;
  for (unsigned tick = 0;
       s.battle() && s.progress().mission->stage == 0 && tick < 100; ++tick) {
    auto clone = app::newMapCampaign(resources);
    clone.restore(s.checkpoint());
    s.step();
    clone.step();
    need(bytes(s) == bytes(clone), "guard phase continuation differs");
    ++continuations;
  }
  need(s.battle() && s.progress().mission->stage == 1 &&
           !s.progress().mission->reinforcement,
       "reinforcement pending checkpoint missing");
  // Block arrival with a living friendly actor, then remove it without altering
  // the objective IDs.
  auto c = s.battle()->combat();
  auto meta = s.battle()->state();
  auto blocker =
      c.spawn(actor(s.missionPlan()->reinforcement[0], 0),
              c.driver()->creatureStats(), false, g::CombatControl::hold);
  meta.units.push_back({blocker, g::UnitRole::sentinel});
  auto checkpoint = s.checkpoint();
  checkpoint.world = c.world().state();
  checkpoint.world.campaign = g::encodeSkirmish(meta);
  s.restore(checkpoint);
  for (unsigned i = 0; i < 3; ++i) {
    s.step();
    need(!s.progress().mission->reinforcement &&
             s.progress().mission->wave.empty(),
         "blocked wave partially spawned");
  }
  auto b = *s.battle();
  b.removeCreature(blocker);
  checkpoint = s.checkpoint();
  checkpoint.world = b.world().state();
  s.restore(checkpoint);
  s.step();
  need(s.progress().mission->reinforcement &&
           s.progress().mission->wave.size() == 1,
       "wave retry failed");
  weaken(s, s.progress().mission->wave);
  unsigned wizardHits = 0;
  for (unsigned tick = 0; s.battle() && tick < 160; ++tick) {
    auto clone = app::newMapCampaign(resources);
    clone.restore(s.checkpoint());
    auto r = s.step();
    clone.step();
    need(bytes(s) == bytes(clone), "wave/wizard continuation differs");
    ++continuations;
    for (const auto &e : r.battle->combat.events)
      if (e.kind == g::CombatEventKind::hit && e.actor.slot >= 2 &&
          e.target.slot == 1)
        ++wizardHits;
  }
  need(!s.battle() &&
           s.progress().lastResult->outcome == g::CampaignOutcome::victory &&
           wizardHits,
       "wizard melee did not settle mission");
  need(s.spellAvailable(g::Spell::cure) &&
           s.progress().region == "arena-finale",
       "Cure reward/progression missing");
  const auto settled = bytes(s);
  for (unsigned i = 0; i < 3; ++i)
    need(!s.step().result, "reward/result replay");
  need(bytes(s) == settled, "idle result mutated");
  s.startMission();
  need(s.spellAvailable(g::Spell::cure) && !s.progress().mission,
       "finale loadout");
  auto finale = s.checkpoint();
  auto fc = s.battle()->combat();
  auto fm = s.battle()->state();
  auto friendlyEntity = actor({1, 3, 1}, 0);
  friendlyEntity.type = fc.navigation()->creatureType();
  auto friendly = fc.spawn(friendlyEntity, fc.driver()->creatureStats());
  fc.damage(friendly, 20, fm.wizards[0].actor);
  fm.units.push_back({friendly, g::UnitRole::sentinel});
  finale.world = fc.world().state();
  finale.world.campaign = g::encodeSkirmish(fm);
  s.restore(finale);
  s.enqueue({g::SkirmishOrderKind::cure, fm.wizards[0].actor, friendly, {}});
  bool healed = false, cureAdmitted = false;
  for (unsigned i = 0; i < 32 && s.battle(); ++i) {
    auto r = s.step();
    for (const auto &e : r.battle->events)
      cureAdmitted |=
          e.kind == g::SkirmishEventKind::casting && e.spell == g::Spell::cure;
    for (const auto &e : r.battle->combat.events)
      healed |= e.kind == g::CombatEventKind::healed;
  }
  need(cureAdmitted, "earned Cure cast was refused in finale");
  need(healed, "earned Cure did not heal in finale");
  std::cout << "{\"success\":true,\"continuations\":" << continuations
            << ",\"natural_ticks\":" << naturalTicks
            << ",\"wizard_melee_hits\":" << wizardHits
            << ",\"blocked_wave\":true,\"reward_cast\":true}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
