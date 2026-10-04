#include "terrain_region.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
FixedRegionPlan planFixedTerrainRegion(const assets::RegionRecipe& recipe,const std::vector<const assets::MapAsset*>& sources){
 if(!recipe.columns || recipe.columns>5 || !recipe.rows || recipe.rows>5 || sources.size()!=recipe.specific.size() || sources.empty())throw std::invalid_argument("Fixed region sources/grid");
 if(!recipe.random.empty())throw std::invalid_argument("Region requires random constraint selection");
 FixedRegionPlan p;p.columns=recipe.columns;p.rows=recipe.rows;std::vector<bool> occupied(p.columns*p.rows);
 for(unsigned i=0;i<sources.size();++i){
  const auto& placement=recipe.specific[i];if(!sources[i])throw std::invalid_argument("Missing fixed region source");const auto& m=*sources[i];
  const auto w=m.metadata[0],h=m.metadata[1];
  if(!w || w>2 || !h || h>2 || !m.width || !m.height || m.width%w || m.height%h || m.width/w!=m.height/h ||
     m.width>128 || m.height>128 || !m.layers || m.layers>32 || m.cells.size()!=std::size_t(m.width)*m.height*m.layers)throw std::invalid_argument("Region section block metadata");
  if(placement.rotation<0 || placement.rotation>3 || placement.column<0 || unsigned(placement.column)>=p.columns || placement.row<0 || unsigned(placement.row)>=p.rows)throw std::invalid_argument("Region requires wildcard placement/rotation");
  const auto side=m.width/w;if(p.side && side!=p.side)throw std::invalid_argument("Region section side disagreement");p.side=side;p.layers=std::max(p.layers,m.layers);
  if(p.columns>128/p.side || p.rows>128/p.side)throw std::out_of_range("Region dimensions");
  for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){
   int dx=int(x),dy=int(y);const auto r=unsigned(placement.rotation);
   if(r==1){dx=-int(y);dy=int(x);}if(r==2){dx=-int(x);dy=-int(y);}if(r==3){dx=int(y);dy=-int(x);}
   const auto column=unsigned((placement.column+dx+int(p.columns))%int(p.columns));const auto row=unsigned((placement.row+dy+int(p.rows))%int(p.rows));
   if(occupied[row*p.columns+column])throw std::invalid_argument("Fixed region sections overlap");
   occupied[row*p.columns+column]=true;
   p.blocks.push_back({i,x*p.side,y*p.side,column,row,r});
  }
 }
 if(std::find(occupied.begin(),occupied.end(),false)!=occupied.end())throw std::invalid_argument("Fixed region has unfilled slots");
 return p;
}
FixedRegionAssembly assembleFixedTerrainRegion(const FixedRegionPlan& p,const std::vector<const assets::MapAsset*>& sources,const assets::TerrainCatalog& t){
 if(!p.columns || p.columns>5 || !p.rows || p.rows>5 || !p.side || p.side>128 || p.columns>128/p.side || p.rows>128/p.side ||
 !p.layers || p.layers>32 || p.blocks.size()!=std::size_t(p.columns)*p.rows)throw std::invalid_argument("Fixed region plan extent");
 FixedRegionAssembly result;auto& m=result.map;m.width=p.columns*p.side;m.height=p.rows*p.side;m.layers=p.layers;
 m.cells.resize(std::size_t(m.width)*m.height*m.layers);for(auto& c:m.cells)c.references.fill(0xffff);
 std::vector<assets::MapAsset> projected;std::size_t total=0;
 for(const auto* source:sources){if(!source)throw std::invalid_argument("Missing region source");total+=source->cells.size();if(total>4u*128*128*32)throw std::out_of_range("Region source storage");projected.push_back(*source);
  for(auto& c:projected.back().cells){result.projectedObjects+=bool(c.flags10&8);c.flags10&=0xfff7;for(auto& ref:c.references){result.projectedReferences+=ref!=0xffff;ref=0xffff;}}
 }
 std::vector<bool> occupied(p.columns*p.rows);
 for(const auto& b:p.blocks){if(b.source>=projected.size() || b.column>=p.columns || b.row>=p.rows || occupied[b.row*p.columns+b.column])throw std::invalid_argument("Region plan coverage");occupied[b.row*p.columns+b.column]=true;
  copyTerrainSection(m,projected[b.source],t,p.side,b.sourceX,b.sourceY,b.column*p.side,b.row*p.side,b.rotation);
 }
 return result;
}
}
