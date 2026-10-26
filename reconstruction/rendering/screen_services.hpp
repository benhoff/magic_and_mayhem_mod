#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
// No-CD 40209ca7. Addresses below identify the recovered wire storage, never
// host pointers. The supplied memory accessor owns admission and byte access.
// Virtual callbacks are synchronous borrowed calls; these callers acquire no
// ownership credit and do not promise that callbacks preserve the registry.
// First four entries of candidate table 5c6564: 4b9b90 clears byte12;
// 4b9ba0/4b9bb0/4b9bc0 return. No allocation, free or ownership credit.
template<class Memory>
void minimalOriginalScreenSlot(Memory& m, std::uint32_t object, std::uint32_t offset) {
    if (offset==0) m.byte(object+0x12,0);
    else if (offset!=4 && offset!=8 && offset!=12)
        throw std::invalid_argument("Unrecovered minimal screen slot");
}

template<class Memory, class Services>
void selectOriginalScreen(Memory& m, Services& p, std::uint32_t target) {
    using U = std::uint32_t;
    if (std::int32_t(m.get(0x6f349c)) <= 0) { p.warning(0x5ec034); return; }
    m.set(0x6e1ef8, m.get(0x6e1efc));
    p.screen(m.get(0x6f34e0), 4);
    U i = m.get(0x6f349c) - 1;
    for (; std::int32_t(i) > 0; --i) {
        const U screen = m.get(0x6f34a0 + i * 4);
        if (screen == target) {
            m.set(0x6f349c, i); m.set(0x6f34e0, screen);
            if (screen) { m.set(0x642c54, 0); p.screen(m.get(0x6f34e0), 12); }
            break;
        }
        p.screen(screen, 4);
    }
    // Literal signed test: ordinary exhaustion reaches zero and does NOT push.
    // A callback that changes depth to zero reaches the negative-index branch.
    if (std::int32_t(i) >= 0) return;
    m.set(0x642c54, 0);
    if (std::int32_t(m.get(0x6f349c)) >= 15) { p.warning(0x5ebff4); return; }
    if (m.get(0x6f34e0)) p.screen(m.get(0x6f34e0), 8);
    m.set(0x6e1ef8, m.get(0x6e1efc));
    const U depth = m.get(0x6f349c) + 1;
    m.set(0x6f34e0, target); m.set(0x6f349c, depth);
    m.set(0x6f34a0 + depth * 4, target);
    while (p.peek(0x100, 0x108)) {}
    while (p.peek(0x201, 0x201)) {}
    m.set(0x642c54, 0); p.screen(m.get(0x6f34e0), 0);
}

template<class Memory, class Platform>
void selectOriginalCursor(Memory& m, Platform& p, std::uint32_t kind) {
    using U = std::uint32_t;
    if (m.get(0x5e04c8) == kind || std::int32_t(kind) < 0 || kind >= 23) return;
    if (m.get(0x642c64)) {
        const U mutex = m.get(0x642c58);
        if (!mutex || p.wait(mutex, 10) != 0) return;
        m.set(0x5e04c8, kind);
        if (kind != 22) {
            m.set(0x642c5c, 0);
            const auto frame = std::int32_t(m.get(0x642684 + kind * 68));
            const auto columns = std::int32_t(m.get(0x641a8c));
            // Original IDIV faults for zero or INT_MIN/-1. Accessor admission
            // must exclude those inputs; do not silently invent fallback data.
            m.set(0x641fc0, m.get(0x6426b8 + kind * 68));
            U x = U(frame % columns) * 40, y = U(frame / columns) * 40;
            m.set(0x641a90, x); m.set(0x641a98, x + 40);
            m.set(0x641a94, y); m.set(0x641a9c, y + 40);
        }
        p.release(m.get(0x642c58)); return;
    }
    m.set(0x5e04c8, kind);
    constexpr U resources[] = {0x6c,0x68,0xa0,0x6a,0x6d,0x6b,0x92,0x93,
        0x94,0x95,0x97,0x98,0x9a,0x99,0x9b,0x9c,0x9d,0x9e,0x9f,0xa1};
    p.cursor(p.load(m.get(0x6cbb68), kind < 20 ? resources[kind] : 0x9d));
}

// Ten DWORDs copied from the 32-entry ring before any callback: x/y, motion,
// button states, button changes, modifiers. Names identify local consumers.
using OriginalControlEvent = std::array<std::uint32_t, 10>;

// The World-only block shares the original selection rectangle and unit
// iteration helpers. Their implementations remain separately supplied services.
template<class Memory, class Services>
void dispatchOriginalWorldControl(Memory& m, Services& p, std::uint32_t index,
                                 const OriginalControlEvent& e, std::uint32_t rectangle) {
    using U=std::uint32_t;
    auto signedMin=[](U a,U b){return std::int32_t(a)<std::int32_t(b)?a:b;};
    auto signedMax=[](U a,U b){return std::int32_t(a)>std::int32_t(b)?a:b;};
    auto clamp=[&](U v,U lo,U hi){return signedMin(signedMax(v,lo),hi);};
    if (e[5] && e[3] && index==m.get(0x689934)) {
        const U x=clamp(e[0],m.get(rectangle),m.get(rectangle+8));
        const U y=clamp(e[1],m.get(rectangle+4),m.get(rectangle+12));
        m.set(0x6a4940,x);m.set(0x6a4944,y);
        m.set(0x690438,x);m.set(0x69043c,y);
        m.set(0x690440,x+1);m.set(0x690444,y+1);
    } else if (index!=m.get(0x689934)) {
        m.set(0x6a4940,clamp(e[0],m.get(rectangle),m.get(rectangle+8)));
        m.set(0x6a4944,clamp(e[1],m.get(rectangle+4),m.get(rectangle+12)));
    }
    U left,right,top,bottom;
    const U x=m.get(0x6a4940),y=m.get(0x6a4944);
    if(std::int32_t(e[0])>std::int32_t(x)) {
        left=x;right=signedMin(e[0]+1,m.get(rectangle+8));
    }else{left=signedMax(e[0],m.get(rectangle));right=x;}
    if(std::int32_t(e[1])>std::int32_t(y)) {
        top=y;bottom=signedMin(e[1]+1,m.get(rectangle+12));
    }else{top=signedMax(e[1],m.get(rectangle+4));bottom=y;}
    m.set(0x690438,left);m.set(0x690440,right);
    m.set(0x69043c,top);m.set(0x690444,bottom);
    if(index!=0)return;
    auto absolute=[](U v){return std::int32_t(v)<0?U(0)-v:v;};
    if(e[2] && e[3] && m.get(0x6903e8)!=3) {
        const U dx=absolute(signedMax(absolute(left-x),absolute(right-x)));
        const U dy=absolute(signedMax(absolute(top-y),absolute(bottom-y)));
        if(std::int32_t(dx)>32 || std::int32_t(dy)>32) {
            const U unit=p.activeUnit();
            if(unit){p.clearUnit(unit);m.set(0x6cbb70,0);}
            m.set(0x6903e8,3);
        }
    }
    if(m.get(0x6903e8)!=3 || !e[5] || e[3])return;
    const U player=m.get(0x644520);
    if(p.lookup(player)) p.commandSelection(p.lookup(m.get(0x644520)),~U(0));
    else m.set(0x6903e8,m.get(0x6e984c)!=0);
    const bool selected=p.lookup(m.get(0x644520))!=0;
    if((!selected&&!m.get(0x6e1f0c)) || e[8])return;
    auto reset=[&]{
        if(selected){m.set(0x6def68,0x506220);m.set(0x6def6c,0);}
        m.set(0x6def60,0);m.set(0x6def64,m.get(0x6def58));
    };
    reset();U unit=p.nextUnit(selected);
    while(unit && !m.get(unit+0x74)) unit=p.nextUnit(selected);
    if(!unit)return;
    if(!selected || !e[7]) {
        U at=m.get(0x6def58);
        for(U i=0;i<m.get(0x6df180);++i,at+=0xe4b)
            if(m.get(at+4) && (m.get(at+0x68)||m.get(at+0x70))) p.clearUnit(at);
        m.set(0x6df17c,0);
    }
    reset();U count=0;
    for(unit=p.nextUnit(selected);unit;unit=p.nextUnit(selected)) {
        if(!m.get(unit+0x74))continue;
        p.activateUnit(unit);++count;
        if(selected && m.get(0x6f3a90)) p.notifyUnit(m.get(unit+0xa8));
    }
    if(std::int32_t(count)>1)m.set(0x6df17c,1);
}

template<class Memory, class Services>
void dispatchOriginalControls(Memory& m, Services& p) {
    using U = std::uint32_t;
    auto inside = [&](U object, U x, U y) {
        return std::int32_t(x) >= std::int32_t(m.get(object+0x11)) &&
               std::int32_t(x) < std::int32_t(m.get(object+0x19)) &&
               std::int32_t(y) >= std::int32_t(m.get(object+0x15)) &&
               std::int32_t(y) < std::int32_t(m.get(object+0x1d));
    };
    auto objectAt = [&](U index) { return m.get(0x642034 + index*4); };
    auto cursor = [&](U kind) { selectOriginalCursor(m,p,kind); };
    auto world = [&] { const U s=m.get(0x6f34e0); return s && m.get(s+4)==2; };
    const U rectangle=m.get(0x6c482c);
    // Idle hover scans from the end. A hit invokes the real cursor selector;
    // rejection callbacks can change ring indexes before the next comparison.
    if (m.get(0x641a74) == m.get(0x641a88)) {
        if (m.get(0x5e04c8) != 22) {
            const U e=0x641ab8+m.get(0x641fbc)*40, x=m.get(e), y=m.get(e+4);
            U i=m.get(0x642c68);
            if (i) for (--i; std::int32_t(i)>=0; --i) {
                const U o=objectAt(i);
                if (inside(o,x,y) && m.get(o+8) && !p.hit(o,x,y)) {
                    cursor(m.get(o+12)); return;
                }
            }
        }
    }
    U index=0, object=0;
    bool captured=false; // Invocation-local latch; a release does not clear it.
    while (m.get(0x641a74) != m.get(0x641a88)) {
        OriginalControlEvent e;
        const U at=m.get(0x641a74);
        for (U i=0;i<10;++i) e[i]=m.get(0x641ab8+at*40+i*4);
        m.set(0x641a74, at+1==32 ? 0 : at+1);
        if (m.byte(0x642c6c) && m.get(0x642c70)<m.get(0x642c68)) {
            index=m.get(0x642c70); object=objectAt(index);
            if (m.get(object+8)) captured=true;
        }
        if (!captured) {
            m.byte(0x642c6c,0);
            if (world() && m.get(0x6903e8)==3) {
                index=0; object=objectAt(0);
            } else {
                index=m.get(0x642c68);
                if (!index) return;
                --index;
                for (; std::int32_t(index)>=0; --index) {
                    object=objectAt(index);
                    if (inside(object,e[0],e[1]) && m.get(object+8) && !p.hit(object,e[0],e[1])) break;
                }
            }
        }
        if (index>=m.get(0x642c68)) index=~U(0);
        else if (index!=~U(0) && m.get(0x5e04c8)!=22) cursor(m.get(object+12));
        m.set(0x642c70,index);
        const U previous=m.get(0x5e04cc);
        if (previous!=~U(0) && previous!=index) p.action(objectAt(previous),0x40,e);
        const bool isWorld=world();
        if (isWorld) dispatchOriginalWorldControl(m,p,index,e,rectangle);
        if (index==~U(0)) {
            // No hit: preserve the leave notification but skip the action.
        } else {
            U flags=index!=m.get(0x5e04cc) ? 0x20 : e[2] ? 0x10 : 0;
            if (e[5]) {
                if (e[3]) { flags|=1; if (m.byte(object+16)) m.byte(0x642c6c,1); }
                else { flags|=2; m.byte(0x642c6c,0); }
            }
            if (e[6]) {
                if (e[4]) { flags|=4; if (m.byte(object+16)) m.byte(0x642c6c,1); }
                else { flags|=8; m.byte(0x642c6c,0); }
            }
            if (flags && m.get(0x6903e8)!=3) p.action(object,flags,e);
        }
        m.set(0x5e04cc,index<m.get(0x642c68) ? index : ~U(0));
    }
}
} // namespace mnm::reconstruction
