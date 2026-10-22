#pragma once
#include "world_occlusion.hpp"
#include "world_queue_sort.hpp"
#include <array>

namespace mnm::reconstruction {
using OcclusionActor=std::array<std::uint16_t,6>;
struct OcclusionCatalog {
    std::uint32_t lastFrame{};
    std::vector<std::vector<unsigned char>> frames;
    std::array<std::array<std::array<std::uint32_t,3>,4>,3> actorFrames{};
    std::uint32_t resolve(std::uint32_t index)const{
        if(index>lastFrame)index=0; // Equality is admitted by the original.
        if(index>=frames.size())throw std::invalid_argument("Occlusion frame extent");
        return index+1;
    }
};
// Source and actor WORDs are borrowed tokens. The original scans backwards,
// excluding row zero, and mutates both kinds and the associated actor flags.
inline void sweepOriginalWorldOcclusion(std::vector<PhysicalWorldRow>& rows,std::uint32_t count,
    std::vector<OcclusionActor>& actors,const OcclusionCatalog& catalog,std::uint32_t clock,
    std::vector<unsigned char>& mask,std::size_t base,bool tall){
    if(count>rows.size()||clock>=4)throw std::invalid_argument("Occlusion sweep extent");
    for(std::uint32_t at=count;at>1;){
        auto& row=rows[--at];
        if(!row[1]||row[1]>catalog.frames.size())throw std::invalid_argument("Occlusion sprite token");
        const bool covered=originalWorldOcclusion(&mask,base,43,catalog.frames[row[1]-1],
            occlusionSigned(row[2]),occlusionSigned(row[3]),occlusionSigned(row[6]),tall);
        if(covered)row[6]=0xfffffffeu;
        if(!row[5])continue;
        if(row[5]>actors.size())throw std::invalid_argument("Occlusion actor token");
        auto& actor=actors[row[5]-1];
        if(actor[0]>=catalog.actorFrames.size())throw std::invalid_argument("Occlusion actor type");
        const auto& selection=catalog.actorFrames[actor[0]][clock];
        if(row[1]==catalog.resolve(selection[0])){
            actor[5]=std::uint16_t(covered?actor[5]|0x40u:actor[5]&0xffbfu);
        }else if(row[1]==catalog.resolve(selection[1])){
            actor[4]=std::uint16_t(covered?actor[4]|4u:actor[4]&0xfffbu);
        }else if(row[1]==catalog.resolve(selection[2])){
            actor[4]=std::uint16_t(covered?actor[4]|0x8000u:actor[4]&0x7fffu);
        }
    }
}
} // namespace mnm::reconstruction
