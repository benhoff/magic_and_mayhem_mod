#pragma once
#include "member_callbacks.hpp"
namespace mnm::reconstruction::actor_services {
// The compact12-byte representation has a direct receiver at+8, with no
// adjustment word. This exact family is identified by base-reset wrappers;
// it is not a generic interpretation of arbitrary callback storage.
template<class M> class CompactMemberCallbacks {
    using U=std::uint32_t;
    M& m;
public:
    using Class=typename MemberCallbacks<M>::Class;
    inline static constexpr std::array<Class,33> classes{{
        {0x5c5eec,0x475b80,0x475ba0,0x475be0},
        {0x5c5f58,0x475b90,0x475bc0,0x475bf0},
        {0x5c6150,0x486780,0x486790,0x4867b0},
        {0x5c6414,0x4bc770,0x4bc940,0x4bcbe0},
        {0x5c644c,0x4bc780,0x4bc960,0x4bcbf0},
        {0x5c6484,0x4bc790,0x4bc980,0x4bcc00},
        {0x5c64bc,0x4bc7c0,0x4bc9a0,0x4bcc10},
        {0x5c64f4,0x4bc7d0,0x4bc9c0,0x4bcc20},
        {0x5c652c,0x4bc7e0,0x4bc9e0,0x4bcc30},
        {0x5c6594,0x4bc7f0,0x4bca00,0x4bcc40},
        {0x5c65cc,0x4bc800,0x4bca20,0x4bcc50},
        {0x5c6604,0x4bc810,0x4bca40,0x4bcc60},
        {0x5c663c,0x4bc820,0x4bca60,0x4bcc70},
        {0x5c6674,0x4bc830,0x4bca80,0x4bcc80},
        {0x5c66ac,0x4bc840,0x4bcaa0,0x4bcc90},
        {0x5c66e4,0x4bc850,0x4bcac0,0x4bcca0},
        {0x5c6970,0x4bc920,0x4bcba0,0x4bcce0},
        {0x5c69a8,0x4bc930,0x4bcbc0,0x4bccf0},
        {0x5c6b8c,0x4ca2a0,0x4ca2b0,0x4ca2d0},
        {0x5c6d08,0x4deaf0,0x4dec20,0x4dee00},
        {0x5c6d80,0x4deb00,0x4dec40,0x4dee10},
        {0x5c6de8,0x4deb10,0x4dec60,0x4dee20},
        {0x5c6e20,0x4deb20,0x4dec80,0x4dee30},
        {0x5c6e80,0x4deb50,0x4decc0,0x4dee50},
        {0x5c6e98,0x4deb60,0x4dece0,0x4dee60},
        {0x5c6ed0,0x4deb70,0x4ded00,0x4dee70},
        {0x5c6f34,0x4debc0,0x4ded60,0x4deea0},
        {0x5c6f3c,0x4debd0,0x4ded80,0x4deeb0},
        {0x5c6f78,0x4dec00,0x4dedc0,0x4deed0},
        {0x5c6f90,0x4dec10,0x4dede0,0x4deee0},
        {0x5c7318,0x53a170,0x53a180,0x53a1a0},
        {0x5c76d8,0x56d350,0x56d360,0x56d380},
        {0x5c7718,0x574a10,0x574a20,0x574a40}
    }};
    explicit CompactMemberCallbacks(M& memory):m(memory){}
    std::optional<U> tryRun(U entry,U self,std::initializer_list<U> args={}){
        for(const auto& c:classes){
            if(entry==c.invoke){
                if(args.size()!=1)throw std::invalid_argument("Compact callback argument extent");
                U receiver=m.get32(self+8),target=m.get32(self+4),argument=*args.begin();
                return m.invoke(target,receiver,{argument});
            }
            if(entry==c.destruct||entry==c.deleting){
                if(args.size()!=(entry==c.deleting?1u:0u))
                    throw std::invalid_argument("Compact callback destructor argument extent");
                m.set32(self,0x5c5c40);
                if(entry==c.deleting&&(*args.begin()&1))m.invoke(0x597870,0,{self});
                return self;
            }
        }
        return std::nullopt;
    }
};
}
