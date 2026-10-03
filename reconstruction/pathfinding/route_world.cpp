#include "route_world.hpp"
#include "route_boundary.hpp"
#include "route_cell_support.hpp"
#include "route_clearance.hpp"
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <unordered_map>

namespace mnm::reconstruction {
namespace {
template<class T> T load(const void* p,std::size_t offset) {
    T v; std::memcpy(&v,static_cast<const std::byte*>(p)+offset,sizeof(v)); return v;
}
void require(bool condition,const char* message) { if(!condition) throw std::invalid_argument(message); }
std::size_t index(const RouteWorldSnapshot& s,Coordinates p) {
    require(p.x>=0 && p.x<s.dimensions.x && p.y>=0 && p.y<s.dimensions.y
        && p.z>=0 && p.z<s.dimensions.z,"snapshot coordinates out of bounds");
    const auto n=std::int64_t(s.rows.at(p.y))+s.layers.at(p.z)+p.x;
    require(n>=0 && std::uint64_t(n)<s.cells.size(),"snapshot cell offset out of bounds");
    return static_cast<std::size_t>(n);
}
void validate(const RouteWorldSnapshot& s) {
    const auto d=s.dimensions;
    require(d.x>0 && d.x<=1024 && d.y>0 && d.y<=1024 && d.z>0 && d.z<=32,
        "snapshot dimensions exceed table bounds");
    require(s.rows.size()==std::size_t(d.y) && s.layers.size()==std::size_t(d.z)
        && s.plane_stride>0 && !s.cells.empty() && !s.terrain.empty(),"snapshot tables incomplete");
    require(s.object_token!=0 && s.cell_base!=0
        && std::uint64_t(s.cell_base)+s.cells.size()*12<=0x100000000ULL,"snapshot tokens out of range");
    for(const auto& cell:s.cells)
        require(load<std::uint16_t>(&cell,0)<s.terrain.size(),"snapshot terrain index out of bounds");
}
}
std::shared_ptr<RouteWorldSnapshot> read_route_world(const std::string& path) {
    std::ifstream input(path,std::ios::binary|std::ios::ate);
    require(bool(input),"cannot open route world snapshot");
    const auto size=input.tellg();
    require(size>=92 && size<=64*1024*1024,"snapshot file size invalid");
    input.seekg(0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    input.read(reinterpret_cast<char*>(bytes.data()),size);
    require(bool(input),"snapshot read failed");
    require(std::memcmp(bytes.data(),"MNMWLD01",8)==0,"unsupported route snapshot format");
    const auto u=[&](unsigned i) { return load<std::uint32_t>(bytes.data(),8+i*4); };
    const auto i=[&](unsigned n) { return load<std::int32_t>(bytes.data(),8+n*4); };
    auto s=std::make_shared<RouteWorldSnapshot>();
    s->object_token=u(0); s->dimensions={i(1),i(2),i(3)}; s->plane_stride=i(4);
    s->boundary=i(5); s->budget=i(6); s->unknown_argument=u(7); s->target={i(8),i(9),i(10)};
    s->default_scalar=u(11); s->slope_global=load<float>(bytes.data(),8+12*4); s->cell_base=u(13);
    std::size_t cursor=92;
    const auto block=[&](unsigned n,void* destination,std::size_t expected) {
        require(u(14+n)==expected && expected<=bytes.size()-cursor,"snapshot block size mismatch");
        std::memcpy(destination,bytes.data()+cursor,expected); cursor+=expected;
    };
    require(s->dimensions.y>0 && s->dimensions.y<=1024 && s->dimensions.z>0 && s->dimensions.z<=32,
        "snapshot table dimensions invalid");
    s->rows.resize(s->dimensions.y); s->layers.resize(s->dimensions.z);
    block(0,s->rows.data(),s->rows.size()*4); block(1,s->layers.data(),s->layers.size()*4);
    require(u(16)%12==0 && u(17)%0x164==0 && u(16)>0 && u(17)>0,
        "snapshot record strides invalid");
    require(u(16)<=bytes.size()-cursor,"snapshot cells truncated");
    s->cells.resize(u(16)/12); block(2,s->cells.data(),s->cells.size()*12);
    require(u(17)<=bytes.size()-cursor,"snapshot terrain truncated");
    s->terrain.resize(u(17)/0x164); block(3,s->terrain.data(),s->terrain.size()*0x164);
    block(4,&s->object,sizeof(s->object)); block(5,s->scalar_type.data(),s->scalar_type.size());
    block(6,s->generator_type.data(),s->generator_type.size());
    require(cursor==bytes.size(),"snapshot trailing bytes"); validate(*s); return s;
}
NeighborHelpers snapshot_neighbors(std::shared_ptr<const RouteWorldSnapshot> s) {
    require(bool(s),"snapshot owner required"); validate(*s);
    const auto dims=s->dimensions;
    auto inverse=std::make_shared<std::unordered_map<NodeId,Coordinates>>();
    for(int z=0;z<dims.z;++z) for(int y=0;y<dims.y;++y) for(int x=0;x<dims.x;++x) {
        const Coordinates p{x,y,z}; const auto n=index(*s,p);
        const auto token=static_cast<NodeId>(s->cell_base+n*12);
        require(inverse->emplace(token,p).second,"snapshot coordinate tables alias cells");
    }
    const OccupancyMapView map{dims,s->cells.data(),s->cells.size(),s->rows.data(),s->rows.size(),
        s->layers.data(),s->layers.size(),s->plane_stride,dims.z};
    const CellValidityMapView view{map,s->terrain.data(),s->terrain.size()};
    MovementHelpers movement; movement.dimensions=dims;
    movement.boundary=[s] { return s->boundary; }; movement.layer_count=[s] { return s->dimensions.z; };
    RecordQueryHelpers query; query.dimensions=dims; query.boundary=movement.boundary;
    movement=with_clearance_tests(movement,view);
    movement=with_record_query(movement,with_cell_support(query,view));
    ValidityHelpers validity; validity.dimensions=dims;
    movement=with_validity_test(movement,with_cell_validity_test(validity,view));
    movement=with_boundary_test(movement);
    CreatureAcceptanceHelpers acceptance; acceptance.dimensions=dims;
    acceptance.object_state=[s](std::uint32_t token) {
        require(token==s->object_token,"snapshot contains only one creature");
        const auto* o=&s->object; const auto* t=s->scalar_type.data();
        return CreatureAcceptanceState{load<std::int32_t>(o,0),load<std::int32_t>(o,0xa8),
            load<std::int32_t>(o,0x104),load<std::int32_t>(o,0x5ec),load<std::uint8_t>(o,0x723),
            load<std::int32_t>(t,8),load<std::int32_t>(t,12),load<std::int32_t>(t,0x44)};
    };
    acceptance=with_movement_test(acceptance,movement);
    CellHelpers cells; cells.dimensions=dims;
    acceptance=with_cell_test(acceptance,with_occupancy_test(cells,map));
    NeighborHelpers n; n.dimensions=dims;
    n.node_at=[s](Coordinates p) { return static_cast<NodeId>(s->cell_base+index(*s,p)*12); };
    n.coordinates=[inverse,s](NodeId token) { (void)s; return inverse->at(token); };
    n.cell=[s](NodeId token) {
        require(token>=s->cell_base && (token-s->cell_base)%12==0,"invalid snapshot cell token");
        const auto& cell=s->cells.at((token-s->cell_base)/12);
        return NeighborCell{load<std::uint16_t>(&cell,0),load<std::uint16_t>(&cell,8),cell.flags_0a};
    };
    n.record=[s](const NeighborDescriptor& desc,Coordinates) {
        require(!desc.mode && desc.object==s->object_token,"snapshot supports creature descriptor only");
        return NeighborRecord{load<std::int32_t>(s->generator_type.data(),0x589),
            load<std::uint32_t>(s->generator_type.data(),0x5ad)!=0,0};
    };
    n.object_d03=[s](std::uint32_t token) {
        require(token==s->object_token,"unknown snapshot creature token"); return load<std::int32_t>(&s->object,0xd03);
    };
    n=with_creature_acceptance(n,acceptance);
    n=with_creature_scalar(n,[s](std::uint32_t token) {
        require(token==s->object_token,"unknown scalar creature token");
        CreatureScalarState result;
        result.type_3c=load<std::int32_t>(s->scalar_type.data(),0x3c);
        result.type_10=load<std::int32_t>(s->scalar_type.data(),0x10);
        std::memcpy(result.type_d8.data(),s->scalar_type.data()+0xd8,sizeof(result.type_d8));
        result.object_778=load<std::int32_t>(&s->object,0x778);
        result.object_77c=load<std::int32_t>(&s->object,0x77c);
        result.default_scalar=s->default_scalar; result.slope_global=s->slope_global; return result;
    });
    return n;
}
SearchResult replay_route_world(std::shared_ptr<const RouteWorldSnapshot> s,RouteContextPrefix& context,
    SearchState& state,std::int32_t& budget) {
    const auto world=neighbor_search_world(snapshot_neighbors(s),s->target,s->object_token);
    return route_search(context,s->object,s->unknown_argument,s->target,budget,state,world);
}
RouteSequenceResult replay_route_sequence_call(RouteSequenceReplay& replay,std::uint32_t token,
    std::uint8_t flag,std::shared_ptr<const RouteWorldSnapshot> s) {
    require(token!=0 && bool(s),"context and world snapshot required");
    auto& session=replay.contexts[token];
    const auto blocked=[&](const char* reason) {
        session.synchronized=false;
        return RouteSequenceResult{false,reason,{SearchStop::queue_empty,0,{}},s->budget};
    };
    if(!flag) {
        if(!session.previous) return blocked("missing_initialization");
        if(!session.synchronized) return blocked("prior_failure");
        if(session.context.unknown_route_flag || !session.state.initialized)
            return blocked("initialization_flag_disagrees");
        const auto& old=*session.previous;
        if(old.cell_base!=s->cell_base || old.dimensions.x!=s->dimensions.x
            || old.dimensions.y!=s->dimensions.y || old.dimensions.z!=s->dimensions.z
            || old.rows!=s->rows || old.layers!=s->layers || old.plane_stride!=s->plane_stride)
            return blocked("map_identity_changed");
        if(old.object_token!=s->object_token || old.unknown_argument!=s->unknown_argument
            || old.target.x!=s->target.x || old.target.y!=s->target.y || old.target.z!=s->target.z
            || std::memcmp(reinterpret_cast<const std::byte*>(&old.object)+8,
                reinterpret_cast<const std::byte*>(&s->object)+8,12)!=0)
            return blocked("request_identity_changed");
    }
    // On a reset, clear before executing so exceptions cannot expose old state.
    if(flag) session=RouteReplaySession{};
    session.context.unknown_route_flag=flag;
    session.synchronized=false;
    auto budget=s->budget;
    auto result=replay_route_world(s,session.context,session.state,budget);
    session.previous=std::move(s);
    session.synchronized=true;
    return {true,"",std::move(result),budget};
}
} // namespace mnm::reconstruction
