#include "scene_history.hpp"
#include <limits>
#include <stdexcept>

namespace mnm::render {
SceneHistory::SceneHistory(GlBlitter& renderer,assets::ResourceManager& resources,const Image& background,SceneLimits limits)
    :scene_(renderer,resources,background,limits),drawLimit_(limits.draws){}
void SceneHistory::beginFrame(CanvasStamp stamp,const std::vector<SceneDraw>& draws,bool reset){
    if(!stamp.canvas||!stamp.sequence)throw std::invalid_argument("Canvas identity and source sequence must be nonzero");
    if(drawing_)throw std::runtime_error("Previous history frame is incomplete");
    if(stamp.canvas==admitted_.canvas&&stamp.sequence<=admitted_.sequence)throw std::invalid_argument("Canvas source sequence regressed");
    if(!reset&&(poisoned_||stamp.canvas!=completed_.canvas||
        completed_.sequence==std::numeric_limits<std::uint64_t>::max()||stamp.sequence!=completed_.sequence+1))
        throw std::invalid_argument("Native history requires a complete contiguous canvas sequence or an explicit native reset");
    scene_.beginFrame(draws,reset?SceneStart::background:SceneStart::retainedCanvas);
    admitted_=stamp;drawing_=true;
}
bool SceneHistory::drawNext(std::size_t budget){
    return drawNext(SceneBatchBudget{budget,std::numeric_limits<std::size_t>::max(),std::chrono::hours(1)});
}
bool SceneHistory::drawNext(const SceneBatchBudget& budget){
    if(!drawing_)throw std::runtime_error("No admitted history frame");
    if(!budget.draws||budget.draws>drawLimit_||budget.uploadBytes<8192||budget.time.count()<=0)throw std::invalid_argument("Invalid scene draw batch budget");
    // Invalid batch budgets do not poison the canvas; execution failures do.
    try{if(!scene_.drawNext(budget))return false;}
    catch(...){drawing_=false;poisoned_=true;throw;}
    completed_=admitted_;drawing_=false;poisoned_=false;return true;
}
void SceneHistory::supplyUpload(const SceneUploadNeed& need,PreparedSpriteFrame planes){
    if(!drawing_)throw std::runtime_error("No admitted history frame");
    try{scene_.supplyUpload(need,std::move(planes));}catch(...){drawing_=false;poisoned_=true;throw;}
}
Image SceneHistory::read(){return scene_.read();}
GpuFrame SceneHistory::presentGpu(){return scene_.presentGpu();}
}
