#include "route_scalar.hpp"
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t signed_bits(std::uint32_t n) {
    return n <= 0x7fffffffU ? static_cast<std::int32_t>(n) : -1-static_cast<std::int32_t>(~n);
}
std::uint32_t ftol(long double n) {
    if (!std::isfinite(n) || n < -9223372036854775808.0L || n >= 9223372036854775808.0L)
        return 0; // masked x87 invalid conversion: INT64_MIN low DWORD
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(n));
}
std::uint32_t metric(Coordinates d) {
    return static_cast<std::uint32_t>(distance_metric(signed_bits(std::uint32_t(d.x)*2),
        signed_bits(std::uint32_t(d.y)*2), d.z));
}
}
std::uint32_t base_movement_scalar(std::int32_t category, Coordinates d,
    const CreatureScalarState& s) {
    if (category>=1 && category<=3 && !s.type_3c) return s.default_scalar;
    int parity=0;
    if (!d.x && !d.y) { d.x=d.z; d.z=0; }
    else {
        if (d.x < -1 || d.x > 1 || d.y < -1 || d.y > 1)
            throw std::invalid_argument("scalar XY delta exceeds native direction table");
        constexpr int directions[9]={7,0,1,6,0,2,5,4,3};
        parity=directions[d.x+3*d.y+4]&1;
    }
    std::uint32_t sum=0;
    if (category>=0 && category<=4) {
        const unsigned bank=(category==1 || category==2 ? 2 : 0)+parity;
        for(unsigned i=0;i<12;++i) sum+=s.type_d8[bank*12+i];
    }
    const std::uint32_t scaled=(metric(d)*sum)>>2;
    const std::uint32_t wrapped=scaled*12;
    return static_cast<std::uint32_t>((std::uint64_t(wrapped)*0xaaaaaaabU)>>35);
}
ScalarResult creature_movement_scalar(std::int32_t argument,std::int32_t category,
    Coordinates d,std::uint32_t prior,const CreatureScalarState& s) {
    const auto raw=base_movement_scalar(category,d,s);
    std::uint32_t base=raw;
    if(s.object_778>0) base*=2;
    if(s.object_77c>0) base=static_cast<std::uint32_t>(signed_bits(base)/2);
    if(!argument) throw std::domain_error("original scalar division by zero");
    std::uint32_t target=(base*4)/static_cast<std::uint32_t>(argument);
    if(category!=4) {
        if(d.z==1) target=static_cast<std::uint32_t>(signed_bits(target*2)/3);
        else if(d.z==-1) target=static_cast<std::uint32_t>(signed_bits(target*3)/2);
    }
    std::uint32_t acceleration=static_cast<std::uint32_t>(s.type_10);
    if(signed_bits(target)<signed_bits(prior)) acceleration=0-acceleration;
    long double adjusted=signed_bits(acceleration);
    if(category!=4) {
        const long double slope=static_cast<long double>(s.slope_global)*76.8000030517578125L;
        if(d.z==1) adjusted-=slope;
        else if(d.z==-1) adjusted+=slope;
        if(d.z==1 || d.z==-1) acceleration=ftol(adjusted);
    }
    const auto a=signed_bits(acceleration);
    const std::uint32_t distance=(metric(d)*9*256)>>2;
    const auto discriminant=signed_bits(prior*prior+distance*acceleration*4);
    long double value;
    if(a>=0) {
        value=(std::sqrt(static_cast<long double>(discriminant))+signed_bits(prior))*0.5L;
        // x87 unordered comparison follows the same branch as less-than.
        if(value>=signed_bits(target)) value=signed_bits(target);
    } else {
        value=(std::sqrt(static_cast<long double>(discriminant>0?discriminant:0))
            +signed_bits(prior))*0.5L;
        if(value<=signed_bits(target)) value=signed_bits(target);
    }
    return {ftol(value),raw};
}
NeighborHelpers with_creature_scalar(NeighborHelpers n,
    std::function<CreatureScalarState(std::uint32_t)> state) {
    if(!state) throw std::invalid_argument("scalar object provider required");
    const auto dimensions=n.dimensions;
    n.scalar=[state,dimensions](const NeighborDescriptor& d,Coordinates from,Coordinates to,
        std::int32_t category,std::uint32_t& prior,std::int32_t argument) {
        const Coordinates delta{wrapped_difference(to.x,from.x,dimensions.x),
            wrapped_difference(to.y,from.y,dimensions.y),
            signed_bits(std::uint32_t(to.z)-std::uint32_t(from.z))};
        prior=creature_movement_scalar(argument,category,delta,prior,state(d.object)).scalar;
    };
    return n;
}
} // namespace mnm::reconstruction
