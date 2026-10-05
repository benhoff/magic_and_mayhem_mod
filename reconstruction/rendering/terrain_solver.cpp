#include "terrain_solver.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
unsigned cell(const RegionPlacementState& s,unsigned x,unsigned y) {
    if (x>=s.grid.columns || y>=s.grid.rows) throw std::out_of_range("Region placement coordinate");
    return x*s.grid.rows+y;
}
void validate(const RegionPlacementState& s) {
    if (!s.grid.columns || s.grid.columns>5 || !s.grid.rows || s.grid.rows>5 ||
        s.grid.cells.size()!=std::size_t(s.grid.columns)*s.grid.rows ||
        s.assignments.size()!=s.grid.cells.size() || s.storedSourceRows.size()!=s.grid.cells.size() || s.candidates.size()!=s.grid.cells.size() ||
        s.bank.internalThreshold<=0 || s.bank.blocks.size()>50) throw std::invalid_argument("Region placement state extent");
    for (const auto& b:s.bank.blocks) if (!b.columns || b.columns>2 || !b.rows || b.rows>2 ||
        b.sourceX>=b.columns || b.sourceY>=b.rows || !b.layers || b.layers>32 || b.selection.section>99 ||
        b.selection.placed<0 || b.selection.maximum<0 || b.selection.maximum>999) throw std::invalid_argument("Region placement descriptor");
    for (unsigned i=0;i<s.assignments.size();++i) {
        if (s.grid.cells[i].occupied!=bool(s.assignments[i])) throw std::invalid_argument("Region assignment occupancy");
        if (s.assignments[i]) {
            const auto b=*s.assignments[i];
            if (b.descriptor>=s.bank.blocks.size() || b.rotation>3 || s.storedSourceRows[i]!=s.bank.blocks[b.descriptor].sourceY ||
                s.grid.cells[i].edges!=rotateRegionEdges(s.bank.blocks[b.descriptor].selection.edges,b.rotation))
                throw std::invalid_argument("Region assignment descriptor/edges");
        }
    }
    if (s.carry && (s.carry->descriptor>=s.bank.blocks.size() || s.carry->rotation>3)) throw std::invalid_argument("Region placement carry");
}
void load(RegionPlacementState& s,unsigned descriptor,unsigned rotation) {
    const auto& d=s.bank.blocks.at(descriptor).selection;
    s.carry=RegionAdmissionCarry{d.section,descriptor,rotation,rotateRegionEdges(d.edges,rotation)};
}
std::array<unsigned,2> neighbor(const RegionPlacementState& s,unsigned x,unsigned y,unsigned d) {
    if (d==0) y=(y+s.grid.rows-1)%s.grid.rows;
    if (d==1) x=(x+1)%s.grid.columns;
    if (d==2) y=(y+1)%s.grid.rows;
    if (d==3) x=(x+s.grid.columns-1)%s.grid.columns;
    return {{x,y}};
}
void write(RegionPlacementState& s,unsigned x,unsigned y) {
    const auto index=cell(s,x,y);
    if (!s.carry || s.assignments[index]) throw std::invalid_argument("Region placement occupied slot");
    auto& d=s.bank.blocks[s.carry->descriptor].selection;
    if (d.placed==std::numeric_limits<std::int32_t>::max()) throw std::out_of_range("Region placement count overflow");
    ++d.placed;s.storedSourceRows[index]=s.bank.blocks[s.carry->descriptor].sourceY;s.grid.cells[index]={true,s.carry->edges};s.assignments[index]=RegionPlacedBlock{s.carry->descriptor,s.carry->rotation};
}
void place(RegionPlacementState& s,unsigned descriptor,unsigned rotation,unsigned x,unsigned y) {
    const auto root=s.bank.blocks.at(descriptor);
    load(s,descriptor,rotation);const auto count=root.selection.placed;write(s,x,y);
    for (unsigned pass=0;pass<root.columns*root.rows-1;++pass) for (unsigned d=0;d<4;++d) {
        const auto label=s.carry->edges[d];if (label<s.bank.internalThreshold) continue;
        const auto pos=neighbor(s,x,y,d);if (s.grid.cells[cell(s,pos[0],pos[1])].occupied) continue;
        const auto found=findRegionConnector(s.bank,label,count,s.carry->descriptor);
        if (!found) throw std::invalid_argument("Region placement missing connector");
        load(s,*found,rotation);x=pos[0];y=pos[1];write(s,x,y);
    }
}
void remove(RegionPlacementState& s,unsigned x,unsigned y) {
    const auto i=cell(s,x,y);if (!s.assignments[i]) throw std::invalid_argument("Region rollback empty slot");
    auto& count=s.bank.blocks[s.assignments[i]->descriptor].selection.placed;
    if (count<=0) throw std::invalid_argument("Region rollback count underflow");
    --count;s.grid.cells[i]={};s.assignments[i].reset();s.storedSourceRows[i]=0xffffffffU;
}
void build(RegionPlacementState& s,unsigned x,unsigned y,bool special) {
    s.candidates[cell(s,x,y)]=regionCandidates(s.grid,s.bank,x,y,special);
    // Original candidate construction leaves the final eligible r=3 admission carry.
    for (unsigned i=0;i<s.bank.blocks.size();++i) {
        const auto& d=s.bank.blocks[i].selection;
        if (!d.specific && d.special==special) s.carry=admitRegionSection(s.grid,s.bank,i,x,y,3).carry;
    }
}
void prune(RegionPlacementState& s,unsigned x,unsigned y,unsigned descriptor,int rotation) {
    auto result=pruneRegionCandidateDetails(s.bank,s.candidates[cell(s,x,y)],descriptor,rotation);
    s.candidates[cell(s,x,y)]=std::move(result.candidates);if (result.carry) s.carry=result.carry;
}
void selectedPlace(RegionPlacementState& s,unsigned x,unsigned y,std::uint32_t seed) {
    const auto& table=s.candidates[cell(s,x,y)];const auto choice=chooseRegionCandidate(seed,table);
    if (!choice) throw std::logic_error("Region solver empty selection");
    place(s,table[choice->index].descriptor,choice->rotation,x,y);
}
}
RegionPlacementState makeRegionPlacementState(unsigned columns,unsigned rows,RegionDescriptorBank bank) {
    RegionPlacementState s;s.grid.columns=columns;s.grid.rows=rows;s.bank=std::move(bank);
    if (!columns || columns>5 || !rows || rows>5) throw std::invalid_argument("Region placement grid dimensions");
    s.grid.cells.resize(columns*rows);s.assignments.resize(columns*rows);s.storedSourceRows.resize(columns*rows);s.candidates.resize(columns*rows);validate(s);return s;
}
void placeRegionSection(RegionPlacementState& state,unsigned descriptor,unsigned rotation,unsigned x,unsigned y) {
    validate(state);cell(state,x,y);if (rotation>3) throw std::invalid_argument("Region placement rotation");
    auto changed=state;place(changed,descriptor,rotation,x,y);state=std::move(changed);
}
void removeRegionBlock(RegionPlacementState& state,unsigned x,unsigned y) {
    validate(state);remove(state,x,y);
}
RegionSolveResult solveRegionPlacement(RegionPlacementState input,std::uint32_t seed) {
    validate(input);RegionSolveResult result;result.state=std::move(input);auto& s=result.state;
    for (auto& table:s.candidates) table.clear();
    int x=0,y=0;unsigned operations=0;
    while (y!=int(s.grid.rows)) {
        if (++operations>10000) throw std::out_of_range("Region solver operation bound");
        if (result.backtracks>49) return result;
        if (s.grid.cells[cell(s,unsigned(x),unsigned(y))].occupied) {
            if (++x>=int(s.grid.columns)) {x=0;++y;}continue;
        }
        build(s,unsigned(x),unsigned(y),false);
        if (s.candidates[cell(s,unsigned(x),unsigned(y))].empty()) build(s,unsigned(x),unsigned(y),true);
        if (!s.candidates[cell(s,unsigned(x),unsigned(y))].empty()) {
            selectedPlace(s,unsigned(x),unsigned(y),seed);
            if (++x>=int(s.grid.columns)) {x=0;++y;}continue;
        }
        bool changedMulti=false;
        while (true) {
            if (++operations>10000) throw std::out_of_range("Region solver rollback bound");
            ++result.backtracks;
            if (x<1 && y<1) return result;
            if (--x<0) {x=int(s.grid.columns)-1;--y;}
            if (y<0) throw std::out_of_range("Region solver rollback before grid");
            auto assignment=s.assignments[cell(s,unsigned(x),unsigned(y))];
            if (!assignment) throw std::invalid_argument("Region solver rollback unassigned slot");
            while (s.bank.blocks[assignment->descriptor].selection.specific) {
                // Preserve the selected original's unusual fixed-section boundary test.
                if (x-1>=0) return result;
                x=int(s.grid.columns)-1;--y;
                if (y<0) throw std::out_of_range("Region fixed rollback before grid");
                assignment=s.assignments[cell(s,unsigned(x),unsigned(y))];
                if (!assignment) throw std::invalid_argument("Region fixed rollback empty slot");
            }
            const auto descriptor=assignment->descriptor,rotation=assignment->rotation;
            const auto block=s.bank.blocks[descriptor];remove(s,unsigned(x),unsigned(y));
            if (block.columns*block.rows==1) {
                if (changedMulti) build(s,unsigned(x),unsigned(y),false);
                prune(s,unsigned(x),unsigned(y),descriptor,int(rotation));
                if (!s.candidates[cell(s,unsigned(x),unsigned(y))].empty()) {
                    selectedPlace(s,unsigned(x),unsigned(y),seed);break;
                }
                continue;
            }
            ++result.multiBlockBacktracks;changedMulti=true;load(s,descriptor,rotation);
            int minX=x,minY=y;unsigned anchorDescriptor=descriptor;
            // Original multi-block rollback walks occupied neighbors for four passes.
            for (unsigned pass=0;pass<4;++pass) for (unsigned d=0;d<4;++d) {
                if (s.carry->edges[d]<s.bank.internalThreshold) continue;
                const auto pos=neighbor(s,unsigned(x),unsigned(y),d);
                const auto index=cell(s,pos[0],pos[1]);if (!s.assignments[index]) continue;
                x=int(pos[0]);y=int(pos[1]);const auto removed=s.assignments[index]->descriptor;
                load(s,removed,rotation);remove(s,unsigned(x),unsigned(y));
                if ((d%2==0 && y<minY) || (d%2==1 && x<minX)) {
                    if (d%2==0) minY=y;else minX=x;anchorDescriptor=removed;
                }
            }
            x=minX;y=minY;int walkX=x,walkY=y;unsigned walkDescriptor=anchorDescriptor;
            build(s,unsigned(x),unsigned(y),false);prune(s,unsigned(x),unsigned(y),anchorDescriptor,-1);
            std::vector<bool> visited(s.grid.cells.size());visited[cell(s,unsigned(x),unsigned(y))]=true;
            for (unsigned pass=0;pass<3;++pass) for (unsigned d=0;d<4;++d) {
                const auto& current=s.bank.blocks[walkDescriptor].selection;
                const auto label=current.edges[d];if (label<s.bank.internalThreshold) continue;
                const auto pos=neighbor(s,unsigned(walkX),unsigned(walkY),d);const auto index=cell(s,pos[0],pos[1]);
                if (visited[index]) continue;
                walkX=int(pos[0]);walkY=int(pos[1]);visited[index]=true;
                if (!s.carry) throw std::invalid_argument("Region rollback connector carry");
                const auto found=findRegionConnector(s.bank,label,current.placed,s.carry->descriptor);
                if (!found) throw std::invalid_argument("Region rollback missing connector");
                walkDescriptor=*found;prune(s,unsigned(x),unsigned(y),walkDescriptor,-1);
            }
            if (!s.candidates[cell(s,unsigned(x),unsigned(y))].empty()) {
                selectedPlace(s,unsigned(x),unsigned(y),seed);break;
            }
        }
    }
    result.complete=result.backtracks<=49;return result;
}
}
