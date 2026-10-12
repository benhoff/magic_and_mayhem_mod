#pragma once
#include "commands.hpp"
#include <map>
namespace mnm::render {
// A compiled native UI layer plus exact World-coordinate provenance. Native
// World pixels are sampled afresh; UI pixels remain owned native renderer data.
class NativeFrameRecipe final {
public:
 NativeFrameRecipe(GlBlitter&,unsigned,std::uint64_t,const std::map<unsigned,SurfaceId>&);
 ~NativeFrameRecipe();
 NativeFrameRecipe(const NativeFrameRecipe&)=delete;
 void record(const SurfaceCommand&,const std::map<unsigned,SurfaceId>&);
 void palette(unsigned,const std::vector<Rgb>&,unsigned,const std::map<unsigned,SurfaceId>&);
 void seal(unsigned,const std::map<unsigned,SurfaceId>&);
 bool ready() const{return ui_ && !dirty_;}
 bool hasWorldOutput() const{return coordinates_!=0;}
 std::uint64_t generation() const{return generation_;}
 GpuFrame render(const GpuFrame&);
 std::size_t surfaces() const{return maps_.size()+bool(ui_)+bool(coordinates_)+bool(work_);}
 std::size_t replayCommands() const{return coordinates_?1:0;}
private:
 GlBlitter& renderer_;
 unsigned world_,final_=0;
 std::uint64_t generation_;
 QSize worldSize_;
 std::map<unsigned,SurfaceId> maps_;
 SurfaceId ui_=0,coordinates_=0,work_=0;
 std::size_t pixels_=0;
 unsigned recorded_=0;
 bool dirty_=true;
 void budget(std::size_t,unsigned count=1) const;
 SurfaceId ensure(unsigned,const std::map<unsigned,SurfaceId>&,bool world=false);
 void drop(SurfaceId&);
 void copy(const SurfaceCommand&,const std::map<unsigned,SurfaceId>&,Rect,int,int,std::optional<std::uint32_t>);
};
}
