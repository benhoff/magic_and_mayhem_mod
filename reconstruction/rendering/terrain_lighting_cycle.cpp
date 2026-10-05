#include "terrain_lighting_cycle.hpp"
#include <limits>
#include <stdexcept>
#include <utility>
namespace mnm::reconstruction {
void TerrainLightingCycle::step(const TerrainLightingSnapshot& input){
 if(ticks_==std::numeric_limits<unsigned>::max())throw std::overflow_error("Terrain lighting tick counter exhausted");
 auto candidate=*this;
 const auto& controls=candidate.objects_.state();
 candidate.creatures_.step(candidate.field_,input.creatures,input.creatureScan,input.player,input.relations,
                          {input.creaturesEnabled,controls.active,controls.requested,input.changed});
 candidate.objects_.step(candidate.field_,input.objects,input.objectScan,input.view,
                        candidate.creatures_.state().published,input.changed,input.objectsEnabled);
 ++candidate.ticks_;
 *this=std::move(candidate);
}
}
