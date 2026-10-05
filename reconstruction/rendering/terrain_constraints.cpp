#include "terrain_constraints.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void validate(const RegionDescriptorBank& bank) {
    if (bank.internalThreshold<=0 || bank.blocks.size()>50) throw std::invalid_argument("Region descriptor bank");
    for (const auto& b:bank.blocks) if (!b.columns || b.columns>2 || !b.rows || b.rows>2 ||
        b.sourceX>=b.columns || b.sourceY>=b.rows || !b.layers || b.layers>32 ||
        b.selection.section>99 || b.selection.maximum<0 || b.selection.maximum>999 || b.selection.placed<0)
        throw std::invalid_argument("Region block descriptor");
}
std::int32_t signedWord(std::uint32_t word) {
    return word<=0x7fffffffU?std::int32_t(word):-1-std::int32_t(0xffffffffU-word);
}
RegionAdmissionCarry carry(const RegionDescriptorBank& bank,unsigned index,unsigned rotation) {
    const auto& d=bank.blocks.at(index).selection;
    return {d.section,index,rotation,rotateRegionEdges(d.edges,rotation)};
}
}
std::vector<RegionBlockDescriptor> expandRegionDescriptors(const assets::MapAsset& map,
    unsigned section,bool specific,std::int32_t maximum,RegionConnectorState& connectors) {
    const auto w=map.metadata[0],h=map.metadata[1];
    if (!w || w>2 || !h || h>2 || !map.width || !map.height || map.width>128 || map.height>128 ||
        map.width%w || map.height%h || map.width/w!=map.height/h || !map.layers || map.layers>32 ||
        section>99 || maximum<0 || maximum>999) throw std::invalid_argument("Region descriptor MAP/caller fields");
    auto next=connectors;
    for (auto label:next.next) if (label<=0 || label>0x7fffffff-4)
        throw std::invalid_argument("Region connector counter");
    const auto outer=[&](unsigned index){return signedWord(map.metadata[index]);};
    const auto take=[&](unsigned index){const auto value=next.next[index];next.next[index]+=4;return value;};
    std::vector<RegionBlockDescriptor> result;
    for (unsigned y=0;y<h;++y) for (unsigned x=0;x<w;++x) {
        RegionBlockDescriptor b;b.sourceX=x;b.sourceY=y;b.columns=w;b.rows=h;b.layers=map.layers;
        auto& d=b.selection;d.section=section;d.specific=specific;d.maximum=specific?1:maximum;
        if (!x && !y) d.edges={{outer(2),w==2?next.next[0]:outer(4),h==2?next.next[3]:outer(6),outer(8)}};
        if (x && !y) d.edges={{outer(3),outer(4),h==2?next.next[1]:outer(7),take(0)}};
        if (!x && y) d.edges={{take(3),w==2?next.next[2]:outer(5),outer(6),outer(9)}};
        if (x && y) d.edges={{take(1),outer(5),outer(7),take(2)}};
        if (w==1 && h==1) {
            auto single=describeSingleRegionSection(map,section,specific,maximum);d.special=single.special;
        }
        result.push_back(b);
    }
    connectors=next;return result;
}
std::optional<unsigned> findRegionConnector(const RegionDescriptorBank& bank,
    std::int32_t label,std::int32_t placed,unsigned current) {
    validate(bank);if (current>=bank.blocks.size() || placed<0 || label<bank.internalThreshold) throw std::invalid_argument("Region connector lookup");
    std::optional<unsigned> result;
    // Original N search retains its last match; E/S/W searches retain their first.
    for (unsigned i=0;i<bank.blocks.size();++i) if (i!=current && bank.blocks[i].selection.placed==placed && bank.blocks[i].selection.edges[0]==label) result=i;
    if (result) return result;
    for (unsigned side=1;side<4;++side) for (unsigned i=0;i<bank.blocks.size();++i)
        if (i!=current && bank.blocks[i].selection.placed==placed && bank.blocks[i].selection.edges[side]==label) return i;
    return std::nullopt;
}
RegionAdmission admitRegionSection(const RegionSelectionGrid& grid,const RegionDescriptorBank& bank,
    unsigned descriptor,unsigned column,unsigned row,unsigned rotation) {
    validate(bank);const auto& root=bank.blocks.at(descriptor);
    RegionAdmission result;result.carry=carry(bank,descriptor,rotation);
    // Reuse the verified initial neighbor, count, occupied and one-axis checks.
    result.admitted=admitSingleRegionSection(grid,root.selection,column,row,rotation);
    const unsigned width=(rotation&1)?root.rows:root.columns,height=(rotation&1)?root.columns:root.rows;
    result.admitted=result.admitted && width<=grid.columns && height<=grid.rows;
    if (!result.admitted || root.columns*root.rows==1) return result;
    std::vector<bool> visited(grid.cells.size());visited[column*grid.rows+row]=true;
    const auto neighbor=[&](unsigned x,unsigned y,unsigned direction){
        if (direction==0) y=(y+grid.rows-1)%grid.rows;
        if (direction==1) x=(x+1)%grid.columns;
        if (direction==2) y=(y+1)%grid.rows;
        if (direction==3) x=(x+grid.columns-1)%grid.columns;
        return std::array<unsigned,2>{{x,y}};
    };
    const auto neighborsAgree=[&](){
        for (unsigned d=0;d<4;++d) {
            const auto pos=neighbor(column,row,d);
            const auto edge=grid.cells[pos[0]*grid.rows+pos[1]].edges[(d+2)%4];
            if (edge!=-1 && result.carry.edges[d]!=edge) return false;
        }
        return true;
    };
    // Preserve the original ordered, fixed-pass connector walk, including
    // coordinate movement when an occupied block fails admission.
    for (unsigned pass=0;pass<root.columns*root.rows-1;++pass) for (unsigned d=0;d<4;++d) {
        const auto label=result.carry.edges[d];
        if (label<bank.internalThreshold) continue;
        const auto pos=neighbor(column,row,d);const auto index=pos[0]*grid.rows+pos[1];
        if (visited[index]) continue;
        column=pos[0];row=pos[1];
        if (grid.cells[index].occupied) {result.admitted=false;continue;}
        const auto found=findRegionConnector(bank,label,root.selection.placed,result.carry.descriptor);
        if (!found) throw std::invalid_argument("Region internal connector has no descriptor");
        result.carry=carry(bank,*found,rotation);
        if (!neighborsAgree()) result.admitted=false;
        visited[index]=true;
    }
    return result;
}
std::vector<RegionCandidate> regionCandidates(const RegionSelectionGrid& grid,
    const RegionDescriptorBank& bank,unsigned column,unsigned row,bool special) {
    validate(bank);std::vector<RegionCandidate> result;
    // Validate grid/coordinates even for an empty bank.
    SingleRegionDescriptor empty;admitSingleRegionSection(grid,empty,column,row,0);
    for (unsigned i=0;i<bank.blocks.size();++i) {
        const auto& d=bank.blocks[i].selection;if (d.specific || d.special!=special) continue;
        std::uint8_t mask=0;
        for (unsigned r=0;r<4;++r) if (admitRegionSection(grid,bank,i,column,row,r).admitted) mask|=1u<<r;
        if (mask) result.push_back({i,mask});
    }
    return result;
}
std::vector<RegionLocation> regionLocations(const RegionSelectionGrid& grid,
    const RegionDescriptorBank& bank,unsigned descriptor,int rotation) {
    validate(bank);(void)bank.blocks.at(descriptor);
    if (rotation< -1 || rotation>3) throw std::invalid_argument("Region location rotation");
    SingleRegionDescriptor empty;admitSingleRegionSection(grid,empty,0,0,0);
    std::vector<RegionLocation> result;
    for (unsigned y=0;y<grid.rows;++y) for (unsigned x=0;x<grid.columns;++x) {
        std::uint8_t mask=0;
        for (unsigned r=0;r<4;++r) if ((rotation<0 || unsigned(rotation)==r) && admitRegionSection(grid,bank,descriptor,x,y,r).admitted) mask|=1u<<r;
        if (mask) result.push_back({x,y,mask});
    }
    return result;
}
RegionPruningResult pruneRegionCandidateDetails(const RegionDescriptorBank& bank,
    const std::vector<RegionCandidate>& input,unsigned tried,int rotation) {
    validate(bank);(void)bank.blocks.at(tried);
    if (input.size()>49 || rotation< -1 || rotation>3) throw std::invalid_argument("Region pruning extent/rotation");
    RegionPruningResult result;
    struct Slot {bool active=false;int descriptor=-1;std::uint8_t mask=0;};
    std::array<Slot,50> slots{};
    for (unsigned i=0;i<input.size();++i) {
        if (input[i].descriptor>=bank.blocks.size() || !input[i].rotations || input[i].rotations>15) throw std::invalid_argument("Region pruning candidate");
        slots[i]={true,int(input[i].descriptor),input[i].rotations};
    }
    const auto shift=[&](unsigned i){for (unsigned j=i;j<49;++j) slots[j]=slots[j+1];};
    for (unsigned i=0;i<50;++i) if (slots[i].descriptor==int(tried)) {
        if (rotation>=0) slots[i].mask&=std::uint8_t(~(1u<<rotation));
        if (i!=49 && (!slots[i].mask || rotation==-1)) shift(i); // No revisit in first pass.
    }
    for (unsigned i=0;i<50;) {
        auto& slot=slots[i];
        if (slot.descriptor>=0) result.carry=carry(bank,unsigned(slot.descriptor),0);
        if (slot.descriptor>=0) for (unsigned r=0;r<4;++r)
            if (rotateRegionEdges(bank.blocks[slot.descriptor].selection.edges,r)==bank.blocks[tried].selection.edges) {
                const int removed=int(r)+rotation;
                if (removed>=0) slot.mask&=std::uint8_t(~(1u<<(removed%4)));
            }
        if (i!=49 && slot.active && !slot.mask) {shift(i);continue;}
        ++i;
    }
    for (unsigned i=0;i<50;) {
        const auto& slot=slots[i];
        if (slot.active && bank.blocks[slot.descriptor].selection.placed==bank.blocks[slot.descriptor].selection.maximum) {
            shift(i);continue;
        }
        ++i;
    }
    slots.back()={}; // The original always clears its fiftieth entry.
    for (const auto& slot:slots) if (slot.active) result.candidates.push_back({unsigned(slot.descriptor),slot.mask});
    return result;
}
std::vector<RegionCandidate> pruneRegionCandidates(const RegionDescriptorBank& bank,
    const std::vector<RegionCandidate>& input,unsigned tried,int rotation) {
    return pruneRegionCandidateDetails(bank,input,tried,rotation).candidates;
}
}
