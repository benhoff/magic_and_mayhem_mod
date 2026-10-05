#include "terrain_specific.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void load(RegionPlacementState& s,unsigned index,unsigned rotation=0) {
    const auto& d=s.bank.blocks.at(index).selection;
    s.carry=RegionAdmissionCarry{d.section,index,rotation,rotateRegionEdges(d.edges,rotation)};
}
bool admit(RegionPlacementState& s,unsigned index,unsigned x,unsigned y,unsigned rotation) {
    auto admission=admitRegionSection(s.grid,s.bank,index,x,y,rotation);
    s.carry=admission.carry;return admission.admitted;
}
std::optional<unsigned> chooseRotation(RegionPlacementState& s,unsigned x,unsigned y,std::uint32_t seed) {
    const auto saved=s.carry.value();std::uint8_t mask=0;
    // Offsets are relative to the current scratch descriptor and orientation,
    // which a failed multi-block admission may have changed.
    for(unsigned offset=0;offset<4;++offset)
        if(admit(s,saved.descriptor,x,y,(saved.rotation+offset)%4)) mask|=1u<<offset;
    load(s,saved.descriptor,saved.rotation);
    return chooseRegionRotation(seed,mask);
}
void locations(RegionSpecificResult& result,unsigned index,int rotation) {
    auto& s=result.state;result.locations.clear();
    for(unsigned y=0;y<s.grid.rows;++y) for(unsigned x=0;x<s.grid.columns;++x) {
        std::uint8_t mask=0;
        if(rotation<0) {
            for(unsigned r=0;r<4;++r) if(admit(s,index,x,y,r)) mask|=1u<<r;
            load(s,index); // Original all-rotation scan restores the root carry.
        } else {
            if(admit(s,index,x,y,unsigned(rotation))) mask=1u<<rotation;
            // Fixed nonzero scans rotate the possibly changed admission carry
            // back to zero; fixed-zero scans leave that carry untouched.
            if(rotation) {
                s.carry->edges=rotateRegionEdges(s.carry->edges,4-unsigned(rotation));
                s.carry->rotation=(s.carry->rotation+4-unsigned(rotation))%4;
            }
        }
        if(mask) result.locations.push_back({x,y,mask});
    }
}
}
RegionSpecificResult placeRegionSpecifics(RegionPlacementState input,
    std::vector<RegionSpecificRequest> requests,std::uint32_t seed) {
    validateRegionPlacementState(input);
    if(requests.size()!=input.bank.blocks.size()) throw std::invalid_argument("Region Specific request extent");
    for(const auto& q:requests) if(q.rotation< -1 || q.rotation>3 || q.column< -1 || q.row< -1 ||
        q.column>=int(input.grid.columns) || q.row>=int(input.grid.rows)) throw std::invalid_argument("Region Specific request fields");
    RegionSpecificResult result;result.state=std::move(input);result.requests=std::move(requests);auto& s=result.state;
    const auto notice=[&](unsigned i,RegionSpecificNotice n){result.diagnostics.push_back({i,n});};
    for(unsigned pass=0;pass<2;++pass) for(unsigned i=0;i<s.bank.blocks.size();++i) {
        auto& d=s.bank.blocks[i].selection;auto& q=result.requests[i];
        if(!d.specific || d.placed>=d.maximum) continue;
        if(q.column==-1 || q.row==-1) q.column=q.row=-1;
        load(s,i);
        if(!pass) {
            if(q.column<0) continue;
            if(q.rotation<0) {
                const auto rotation=chooseRotation(s,unsigned(q.column),unsigned(q.row),seed);
                if(rotation) placeRegionSection(s,i,*rotation,unsigned(q.column),unsigned(q.row));
                else {notice(i,RegionSpecificNotice::FixedLocationRejected);q.column=q.row=-1;}
            } else if(admit(s,i,unsigned(q.column),unsigned(q.row),unsigned(q.rotation))) {
                placeRegionSection(s,i,unsigned(q.rotation),unsigned(q.column),unsigned(q.row));
            } else {
                notice(i,RegionSpecificNotice::RequestedRotationRejected);
                const auto offset=chooseRotation(s,unsigned(q.column),unsigned(q.row),seed);
                if(offset) placeRegionSection(s,i,(unsigned(q.rotation)+*offset)%4,unsigned(q.column),unsigned(q.row));
                else {notice(i,RegionSpecificNotice::FixedLocationRejected);q.column=q.row=-1;}
            }
        } else if(q.column==-1) {
            locations(result,i,q.rotation);
            auto choice=chooseRegionLocation(seed,result.locations);
            if(!choice && q.rotation>=0) {
                notice(i,RegionSpecificNotice::RequestedLocationsEmpty);
                locations(result,i,-1);choice=chooseRegionLocation(seed,result.locations);
            }
            if(!choice) {
                notice(i,RegionSpecificNotice::AllLocationsEmpty);
                // Original suppresses this descriptor by setting count to max,
                // even though no block was written.
                d.placed=d.maximum;
            } else {
                const auto p=result.locations[choice->index];placeRegionSection(s,i,choice->rotation,p.column,p.row);
            }
        }
    }
    return result;
}
}
