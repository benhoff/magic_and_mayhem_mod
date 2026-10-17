#include "skirmish.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace mnm::game {
namespace {
constexpr std::array<std::uint8_t, 8> missionMagic{{'M','N','M','A','R','M','0','3'}};
constexpr std::array<std::uint8_t, 8> customMagic{{'M', 'N', 'M', 'A', 'R', 'M', '0', '2'}};
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
    require(static_cast<unsigned>(o.kind) <= 6 &&
                ((o.kind == SkirmishOrderKind::bolt || o.kind == SkirmishOrderKind::attack || o.kind == SkirmishOrderKind::cure || o.kind == SkirmishOrderKind::sphere) ==
                 bool(o.target)) &&
                ((o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::move) ==
                 bool(o.destination)),
            "invalid skirmish order encoding");

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
bool placement(const CombatSession &combat, const Navigation &nav, const SkirmishMission &mission, Point p) {
    if (!missionCell(mission, p))
        return false;
    nav.validateSpawnPosition(p);
    for (const auto &slot : combat.world().state().slots)
        if (slot.entity && slot.entity->family == Family::creature && !slot.entity->cleaned && point(*slot.entity) == p)
            return false;
    return true;
}
Outcome outcome(const CombatState &c, const SkirmishState &s) {
    const bool player = fighter(c, s.wizards[0].actor)->health != 0,
               enemy = fighter(c, s.wizards[1].actor)->health != 0;
    return player ? (enemy || (s.missionControlled && !s.victoryEnabled) ? Outcome::running : Outcome::victory)
                  : (enemy ? Outcome::defeat : Outcome::draw);
}
} // namespace
bool arenaCell(Point p) { return p.x >= 1 && p.x <= 6 && p.y >= 1 && p.y <= 6 && p.z == 1; }
namespace {
bool pointLess(Point a, Point b) {
    return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
}
const SkirmishMission &effective(const SkirmishState &s) {
    return s.mission ? *s.mission : arenaMission();
}
void missionOrder(const SkirmishMission &m, const SkirmishOrder &o) {
    orderShape(o);
    require(!o.destination || missionCell(m, *o.destination), "Order outside mission standing cells");
}
void navigationMission(const SkirmishMission &m, const Navigation &nav) {
    validateMission(m);
    require(nav.binding().dimensions == m.dimensions, "mission navigation dimensions mismatch");
    for (const auto p : m.standing) nav.validateSpawnPosition(p);
}
}
const SkirmishMission &arenaMission() {
    static const auto mission = [] {
        SkirmishMission m{"native-skirmish-arena-v1", {8,8,3}, {{{1,6,1},{6,1,1}}},
                          {{5,2,1},{5,1,1},{6,2,1},{4,2,1}}, {}};
        for (int x=1; x<=6; ++x) for (int y=1; y<=6; ++y) m.standing.push_back({x,y,1});
        return m;
    }();
    return mission;
}
bool missionCell(const SkirmishMission &m, Point p) {
    return std::binary_search(m.standing.begin(), m.standing.end(), p, pointLess);
}
void validateMission(const SkirmishMission &m) {
    require(!m.map.empty() && m.map.size() <= 128 &&
                std::all_of(m.map.begin(),m.map.end(),[](unsigned char c){return c>=33 && c<=126;}),
            "invalid mission map identity");
    require(m.dimensions.x>=4 && m.dimensions.x<=16 && m.dimensions.y>=4 && m.dimensions.y<=16 &&
                m.dimensions.z>=2 && m.dimensions.z<=32 && m.standing.size()>=2 &&
                m.standing.size()<=4096 && !m.aiSummons.empty() && m.aiSummons.size()<=32,
            "mission bounds exceeded");
    std::optional<Point> previous;
    for (auto p:m.standing) {
        require(p.x>0 && p.x<m.dimensions.x-1 && p.y>0 && p.y<m.dimensions.y-1 &&
                    p.z>0 && p.z<m.dimensions.z && (!previous || pointLess(*previous,p)),
                "invalid/duplicate/unordered mission standing cell");
        previous=p;
    }
    require(!(m.wizards[0]==m.wizards[1]) && missionCell(m,m.wizards[0]) &&
                missionCell(m,m.wizards[1]), "invalid mission wizard spawns");
    std::vector<Point> seen;
    for (auto p:m.aiSummons) {
        require(missionCell(m,p) && distance(p,m.wizards[1])<=2 &&
                    !(p==m.wizards[0]) && !(p==m.wizards[1]) &&
                    std::find(seen.begin(),seen.end(),p)==seen.end(), "invalid mission AI summon cell");
        seen.push_back(p);
    }
}
bool sameMission(const SkirmishMission &a, const SkirmishMission &b) {
    return a.map==b.map && a.dimensions==b.dimensions && a.wizards==b.wizards &&
           a.aiSummons==b.aiSummons && a.standing==b.standing;
}

Bytes encodeSkirmish(const SkirmishState &s) {
    require(s.units.size() >= 2 && s.units.size() <= 32 && s.projectiles.size() <= 32 &&
                s.pending.size() <= 64,
            "skirmish storage bounds exceeded");
    const auto &mission=effective(s);
    validateMission(mission);
    const bool custom=!sameMission(mission,arenaMission());
    const auto &header=s.missionControlled ? missionMagic : custom ? customMagic : magic;
    Bytes b(header.begin(), header.end());
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
    if(s.missionControlled) {word(custom);word(s.victoryEnabled);}
    else require(s.victoryEnabled,"uncontrolled mission victory gate");
    if (custom) {
        word(unsigned(mission.map.size()));
        b.insert(b.end(),mission.map.begin(),mission.map.end());
        pos(mission.dimensions);
        for (auto p:mission.wizards) pos(p);
        word(unsigned(mission.aiSummons.size()));for (auto p:mission.aiSummons) pos(p);
        word(unsigned(mission.standing.size()));for (auto p:mission.standing) pos(p);
    }
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
        missionOrder(mission,o);
        word(static_cast<unsigned>(o.kind));
        handle(o.subject);
        target(o.target);
        pos(o.destination.value_or(Point{}));
    }
    return b;
}
SkirmishState decodeSkirmish(const Bytes &b) {
    const bool controlled=b.size()>=8 && std::equal(missionMagic.begin(),missionMagic.end(),b.begin());
    bool custom=b.size()>=8 && std::equal(customMagic.begin(),customMagic.end(),b.begin());
    require(b.size() >= 28 && b.size() <= (custom || controlled ? 65536U : 8192U) &&
                (controlled || custom || std::equal(magic.begin(), magic.end(), b.begin())),
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
    if(controlled) {s.missionControlled=true;const auto c=word(),v=word();require(c<=1 && v<=1,"invalid mission controls");custom=c;s.victoryEnabled=v;}
    if (custom) {
        SkirmishMission m;
        const auto length=word();
        require(length>=1 && length<=128 && length<=b.size()-p,"invalid mission identity size");
        m.map.assign(b.begin()+p,b.begin()+p+length);p+=length;
        m.dimensions=pos();for (auto &v:m.wizards) v=pos();
        auto count=word();require(count>=1 && count<=32,"invalid mission AI count");
        for (unsigned i=0;i<count;++i) m.aiSummons.push_back(pos());
        count=word();require(count>=2 && count<=4096,"invalid mission standing count");
        for (unsigned i=0;i<count;++i) m.standing.push_back(pos());
        validateMission(m);require(!sameMission(m,arenaMission()),"noncanonical mission encoding");
        s.mission=std::move(m);
    }
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
        missionOrder(effective(s),o);
        s.pending.push_back(o);
    }
    require(p == b.size(), "skirmish trailing bytes");
    return s;
}
void validateSkirmish(const State &world, const CombatState &combat, const SkirmishState &s) {
    (void)encodeSkirmish(s);
    const auto &mission=effective(s);
    require(world.map == mission.map && world.navigation &&
                world.navigation->dimensions == mission.dimensions,
            "skirmish arena binding mismatch");
    require(s.units.size() == combat.fighters.size() && static_cast<unsigned>(s.outcome) <= 3 &&
                s.aiWait <= SkirmishRules::aiInterval && s.nextProjectile >= 1,
            "invalid skirmish state/counts");
    std::optional<unsigned> previous;
    for (const auto &r : s.units) {
        const auto *f = fighter(combat, r.actor);
        const auto *e = entity(world, r.actor);
        require(f && e && e->owner <= 1 && missionCell(mission,point(*e)) &&
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
        else if (!decodeBattleSystems(world.systems).rules.identity)
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
                    e->owner == owner && point(*e) == mission.wizards[owner],
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
                        missionCell(mission,w.aim),
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
                    fighter(combat, p.target) && missionCell(mission,p.origin) && missionCell(mission,p.destination) &&
                    p.origin == point(*entity(world, p.source)) && p.remaining >= 1 &&
                    p.remaining <= SkirmishRules::flightTicks,
                "invalid projectile continuation");
        last = p.id;
    }
    for (const auto &o : s.pending) {
        missionOrder(mission,o);
        const auto *e = entity(world, o.subject);
        const auto role =
            std::find_if(s.units.begin(), s.units.end(),
                         [&](const UnitRoleRecord &r) { return r.actor == o.subject; });
        const bool casting =
            o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::bolt || o.kind == SkirmishOrderKind::cure || o.kind == SkirmishOrderKind::sphere;
        require(e && e->owner == 0 && role != s.units.end() &&
                    casting == (role->role == UnitRole::wizard),
                "pending order identity/ownership/role mismatch");
        if (o.target) {
            const auto *t = entity(world, *o.target);
            require(t && t->owner == (o.kind == SkirmishOrderKind::cure ? 0U : 1U) && fighter(combat, *o.target),
                    "pending target identity/hostility mismatch");
            if (o.kind==SkirmishOrderKind::sphere)
                require(decodeBattleSystems(world.systems).effects.identity, "pending Sphere without effect runtime");
            if (o.kind==SkirmishOrderKind::cure)
                require(decodeBattleSystems(world.systems).effects.identity && std::any_of(s.units.begin(),s.units.end(),[&](const auto &r){return r.actor==*o.target && r.role==UnitRole::sentinel;}), "pending Cure target/runtime mismatch");
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
                                 std::shared_ptr<const HealthRules> rules,
                                 std::shared_ptr<const SkirmishEffects> effects,
                                 std::shared_ptr<const CombatDriver> driver)
    : SkirmishSession(arenaMission(),std::move(nav),std::move(rules),std::move(effects),std::move(driver)) {}
SkirmishSession::SkirmishSession(SkirmishMission mission, std::shared_ptr<const Navigation> nav,
                                 std::shared_ptr<const HealthRules> rules,
                                 std::shared_ptr<const SkirmishEffects> effects,
                                 std::shared_ptr<const CombatDriver> driver)
    : mission_(std::move(mission)), combat_(
          [&] {
              require(bool(nav), "mission navigation required");
              navigationMission(mission_,*nav);
              World w(32);
              auto state = w.state();
              state.map = mission_.map;
              state.navigation = nav->binding();
              w.restore(state);
              return w;
          }(), nav, std::move(rules), std::move(driver)), navigation_(std::move(nav)), effects_(std::move(effects)) {
    if (effects_) combat_.bindEffects(effects_->identity(), effects_->initial());
    SkirmishState s;
    if (!sameMission(mission_,arenaMission())) s.mission=mission_;
    for (unsigned owner = 0; owner < 2; ++owner) {
        const auto p = mission_.wizards[owner];
        Entity e;
        e.type = navigation_->wizardType();
        e.owner = owner;
        e.x = p.x;e.y = p.y;e.z = p.z;
        auto h = combat_.spawn(e, {SkirmishRules::wizardHealth, 1, 2, 3}, false, CombatControl::hold);
        s.wizards[owner].actor = h;s.units.push_back({h, UnitRole::wizard});
    }
    publish(combat_, std::move(s));
}
SkirmishSession::SkirmishSession(World world, std::shared_ptr<const Navigation> nav,
                                 std::shared_ptr<const HealthRules> rules,
                                 std::shared_ptr<const SkirmishEffects> effects,
                                 std::shared_ptr<const CombatDriver> driver)
    : mission_(effective(decodeSkirmish(world.state().campaign))),
      combat_(std::move(world), nav, std::move(rules), std::move(driver)), navigation_(std::move(nav)), effects_(std::move(effects)) {
    navigationMission(mission_,*navigation_);
    require(combat_.effects().identity == (effects_ ? effects_->identity() : 0), "battle effect runtime binding mismatch");
    if (effects_) effects_->validate(combat_, state());
    validateSkirmish(this->world().state(), combat_.combat(), state());
}
void SkirmishSession::publish(CombatSession combat, SkirmishState s) {
    auto world = combat.world().state();
    world.campaign = encodeSkirmish(s);
    validateSkirmish(world, combat.combat(), s);
    require(combat.effects().identity == (effects_ ? effects_->identity() : 0), "battle effect runtime binding mismatch");
    if (effects_) effects_->validate(combat, s);
    World w(0, combat.world().limits());
    w.restore(std::move(world));
    CombatSession complete(std::move(w), navigation_, combat.healthRules(), combat.driver());
    combat_ = std::move(complete);
}
void SkirmishSession::enqueue(SkirmishOrder o) {
    missionOrder(mission_,o);
    auto s = state();
    require(s.outcome == Outcome::running, "The skirmish has ended");
    const auto c = combat_.combat();
    const auto *f = fighter(c, o.subject);
    const auto *e = world().find(o.subject);
    require(f && f->health && e && e->owner == 0, "Select a living friendly unit");
    const auto role = std::find_if(s.units.begin(), s.units.end(), [&](const UnitRoleRecord &r) {
                          return r.actor == o.subject;
                      })->role;
    const bool casting = o.kind == SkirmishOrderKind::summon || o.kind == SkirmishOrderKind::bolt || o.kind == SkirmishOrderKind::cure || o.kind == SkirmishOrderKind::sphere;
    require(casting == (role == UnitRole::wizard),
            casting ? "Select your wizard to cast" : "Select a Sentinel for creature orders");
    if (o.target) {
        const auto *t = world().find(*o.target);
        const auto *tf = fighter(c, *o.target);
        require(t && tf && tf->health && t->owner == (o.kind == SkirmishOrderKind::cure ? 0U : 1U), "Select a living eligible target");
        if (o.kind == SkirmishOrderKind::sphere)
            require(effects_ && effects_->supports(Spell::sphere), "Magic Sphere runtime unavailable");
        if (o.kind == SkirmishOrderKind::cure)
            require(effects_ && std::any_of(s.units.begin(),s.units.end(),[&](const auto &r) {return r.actor==*o.target && r.role==UnitRole::sentinel;}), "Cure requires an enabled runtime and a friendly creature");
    }
    require(s.pending.size() < 64, "Order queue is full");
    s.pending.push_back(o);
    publish(combat_, std::move(s));
}
void SkirmishSession::removeCreature(Handle h) {
    auto s=state();auto role=std::find_if(s.units.begin(),s.units.end(),[&](const auto &r){return r.actor==h;});
    require(role!=s.units.end() && role->role==UnitRole::sentinel,"Only enrolled creatures may be removed");
    auto combat=combat_;s.units.erase(role);
    s.pending.erase(std::remove_if(s.pending.begin(),s.pending.end(),[&](const auto &o){return o.subject==h || o.target==std::optional<Handle>(h);}),s.pending.end());
    for(auto &w:s.wizards)if(w.target==std::optional<Handle>(h))clear(w);
    s.projectiles.erase(std::remove_if(s.projectiles.begin(),s.projectiles.end(),[&](const auto &p){return p.target==h;}),s.projectiles.end());
    if(effects_)effects_->removed(combat,s,h);
    // Stage enrollment before releasing the actor so profile validation never
    // observes an old role pointing at a newly empty/reused slot.
    auto staged=combat.world().state();staged.campaign=encodeSkirmish(s);
    World world(0,combat.world().limits());world.restore(std::move(staged));
    CombatSession removing(std::move(world),navigation_,combat.healthRules(),combat.driver());
    removing.remove(h);
    publish(std::move(removing),std::move(s));
}
void SkirmishSession::beginMission() {
    auto s=state();require(!world().state().tick && !s.missionControlled,"mission already started");
    s.missionControlled=true;s.victoryEnabled=false;publish(combat_,std::move(s));
}
std::optional<std::vector<Handle>> SkirmishSession::spawnMissionGroup(const std::vector<Point>& locations) {
    auto s=state();require(s.missionControlled && s.outcome==Outcome::running,"mission group outside running mission");
    require(!locations.empty() && locations.size()<=8,"mission group bound");
    auto combat=combat_;std::vector<Handle> handles;
    for(auto p:locations) {
        if(s.units.size()>=32 || !missionCell(mission_,p) ||
           !placement(combat,*navigation_,mission_,p) || (combat.driver() && !combat.driver()->canSpawn(combat,p))) return {};
        Entity e;e.type=navigation_->creatureType();e.owner=1;e.x=p.x;e.y=p.y;e.z=p.z;
        auto h=combat.spawn(e,combat.driver() ? combat.driver()->creatureStats() : CombatStats{SkirmishRules::creatureHealth,SkirmishRules::creatureDamage,2,3});
        handles.push_back(h);s.units.push_back({h,UnitRole::sentinel});
    }
    std::sort(s.units.begin(),s.units.end(),[](const auto&a,const auto&b){return a.actor.slot<b.actor.slot;});
    publish(std::move(combat),std::move(s));return handles;
}
SkirmishReport SkirmishSession::step(TickInput input, std::optional<bool> victory) {
    SkirmishReport report;
    auto s = state();
    if (!input.admitted || s.outcome != Outcome::running)
        return report;
    if(victory) {require(s.missionControlled,"victory gate requires mission control");s.victoryEnabled=*victory;}
    auto combat = combat_;
    if (input.alternateMode) {
        report.combat = combat.stepBattle(input);
        if (effects_) {
            auto extra=effects_->step(combat,s,input);
            report.combat.events.insert(report.combat.events.end(),extra.events.begin(),extra.events.end());
        }
        s.outcome = outcome(combat.combat(), s);
        if (s.outcome != Outcome::running) {
            for (auto &wizard : s.wizards) clear(wizard);
            s.projectiles.clear(); s.pending.clear();
            combat.holdAll();
            if (effects_) effects_->cancel(combat, s);
            report.events.push_back({SkirmishEventKind::finished, s.wizards[0].actor, {}, Spell::none, SkirmishRefusal::none});
        }
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
                     (!placement(combat, *navigation_, mission_, aim) || (combat.driver() && !combat.driver()->canSpawn(combat, aim))))
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
    // Runtimes may publish a replacement metadata value; retain the admitted
    // batch independently so its current order/iterator cannot be invalidated.
    auto admittedOrders = std::move(s.pending);
    s.pending.clear();
    for (const auto &o : admittedOrders) {
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
        } else if (o.kind == SkirmishOrderKind::cure || o.kind == SkirmishOrderKind::sphere) {
            const auto selectedSpell=o.kind == SkirmishOrderKind::cure ? Spell::cure : Spell::sphere;
            auto refusal=SkirmishRefusal::busy;
            if (s.wizards[0].spell==Spell::none)
                refusal=effects_ ? effects_->castSpell(combat,s,selectedSpell,o.subject,*o.target) : SkirmishRefusal::target;
            emit(refusal==SkirmishRefusal::none ? SkirmishEventKind::casting : SkirmishEventKind::refused,
                 o.subject,o.target,selectedSpell,refusal);
        } else
            combat.enqueue({o.kind == SkirmishOrderKind::move   ? CombatOrderKind::move
                            : o.kind == SkirmishOrderKind::stop ? CombatOrderKind::stop
                                                                : CombatOrderKind::attack,
                            o.subject, o.target, o.destination});
    }
    s.pending.clear();
    if(s.missionControlled) {}
    else if (s.aiWait)
        --s.aiWait;
    else {
        s.aiWait = SkirmishRules::aiInterval;
        auto &w = s.wizards[1];
        if (fighter(combat.combat(), w.actor)->health && w.spell == Spell::none) {
            if (creatures(combat, s, 1) < 1 && w.mana >= SkirmishRules::summonCost) {
                bool found = false;
                for (Point p : mission_.aiSummons)
                    if (placement(combat, *navigation_, mission_, p) && (!combat.driver() || combat.driver()->canSpawn(combat, p))) {
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
    report.combat = combat.stepBattle(input);
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
                s.units.size() < 32 && placement(combat, *navigation_, mission_, w.aim) &&
                (!combat.driver() || combat.driver()->canSpawn(combat, w.aim))) {
                Entity e;
                e.type = navigation_->creatureType();
                e.owner = owner;
                e.x = w.aim.x;
                e.y = w.aim.y;
                e.z = w.aim.z;
                const auto h = combat.spawn(
                    e, combat.driver() ? combat.driver()->creatureStats() :
                       CombatStats{SkirmishRules::creatureHealth, SkirmishRules::creatureDamage, 2, 3});
                s.units.push_back({h, UnitRole::sentinel});
                std::sort(s.units.begin(),s.units.end(),[](const auto &a,const auto &b){return a.actor.slot<b.actor.slot;});
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
    if (effects_) {
        auto extra=effects_->step(combat,s,input);
        report.combat.events.insert(report.combat.events.end(),extra.events.begin(),extra.events.end());
    }
    s.outcome = outcome(combat.combat(), s);
    if (s.outcome != Outcome::running) {
        for (auto &w : s.wizards)
            clear(w);
        s.projectiles.clear();
        s.pending.clear();
        combat.holdAll();
        if (effects_) effects_->cancel(combat,s);
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
    SkirmishSession candidate(std::move(w), std::move(nav), combat_.healthRules(), effects_, combat_.driver());
    require(sameMission(candidate.mission(),mission_),"skirmish mission resources mismatch");
    *this = std::move(candidate);
}
} // namespace mnm::game
