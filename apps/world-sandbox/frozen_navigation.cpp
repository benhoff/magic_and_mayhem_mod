#include "frozen_navigation.hpp"
#include "route_world.hpp"
#include "creature_motion.hpp"
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
    r::MotionInputs motionInputs(const g::Entity& e,const g::RoutePoint& waypoint) const {
        if((waypoint.category!=0 && waypoint.category!=4) || e.type==12 ||
           read<std::uint8_t>(&frozen_->object,0x722)!=0 ||
           read<std::int32_t>(&frozen_->object,0x108)!=0)
            throw std::invalid_argument("sample driver requires ordinary forward movement profile");
        r::CreatureScalarState scalar;
        scalar.type_3c=read<std::int32_t>(frozen_->scalar_type.data(),0x3c);
        scalar.type_10=read<std::int32_t>(frozen_->scalar_type.data(),0x10);
        std::memcpy(scalar.type_d8.data(),frozen_->scalar_type.data()+0xd8,sizeof(scalar.type_d8));
        scalar.default_scalar=frozen_->default_scalar;scalar.slope_global=frozen_->slope_global;
        const auto delta=r::Coordinates{r::wrapped_difference(waypoint.position.x,e.x,binding_.dimensions.x),
            r::wrapped_difference(waypoint.position.y,e.y,binding_.dimensions.y),waypoint.position.z-e.z};
        const auto duration=r::base_movement_scalar(waypoint.category,delta,scalar);
        if(!waypoint.scalar || waypoint.scalar>1000000 || !duration || duration>1000000)
            throw std::invalid_argument("unsupported sample motion rate/duration");
        r::MotionInputs p;p.rate=waypoint.scalar;p.duration=duration;p.gridX=e.x;p.gridY=e.y;
        p.heightOrigin=e.z*16;p.heightDelta=delta.z*16;p.direction=waypoint.direction;p.vertical=waypoint.verticalDelta!=0;
        const auto bank=p.vertical?0:(waypoint.direction&1);
        for(unsigned i=0;i<12;++i) p.samples[i]=read<std::int32_t>(frozen_->scalar_type.data(),0xd8+(bank*12+i)*4);
        return p;
    }
    g::FineMotion prepareFineMotion(const g::Entity& e,const g::RoutePoint& waypoint) const override {
        const auto p=motionInputs(e,waypoint);g::FineMotion f;
        f.rate=p.rate;f.duration=p.duration;f.heightOrigin=p.heightOrigin;f.heightDelta=p.heightDelta;
        f.fine={e.x*32,e.y*32,e.z*16};
        // Check sample/config admission even if no iteration is admitted yet.
        r::MotionState probe;probe.fineX=f.fine.x;probe.fineY=f.fine.y;probe.fineZ=f.fine.z;
        (void)r::advance_creature_motion(probe,p);return f;
    }
    void validateFineMotion(const g::Entity& e,const g::RoutePoint& waypoint,const g::FineMotion& f) const override {
        const auto expected=prepareFineMotion(e,waypoint);const auto p=motionInputs(e,waypoint);
        int cycle=0,prefix=0;
        for(unsigned i=0;i<12;++i) {const auto step=p.samples[i]*(p.vertical?2:1);cycle+=step;if(i<f.frame) prefix+=step;}
        constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
        const auto x=p.vertical?0:dx[p.direction],y=p.vertical?0:dy[p.direction];
        if(f.accumulator<0 || f.accumulator>=p.duration || f.progress<prefix || (f.progress-prefix)%cycle ||
           f.travelX!=x*f.progress || f.travelY!=y*f.progress ||
           f.residualX!=x*((f.progress-prefix)/6-f.progress/6) ||
           f.residualY!=y*((f.progress-prefix)/6-f.progress/6))
            throw std::invalid_argument("saved sample cursor/displacement disagrees with profile");
        if(f.rate!=expected.rate || f.duration!=expected.duration || f.heightOrigin!=expected.heightOrigin || f.heightDelta!=expected.heightDelta)
            throw std::invalid_argument("saved sample motion profile disagrees with map");
    }
    bool advanceFineMotion(const g::Entity& e,const g::RoutePoint& waypoint,g::FineMotion& f) const override {
        validateFineMotion(e,waypoint,f);const auto inputs=motionInputs(e,waypoint);
        r::MotionState s{f.accumulator,f.progress,f.travelX,f.travelY,f.fine.x,f.fine.y,f.fine.z,f.residualX,f.residualY,f.frame};
        const auto complete=r::advance_creature_motion(s,inputs);
        f.accumulator=s.accumulator;f.progress=s.progress;f.travelX=s.travelX;f.travelY=s.travelY;
        f.fine={s.fineX,s.fineY,s.fineZ};f.residualX=s.residualX;f.residualY=s.residualY;f.frame=s.frame;
        return complete;
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
