#include "sprite_atlas.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace mnm::render {
SpriteAtlas::Key SpriteAtlas::key(const ResourceUploadRequest& r){
    return {int(r.id.kind),r.id.name,r.revision,r.frame,r.shadow?r.shadow->first:-1,r.shadow?r.shadow->last:-1,r.indexed,r.indexed?std::nullopt:r.colours};
}
SpriteAtlas::SpriteAtlas(GlBlitter& renderer,std::uint64_t pixels,std::size_t frames)
    :renderer_(renderer),pixelLimit_(pixels),frameLimit_(frames){
    if(pixels<2||pixels>16777216||!frames||frames>16384)throw std::invalid_argument("Invalid sprite atlas budgets");
}
SpriteAtlas::~SpriteAtlas(){clear();}
bool SpriteAtlas::ready(const ResourceUploadRequest& r) const{
    const auto it=entries_.find(key(r));return it!=entries_.end()&&!it->second.pending;
}
void SpriteAtlas::erasePage(Page* page){
    for(auto it=entries_.begin();it!=entries_.end();)if(it->second.page==page){it=entries_.erase(it);--stats_.frames;++stats_.evictions;}else ++it;
    for(auto it=pages_.begin();it!=pages_.end();++it)if(&*it==page){
        renderer_.destroy(it->mask);renderer_.destroy(it->pixels);
        stats_.pixels-=std::uint64_t(it->size)*it->size*2;stats_.surfaces-=2;pages_.erase(it);return;
    }
}
void SpriteAtlas::evict(){
    if(pages_.empty()){
        if(entries_.empty())throw std::runtime_error("No atlas space to evict");
        entries_.erase(entries_.begin());--stats_.frames;++stats_.evictions;return;
    }
    erasePage(&*std::min_element(pages_.begin(),pages_.end(),[](const auto& a,const auto& b){return a.used<b.used;}));
}
void SpriteAtlas::supply(const ResourceUploadRequest& r,PreparedSpriteFrame prepared){
    const auto& p=prepared.pixels;const auto& m=prepared.mask;
    if(p.width<0||p.height<0||p.width>2048||p.height>2048||(p.width==0)!=(p.height==0)||p.width!=m.width||p.height!=m.height||
       p.pixels.size()!=std::size_t(p.width)*p.height||m.pixels.size()!=p.pixels.size()||bool(prepared.palette)!=r.indexed)
        throw std::invalid_argument("Invalid prepared atlas planes");
    int minimumSide=1;while(minimumSide<std::max(p.width,p.height))minimumSide*=2;
    if(p.width&&std::uint64_t(minimumSide)*minimumSide*2>pixelLimit_)throw std::runtime_error("Atlas frame exceeds page budget");
    const auto k=key(r);if(entries_.count(k))throw std::runtime_error("Duplicate atlas completion");
    // Stale revisions are retired before reusing their positions.
    for(auto it=entries_.begin();it!=entries_.end();){
        if(std::get<0>(it->first)==int(r.id.kind)&&std::get<1>(it->first)==r.id.name&&std::get<2>(it->first)!=r.revision){
            if(it->second.page)--it->second.page->entries;
            it=entries_.erase(it);--stats_.frames;++stats_.evictions;
        }else ++it;
    }
    for(auto it=pages_.begin();it!=pages_.end();){auto* page=&*it++;if(!page->entries)erasePage(page);}
    while(stats_.frames>=frameLimit_)evict();
    Entry entry;entry.originX=prepared.originX;entry.originY=prepared.originY;entry.palette=prepared.palette;
    if(p.width){
        Page* selected=nullptr;int x=0,y=0;
        for(auto& page:pages_){
            int px=page.x,py=page.y;if(px+p.width>page.size){px=0;py+=page.rowHeight;}
            if(px+p.width<=page.size&&py+p.height<=page.size){selected=&page;x=px;y=py;break;}
        }
        if(!selected){
            int size=1024;while(size<std::max(p.width,p.height))size*=2;
            // Small test/host budgets can choose smaller pages, without relaxing bounds.
            while(std::uint64_t(size)*size*2>pixelLimit_&&size/2>=std::max(p.width,p.height))size/=2;
            const auto pixels=std::uint64_t(size)*size*2;const auto global=renderer_.stats();
            if(pixels>pixelLimit_||global.surfaces-stats_.surfaces+2>64||global.pixels-stats_.pixels+pixels>16777216)
                throw std::runtime_error("Atlas page exceeds available storage");
            while(stats_.pixels+pixels>pixelLimit_||renderer_.stats().surfaces+2>64||renderer_.stats().pixels+pixels>16777216)evict();
            const auto colour=renderer_.allocate(size,size,spriteFormat);SurfaceId mask=0;
            try{mask=renderer_.allocate(size,size,{8,{}});pages_.push_back({colour,mask,size,0,0,0,++clock_,0});}
            catch(...){if(mask)renderer_.destroy(mask);renderer_.destroy(colour);throw;}
            selected=&pages_.back();stats_.pixels+=pixels;stats_.surfaces+=2;
        }
        entry.page=selected;entry.region={x,y,x+p.width,y+p.height};entry.pending=std::move(prepared);
        if(y!=selected->y)selected->rowHeight=0;
        selected->x=x+p.width;selected->y=y;selected->rowHeight=std::max(selected->rowHeight,entry.region.bottom-y);++selected->entries;
    }
    entries_.emplace(k,std::move(entry));++stats_.frames;++stats_.uploads;
}
bool SpriteAtlas::advance(const ResourceUploadRequest& r,std::size_t& bytes){
    const auto it=entries_.find(key(r));if(it==entries_.end())return false;
    auto& e=it->second;if(!e.pending)return true;
    const auto& plane=e.plane?e.pending->mask:e.pending->pixels;
    const auto rowBytes=std::size_t(plane.width)*sizeof(std::uint32_t);
    const auto rows=int(std::min(bytes/rowBytes,std::size_t(plane.height-e.row)));if(!rows)return false;
    const auto first=plane.pixels.begin()+std::size_t(e.row)*plane.width;
    const auto maximum=e.plane?1u:r.indexed?255u:65535u;
    if(std::any_of(first,first+std::size_t(rows)*plane.width,[maximum](auto value){return value>maximum;}))throw std::runtime_error("Atlas pixel exceeds plane format");
    renderer_.updateRegionRows(e.plane?e.page->mask:e.page->pixels,e.region.left,e.region.top,plane,e.row,rows);
    bytes-=std::size_t(rows)*rowBytes;e.row+=rows;
    if(e.row==plane.height){e.row=0;++e.plane;}
    if(e.plane==2)e.pending.reset();
    return !e.pending;
}
void SpriteAtlas::draw(const ResourceUploadRequest& r,SurfaceId destination,int anchorX,int anchorY,std::optional<Rect> viewport,const SpriteComposite& composite){
    const auto it=entries_.find(key(r));if(it==entries_.end()||it->second.pending)throw std::runtime_error("Atlas frame not ready");
    auto& e=it->second;++stats_.hits;if(!e.page)return;e.page->used=++clock_;
    const auto x=std::int64_t(anchorX)-e.originX,y=std::int64_t(anchorY)-e.originY;
    const auto w=e.region.right-e.region.left,h=e.region.bottom-e.region.top;
    std::int64_t left=x,top=y,right=x+w,bottom=y+h;
    if(viewport){if(viewport->left<0||viewport->top<0||viewport->right<=viewport->left||viewport->bottom<=viewport->top)throw std::invalid_argument("Invalid atlas viewport");
        left=std::max<std::int64_t>(left,viewport->left);top=std::max<std::int64_t>(top,viewport->top);right=std::min<std::int64_t>(right,viewport->right);bottom=std::min<std::int64_t>(bottom,viewport->bottom);}
    if(left>=right||top>=bottom)return;
    if(left<0||top<0||right>INT32_MAX||bottom>INT32_MAX)throw std::runtime_error("Atlas placement outside coordinates");
    const Rect source{int(e.region.left+left-x),int(e.region.top+top-y),int(e.region.left+right-x),int(e.region.top+bottom-y)};
    const auto* palette=e.palette?(r.colours?&*r.colours:&*e.palette):nullptr;
    renderer_.composite(e.page->pixels,destination,source,int(left),int(top),e.page->mask,composite,palette);
}
void SpriteAtlas::release(const assets::ResourceId& id){
    for(auto it=entries_.begin();it!=entries_.end();)if(std::get<0>(it->first)==int(id.kind)&&std::get<1>(it->first)==id.name){
        if(it->second.page)--it->second.page->entries;
        it=entries_.erase(it);--stats_.frames;++stats_.evictions;
    }else ++it;
    for(auto it=pages_.begin();it!=pages_.end();){auto* page=&*it++;if(!page->entries)erasePage(page);}
}
void SpriteAtlas::clear(){while(!pages_.empty())erasePage(&pages_.front());stats_.evictions+=entries_.size();entries_.clear();stats_.frames=0;}
}
