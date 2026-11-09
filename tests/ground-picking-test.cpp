#include "ground_picking.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool b){if(!b)throw std::runtime_error("Owned ground picking contract");}
int main(){
    const PickingMask mask(3,2,-2,1,{0,1,0,1,0,1});
    require(mask.hit(10,10,13,9));
    require(!mask.hit(10,10,12,9));
    require(mask.hit(10,10,12,10));
    require(!mask.hit(10,10,15,9));
    require(!mask.hit(10,10,13,11));
    const auto hi=std::numeric_limits<std::int32_t>::max();
    const auto lo=std::numeric_limits<std::int32_t>::min();
    const PickingMask wrapped(2,1,0,0,{1,1});
    require(!wrapped.hit(hi,0,hi,0)); // Original signed wrapped right bound.
    require(wrapped.hit(lo,0,lo,0));
    require(!PickingMask(0,0,0,0,{}).hit(0,0,0,0));
    unsigned refused=0;
    const auto refuses=[&](auto f){try{f();}catch(const std::exception&){++refused;return;}require(false);};
    refuses([]{PickingMask m(1,0,0,0,{});});
    refuses([]{PickingMask m(1,1,0,0,{});});
    refuses([]{PickingMask m(1,1,0,0,{2});});
    refuses([]{PickingMask m(2049,1,0,0,std::vector<std::uint8_t>(2049));});
    std::vector<PickingMask> masks;
    masks.emplace_back(0,0,0,0,std::vector<std::uint8_t>{});
    masks.emplace_back(1,1,0,0,std::vector<std::uint8_t>{1});
    masks.emplace_back(3,1,0,0,std::vector<std::uint8_t>{0,0,1});
    TerrainDefinition d;d.body.fill(1);d.second.fill(2);
    std::vector<TerrainDefinition> definitions(2);definitions[1]=d;
    TerrainCamera camera;camera.span=1;camera.diagonal=0;camera.column=2;camera.row=3;
    camera.cutLevel=3;camera.f11=100;camera.f15=100;camera.f41=0;
    std::vector<GroundTile> cells(8*9*3);
    const unsigned cell=(1*9+3)*8+2;
    cells[cell]={1,0x8004,0}; // Child is suppressed for drawing, still queried.
    for(unsigned view=0;view<4;++view){
        camera.view=view;
        const auto picked=pickWorldGround(8,9,3,camera,{0,0,640,480},1,false,cells,definitions,masks,102,84);
        require(picked.ground.has_value());
        require(picked.ground->column==2 && picked.ground->row==3 && picked.ground->layer==1);
        require(picked.submittedRows==1);
        require(picked.terrainDepth==spriteDepthKey({80,112,16,0},view));
        require(picked.flags[cell][0]==0x8004);
    }
    camera.view=0;
    require(!pickWorldGround(8,9,3,camera,{},1,false,cells,definitions,masks,99,84).ground);
    auto noBody=definitions;noBody[1].body.fill(0);
    require(!pickWorldGround(8,9,3,camera,{},1,false,cells,noBody,masks,102,84).ground);
    cells[cell].flags8|=3;camera.mode=1;
    require(!pickWorldGround(8,9,3,camera,{},1,false,cells,definitions,masks,102,84).ground);
    camera.mode=0;cells[cell]={1,0x8004,0x40};
    require(!pickWorldGround(8,9,3,camera,{},1,false,cells,definitions,masks,102,84).ground);
    require(pickWorldGround(8,9,3,camera,{},1,true,cells,definitions,masks,102,84).ground.has_value());
    // Cut level 3 suppresses layer1 when concealed, but permits layer2.
    cells[cell]={1,0x8004,0};camera.view=1;camera.cutLevel=1;
    require(!pickWorldGround(8,9,3,camera,{},1,false,cells,definitions,masks,102,84).ground);
    camera.view=0;camera.cutLevel=1;
    require(pickWorldGround(8,9,3,camera,{},1,false,cells,definitions,masks,102,84).ground.has_value());
    // Rotated original carry: physical layer1, annotation layer0 and column==width.
    TerrainCamera carry;carry.view=2;carry.column=carry.row=0;carry.span=3;
    carry.diagonal=0;carry.f41=0;carry.f11=100;carry.f15=64;carry.cutLevel=1;
    const auto visits=traverseTerrain(4,5,3,carry,{},[](auto,auto,auto){return std::array<std::uint16_t,2>{0,0};});
    require(visits[3].cell==20 && visits[3].column==4 && visits[3].layer==0);
    std::vector<GroundTile> alias(4*5*3);alias[20]={1,0x8004,0};
    auto aliasDefinitions=definitions;aliasDefinitions[1].second.fill(0);
    const auto picked=pickWorldGround(4,5,3,carry,{},1,false,alias,aliasDefinitions,masks,visits[3].anchorX,visits[3].anchorY);
    require(picked.ground && picked.ground->column==4 && picked.ground->row==4 && picked.ground->layer==0);
    require(classifyWorldHit({5,4,3,true,true,true})==WorldHitClass::actor);
    require(classifyWorldHit({4,5,3,true,true,true})==WorldHitClass::artifact);
    require(classifyWorldHit({5,5,3,true,true,true})==WorldHitClass::ground);
    require(classifyWorldHit({5,4,5,true,true,true})==WorldHitClass::ground);
    require(classifyWorldHit({5,4,3,false,true,true})==WorldHitClass::ground);
    require(classifyWorldHit({5,5,3,true,true,false})==WorldHitClass::none);
    require(classifyWorldHit({lo,hi,0,true,true,false})==WorldHitClass::artifact);
    const auto before=cells;
    refuses([&]{pickWorldGround(0,9,3,camera,{},1,false,cells,definitions,masks,0,0);});
    refuses([&]{pickWorldGround(8,9,3,camera,{},4,false,cells,definitions,masks,0,0);});
    auto invalid=cells;invalid[0].definition=2;
    refuses([&]{pickWorldGround(8,9,3,camera,{},1,false,invalid,definitions,masks,0,0);});
    auto badCamera=camera;badCamera.view=4;
    refuses([&]{pickWorldGround(8,9,3,badCamera,{},1,false,cells,definitions,masks,0,0);});
    refuses([&]{pickWorldGround(8,9,3,camera,{},1,false,cells,definitions,{},0,0);});
    for(unsigned n=0;n<cells.size();++n)require(cells[n].definition==before[n].definition && cells[n].flags8==before[n].flags8 && cells[n].flags10==before[n].flags10);
    std::cout<<"Owned mask edges/wrap, suppressed child, four-view Z, cut/fog, physical alias, class ties and "<<refused<<" atomic refusals pass\n";
}
