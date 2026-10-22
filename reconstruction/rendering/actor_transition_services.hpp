#pragma once

#include "actor_services.hpp"

namespace mnm::reconstruction::actor_services {

inline U behaviorCallback(U kind) {
    switch (kind) {
    case 1: return 0x50acb0;
    case 2: return 0x510010;
    case 3: return 0x50afa0;
    case 4: return 0x50ccd0;
    case 5: return 0x50d580;
    case 6: return 0x50d840;
    case 7: return 0x50d860;
    case 8: return 0x50d850;
    case 9: return 0x50d870;
    case 10: return 0x50d8f0;
    case 11: return 0x51c510;
    case 13: return 0x51c230;
    case 14: return 0x521750;
    case 15: return 0x521770;
    case 16: return 0x521780;
    default: return 0;
    }
}

// Recovered transition control only. The selected concrete task handler is an
// explicit downstream provider; naming its address does not recover that body.
template<class M> class TransitionServices {
    M& m;
public:
    explicit TransitionServices(M& memory) : m(memory) {}
    U run(U entry, U actor, std::initializer_list<U> args = {}) {
        if (entry != 0x50aa30 || args.size() != 1)
            throw std::invalid_argument("Behavior transition arguments");
        U kind = *args.begin();
        if (!m.get32(actor + 0xe4)) {
            if (kind == 5) { if (m.get32(actor + 0x5ec) == 5) return 0; }
            else if (kind != 16) return 0;
        }
        if (m.get32(actor + 0x618)) { m.set32(actor + 0x5f0, kind); return 0; }
        if (m.get32(actor + 0x5ec) == 2) {
            if (U effect = m.get32(actor + 0x6e8)) {
                m.invoke(0x4943f0, effect, {}); m.set32(actor + 0x6e8, 0);
            }
            m.invoke(0x512340, actor, {});
        }
        m.set32(actor + 0x5ec, kind);
        if (kind != 8 && kind != 14) m.set32(actor + 0x5f0, 0);
        U callback = behaviorCallback(kind);
        m.set32(actor + 0x5f4, callback); m.set32(actor + 0x5f8, 0);
        m.invoke(callback, actor, {});
        return 0;
    }
};
} // namespace mnm::reconstruction::actor_services
