#include "terrain_selection.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
TerrainRegionRandom::TerrainRegionRandom(std::uint32_t seed) {
    for (unsigned i=250; i-- >0;) {
        const std::uint64_t product=std::uint64_t(seed)*0x41c64e6d;
        const auto low=static_cast<std::uint32_t>(product);
        seed=low+0x3039U;
        const auto high=static_cast<std::uint32_t>((product>>32)<<16)+0xffffU+std::uint32_t(seed<low);
        words_[i]=(seed>>16)|(high&0xffff0000U);
    }
    std::uint32_t bit=0x80000000U, keep=0xffffffffU;
    for (unsigned i=3; bit; i+=7, bit>>=1, keep>>=1) words_[i]=(words_[i]&keep)|bit;
}
std::uint32_t TerrainRegionRandom::next() {
    const auto value=words_[left_]^words_[right_]; words_[left_]=value;
    left_=(left_+1)%250; right_=(right_+1)%250;
    return value;
}
namespace {
void validate(const RegionSelectionGrid& grid) {
    if (!grid.columns || grid.columns>5 || !grid.rows || grid.rows>5 ||
        grid.cells.size()!=std::size_t(grid.columns)*grid.rows)
        throw std::invalid_argument("Region selection grid extent");
}
void validate(const SingleRegionDescriptor& d) {
    if (d.section>99 || d.maximum<0 || d.maximum>999 || d.placed<0)
        throw std::invalid_argument("Region selection descriptor");
}
unsigned pickRotation(std::uint32_t word, std::uint8_t mask) {
    unsigned count=0;
    for (unsigned r=0; r<4; ++r) count+=bool(mask&(1u<<r));
    if (!count || mask>15) throw std::invalid_argument("Region rotation mask");
    auto ordinal=(word&0x7fffffffU)%count;
    for (unsigned r=0; r<4; ++r) if (mask&(1u<<r)) {
        if (!ordinal) return r;
        --ordinal;
    }
    throw std::logic_error("Region rotation ordinal");
}
template<class T> std::optional<RegionChoice> choose(std::uint32_t seed,
    const std::vector<T>& table, unsigned capacity) {
    if (table.size()>capacity) throw std::invalid_argument("Region selection table capacity");
    for (const auto& entry:table) if (!entry.rotations || entry.rotations>15)
        throw std::invalid_argument("Region selection table rotation mask");
    if (table.empty()) return std::nullopt;
    TerrainRegionRandom random(seed);
    const auto index=(random.next()&0x7fffffffU)%table.size();
    return RegionChoice{unsigned(index),pickRotation(random.next(),table[index].rotations)};
}
}
SingleRegionDescriptor describeSingleRegionSection(const assets::MapAsset& map,
    unsigned section, bool specific, std::int32_t maximum) {
    if (map.metadata[0]!=1 || map.metadata[1]!=1 || !map.width || map.width!=map.height ||
        map.width>128 || !map.layers || map.layers>32)
        throw std::invalid_argument("Selection requires a square single-block section");
    SingleRegionDescriptor d; d.section=section; d.specific=specific; d.maximum=specific?1:maximum;
    for (unsigned i=0; i<4; ++i) {
        // Preserve all 32 bits, including the original -1 sentinel.
        const auto word=map.metadata[2+2*i];
        d.edges[i]=word<=0x7fffffffU?std::int32_t(word):-1-std::int32_t(0xffffffffU-word);
    }
    for (unsigned i=0; i<4; ++i) if (d.edges[i]!=0) {
        bool unique=true;
        for (unsigned j=0; j<4; ++j) if (j!=i && d.edges[i]==d.edges[j]) unique=false;
        d.special|=unique;
    }
    validate(d); return d;
}
RegionEdges rotateRegionEdges(const RegionEdges& edges, unsigned rotation) {
    if (rotation>3) throw std::invalid_argument("Region rotation");
    RegionEdges result{};
    for (unsigned i=0; i<4; ++i) result[(i+rotation)%4]=edges[i];
    return result;
}
bool admitSingleRegionSection(const RegionSelectionGrid& grid, const SingleRegionDescriptor& d,
    unsigned column, unsigned row, unsigned rotation) {
    validate(grid); validate(d);
    if (column>=grid.columns || row>=grid.rows) throw std::out_of_range("Region selection coordinates");
    const auto edges=rotateRegionEdges(d.edges,rotation);
    const auto& north=grid.cells[column*grid.rows+(row+grid.rows-1)%grid.rows].edges;
    const auto& east=grid.cells[((column+1)%grid.columns)*grid.rows+row].edges;
    const auto& south=grid.cells[column*grid.rows+(row+1)%grid.rows].edges;
    const auto& west=grid.cells[((column+grid.columns-1)%grid.columns)*grid.rows+row].edges;
    // The selected original's west comparison gates on neighbor W, not E.
    // Preserve this asymmetry rather than correcting generator behavior.
    return !grid.cells[column*grid.rows+row].occupied && d.placed!=d.maximum &&
        (north[2]==-1 || edges[0]==north[2]) &&
        (east[3]==-1 || edges[1]==east[3]) &&
        (south[0]==-1 || edges[2]==south[0]) &&
        (west[3]==-1 || edges[3]==west[1]) &&
        (grid.columns!=1 || edges[1]==edges[3]) &&
        (grid.rows!=1 || edges[0]==edges[2]);
}
std::vector<RegionCandidate> singleRegionCandidates(const RegionSelectionGrid& grid,
    const std::vector<SingleRegionDescriptor>& descriptors, unsigned column, unsigned row, bool special) {
    validate(grid);
    if (descriptors.size()>50) throw std::invalid_argument("Region descriptor capacity");
    if (column>=grid.columns || row>=grid.rows) throw std::out_of_range("Region selection coordinates");
    std::vector<RegionCandidate> result;
    for (unsigned i=0; i<descriptors.size(); ++i) {
        validate(descriptors[i]);
        if (descriptors[i].specific || descriptors[i].special!=special) continue;
        std::uint8_t mask=0;
        for (unsigned r=0; r<4; ++r) if (admitSingleRegionSection(grid,descriptors[i],column,row,r)) mask|=1u<<r;
        if (mask) result.push_back({i,mask});
    }
    return result;
}
std::vector<RegionLocation> singleRegionLocations(const RegionSelectionGrid& grid,
    const SingleRegionDescriptor& d, int rotation) {
    validate(grid); validate(d);
    if (rotation< -1 || rotation>3) throw std::invalid_argument("Region requested rotation");
    std::vector<RegionLocation> result;
    // The original compacts valid locations in row/column scan order.
    for (unsigned row=0; row<grid.rows; ++row) for (unsigned column=0; column<grid.columns; ++column) {
        std::uint8_t mask=0;
        for (unsigned r=0; r<4; ++r) if ((rotation<0 || unsigned(rotation)==r) &&
            admitSingleRegionSection(grid,d,column,row,r)) mask|=1u<<r;
        if (mask) result.push_back({column,row,mask});
    }
    return result;
}
std::optional<RegionChoice> chooseRegionCandidate(std::uint32_t seed, const std::vector<RegionCandidate>& table) {
    for (const auto& entry:table) if (entry.descriptor>=50) throw std::invalid_argument("Region descriptor index");
    return choose(seed,table,50);
}
std::optional<RegionChoice> chooseRegionLocation(std::uint32_t seed, const std::vector<RegionLocation>& table) {
    for (const auto& entry:table) if (entry.column>=5 || entry.row>=5) throw std::invalid_argument("Region location coordinates");
    return choose(seed,table,25);
}
std::optional<unsigned> chooseRegionRotation(std::uint32_t seed, std::uint8_t mask) {
    if (mask>15) throw std::invalid_argument("Region rotation mask");
    if (!mask) return std::nullopt;
    TerrainRegionRandom random(seed);
    return pickRotation(random.next(),mask);
}
}
