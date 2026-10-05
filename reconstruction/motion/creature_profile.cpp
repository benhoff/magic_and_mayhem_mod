#include "creature_profile.hpp"
#include "route_scalar.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace mnm::reconstruction {
assets::CreatureMovementConfig normalizeCreatureMovementConfig(assets::CreatureMovementConfig c){
    c.height=std::clamp(c.height,1,5);c.width=std::clamp(c.width,1,2);c.acceleration=std::max(c.acceleration,0);
    c.groundSpeed=std::clamp(c.groundSpeed,0,1000);c.flyingSpeed=std::clamp(c.flyingSpeed,0,1000);return c;
}
std::array<std::uint32_t,48> groundMovementSamples(const assets::Animation& ani){
    if(ani.starts.size()<9)throw std::invalid_argument("Ground ANI requires eight facings");
    std::array<std::uint32_t,48> samples{};
    for(unsigned facing=0;facing<8;++facing){
        const auto a=ani.starts[facing],z=ani.starts[facing+1];
        if(a>=z || z>ani.records.size())throw std::invalid_argument("Ground ANI extent");
        std::vector<const assets::AnimationRecord*> sprites;
        bool event=false;const assets::AnimationRecord* repeated=nullptr;
        for(unsigned i=a;i<z;++i){const auto& r=ani.records[i];
            if(r.opcode==0){if(event){if(repeated)throw std::invalid_argument("Extra ground ANI repeat");repeated=&r;}else sprites.push_back(&r);}
            else if(r.opcode==5 && r.argument==2 && !event && sprites.size()==12)event=true;
            else if(r.opcode==6 && i+1==z && event && repeated){}
            else throw std::invalid_argument("Unsupported ground ANI controls");
        }
        if(sprites.size()!=12 || !event || !repeated || ani.records[z-1].opcode!=6 || repeated->argument!=sprites.front()->argument)
            throw std::invalid_argument("Ground ANI requires twelve sprites, event 2, repeat and stop");
        if(facing>=2)continue; // Original builds banks from facings zero and one.
        std::int32_t first,last;std::memcpy(&first,&sprites.front()->metadata[0],4);std::memcpy(&last,&repeated->metadata[0],4);
        auto distance=std::int64_t(first)-last;if(distance<0)distance=-distance;
        if(distance<1 || distance>4096)throw std::invalid_argument("Ground ANI displacement outside bounded profile");
        const auto total=std::uint32_t(distance)*(facing?3:6);
        std::uint32_t prior=0;
        for(unsigned i=0;i<12;++i){const auto cumulative=total*(i+1)/12;samples[facing*12+i]=cumulative-prior;prior=cumulative;}
    }
    return samples;
}
std::uint32_t groundMovementMaximum(const std::array<std::uint32_t,48>& samples){
    CreatureScalarState s;s.type_d8=samples;std::uint32_t maximum=0;
    for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)for(int z=-1;z<=1;++z)
        if(x || y || z){
            auto target=base_movement_scalar(0,{x,y,z},s);
            // 505160 takes the target output of 505920 with argument four.
            if(z==1)target=target*2/3;else if(z==-1)target=target*3/2;
            maximum=std::max(maximum,target);
        }
    return maximum;
}
}
