#pragma once
#include <optional>
#include "actor_services.hpp"
#include "actor_service_controllers.hpp"
#include "actor_service_world.hpp"
#include "actor_service_ui.hpp"
#include "actor_service_dependencies.hpp"
#include "actor_service_popups.hpp"
#include "actor_service_collections.hpp"
#include "actor_service_effects.hpp"
#include "actor_service_construction.hpp"
#include "actor_service_providers.hpp"

namespace mnm::reconstruction::actor_services {
// Dispatch only recovered engine bodies. Unknown imports and interfaces are
// returned to the owning backend; an address association is never a supplier.
template<class M> class Composition {
    M& m;
public:
    explicit Composition(M& memory):m(memory){}
    std::optional<U> tryRun(U entry,U p,std::initializer_list<U> args={},U second=0){
        switch(entry){
        case 0x506290:{U a[3]{};unsigned i=0;for(U v:args){if(i==3)throw std::invalid_argument("activation arguments");a[i++]=v;}actor_lifecycle::activate(m,p,a[0],a[1],a[2]);return 0;}
        case 0x506d30:actor_lifecycle::release(m,p);return 0;
        case 0x51f5e0:actor_lifecycle::cleanup(m,p,args.size()?*args.begin():0);return 0;
        case 0x407b90:case 0x411e50:case 0x464030:case 0x464ad0:case 0x464c80:case 0x464ca0:case 0x464e20:case 0x465490:case 0x466870:case 0x474b10:case 0x474b40:case 0x49bcf0:case 0x49c530:case 0x49e170:case 0x4d3ea0:case 0x4de090:case 0x4e1260:case 0x506080:case 0x5075f0:case 0x50eb60:case 0x5121c0:case 0x512340:case 0x5137d0:case 0x51a880:case 0x52ae60:case 0x52d0c0:case 0x54dad0:case 0x54dba0:case 0x54dcb0:case 0x54ddf0:case 0x556370:case 0x571d20:case 0x57d0d0:case 0x58f730:case 0x58f750:case 0x58f7f0:case 0x58f830:case 0x58f890:case 0x597850:case 0x597870:case 0x5979b0:case 0x59c441:
            return Services<M>(m).run(entry,p,args);
        case 0x463cb0:case 0x464cb0:case 0x472730:case 0x494a50:case 0x5057b0:case 0x509bd0:case 0x50ab30:case 0x50db60:case 0x520e70:case 0x522340:
            return Controllers<M>(m).run(entry,p,args,second);
        case 0x4943f0:case 0x508250:case 0x508b00:case 0x509230:case 0x5093e0:case 0x51a8f0:case 0x51aa30:case 0x520790:case 0x5244f0:
            return WorldServices<M>(m).run(entry,p,args);
        case 0x414890:case 0x4637c0:case 0x49d640:case 0x4d3a30:case 0x4d4970:case 0x4d9ac0:case 0x4da690:case 0x4da720:case 0x5055a0:case 0x56c430:
            return UiServices<M>(m).run(entry,p,args);
        case 0x465130:case 0x4667c0:case 0x485b10:case 0x4a5000:case 0x4cb9b0:case 0x4d39b0:case 0x4d3b20:case 0x4d6c60:case 0x4dea00:case 0x50e3d0:case 0x513f50:case 0x5338c0:case 0x54dce0:case 0x58a680:case 0x58d5b0:case 0x5977d0:case 0x597880:case 0x598f70:
            return Dependencies<M>(m).run(entry,p,args);
        case 0x4d3b40:case 0x4d3c90:case 0x4d5640:case 0x4d6620:case 0x4d6c80:case 0x4d78b0:case 0x4d7a00:case 0x4d7a90:case 0x4d9f30:
            return PopupServices<M>(m).run(entry,p,args);
        case 0x42a340:case 0x49cdb0:case 0x514e40:case 0x564e90:
            return CollectionServices<M>(m).run(entry,p,args,second);
        case 0x468470:case 0x469000:case 0x490260:
            return EffectServices<M>(m).run(entry,p,args,second);
        case 0x463270:case 0x464350:case 0x4a4df0:case 0x4a4f70:case 0x58b3e0:case 0x59bf87:case 0x59c4ab:
            return ConstructionServices<M>(m).run(entry,p,args);
        case 0x464b20:case 0x464b30:case 0x464c50:case 0x464d70:case 0x464ec0:case 0x465000:case 0x466900:case 0x4a5a90:case 0x4d47f0:case 0x4d72d0:case 0x4f7970:case 0x510480:case 0x545590:case 0x545690:case 0x5456b0:case 0x5457e0:case 0x545ae0:case 0x545e60:case 0x545e80:case 0x545e90:case 0x54aa00:case 0x54aa10:case 0x54aa80:case 0x54e200:case 0x564d60:case 0x565180:case 0x566d80:case 0x56b790:case 0x56ba50:case 0x56ba60:case 0x56ba70:case 0x56ba80:case 0x57dc20:case 0x57dc60:
            return Providers<M>(m).run(entry,p,args,second);
        default:return std::nullopt;
        }
    }
};
}
