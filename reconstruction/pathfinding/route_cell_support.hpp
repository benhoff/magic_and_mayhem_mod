#pragma once
#include "route_cell_validity.hpp"
#include "route_record_query.hpp"

namespace mnm::reconstruction {
// 0x4f3320: current terrain support or a qualifying cell one plane below.
std::uint32_t test_cell_support(Coordinates position,const CreatureMovementParameters& parameters,
    const CellValidityMapView& view);
RecordQueryHelpers with_cell_support(RecordQueryHelpers query,CellValidityMapView view);
} // namespace mnm::reconstruction
