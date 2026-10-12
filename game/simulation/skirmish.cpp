#include "skirmish.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace mnm::game {
namespace {
constexpr std::array<std::uint8_t, 8> magic{{'M', 'N', 'M', 'A', 'R', 'M', '0', '1'}};
void require(bool v, const char *message) {
    if (!v)
        throw std::invalid_argument(message);
}
std::uint64_t rulesHash() {
    std::uint64_t hash = 14695981039346656037ULL;
    for (unsigned v :
         {1U, SkirmishRules::manaMaximum, SkirmishRules::manaInitial,
          SkirmishRules::regenerationTicks, SkirmishRules::summonCost, SkirmishRules::boltCost,
          SkirmishRules::castTicks, SkirmishRules::flightTicks, SkirmishRules::boltDamage,
          SkirmishRules::wizardHealth, SkirmishRules::creatureHealth, SkirmishRules::creatureDamage,
          SkirmishRules::aiInterval, SkirmishRules::livingCreatures, 8U, 8U, 3U})
        for (unsigned i = 0; i < 4; ++i) {
            hash ^= std::uint8_t(v >> (8 * i));
            hash *= 1099511628211ULL;
        }
    return hash;
}
const Fighter *fighter(const CombatState &c, Handle h) {
    for (const auto &f : c.fighters)
        if (f.actor == h)
            return &f;
    return nullptr;
}
const Entity *entity(const State &s, Handle h) {
    if (h.slot >= s.slots.size())
        return nullptr;
    const auto &slot = s.slots[h.slot];
    return slot.generation == h.generation && slot.entity ? &*slot.entity : nullptr;
}
Point point(const Entity &e) { return {e.x, e.y, e.z}; }
unsigned distance(Point a, Point b) {
    return unsigned(
        std::max(std::llabs(std::int64_t(a.x) - b.x), std::llabs(std::int64_t(a.y) - b.y)));
}
void clear(Wizard &w) {
    w.spell = Spell::none;
    w.remaining = 0;
    w.aim = {};
    w.target.reset();
}
void orderShape(const SkirmishOrder &o) {
    require(static_cast<unsigned>(o.kind) <= 4 &&
                ((o.kind == SkirmishOrderKind::bolt || o.kind == SkirmishOrderKind::attack) ==
                 bool(o.target)) &&
                ((o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::move) ==
                 bool(o.destination)),
            "invalid skirmish order encoding");
    if (o.destination)
        require(arenaCell(*o.destination), "Order outside the arena");
}
unsigned creatures(const CombatSession &combat, const SkirmishState &s, unsigned owner) {
    const auto c = combat.combat();
    unsigned n = 0;
    for (const auto &r : s.units)
        if (r.role == UnitRole::sentinel && fighter(c, r.actor)->health &&
            combat.world().find(r.actor)->owner == owner)
            ++n;
    return n;
}
bool placement(const CombatSession &combat, const Navigation &nav, Point p) {
    if (!arenaCell(p))
        return false;
    nav.validateSpawnPosition(p);
    for (const auto &slot : combat.world().state().slots)
        if (slot.entity && !slot.entity->cleaned && point(*slot.entity) == p)
            return false;
    return true;
}
Outcome outcome(const CombatState &c, const SkirmishState &s) {
    const bool player = fighter(c, s.wizards[0].actor)->health != 0,
               enemy = fighter(c, s.wizards[1].actor)->health != 0;
    return player ? (enemy ? Outcome::running : Outcome::victory)
                  : (enemy ? Outcome::defeat : Outcome::draw);
}
} // namespace
bool arenaCell(Point p) { return p.x >= 1 && p.x <= 6 && p.y >= 1 && p.y <= 6 && p.z == 1; }
Bytes encodeSkirmish(const SkirmishState &s) {
    require(s.units.size() >= 2 && s.units.size() <= 32 && s.projectiles.size() <= 32 &&
                s.pending.size() <= 64,
            "skirmish storage bounds exceeded");
    Bytes b(magic.begin(), magic.end());
    auto word = [&](std::uint32_t n) {
        for (unsigned i = 0; i < 4; ++i)
            b.push_back(std::uint8_t(n >> (8 * i)));
    };
    auto handle = [&](Handle h) {
        word(h.slot);
        word(h.generation);
    };
    auto pos = [&](Point p) {
        word(unsigned(p.x));
        word(unsigned(p.y));
        word(unsigned(p.z));
    };
    auto target = [&](std::optional<Handle> h) {
        word(h ? 1 : 0);
        handle(h.value_or(Handle{}));
    };
    const auto hash = rulesHash();
    word(std::uint32_t(hash));
    word(std::uint32_t(hash >> 32));
    word(static_cast<unsigned>(s.outcome));
    word(s.aiWait);
    word(s.nextProjectile);
    for (const auto &w : s.wizards) {
        handle(w.actor);
        word(w.mana);
        word(w.regeneration);
        word(static_cast<unsigned>(w.spell));
        word(w.remaining);
        pos(w.aim);
        target(w.target);
    }
    word(unsigned(s.units.size()));
    for (const auto &r : s.units) {
        handle(r.actor);
        word(static_cast<unsigned>(r.role));
    }
    word(unsigned(s.projectiles.size()));
    for (const auto &p : s.projectiles) {
        word(p.id);
        word(p.owner);
        handle(p.source);
        handle(p.target);
        pos(p.origin);
        pos(p.destination);
        word(p.remaining);
    }
    word(unsigned(s.pending.size()));
    for (const auto &o : s.pending) {
        orderShape(o);
        word(static_cast<unsigned>(o.kind));
        handle(o.subject);
        target(o.target);
        pos(o.destination.value_or(Point{}));
    }
    return b;
}
SkirmishState decodeSkirmish(const Bytes &b) {
    require(b.size() >= 28 && b.size() <= 8192 && std::equal(magic.begin(), magic.end(), b.begin()),
            "unknown or oversized skirmish subsystem");
    std::size_t p = 8;
    auto word = [&]() {
        require(p <= b.size() && b.size() - p >= 4, "truncated skirmish subsystem");
        std::uint32_t n = 0;
        for (unsigned i = 0; i < 4; ++i)
            n |= std::uint32_t(b[p++]) << (i * 8);
        return n;
    };
    auto handle = [&]() { return Handle{word(), word()}; };
    auto pos = [&]() {
        auto signedWord = [&]() {
            const auto n = word();
            return std::int32_t(n <= 0x7fffffffU ? std::int64_t(n)
                                                 : std::int64_t(n) - 0x100000000LL);
        };
        return Point{signedWord(), signedWord(), signedWord()};
    };
    auto target = [&]() -> std::optional<Handle> {
        const auto flag = word();
        const auto h = handle();
        require(flag <= 1 && (flag || h == Handle{}), "invalid skirmish target flag");
        if (flag)
            return h;
        return {};
    };
    const auto low = word(), high = word();
    require((std::uint64_t(high) << 32 | low) == rulesHash(),
            "skirmish rules fingerprint mismatch");
    SkirmishState s;
    s.outcome = static_cast<Outcome>(word());
    s.aiWait = word();
    s.nextProjectile = word();
    for (auto &w : s.wizards) {
        w.actor = handle();
        w.mana = word();
        w.regeneration = word();
        w.spell = static_cast<Spell>(word());
        w.remaining = word();
        w.aim = pos();
        w.target = target();
    }
    auto n = word();
    require(n >= 2 && n <= 32, "invalid skirmish unit count");
    for (unsigned i = 0; i < n; ++i)
        s.units.push_back({handle(), static_cast<UnitRole>(word())});
    n = word();
    require(n <= 32, "invalid projectile count");
    for (unsigned i = 0; i < n; ++i) {
        Projectile v;
        v.id = word();
        v.owner = word();
        v.source = handle();
        v.target = handle();
        v.origin = pos();
        v.destination = pos();
        v.remaining = word();
        s.projectiles.push_back(v);
    }
    n = word();
    require(n <= 64, "invalid skirmish order count");
    for (unsigned i = 0; i < n; ++i) {
        SkirmishOrder o;
        o.kind = static_cast<SkirmishOrderKind>(word());
        o.subject = handle();
        o.target = target();
        const auto cell = pos();
        if (o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::move)
            o.destination = cell;
        else
            require(cell == Point{}, "unexpected skirmish order coordinates");
        orderShape(o);
        s.pending.push_back(o);
    }
    require(p == b.size(), "skirmish trailing bytes");
    return s;
}
void validateSkirmish(const State &world, const CombatState &combat, const SkirmishState &s) {
    (void)encodeSkirmish(s);
    require(world.map == "native-skirmish-arena-v1" && world.navigation &&
                world.navigation->dimensions == Point{8, 8, 3},
            "skirmish arena binding mismatch");
    require(s.units.size() == combat.fighters.size() && static_cast<unsigned>(s.outcome) <= 3 &&
                s.aiWait <= SkirmishRules::aiInterval && s.nextProjectile >= 1,
            "invalid skirmish state/counts");
    std::optional<unsigned> previous;
    for (const auto &r : s.units) {
        const auto *f = fighter(combat, r.actor);
        const auto *e = entity(world, r.actor);
        require(f && e && e->owner <= 1 && arenaCell(point(*e)) &&
                    static_cast<unsigned>(r.role) <= 1 && (!previous || r.actor.slot > *previous),
                "invalid skirmish unit identity/role/order");
        previous = r.actor.slot;
        const auto wizardOwner = e->owner;
        if (r.role == UnitRole::wizard)
            require(r.actor == s.wizards[wizardOwner].actor && f->control == CombatControl::hold &&
                        !active(e->motion->action) &&
                        f->stats.maximumHealth == SkirmishRules::wizardHealth &&
                        f->stats.damage == 1 && f->stats.windupTicks == 2 &&
                        f->stats.recoveryTicks == 3,
                    "invalid stationary wizard");
        else
            require(f->stats.maximumHealth == SkirmishRules::creatureHealth &&
                        f->stats.damage == SkirmishRules::creatureDamage &&
                        f->stats.windupTicks == 2 && f->stats.recoveryTicks == 3,
                    "invalid sentinel profile");
    }
    require(!(s.wizards[0].actor == s.wizards[1].actor), "duplicate wizard identity");
    for (unsigned owner = 0; owner < 2; ++owner) {
        const auto &w = s.wizards[owner];
        const auto *e = entity(world, w.actor);
        const auto *f = fighter(combat, w.actor);
        require(e && f &&
                    std::any_of(s.units.begin(), s.units.end(),
                                [&](const UnitRoleRecord &r) {
                                    return r.actor == w.actor && r.role == UnitRole::wizard;
                                }) &&
                    e->owner == owner && point(*e) == (owner ? Point{6, 1, 1} : Point{1, 6, 1}),
                "wizard role/position mismatch");
        require(w.mana <= SkirmishRules::manaMaximum &&
                    w.regeneration < SkirmishRules::regenerationTicks &&
                    static_cast<unsigned>(w.spell) <= 2,
                "invalid wizard mana/spell state");
        if (w.spell == Spell::none)
            require(!w.remaining && !w.target && w.aim == Point{},
                    "idle wizard retains cast state");
        else {
            require(f->health && w.remaining >= 1 && w.remaining <= SkirmishRules::castTicks &&
                        arenaCell(w.aim),
                    "invalid cast continuation");
            require((w.spell == Spell::bolt) == bool(w.target), "cast target/phase mismatch");
            if (w.target) {
                const auto *t = entity(world, *w.target);
                require(t && fighter(combat, *w.target) && t->owner != owner,
                        "cast target identity/hostility mismatch");
            }
        }
    }
    unsigned last = 0;
    for (const auto &p : s.projectiles) {
        const auto *t = entity(world, p.target);
        require(p.id > last && p.id < s.nextProjectile && p.owner <= 1 &&
                    p.source == s.wizards[p.owner].actor && t && t->owner != p.owner &&
                    fighter(combat, p.target) && arenaCell(p.origin) && arenaCell(p.destination) &&
                    p.origin == point(*entity(world, p.source)) && p.remaining >= 1 &&
                    p.remaining <= SkirmishRules::flightTicks,
                "invalid projectile continuation");
        last = p.id;
    }
    for (const auto &o : s.pending) {
        orderShape(o);
        const auto *e = entity(world, o.subject);
        const auto role =
            std::find_if(s.units.begin(), s.units.end(),
                         [&](const UnitRoleRecord &r) { return r.actor == o.subject; });
        const bool casting =
            o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::bolt;
        require(e && e->owner == 0 && role != s.units.end() &&
                    casting == (role->role == UnitRole::wizard),
                "pending order identity/ownership/role mismatch");
        if (o.target) {
            const auto *t = entity(world, *o.target);
            require(t && t->owner == 1 && fighter(combat, *o.target),
                    "pending target identity/hostility mismatch");
        }
    }
    require(world.pending.empty() && combat.pending.empty(),
            "skirmish checkpoint retains lower-layer orders");
    require(s.outcome == outcome(combat, s), "skirmish outcome/health mismatch");
    if (s.outcome != Outcome::running) {
        require(s.projectiles.empty() && s.pending.empty(),
                "terminal skirmish retains effects/orders");
        for (const auto &w : s.wizards)
            require(w.spell == Spell::none, "terminal skirmish retains casting");
        for (const auto &f : combat.fighters)
            require(f.control == CombatControl::hold &&
                        !active(entity(world, f.actor)->motion->action),
                    "terminal skirmish retains simulation work");
    }
}
SkirmishSession::SkirmishSession(std::shared_ptr<const Navigation> nav,
                                 std::shared_ptr<const HealthRules> rules)
    : combat_(
          [&] {
              require(bool(nav), "arena navigation required");
              World w(32);
              auto state = w.state();
              state.map = "native-skirmish-arena-v1";
              state.navigation = nav->binding();
              w.restore(state);
              return w;
          }(),
          nav, std::move(rules)),
      navigation_(std::move(nav)) {
    SkirmishState s;
    for (unsigned owner = 0; owner < 2; ++owner) {
        const auto p = owner ? Point{6, 1, 1} : Point{1, 6, 1};
        navigation_->validateSpawnPosition(p);
        Entity e;
        e.type = navigation_->creatureType();
        e.owner = owner;
        e.x = p.x;
        e.y = p.y;
        e.z = p.z;
        auto h =
            combat_.spawn(e, {SkirmishRules::wizardHealth, 1, 2, 3}, false, CombatControl::hold);
        s.wizards[owner].actor = h;
        s.units.push_back({h, UnitRole::wizard});
    }
    publish(combat_, std::move(s));
}
SkirmishSession::SkirmishSession(World world, std::shared_ptr<const Navigation> nav,
                                 std::shared_ptr<const HealthRules> rules)
    : combat_(std::move(world), nav, std::move(rules)), navigation_(std::move(nav)) {
    validateSkirmish(this->world().state(), combat_.combat(), state());
}
void SkirmishSession::publish(CombatSession combat, SkirmishState s) {
    auto world = combat.world().state();
    world.campaign = encodeSkirmish(s);
    validateSkirmish(world, combat.combat(), s);
    World w(0, combat.world().limits());
    w.restore(std::move(world));
    CombatSession complete(std::move(w), navigation_, combat.healthRules());
    combat_ = std::move(complete);
}
void SkirmishSession::enqueue(SkirmishOrder o) {
    orderShape(o);
    auto s = state();
    require(s.outcome == Outcome::running, "The skirmish has ended");
    const auto c = combat_.combat();
    const auto *f = fighter(c, o.subject);
    const auto *e = world().find(o.subject);
    require(f && f->health && e && e->owner == 0, "Select a living friendly unit");
    const auto role = std::find_if(s.units.begin(), s.units.end(), [&](const UnitRoleRecord &r) {
                          return r.actor == o.subject;
                      })->role;
    const bool casting = o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::bolt;
    require(casting == (role == UnitRole::wizard),
            casting ? "Select your wizard to cast" : "Select a Sentinel for creature orders");
    if (o.target) {
        const auto *t = world().find(*o.target);
        const auto *tf = fighter(c, *o.target);
        require(t && tf && tf->health && t->owner == 1, "Select a living enemy target");
    }
    require(s.pending.size() < 64, "Order queue is full");
    s.pending.push_back(o);
    publish(combat_, std::move(s));
}
SkirmishReport SkirmishSession::step(TickInput input) {
    SkirmishReport report;
    auto s = state();
    if (!input.admitted || s.outcome != Outcome::running)
        return report;
    auto combat = combat_;
    if (input.alternateMode) {
        report.combat = combat.step(input);
        publish(std::move(combat), std::move(s));
        return report;
    }
    const auto oldProjectiles = s.projectiles.size();
    std::array<bool, 2> started{};
    const auto emit = [&](SkirmishEventKind k, Handle a, std::optional<Handle> t = {},
                          Spell spell = Spell::none,
                          SkirmishRefusal refusal = SkirmishRefusal::none) {
        report.events.push_back({k, a, t, spell, refusal});
    };
    for (auto &w : s.wizards)
        if (fighter(combat.combat(), w.actor)->health &&
            ++w.regeneration == SkirmishRules::regenerationTicks) {
            w.regeneration = 0;
            w.mana = std::min(w.mana + 1, SkirmishRules::manaMaximum);
        }
    auto cast = [&](unsigned owner, Spell spell, Point aim, std::optional<Handle> target) {
        auto &w = s.wizards[owner];
        SkirmishRefusal refusal = SkirmishRefusal::none;
        if (!fighter(combat.combat(), w.actor)->health)
            refusal = SkirmishRefusal::identity;
        else if (w.spell != Spell::none)
            refusal = SkirmishRefusal::busy;
        const auto cost =
            spell == Spell::summon ? SkirmishRules::summonCost : SkirmishRules::boltCost;
        if (refusal == SkirmishRefusal::none && w.mana < cost)
            refusal = SkirmishRefusal::mana;
        if (refusal == SkirmishRefusal::none && spell == Spell::summon) {
            if (creatures(combat, s, owner) >= SkirmishRules::livingCreatures ||
                s.units.size() >= 32)
                refusal = SkirmishRefusal::capacity;
            else if (distance(point(*combat.world().find(w.actor)), aim) > 2 ||
                     !placement(combat, *navigation_, aim))
                refusal = SkirmishRefusal::placement;
        }
        if (refusal == SkirmishRefusal::none && spell == Spell::bolt) {
            const auto *e = target ? combat.world().find(*target) : nullptr;
            if (!e || !fighter(combat.combat(), *target)->health || e->owner == owner)
                refusal = SkirmishRefusal::target;
        }
        if (refusal != SkirmishRefusal::none) {
            emit(SkirmishEventKind::refused, w.actor, target, spell, refusal);
            return;
        }
        w.mana -= cost;
        w.spell = spell;
        w.remaining = SkirmishRules::castTicks;
        w.aim = aim;
        w.target = target;
        started[owner] = true;
        emit(SkirmishEventKind::casting, w.actor, target, spell);
    };
    for (const auto &o : s.pending) {
        const auto *e = combat.world().find(o.subject);
        const auto c = combat.combat();
        const auto *f = fighter(c, o.subject);
        if (!e || !f || !f->health || e->owner != 0) {
            emit(SkirmishEventKind::refused, o.subject, o.target, Spell::none,
                 SkirmishRefusal::identity);
            continue;
        }
        if (o.kind == SkirmishOrderKind::summon)
            cast(0, Spell::summon, *o.destination, {});
        else if (o.kind == SkirmishOrderKind::bolt) {
            const auto *target = combat.world().find(*o.target);
            if (target)
                cast(0, Spell::bolt, point(*target), o.target);
            else
                emit(SkirmishEventKind::refused, o.subject, o.target, Spell::bolt,
                     SkirmishRefusal::target);
        } else
            combat.enqueue({o.kind == SkirmishOrderKind::move   ? CombatOrderKind::move
                            : o.kind == SkirmishOrderKind::stop ? CombatOrderKind::stop
                                                                : CombatOrderKind::attack,
                            o.subject, o.target, o.destination});
    }
    s.pending.clear();
    if (s.aiWait)
        --s.aiWait;
    else {
        s.aiWait = SkirmishRules::aiInterval;
        auto &w = s.wizards[1];
        if (fighter(combat.combat(), w.actor)->health && w.spell == Spell::none) {
            if (creatures(combat, s, 1) < 1 && w.mana >= SkirmishRules::summonCost) {
                bool found = false;
                for (Point p : std::array<Point, 4>{{{5, 2, 1}, {5, 1, 1}, {6, 2, 1}, {4, 2, 1}}})
                    if (placement(combat, *navigation_, p)) {
                        cast(1, Spell::summon, p, {});
                        found = true;
                        break;
                    }
                if (!found && w.mana >= SkirmishRules::boltCost)
                    cast(1, Spell::bolt, point(*combat.world().find(s.wizards[0].actor)),
                         s.wizards[0].actor);
            } else if (w.mana >= SkirmishRules::boltCost)
                cast(1, Spell::bolt, point(*combat.world().find(s.wizards[0].actor)),
                     s.wizards[0].actor);
        }
    }
    report.combat = combat.step(input);
    for (unsigned owner = 0; owner < 2; ++owner) {
        auto &w = s.wizards[owner];
        if (!fighter(combat.combat(), w.actor)->health) {
            clear(w);
            continue;
        }
        if (w.spell == Spell::none || started[owner] || --w.remaining)
            continue;
        if (w.spell == Spell::summon) {
            if (creatures(combat, s, owner) < SkirmishRules::livingCreatures &&
                s.units.size() < 32 && placement(combat, *navigation_, w.aim)) {
                Entity e;
                e.type = navigation_->creatureType();
                e.owner = owner;
                e.x = w.aim.x;
                e.y = w.aim.y;
                e.z = w.aim.z;
                const auto h = combat.spawn(
                    e, {SkirmishRules::creatureHealth, SkirmishRules::creatureDamage, 2, 3});
                s.units.push_back({h, UnitRole::sentinel});
                emit(SkirmishEventKind::summoned, w.actor, h, w.spell);
            } else {
                w.mana = std::min(w.mana + SkirmishRules::summonCost, SkirmishRules::manaMaximum);
                emit(SkirmishEventKind::refused, w.actor, {}, w.spell, SkirmishRefusal::placement);
            }
        } else {
            if (fighter(combat.combat(), *w.target)->health && s.projectiles.size() < 32 &&
                s.nextProjectile < std::numeric_limits<unsigned>::max()) {
                s.projectiles.push_back({s.nextProjectile++, owner, w.actor, *w.target,
                                         point(*combat.world().find(w.actor)), w.aim,
                                         SkirmishRules::flightTicks});
                emit(SkirmishEventKind::launched, w.actor, w.target, w.spell);
            } else
                emit(SkirmishEventKind::refused, w.actor, w.target, w.spell,
                     SkirmishRefusal::target);
        }
        clear(w);
    }
    std::vector<Projectile> retained;
    for (std::size_t i = 0; i < s.projectiles.size(); ++i) {
        auto p = s.projectiles[i];
        if (i >= oldProjectiles || --p.remaining) {
            retained.push_back(p);
            continue;
        }
        const auto *e = combat.world().find(p.target);
        if (fighter(combat.combat(), p.target)->health && e && point(*e) == p.destination) {
            auto hit = combat.damage(p.target, SkirmishRules::boltDamage, p.source);
            report.combat.events.insert(report.combat.events.end(), hit.events.begin(),
                                        hit.events.end());
            emit(SkirmishEventKind::impact, p.source, p.target, Spell::bolt);
        } else
            emit(SkirmishEventKind::missed, p.source, p.target, Spell::bolt);
    }
    s.projectiles = std::move(retained);
    s.outcome = outcome(combat.combat(), s);
    if (s.outcome != Outcome::running) {
        for (auto &w : s.wizards)
            clear(w);
        s.projectiles.clear();
        s.pending.clear();
        combat.holdAll();
        emit(SkirmishEventKind::finished, s.wizards[0].actor);
    }
    publish(std::move(combat), std::move(s));
    return report;
}
void SkirmishSession::restore(const std::filesystem::path &path,
                              const NavigationResolver &resolver) {
    require(bool(resolver), "skirmish resource resolver required");
    auto state = readSnapshot(path, world().limits());
    auto nav = resolver(state);
    require(bool(nav), "arena resources unavailable");
    World w(0, world().limits());
    w.restore(std::move(state));
    SkirmishSession candidate(std::move(w), std::move(nav), combat_.healthRules());
    *this = std::move(candidate);
}
} // namespace mnm::game
