#include "frozen_navigation.hpp"
#include "route_world.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace mnm::sandbox {
namespace {
namespace r=mnm::reconstruction;
namespace g=mnm::game;
template<class T> T read(const void* p,std::size_t offset) {T value;std::memcpy(&value,static_cast<const std::byte*>(p)+offset,sizeof(value));return value;}
template<class T> void put(void* p,std::size_t offset,T value) {std::memcpy(static_cast<std::byte*>(p)+offset,&value,sizeof(value));}
r::Coordinates coordinate(g::Point p) {return {p.x,p.y,p.z};}
g::Point point(r::Coordinates p) {return {p.x,p.y,p.z};}
class FrozenNavigation final:public g::Navigation {
    std::shared_ptr<const r::RouteWorldSnapshot> frozen_;
    g::NavigationBinding binding_;
    std::shared_ptr<r::RouteWorldSnapshot> inputs(const g::Entity& e,g::Point target) const {
        if(e.type!=creatureType() || !g::contains({e.x,e.y,e.z},binding_) || !g::contains(target,binding_))
            throw std::invalid_argument("frozen navigation profile/coordinates mismatch");
        auto copy=std::make_shared<r::RouteWorldSnapshot>(*frozen_);
        put(&copy->object,8,e.x);put(&copy->object,12,e.y);put(&copy->object,16,e.z);
        copy->target=coordinate(target);return copy;
    }
public:
    FrozenNavigation(std::shared_ptr<const r::RouteWorldSnapshot> snapshot,std::uint64_t hash):frozen_(std::move(snapshot)) {
        const auto d=frozen_->dimensions;
        if(std::uint64_t(d.x)*d.y*d.z>4096 || frozen_->cells.size()>4096 || !std::isfinite(frozen_->slope_global))
            throw std::invalid_argument("movement sandbox frozen map exceeds bounded slice");
        const auto width=read<std::int32_t>(frozen_->scalar_type.data(),8);
        const auto height=read<std::int32_t>(frozen_->scalar_type.data(),12);
        if(width<1 || width>2 || height<1 || height>d.z || frozen_->boundary<0 || frozen_->boundary>d.z)
            throw std::invalid_argument("unsupported frozen creature/map geometry");
        binding_={point({d.x,d.y,d.z}),hash};
        (void)r::snapshot_neighbors(frozen_); // Validate full row/layer ownership at resource admission.
    }
    g::NavigationBinding binding() const override {return binding_;}
    std::uint32_t creatureType() const override {return read<std::uint32_t>(&frozen_->object,0xa8);}
    g::RoutePlan plan(const g::Entity& e,g::Point destination,std::uint32_t budget) const override {
        auto snapshot=inputs(e,destination);r::RouteContextPrefix context{};context.unknown_route_flag=1;r::SearchState state;
        auto remaining=static_cast<std::int32_t>(budget);
        auto result=r::replay_route_world(snapshot,context,state,remaining);
        if(result.stop==r::SearchStop::budget_exhausted) return {g::PlanStatus::budgetExhausted,{}};
        auto neighbors=r::snapshot_neighbors(snapshot);
        if(result.path.empty() || point(neighbors.coordinates(result.path.back()))!=destination) return {g::PlanStatus::unreachable,{}};
        g::RoutePlan out{g::PlanStatus::reachable,{}};
        for(std::uint32_t i=0;i<context.route.waypoint_count;++i) {
            r::Waypoint waypoint;std::memcpy(&waypoint,context.route.unknown_14.data()+i*sizeof(waypoint),sizeof(waypoint));
            const auto metadata=r::neighbor_movement(state.records.at(result.path.at(i+1)).movement);
            out.route.push_back({{waypoint.x,waypoint.y,waypoint.z},waypoint.direction,waypoint.vertical_delta,waypoint.category,metadata.scalar});
        }
        return out;
    }
    bool accepts(const g::Entity& e,const g::RoutePoint& waypoint) const override {
        if(!e.motion) return false;
        auto snapshot=inputs(e,e.motion->destination);auto neighbors=r::snapshot_neighbors(snapshot);
        const auto from=r::Coordinates{e.x,e.y,e.z};
        auto candidates=r::generate_neighbors(neighbors.node_at(from),r::MovementPayload{},
            r::creature_descriptor(snapshot->object,snapshot->object_token,snapshot->unknown_argument,snapshot->target),neighbors);
        for(const auto& c:candidates) if(point(neighbors.coordinates(c.node))==waypoint.position && c.movement.category==waypoint.category) {
            if(waypoint.position.x==e.x && waypoint.position.y==e.y)
                return waypoint.verticalDelta==waypoint.position.z-e.z;
            const auto dx=r::wrapped_difference(waypoint.position.x,e.x,binding_.dimensions.x);
            const auto dy=r::wrapped_difference(waypoint.position.y,e.y,binding_.dimensions.y);
            if(dx<-1 || dx>1 || dy<-1 || dy>1) return false;
            constexpr std::array<int,9> directions{{7,0,1,6,0,2,5,4,3}};
            return waypoint.verticalDelta==0 && waypoint.direction==directions.at(dx+3*dy+4);
        }
        return false;
    }
};
}
std::shared_ptr<const game::Navigation> loadFrozenNavigation(const std::string& path) {
    if(path.find('\0')!=std::string::npos) throw std::invalid_argument("map path contains NUL");
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f) throw std::runtime_error("cannot open frozen movement map");
    const auto size=f.tellg();if(size<92 || size>64*1024*1024) throw std::invalid_argument("invalid frozen map size");
    f.seekg(0);std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    f.read(reinterpret_cast<char*>(bytes.data()),size);if(!f) throw std::runtime_error("cannot read frozen movement map");
    std::uint64_t hash=14695981039346656037ULL;
    for(auto byte:bytes) {hash^=std::to_integer<std::uint8_t>(byte);hash*=1099511628211ULL;}
    return std::make_shared<FrozenNavigation>(reconstruction::decode_route_world(bytes),hash);
}
}
