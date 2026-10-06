#include "effect_animation_binding.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
EffectAnimationBinding bindEffectAnimation(std::uint32_t animationOrdinal,
    const std::vector<EffectAnimationEntry>& entries,
    const std::vector<assets::Animation>& animations){
    if(animationOrdinal>=entries.size())
        throw std::invalid_argument("Effect animation metadata ordinal outside owned entries");
    const auto entry=entries[animationOrdinal];
    if(entry.assetIndex>=animations.size())
        throw std::invalid_argument("Effect ANI asset ordinal outside owned assets");
    const auto& animation=animations[entry.assetIndex];
    if(std::uint64_t(entry.sequence)+1>=animation.starts.size())
        throw std::invalid_argument("Effect ANI sequence outside owned table");
    const auto first=animation.starts[entry.sequence],end=animation.starts[entry.sequence+1];
    if(first>=end || end>animation.records.size() || end-first>65536)
        throw std::invalid_argument("Effect ANI selected record extent outside owned bounds");
    std::vector<assets::AnimationRecord> sequence(animation.records.begin()+first,animation.records.begin()+end);
    for(const auto& record:sequence)
        if(record.opcode==0 && record.argument<0)
            throw std::invalid_argument("Effect ANI sprite ordinal is negative");
    EffectAnimationBinding binding{animationOrdinal,entry,NoCdAnimationPlayer(std::move(sequence))};
    binding.player.start();
    return binding;
}
}
