#pragma once
#include "commands.hpp"
#include <map>
namespace mnm::render {
// Internal GPU recipe. Static inputs are owned, versioned native surfaces;
// World-dependent destinations are reset and drawn anew on every native frame.
class NativeFrameRecipe final {
public:
 NativeFrameRecipe(GlBlitter&,unsigned world,std::uint64_t generation,const std::map<unsigned,SurfaceId>&);
 ~NativeFrameRecipe();
 NativeFrameRecipe(const NativeFrameRecipe&)=delete;
 void record(const SurfaceCommand&,const std::map<unsigned,SurfaceId>&);
 void palette(unsigned,const std::vector<Rgb>&,unsigned,const std::map<unsigned,SurfaceId>&);
 void seal(unsigned,const std::map<unsigned,SurfaceId>&);
 bool ready() const{return final_!=0 && !dirty_;}
 std::uint64_t generation() const{return generation_;}
 GpuFrame render(const GpuFrame& world);
 std::size_t surfaces() const{return planes_.size()*2+frozen_.size();}
 std::size_t replayCommands() const{return steps_.size();}
private:
 struct Plane {SurfaceId baseline,target;};
 struct Step {SurfaceCommand command;SurfaceId frozenSource=0;};
 GlBlitter& renderer_;
 unsigned world_,final_=0;
 SurfaceId frozenFinal_=0;
 std::uint64_t generation_;
 std::map<unsigned,Plane> planes_;
 std::map<unsigned,std::uint64_t> revisions_;
 std::map<std::pair<unsigned,std::uint64_t>,SurfaceId> frozen_;
 std::vector<Step> steps_;
 std::size_t pixels_=0,bytes_=0,recorded_=0;
 bool dirty_=true;
 void ensure(unsigned,const std::map<unsigned,SurfaceId>&);
 SurfaceId freeze(unsigned,const std::map<unsigned,SurfaceId>&);
 void budget(SurfaceId,unsigned copies) const;
 void add(const SurfaceCommand&,SurfaceId frozenSource=0);
 SurfaceId target(unsigned id) const{return planes_.at(id).target;}
};
}
