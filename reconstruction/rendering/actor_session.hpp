#pragma once

#include "actor_service_composition.hpp"
#include "actor_source_services.hpp"
#include "actor_transition_services.hpp"
#include "actor_visual_lifecycle.hpp"
#include "member_callbacks.hpp"
#include <array>
#include <stdexcept>

namespace mnm::reconstruction::actor_services {

// Bridge the already recovered pool construction/reset/destruction to the same
// memory, allocator and dispatch used by actor activation and source ownership.
template<class M> class SessionLifecycleBackend {
    M& m;
public:
    explicit SessionLifecycleBackend(M& memory) : m(memory) {}
    U r(U at) { return m.get32(at); }
    void w(U at, U value) { m.set32(at, value); }
    unsigned char r8(U at) { return static_cast<unsigned char>(m.get8(at)); }
    void w8(U at, unsigned char value) { m.set8(at, value); }
    U allocate(U bytes) { return m.invoke(0x597850, 0, {bytes}); }
    void free(U address) { m.invoke(0x597870, 0, {address}); }
    void arrayFree(U address) { free(address); }
    U scratch() { return m.temporary(20); }
    [[noreturn]] void fault(const char* reason) { throw std::invalid_argument(reason); }
    [[noreturn]] void loadError() { throw std::runtime_error("Actor profile load failed"); }
    template<class... A> U call(U entry, U self, A... args) {
        return m.invoke(entry, self, {U(args)...});
    }
    U fastcall(U entry, U self, U second) { return m.invokeFast(entry, self, second); }
};

template<class M> class SessionComposition {
    M& m;
public:
    explicit SessionComposition(M& memory) : m(memory) {}
    std::optional<U> tryRun(U entry, U self, std::initializer_list<U> args = {}, U second = 0) {
        if(auto result=MemberCallbacks<M>(m).tryRun(entry,self,args))return result;
        switch (entry) {
        case 0x50aa30:
            return TransitionServices<M>(m).run(entry, self, args);
        case 0x407920: case 0x4079f0: case 0x416590: case 0x416fe0:
        case 0x514810: case 0x5183f0: case 0x51e790: case 0x42afd0:
        case 0x429900: case 0x407780: case 0x407820: case 0x407650:
        case 0x417e60: case 0x417580:
            return SourceServices<M>(m).run(entry, self, args);
        default: break;
        }
        SessionLifecycleBackend<M> backend(m);
        mnm::reconstruction::ActorLifecycle<SessionLifecycleBackend<M>> life(backend);
        U a[2]{}; unsigned n = 0;
        // Non-lifecycle service arities are checked by their own dispatcher.
        switch (entry) {
        case 0x4bfef0: case 0x5060a0:
            for (U value : args) { if (n == 2) throw std::invalid_argument("Pool argument extent"); a[n++] = value; }
            break;
        default: break;
        }
        switch (entry) {
        case 0x4bfef0: return life.constructPool(self, a[0]);
        case 0x4c00d0: life.destroyPool(self); return 0;
        case 0x473100: life.prepare(self); return 0;
        case 0x505db0: life.construct(self); return 0;
        case 0x505f70: life.destruct(self); return 0;
        case 0x5060a0: life.reset(self, a[0]); return 0;
        default: return Composition<M>(m).tryRun(entry, self, args, second);
        }
    }
};

// Native orchestration of the recovered entries. M retains sole ownership of
// actors, profiles, map, source managers, allocation records and platform ends.
// Host phase checks are explicit native policy, not recovered game branches.
template<class M> class ActorSession {
    M& m;
    bool constructed_ = false;
    static constexpr U manager = 0x6def58;
    static constexpr std::array<U, 2> sources{{0x690600, 0x644550}};
public:
    explicit ActorSession(M& memory) : m(memory) {}
    ActorSession(const ActorSession&) = delete;
    ActorSession& operator=(const ActorSession&) = delete;
    bool begin(U capacity) {
        if (constructed_) throw std::logic_error("Actor session already constructed");
        if (!capacity || capacity > 4096) throw std::invalid_argument("Actor session capacity");
        constructed_ = true;
        m.invoke(0x4bfef0, manager, {capacity});
        return m.get32(manager) != 0;
    }
    U admit(U type, U owner, U x, U y, U z, bool preloaded) {
        if (!constructed_) throw std::logic_error("Actor session is not constructed");
        U actor = actor_lifecycle::firstFree(m, manager);
        if (!actor) return 0;
        collectSources();
        if (m.get32(actor + 0x678) || m.get32(actor + 0x67c))
            throw std::logic_error("A source still owns the reusable actor slot");
        U slot = (actor - m.get32(manager)) / actor_lifecycle::Stride;
        // Reset alone overwrites the histogram pointer. Retire the previous
        // record's owned storage before reconstructing and resetting its slot.
        m.invoke(0x505f70, actor, {});
        m.invoke(0x505db0, actor, {});
        m.invoke(0x5060a0, actor, {slot});
        m.set32(actor + 8, x); m.set32(actor + 12, y); m.set32(actor + 16, z);
        m.invoke(0x506290, actor, {type, owner, preloaded ? 1u : 0u});
        return actor;
    }
    U registerSourceFamily(U actor, U family) {
        if (!constructed_ || family >= sources.size()) throw std::invalid_argument("Actor source family");
        U base = m.get32(manager), capacity = m.get32(manager + 4);
        if (!base || actor < base || (actor - base) % actor_lifecycle::Stride ||
            (actor - base) / actor_lifecycle::Stride >= capacity || !m.get32(actor + 4))
            throw std::invalid_argument("Source registration requires a live owned actor");
        if (m.get32(actor + 0x678 + family*4)) throw std::logic_error("Actor source family already registered");
        m.invoke(0x416590, sources[family], {actor});
        return m.get32(actor + 0x678 + family*4);
    }
    void retire(U actor, bool alternate = false) {
        if (!constructed_) throw std::logic_error("Actor session is not constructed");
        U base = m.get32(manager), capacity = m.get32(manager + 4);
        if (!base || actor < base || (actor - base) % actor_lifecycle::Stride ||
            (actor - base) / actor_lifecycle::Stride >= capacity || !m.get32(actor + 4))
            throw std::invalid_argument("Actor retirement requires a live owned slot");
        m.invoke(0x506d30, actor, {});
        m.invoke(0x51f5e0, actor, {alternate ? 1u : 0u});
    }
    void collectSources() {
        if (!constructed_) throw std::logic_error("Actor session is not constructed");
        for (U source : sources) m.invoke(0x417580, source, {});
    }
    void close() {
        if (!constructed_) return;
        U base = m.get32(manager), capacity = m.get32(manager + 4);
        for (U slot = 0; base && slot < capacity; ++slot) {
            U actor = base + slot * actor_lifecycle::Stride;
            if (m.get32(actor + 4)) retire(actor);
        }
        collectSources();
        SourceServices<M> sourceOwner(m);
        for (U source : sources) if (!sourceOwner.quiescent(source))
            throw std::logic_error("Protected sources must retire before the actor pool");
        for (U source : sources) sourceOwner.releaseEmptyBacking(source);
        m.invoke(0x4c00d0, manager, {});
        constructed_ = false;
    }
    bool constructed() const { return constructed_; }
};
} // namespace mnm::reconstruction::actor_services
