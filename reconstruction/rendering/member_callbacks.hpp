#pragma once
#include <array>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stdexcept>

namespace mnm::reconstruction::actor_services {
// Pinned NoCD member-callback classes. M owns checked PE32 storage and dispatch;
// it must reject freed callback storage before reading any callback fields.
// The member target is an explicit service, not an arbitrary host function cast.
template<class M> class MemberCallbacks {
    using U=std::uint32_t;
    M& m;
public:
    struct Class { U table, invoke, deleting, destruct; };
    inline static constexpr std::array<Class,13> classes{{
        {0x5c5c38,0x4643d0,0x4643f0,0x464410},
        {0x5c5e10,0x472e90,0x472eb0,0x472ed0},
        {0x5c67f0,0x4bc860,0x4bcae0,0x4bccb0},
        {0x5c6894,0x4bc880,0x4bcb60,0x4bccc0},
        {0x5c6938,0x4bc8a0,0x4bcb80,0x4bccd0},
        {0x5c6e44,0x4deb30,0x4deca0,0x4dee40},
        {0x5c6ef4,0x4deb80,0x4ded20,0x4dee80},
        {0x5c6f1c,0x4deba0,0x4ded40,0x4dee90},
        {0x5c6f60,0x4debe0,0x4deda0,0x4deec0},
        {0x5c7264,0x52cbc0,0x52cbe0,0x52cc00},
        {0x5c75a0,0x5524a0,0x5524c0,0x5524e0},
        {0x5c75b0,0x556a70,0x556a90,0x556ab0},
        {0x5c77b4,0x579ca0,0x579cc0,0x579ce0}
    }};
    explicit MemberCallbacks(M& memory):m(memory){}
    std::optional<U> tryRun(U entry,U self,std::initializer_list<U> args={}){
        for(const auto& c:classes){
            if(entry==c.invoke){
                if(args.size()!=1)throw std::invalid_argument("Member callback argument extent");
                U adjustment=m.get32(self+8),receiver=m.get32(self+12)+adjustment;
                U target=m.get32(self+4),argument=*args.begin();
                // Do not touch self after dispatch: the target can delete it.
                return m.invoke(target,receiver,{argument});
            }
            if(entry==c.destruct||entry==c.deleting){
                if(args.size()!=(entry==c.deleting?1u:0u))
                    throw std::invalid_argument("Member callback destructor argument extent");
                m.set32(self,0x5c5c40);
                if(entry==c.deleting&&(*args.begin()&1))m.invoke(0x597870,0,{self});
                return self;
            }
        }
        return std::nullopt;
    }
};
}
