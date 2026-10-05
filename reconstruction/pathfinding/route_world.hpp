#pragma once
#include "route_scalar.hpp"
#include "route_cell_validity.hpp"
#include <memory>
#include <string>

namespace mnm::reconstruction {
// Owning frozen inputs. Engine pointers remain tokens; callbacks retain ownership.
struct RouteWorldSnapshot {
    MapDimensions dimensions{};
    std::int32_t plane_stride=0, boundary=0, budget=0;
    std::uint32_t object_token=0, unknown_argument=0, cell_base=0, default_scalar=0;
    float slope_global=0;
    Coordinates target{};
    std::vector<std::int32_t> rows,layers;
    std::vector<OccupancyCell> cells;
    std::vector<TerrainValidityRecord> terrain;
    ObjectPrefix object{};
    std::array<std::byte,0x198> scalar_type{};
    std::array<std::byte,0x5c9> generator_type{};
};
std::shared_ptr<RouteWorldSnapshot> read_route_world(const std::string& path);
// Same bounded parser for owned bytes, allowing adapters to fingerprint exactly
// the inputs they decode rather than reopening a file between hash and decode.
std::shared_ptr<RouteWorldSnapshot> decode_route_world(const std::vector<std::byte>& bytes);
NeighborHelpers snapshot_neighbors(std::shared_ptr<const RouteWorldSnapshot> snapshot);
SearchResult replay_route_world(std::shared_ptr<const RouteWorldSnapshot> snapshot,
    RouteContextPrefix& context,SearchState& state,std::int32_t& remaining_budget);
struct RouteReplaySession {
    RouteContextPrefix context{};
    SearchState state;
    std::shared_ptr<const RouteWorldSnapshot> previous;
    bool synchronized=false;
};
struct RouteSequenceReplay { std::map<std::uint32_t,RouteReplaySession> contexts; };
struct RouteSequenceResult {
    bool replayed=false;
    std::string reason;
    SearchResult search{SearchStop::queue_empty,0,{}};
    std::int32_t budget_remaining=0;
};
// A nonzero observed flag starts a new search; zero resumes only captured state.
// Failed/desynchronized contexts require another fresh call before resumption.
RouteSequenceResult replay_route_sequence_call(RouteSequenceReplay& replay,std::uint32_t context_token,
    std::uint8_t flag_before,std::shared_ptr<const RouteWorldSnapshot> snapshot);
} // namespace mnm::reconstruction
