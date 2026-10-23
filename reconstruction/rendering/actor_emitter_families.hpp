#pragma once
#include "actor_emission_provider.hpp"
#include "particle_classes.hpp"
namespace mnm::reconstruction::actor_services {
// PE32 addresses are values behind M. Never read a full160-byte model from a
// smaller original allocation. Kernels retain their independent isolated scope.
template <class M> class FamilyComposition {
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  ParticleWorld world() {
    return {signed32(r(0x5eb480)), signed32(r(0x5eb484)),
            signed32(r(0x5eb488))};
  }
  U size(U p) {
    switch (r(p)) {
    case 0x5c7420:
      return 69;
    case 0x5c7450:
    case 0x5c7470:
      return 84;
    case 0x5c74cc:
      return 65;
    case 0x5c7494:
      return 50;
    case 0x5c74b0:
      return 90;
    case 0x5c752c:
      return 130;
    default:
      return 100;
    }
  }
  bool cached(U p) { return r(p) == 0x5c74b0 || r(p) == 0x5c752c; }
  ParticleObject load(U p, U n) {
    ParticleObject o;
    for (U k = 0; k < n; ++k)
      o.bytes[k] = m.get8(p + k);
    return o;
  }
  void save(U p, const ParticleObject &o, U n) {
    for (U k = 0; k < n; ++k)
      m.set8(p + k, o.bytes[k]);
  }
  Kind8Random random() {
    Kind8Random x(0);
    x.first = r(0x6574c8);
    x.second = r(0x6574cc);
    for (U k = 0; k < 250; ++k)
      x.words[k] = r(0x6574d0 + k * 4);
    return x;
  }
  void publish(const Kind8Random &x) {
    w(0x6574c8, x.first);
    w(0x6574cc, x.second);
    for (U k = 0; k < 250; ++k)
      w(0x6574d0 + k * 4, x.words[k]);
  }
  ParticleConstructorConfig constructors() {
    ParticleConstructorConfig c;
    c.rgb555 = r(0x6e1f88) != 0;
    c.additivePhase = signed32(r(0x5e1464));
    c.additiveSpeed = signed32(r(0x5e1468));
    c.duration = signed32(r(0x5e1428));
    c.dimension = signed32(r(0x5e142c));
    c.planeEnabled = r(0x5e1430) != 0;
    c.effectCount = signed32(r(0x5e1424));
    for (U k = 0; k < 4; ++k)
      c.ownerEmitter[k] = signed32(r(0x5e149c + k * 4));
    return c;
  }
  ParticleEmissionConfig emission() {
    ParticleEmissionConfig c;
    c.rgb555 = r(0x6e1f88) != 0;
    c.additiveSpeed = signed32(r(0x5e1468));
    c.fallLimit = signed32(r(0x5e146c));
    c.levelLoss = signed32(r(0x5e1470));
    c.levelRestore = signed32(r(0x5e1474));
    c.duration = signed32(r(0x5e1428));
    c.colourThreshold = signed32(r(0x5e1424));
    c.trails = signed32(r(0x6e1f5c));
    return c;
  }
  ParticleOwnerFacts owner(U token) {
    ParticleOwnerFacts f;
    f.present = token != 0;
    if (token) {
      f.active = r(token + 0xe4) != 0;
      for (U k = 0; k < 3; ++k)
        f.position[k] = signed32(r(token + 0x14 + k * 4));
      U d = r(token + 0xac);
      f.horizontalOffset = signed32(r(d + 0x198));
      f.verticalOffset = signed32(r(d + 0x19c));
    }
    return f;
  }
  struct Terrain {
    ParticleTerrainFacts facts;
    U grids[2]{}, descriptors{};
    void decode(ParticleObject &o) {
      U p = U(o.get(0x3e));
      if (p) {
        bool found = false;
        for (U g = 0; g < 2; ++g)
          if (grids[g] && p >= grids[g] && p < grids[g] + 576 &&
              (p - grids[g]) % 12 == 0) {
            o.put(0x3e, 1 + g * 48 + (p - grids[g]) / 12);
            found = true;
            break;
          }
        if (!found)
          throw std::invalid_argument("Unretained terrain cache");
      }
      p = U(o.get(0x42));
      if (p) {
        if (p < descriptors || p >= descriptors + 1068 ||
            (p - descriptors) % 356)
          throw std::invalid_argument("Unretained descriptor");
        o.put(0x42, 0x2000 + (p - descriptors) / 356);
      }
    }
    ParticleObject encode(ParticleObject o) const {
      U p = U(o.get(0x3e));
      if (p) {
        if (p > 96 || !grids[(p - 1) / 48])
          throw std::invalid_argument("Unknown terrain generation");
        o.put(0x3e, grids[(p - 1) / 48] + ((p - 1) % 48) * 12);
      }
      p = U(o.get(0x42));
      if (p >= 0x2000 && p < 0x2003)
        o.put(0x42, descriptors + (p - 0x2000) * 356);
      return o;
    }
  };
  Terrain terrain(const ParticleObject *o = nullptr) {
    Terrain t;
    t.grids[0] = r(0x6c54dc);
    t.descriptors = r(0x65660c);
    if (o) {
      U p = U(o->get(0x3e));
      if (p && (p < t.grids[0] || p >= t.grids[0] + 576)) {
        U index = U(o->get(0x3a)) * 12 + U(o->get(0x36)) * 4 + U(o->get(0x32));
        if (index >= 48)
          throw std::invalid_argument("Terrain cache coordinates");
        t.grids[1] = p - index * 12;
      }
    }
    for (U g = 0; g < 2; ++g)
      if (t.grids[g])
        for (U k = 0; k < 576; ++k)
          reinterpret_cast<unsigned char *>(t.facts.cells[g].data())[k] =
              m.get8(t.grids[g] + k);
    for (U k = 0; k < 48; ++k)
      t.facts.shades[0][k] = static_cast<std::int8_t>(m.get8(r(0x6c5c5c) + k));
    for (U b = 0; b < 3; ++b)
      for (U k = 0; k < 356; ++k)
        t.facts.descriptors[b][k] = m.get8(t.descriptors + b * 356 + k);
    return t;
  }
  std::vector<std::uint16_t> plane(const ParticleObject &o) {
    std::vector<std::uint16_t> p;
    if (U at = U(o.get(0x2e))) {
      p.resize(U(o.get(0x20)) * U(o.get(0x24)) * 3);
      for (U k = 0; k < p.size(); ++k)
        p[k] = m.get16(at + k * 2);
    }
    return p;
  }
  void savePlane(const ParticleObject &o, const std::vector<std::uint16_t> &p) {
    if (!p.empty()) {
      U at = U(o.get(0x2e));
      for (U k = 0; k < p.size(); ++k)
        m.set16(at + k * 2, p[k]);
    }
  }
  // 548860 invokes the base shape colour method when shade changes. Its
  // raw-request equality check precedes quantization and plane regeneration.
  // Keep that order rather than using the kernel's quantized words alone.
  void refreshTerrainPlane(const ParticleObject &before,
                           ParticleObject &after) {
    if (before.get(0x46) == after.get(0x46))
      return;
    auto values = plane(before);
    std::array<std::int32_t, 3> requested{};
    for (U k = 0; k < 3; ++k)
      requested[k] =
          particleDivide(particleMul(after.get(0x4a + k * 4),
                                     particleAdd(after.get(0x46), 127)),
                         127);
    std::copy_n(before.bytes.begin() + 0x28, 6, after.bytes.begin() + 0x28);
    colourEmittedParticle(ParticleChildKind::colour, after, values, requested,
                          r(0x6e1f88) != 0);
    savePlane(after, values);
  }
  ParticleAnimationFacts animation(U category) {
    ParticleAnimationFacts f;
    f.primaryOrdinals = signed32(r(0x6f2174 + category * 4));
    f.fallbackB = r(0x6a5f88 + category * 0x5c9) == 2;
    f.fallbackPeriodA = signed32(r(0x6f2148));
    f.fallbackPeriodB = signed32(r(0x6f220c));
    U bases[] = {0x6f1d58 + category * 36, 0x6f2240, 0x6f2218};
    for (U b = 0; b < 3; ++b) {
      auto &a = b == 0 ? f.primary : b == 1 ? f.fallbackA : f.fallback;
      U header = r(bases[b]), offsets = r(bases[b] + 8),
        data = r(bases[b] + 12);
      U count = r(header + 12);
      (b == 0   ? f.primaryFrameCount
       : b == 1 ? f.fallbackFrameCountA
                : f.fallbackFrameCountB) = count;
      for (U k = 0; k < 64; ++k)
        a[k] = data + r(offsets + k * 4);
    }
    return f;
  }
  struct Backend {
    FamilyComposition &b;
    U token, n;
    ParticleObject &parent;
    Kind8Random &rng;
    Terrain *terrainState;
    struct Child {
      ParticleObject object;
      U n;
    };
    std::map<U, Child> children;
    void flush() {
      b.save(token, terrainState ? terrainState->encode(parent) : parent, n);
      for (const auto &c : children)
        b.save(c.first, c.second.object, c.second.n);
      b.publish(rng);
    }
    U allocate(U nbytes) {
      flush();
      return b.m.invoke(0x597850, 0, {nbytes});
    }
    void construct(U p, ParticleChildKind kind,
                   const std::array<std::int32_t, 3> &pos,
                   const std::array<std::int32_t, 3> &vel) {
      flush();
      U args[] = {U(pos[0]), U(pos[1]), U(pos[2]),
                  U(vel[0]), U(vel[1]), U(vel[2])};
      if (kind == ParticleChildKind::ownerAdditive) {
        b.m.invoke(0x545650, p,
                   {args[0], args[1], args[2], args[3], args[4], args[5]});
        auto o = b.load(p, 100);
        Kind8Random unused(0);
        constructEmittedParticle(kind, o, b.world(), pos, vel, b.constructors(),
                                 {}, unused);
        b.save(p, o, 100);
      } else
        b.m.invoke(kind == ParticleChildKind::randomAdditive ? 0x546660
                   : kind == ParticleChildKind::colour       ? 0x548560
                                                             : 0x548650,
                   p,
                   kind == ParticleChildKind::randomAdditive
                       ? std::initializer_list<U>{args[0], args[1], args[2]}
                       : std::initializer_list<U>{args[0], args[1], args[2],
                                                  args[3], args[4], args[5]});
      rng = b.random();
      children.emplace(p, Child{b.load(p, b.size(p)), b.size(p)});
    }
    ParticleObject &object(U p) { return children.at(p).object; }
    void colour(U p, const std::array<std::int32_t, 3> &rgb) {
      flush();
      b.m.invoke(b.r(b.r(p) + 24), p, {U(rgb[0]), U(rgb[1]), U(rgb[2])});
      children.at(p).object = b.load(p, children.at(p).n);
    }
    bool insert(U p) {
      flush();
      bool accepted = b.m.invoke(0x5457e0, 0x6cbb8a, {p}) != 0;
      if (accepted)
        children.erase(p);
      return accepted;
    }
    void destroy(U p) {
      flush();
      children.erase(p);
      b.m.invoke(b.r(b.r(p)), p, {1});
    }
  };

public:
  explicit FamilyComposition(M &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U self, std::initializer_list<U> args = {},
                          U second = 0) {
    auto a = args.begin();
    switch (entry) {
    case 0x546660:
    case 0x547210:
    case 0x5472c0: {
      U n = entry == 0x546660 ? 69 : 84;
      auto o = load(self, n);
      ParticleConstructorArguments in;
      for (U k = 0; k < 3; ++k)
        in.position[k] = signed32(a[k]);
      if (entry == 0x5472c0)
        for (U k = 0; k < 4; ++k)
          in.emitterParameters[k] = signed32(a[3 + k]);
      auto rng = random();
      constructParticleVariant(
          entry == 0x546660   ? ParticleVariant::randomAdditive
          : entry == 0x547210 ? ParticleVariant::defaultOwnerEmitter
                              : ParticleVariant::customOwnerEmitter,
          o, world(), in, constructors(), {}, {}, rng);
      save(self, o, n);
      publish(rng);
      return self;
    }
    case 0x548ac0: {
      auto o = load(self, 65);
      std::array<std::int32_t, 3> p{signed32(a[0]), signed32(a[1]),
                                    signed32(a[2])},
          v{signed32(a[3]), signed32(a[4]), signed32(a[5])};
      constructParticleMotion(o, world(), p, v, false);
      o.put(0, 0x5c74cc);
      o.put(0x10, 10);
      o.put(0x14, 20);
      o.put(0x18, -2);
      for (U k = 0; k < 3; ++k)
        o.put(0x1c + k * 4, v[k]);
      auto rng = random();
      o.put(0x28, (rng.next() & 0x7fffffffu) % 6 + 3);
      o.put(0x2c, 224);
      o.put(0x30, 32);
      o.put(0x34, 0);
      o.put(0x38, 64);
      o.put(0x3c, 0);
      o.bytes[0x40] = 1;
      save(self, o, 65);
      publish(rng);
      return self;
    }
    case 0x548560:
    case 0x548650: {
      bool cache = entry == 0x548650;
      U n = cache ? 90 : 50;
      auto o = load(self, n);
      auto t = terrain();
      auto rng = random();
      std::array<std::int32_t, 3> p{signed32(a[0]), signed32(a[1]),
                                    signed32(a[2])},
          v{signed32(a[3]), signed32(a[4]), signed32(a[5])};
      auto values = constructEmittedParticle(
          cache ? ParticleChildKind::terrainColour : ParticleChildKind::colour,
          o, world(), p, v, constructors(), t.facts, rng);
      save(self, cache ? t.encode(o) : o, n);
      if (!values.empty()) {
        U ptr = m.invoke(0x597850, 0, {U(values.size() * 2)});
        w(self + 0x2e, ptr);
        if (ptr) {
          o.put(0x2e, ptr);
          savePlane(o, values);
        }
      }
      return self;
    }
    case 0x549d70: {
      m.invoke(0x548650, self, {a[2], a[3], a[4], a[5], a[6], a[7]});
      w(self + 0x5a, 0);
      w(self + 0x5e, U(signed32(r(0x5e1424)) / 10));
      w(self + 0x72, a[8]);
      w(self + 0x76, a[0]);
      w(self + 0x7a, a[1]);
      auto rng = random();
      w(self + 0x7e, rng.next() & 7);
      publish(rng);
      w(self, 0x5c752c);
      m.invoke(0x54a5f0, a[0], {});
      return self;
    }
    case 0x54a5f0:
      if (!r(0x6f1d58 + self * 36))
        throw std::invalid_argument(
            "Resource preparation outside preloaded scope");
      return 0;
    case 0x5468a0:
    case 0x547380:
    case 0x548bc0:
    case 0x54a140: {
      U n = size(self);
      auto o = load(self, n);
      auto rng = random();
      auto c = emission();
      Terrain t;
      bool isCached = cached(self);
      if (isCached) {
        t = terrain(&o);
        t.decode(o);
      }
      Backend b{*this, self, n, o, rng, isCached ? &t : nullptr, {}};
      auto before = o;
      U result;
      if (entry == 0x5468a0)
        result = updateAdditiveEmitter(o, world(), c, rng, b);
      else if (entry == 0x547380) {
        U token = U(o.get(0x44));
        auto f = owner(token);
        U alive = 1;
        if (f.present && f.active) { // The virtual predicate reads the bound
                                     // owner; no external callback.
          alive = m.invoke(r(r(self) + 28), self, {});
        }
        U d = U(o.get(0x3c));
        c.childDimensions = {signed32(r(d)), signed32(r(d + 4))};
        result = updateOwnerEmitter(o, world(), f, alive, c, rng, b);
      } else if (entry == 0x548bc0)
        result =
            updateFlagEmitter(o, world(), owner(U(o.get(0x3c))), c, rng, b);
      else {
        ParticleTerrainValidity validity;
        result = updateFadingEmitter(o, world(), t.facts, validity,
                                     animation(U(o.get(0x76))), c, rng, b);
        if (!validity.descriptorDefined)
          throw std::invalid_argument("Undefined original descriptor scratch");
      }
      if (isCached)
        refreshTerrainPlane(before, o);
      b.flush();
      return result;
    }
    case 0x548860: {
      auto o = load(self, 90);
      auto t = terrain(&o);
      t.decode(o);
      auto before = o;
      ParticleTerrainValidity validity;
      U result =
          updateTerrainParticle(o, world(), t.facts, signed32(r(0x5e1428)),
                                r(0x6e1f88) != 0, validity);
      if (!validity.descriptorDefined)
        throw std::invalid_argument("Undefined original descriptor scratch");
      refreshTerrainPlane(before, o);
      save(self, t.encode(o), 90);
      return result;
    }
    case 0x54aa30: {
      U n = size(self);
      auto o = load(self, n);
      U result = updateGravityParticle(o, world());
      save(self, o, n);
      return result;
    }
    case 0x54ab40:
    case 0x54ab80: {
      auto o = load(self, size(self));
      auto p = plane(o);
      colourEmittedParticle(entry == 0x54ab80 ? ParticleChildKind::terrainColour
                                              : ParticleChildKind::colour,
                            o, p,
                            {signed32(a[0]), signed32(a[1]), signed32(a[2])},
                            r(0x6e1f88) != 0);
      save(self, o, size(self));
      savePlane(o, p);
      return 0;
    }
    case 0x546630:
    case 0x547280:
    case 0x547340:
    case 0x548b90:
    case 0x548600:
    case 0x5487f0:
    case 0x549e30: {
      U d = entry == 0x546630   ? 0x546650
            : entry == 0x547280 ? 0x5472a0
            : entry == 0x547340 ? 0x547360
            : entry == 0x548b90 ? 0x548bb0
            : entry == 0x548600 ? 0x548620
            : entry == 0x5487f0 ? 0x548810
                                : 0x549e50;
      m.invoke(d, self, {});
      if (*a & 1)
        m.invoke(0x597870, 0, {self});
      return self;
    }
    case 0x548620:
    case 0x548810:
    case 0x549e50:
      if (U ptr = r(self + 0x2e)) {
        m.invoke(0x597870, 0, {ptr});
        w(self + 0x2e, 0);
      }
      [[fallthrough]];
    case 0x546650:
    case 0x5472a0:
    case 0x547360:
    case 0x548bb0:
      w(self, 0x5c73a8);
      return 0;
    case 0x5472b0:
    case 0x547370: // Exact predicates recovered below; remain behind the owner
                   // boundary.
      return r(r(self + 0x44) + (entry == 0x5472b0 ? 0x798 : 0x110));
    case 0x54a9e0:
    case 0x54a9f0:
    case 0x54aa60:
    case 0x54aa70:
    case 0x54aac0:
    case 0x54aad0:
    case 0x54aae0:
    case 0x548840:
    case 0x54ac10:
    case 0x54ac80:
    case 0x54acf0:
    case 0x54ab20:
    case 0x54ab30:
    case 0x54ad10:
    case 0x54ad20: {
      auto o = load(self, size(self));
      std::array<std::int32_t, 2> dims{};
      if ((entry == 0x54ad10 || entry == 0x54ad20) && o.get(0x5a)) {
        U p = U(o.get(0x5a));
        dims = {signed32(r(p + 4)), signed32(r(p + 8))};
      }
      return U(originalParticleProperty(entry, o, signed32(r(0x5e1428)), dims));
    }
    case 0x54ac00:
      m.set8(self + 0x40, 1);
      return 0;
    default:
      return EmissionComposition<M>(m).tryRun(entry, self, args, second);
    }
  }
};
} // namespace mnm::reconstruction::actor_services
