#include "../../assets/effect_animation_catalog.hpp"
#include "../../assets/wizard.hpp"
#include "../../reconstruction/rendering/effect_animation_selection.hpp"
#include "../command-session/native_geometry.hpp"
#include "wizards.hpp"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace mnm::encounter {
namespace {
void need(bool b, const char *why) {
  if (!b)
    throw std::invalid_argument(why);
}
void putWord(game::Bytes &b, unsigned v) {
  for (unsigned i = 0; i < 4; ++i)
    b.push_back(std::uint8_t(v >> (8 * i)));
}
unsigned word(const game::Bytes &b, std::size_t &at) {
  need(at <= b.size() && b.size() - at >= 4, "wizard input word");
  unsigned n = 0;
  for (unsigned i = 0; i < 4; ++i)
    n |= unsigned(b[at++]) << (8 * i);
  return n;
}
void blob(game::Bytes &out, const game::Bytes &v) {
  putWord(out, unsigned(v.size()));
  out.insert(out.end(), v.begin(), v.end());
}
game::Bytes blob(const game::Bytes &b, std::size_t &at) {
  const auto n = word(b, at);
  need(n <= 8 * 1024 * 1024 && at <= b.size() && n <= b.size() - at,
       "wizard input blob");
  game::Bytes v(b.begin() + at, b.begin() + at + n);
  at += n;
  return v;
}
game::Bytes read(const std::filesystem::path &requested) {
  auto path = requested;
  if (!std::filesystem::exists(path)) {
    const auto fold = [](std::string v) {
      for (auto &c : v)
        if (c >= 'A' && c <= 'Z')
          c = char(c - 'A' + 'a');
      return v;
    };
    unsigned matches = 0;
    for (const auto &entry :
         std::filesystem::directory_iterator(path.parent_path()))
      if (fold(entry.path().filename().string()) ==
          fold(requested.filename().string())) {
        path = entry.path();
        ++matches;
      }
    need(matches == 1, "wizard asset case match missing or ambiguous");
  }
  const auto n = std::filesystem::file_size(path);
  need(n <= 8 * 1024 * 1024, "wizard asset extent");
  std::ifstream f(path, std::ios::binary);
  need(bool(f), "wizard asset missing");
  game::Bytes b(n);
  f.read(reinterpret_cast<char *>(b.data()), std::streamsize(n));
  need(bool(f), "wizard asset read");
  return b;
}
assets::Config unpack(const game::Bytes &b) {
  auto packed = assets::decodePackedContainer(b);
  need(std::holds_alternative<assets::PackedContainer>(packed),
       "wizard packed configuration");
  auto decoded =
      assets::decodeConfig(std::get<assets::PackedContainer>(packed).decoded);
  need(std::holds_alternative<assets::Config>(decoded), "wizard configuration");
  return std::get<assets::Config>(std::move(decoded));
}
int number(const assets::Config &c, const std::string &section, const char *key,
           int fallback = 0) {
  const auto p = c.find(section, key);
  if (!p)
    return fallback;
  int n = 0;
  const auto result = std::from_chars(p->data(), p->data() + p->size(), n);
  need(result.ec == std::errc{} && result.ptr == p->data() + p->size(),
       "wizard numeric configuration");
  return n;
}
std::array<int, 28> matrix(const assets::Config &c, const char *section,
                           unsigned type = 0) {
  const auto p = c.find(section, "Creat_" + std::string(type < 10 ? "0" : "") +
                                     std::to_string(type));
  need(p, "wizard combat row");
  std::istringstream in(*p);
  std::array<int, 28> out{};
  std::string v;
  unsigned i = 0;
  while (std::getline(in, v, ',')) {
    need(i < 28, "wizard combat row extent");
    const auto first = v.find_first_not_of(" \t"),
               last = v.find_last_not_of(" \t");
    need(first != v.npos, "wizard combat row value");
    const auto text = v.substr(first, last - first + 1);
    const auto q =
        std::from_chars(text.data(), text.data() + text.size(), out[i++]);
    need(q.ec == std::errc{} && q.ptr == text.data() + text.size(),
         "wizard combat row integer");
  }
  need(i == 28, "wizard combat row width");
  return out;
}
} // namespace
game::Bytes loadWizardInputs(game::Bytes base,
                             const std::filesystem::path &root) {
  game::Bytes out{'M', 'N', 'M', 'W', 'I', 'Z', '0', '1'};
  blob(out, base);
  for (const char *path :
       {"Wizards/Greek41.wzd", "Creatures/Greek Male.ani",
        "CFG/Encrypted/spells.cfg", "CFG/Encrypted/effectani.cfg"})
    blob(out, read(root / path));
  const auto recipes = assets::readEffectAnimationRecipes(
      unpack(read(root / "CFG/Encrypted/effectani.cfg")));
  putWord(out, recipes.fileCount);
  for (unsigned i = 0; i < recipes.fileCount; ++i)
    blob(out, read(root / ("Sprites/EFFECTS" + std::to_string(i) + ".ani")));
  return out;
}
Profile decodeWizardInputs(game::Bytes bytes) {
  need(bytes.size() >= 8 &&
           std::equal(bytes.begin(), bytes.begin() + 8, "MNMWIZ01"),
       "wizard input version");
  std::size_t at = 8;
  auto base = blob(bytes, at);
  auto p = decodeInputs(base);
  need(p.scenario == 10 && p.initial.world.creatures.size() == 8,
       "wizard base encounter");
  auto wizardResult = assets::decodeWizard(blob(bytes, at));
  need(std::holds_alternative<assets::WizardDefinition>(wizardResult),
       "wizard definition decode");
  const auto wizard =
      std::get<assets::WizardDefinition>(std::move(wizardResult));
  auto animation = assets::decodeAnimation(blob(bytes, at));
  need(std::holds_alternative<assets::Animation>(animation),
       "wizard animation decode");
  p.native.animation.push_back(
      std::get<assets::Animation>(std::move(animation)));
  const auto spells = unpack(blob(bytes, at));
  const auto recipes =
      assets::readEffectAnimationRecipes(unpack(blob(bytes, at)));
  const auto count = word(bytes, at);
  need(count == recipes.fileCount && count <= 64,
       "wizard effect animation count");
  const unsigned firstEffect = unsigned(p.native.animation.size());
  for (unsigned i = 0; i < count; ++i) {
    auto a = assets::decodeAnimation(blob(bytes, at));
    need(std::holds_alternative<assets::Animation>(a),
         "wizard effect ANI decode");
    p.native.animation.push_back(std::get<assets::Animation>(std::move(a)));
  }
  need(at == bytes.size(), "wizard input trailing data");
  // Reuse immutable encrypted creature/HTH inputs from the inner profile.
  std::size_t inner = 8;
  word(base, inner);
  for (unsigned i = 0; i < 4; ++i)
    word(base, inner);
  const auto cfg = unpack(blob(base, inner)), hth = unpack(blob(base, inner));
  p.stun[0] = matrix(hth, "Stun_Matrix", 0);
  p.stun[17] = matrix(hth, "Stun_Matrix", 17);
  auto &type = p.native.types[0];
  type = p.native.types[17];
  type.bodyAsset = type.regularOverlayAsset = type.meleeOverlayAsset = 1;
  type.width = number(cfg, "CREATURE_0", "TileSizeXY");
  type.height = number(cfg, "CREATURE_0", "TileHeight");
  type.movementKind = number(cfg, "CREATURE_0", "SwimmingAbility");
  type.periodicResistance = unsigned(number(cfg, "CREATURE_0", "ImmuneSystem"));
  type.scalar.type_10 = number(cfg, "CREATURE_0", "Acceleration");
  type.scalar.type_d8 =
      reconstruction::groundMovementSamples(p.native.animation[1]);
  type.scalar.type_3c =
      reconstruction::groundMovementMaximum(type.scalar.type_d8);
  auto &c = p.initial;
  auto &state = c.native.wizards.emplace();
  state.spells.resize(104);
  state.admission.spells.resize(104);
  state.admission.profiles.resize(8);
  state.admission.resource.resize(8);
  state.admission.direction.resize(8);
  state.effects.resize(state.effectCapacity);
  // Explicit native producer fixture, using the pinned file defaults5/5.
  state.regenerationRate = 5;
  state.regenerationPeriod = 5;
  for (unsigned i = 0; i < 104; ++i) {
    const auto section = "SPELL_" + std::to_string(i);
    auto &v = state.spells[i];
    v.magnitude = number(spells, section, i == 41 ? "SpellStrength" : "Damage");
    v.duration = number(spells, section, "Duration");
    v.range = number(spells, section, "Range");
    v.speed = number(spells, section, "Speed");
    auto &admission = state.admission.spells[i];
    admission.cost = unsigned(number(spells, section, "ManaCost"));
    admission.resourceMode = 1;
    admission.resourceDelta =
        unsigned(number(spells, section, "BalanceAmount"));
  }
  for (unsigned type : {0u,17u}) {
    const auto *text = cfg.find("CREATURE_"+std::to_string(type), "LRCOffsets");
    need(text, "wizard cast muzzle definition");
    std::istringstream values(*text);
    std::string token;
    unsigned n=0;
    while (std::getline(values, token, ',')) {
      need(n<24, "wizard cast muzzle extent");
      std::istringstream value(token);
      int number=0;
      need(bool(value>>number) && (value>>std::ws).eof(), "wizard cast muzzle integer");
      need(number>=-128 && number<=128, "wizard cast muzzle bound");
      auto &p=state.admission.muzzle[type][n/3];
      if(n%3==0)p.x=number;else if(n%3==1)p.y=number;else p.z=number;
      ++n;
    }
    need(n==24, "wizard cast muzzle width");
  }
  // Selected Cure/direct summon inputs are stationary-delivery host bindings;
  // Magic Sphere's distinct admission arm constructs actual type3 motion.
  for (unsigned spell : {17u, 41u, 73u})
    need(state.admission.spells[spell].cost > 0, "wizard selected spell cost");
  const auto cure = recipes.entries.at(
      reconstruction::selectEffectAnimation(77, 41, {}, UINT32_MAX, {}));
  state.cureAsset = firstEffect + cure.assetIndex;
  state.cureSequence = cure.sequence;
  const auto projectile = recipes.entries.at(
      reconstruction::selectEffectAnimation(3, 73, {}, UINT32_MAX, {}));
  state.projectileAsset = firstEffect + projectile.assetIndex;
  state.projectileSequence = projectile.sequence;
  const auto maximumHealth = unsigned(
      wizard.stats.maxHealth.value_or(number(cfg, "CREATURE_0", "Health")));
  const auto maximumMana = unsigned(wizard.stats.maxMana.value_or(100));
  need(maximumHealth > 0 && maximumHealth <= 100000 && maximumMana > 0 &&
           maximumMana <= 0x7fffff,
       "wizard installed stats");
  for (auto &cell : c.native.cells)
    cell.occupant = 0xffff;
  for (unsigned i = 0; i < 8; ++i) {
    auto &a = c.world.creatures[i];
    a.ai.owner = i < 3 || i == 6 ? 0 : 1;
    if (i == 0 || i == 3) {
      a.ai.type = 0;
      a.ai.kind = 0;
      a.width = unsigned(type.width);
      a.height = unsigned(type.height);
      a.movementKind = unsigned(type.movementKind);
      a.ai.health = a.ai.maximumHealth = maximumHealth;
      a.ai.bravery = number(cfg, "CREATURE_0", "Bravery");
      a.ai.hth = matrix(hth, "HTH_Matrix");
      a.ai.enemyRadius = matrix(hth, "FearHate_Matrix");
      a.ai.allyRadius = matrix(hth, "Assist_Matrix");
      a.ai.retaliateThreshold = matrix(hth, "Evasion_Matrix");
      c.execution[i].roamRange = c.execution[i].roamInterval = 0;
      cs::NativeWizardController v;
      v.actor = i;
      v.maximumMana = maximumMana;
      for (unsigned spell : {17u, 41u, 73u})
        v.learned[spell] = true; // Authored mid-game knowledge fixture,
                                 // independent of WZD start-spells.
      state.controllers.push_back(v);
      state.admission.resource[i] = maximumMana * 256u;
    }
    a.aimXY = a.width == 1 ? 16 : 32;
    a.aimZ = int(a.height) * 8;
    if (i >= 6) {
      a.ai.active = false;
      a.ai.health = 0;
    }
  }
  cs::NativeCreatureGeometry geometry(c, p.native);
  for (auto &a : c.world.creatures)
    if (a.ai.active) {
      bool found = false;
      const auto origin = a.ai.position;
      for (int radius = 0; radius <= 12 && !found; ++radius)
        for (int dx = -radius; dx <= radius && !found; ++dx)
          for (int dy = -radius; dy <= radius && !found; ++dy) {
            cs::model::Cell q{origin.x + dx, origin.y + dy, origin.z};
            if (q.x < 0 || q.y < 0 || q.x >= c.world.dimensions.x ||
                q.y >= c.world.dimensions.y)
              continue;
            if (geometry.legal(a, q) && geometry.ground(a, q)) {
              a.ai.position = q;
              a.ai.home = a.destination = q;
              c.execution[a.ai.slot].roamHome = q;
              geometry.snap(a, q);
              geometry.reserve(a, q, true);
              found = true;
            }
          }
      need(found, "wizard encounter legal stand");
    }
  cs::prepareNativeContinuation(c, p.native);
  p.bytes = std::move(bytes);
  return p;
}
} // namespace mnm::encounter
