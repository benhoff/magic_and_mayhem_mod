#include "cursor_compositor.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <limits>
#include <mutex>
#include <stdexcept>

namespace mnm::render {
struct CursorCompositor::Shared {
    struct Admission {bool accepting=true;std::size_t pending=0;int width,height,cellWidth,cellHeight;};
    struct Job {Token token=0;Frame frame;unsigned priority=0;std::uint64_t sequence=0;};
    std::mutex mutex;
    bool closed=false;
    std::uint64_t nextSequence=1;
    std::unordered_map<Token,Admission> generations;
    std::array<Job,128> queue{};
};
struct CursorCompositor::Generation {
    SurfaceBackend* backend=nullptr;
    SurfaceId canvas=0,atlas=0,saved=0;
    ~Generation(){for(auto id:{saved,atlas,canvas})if(id)backend->release(id);}
    int width,height,cellWidth,cellHeight;
    bool drawn=false;
    Rect previous{};
    Frame frame{{},0,0,false};
    std::uint32_t key=31;
    Image recoveryCanvas,recoveryAtlas,recoverySaved;
};
namespace {
std::atomic<std::uint64_t> nextCursorToken{1};
const PixelFormat cursorFormat{16,{0xf800,0x7e0,31}};
void checkedCopy(SurfaceBackend& b,SurfaceId s,SurfaceId d,Rect sr,Rect dr,bool keyed=false){
    SurfaceCopyRequest request;request.source=sr;request.destination=dr;
    request.flags=keyed?0x8000:0;
    if(b.copy(s,d,request).hresult)throw std::runtime_error("Cursor surface copy failed");
}
}
CursorCompositor::Publication CursorCompositor::Publisher::publish(Token token,const Frame& frame,unsigned priority) const {
    if(!shared_)return Publication::closed;
    std::lock_guard<std::mutex> lock(shared_->mutex);
    if(shared_->closed)return Publication::closed;
    const auto it=shared_->generations.find(token);
    if(it==shared_->generations.end() || !it->second.accepting)return Publication::retired;
    const auto& a=it->second;
    if(priority>2 || (frame.visible && (frame.atlas.left<0 || frame.atlas.top<0 || frame.atlas.right<=frame.atlas.left || frame.atlas.bottom<=frame.atlas.top ||
        frame.atlas.right>a.width || frame.atlas.bottom>a.height ||
        frame.atlas.right-frame.atlas.left>a.cellWidth || frame.atlas.bottom-frame.atlas.top>a.cellHeight)))return Publication::invalid;
    for(auto& job:shared_->queue)if(!job.token){if(shared_->nextSequence==std::numeric_limits<std::uint64_t>::max())std::terminate();job={token,frame,priority,shared_->nextSequence++};++it->second.pending;return Publication::accepted;}
    return Publication::full;
}
void CursorCompositor::Publisher::stop() const {
    if(!shared_)return;
    std::lock_guard<std::mutex> lock(shared_->mutex);
    shared_->closed=true;
    for(auto& job:shared_->queue)if(job.token){--shared_->generations.at(job.token).pending;job.token=0;}
}
CursorCompositor::CursorCompositor():owner_(std::this_thread::get_id()),shared_(std::make_shared<Shared>()),backend_(std::make_unique<SurfaceBackend>()){}
CursorCompositor::~CursorCompositor(){if(std::this_thread::get_id()!=owner_)std::terminate();shutdown();}
void CursorCompositor::owner() const {if(std::this_thread::get_id()!=owner_)throw std::runtime_error("Cursor compositor requires its owner thread");}
void CursorCompositor::graphics() const {owner();if(!backend_)throw std::runtime_error("Cursor graphics suspended");}
CursorCompositor::Publisher CursorCompositor::publisher() const {return Publisher(shared_);}
CursorCompositor::Token CursorCompositor::create(const Image& canvas,const Image& atlas,int cw,int ch,std::uint32_t key){
    graphics();
    if(cw<1 || ch<1 || cw>4096 || ch>4096 || atlas.width<1 || atlas.height<1 || canvas.width<1 || canvas.height<1 ||
       canvas.width>4096 || canvas.height>4096 || atlas.width>4096 || atlas.height>4096 || key>65535)throw std::invalid_argument("Invalid cursor allocation");
    {std::lock_guard<std::mutex> lock(shared_->mutex);if(shared_->closed)throw std::runtime_error("Cursor compositor closed");}
    if(generations_.size()>=8)throw std::runtime_error("Cursor generation budget exhausted");
    auto g=std::make_unique<Generation>();g->backend=backend_.get();g->width=canvas.width;g->height=canvas.height;g->cellWidth=cw;g->cellHeight=ch;g->key=key;
    Token token=nextCursorToken.fetch_add(1);
    if(!token || token==std::numeric_limits<Token>::max())std::terminate();
    try {
        g->canvas=backend_->create(canvas,cursorFormat);g->atlas=backend_->create(atlas,cursorFormat);
        backend_->setSourceKey(g->atlas,key);
        g->saved=backend_->create({cw,ch,std::vector<std::uint32_t>(std::size_t(cw)*ch,0)},cursorFormat);
        // Install owned resources before publication becomes visible. Owner-only
        // map entries are never erased by publisher threads.
        generations_.emplace(token,std::move(g));
        std::lock_guard<std::mutex> lock(shared_->mutex);
        if(shared_->closed){auto& installed=*generations_.at(token);release(installed);generations_.erase(token);throw std::runtime_error("Cursor compositor closed during allocation");}
        shared_->generations.emplace(token,Shared::Admission{true,0,atlas.width,atlas.height,cw,ch});
    } catch(...) {
        if(g)release(*g);
        const auto it=generations_.find(token);
        if(it!=generations_.end()){release(*it->second);generations_.erase(it);}
        throw;
    }
    checkpointCurrent_=false;return token;
}
void CursorCompositor::release(Generation& g){
    for(auto* id:{&g.saved,&g.atlas,&g.canvas})if(*id){backend_->release(*id);*id=0;}
}
CursorCompositor::Generation& CursorCompositor::generation(Token token){
    const auto it=generations_.find(token);if(it==generations_.end())throw std::invalid_argument("Unknown cursor generation");return *it->second;
}
void CursorCompositor::retire(Token token){owner();std::lock_guard<std::mutex> lock(shared_->mutex);auto it=shared_->generations.find(token);if(it==shared_->generations.end())throw std::invalid_argument("Unknown cursor generation");it->second.accepting=false;}
std::size_t CursorCompositor::collect(){
    graphics();std::lock_guard<std::mutex> lock(shared_->mutex);std::size_t count=0;
    for(auto it=shared_->generations.begin();it!=shared_->generations.end();){
        if(!it->second.accepting && !it->second.pending){auto owned=generations_.find(it->first);release(*owned->second);generations_.erase(owned);it=shared_->generations.erase(it);++count;}else ++it;
    }
    if(count)checkpointCurrent_=false;
    return count;
}
void CursorCompositor::restore(Generation& g){
    if(g.drawn){checkedCopy(*backend_,g.saved,g.canvas,{0,0,g.previous.right-g.previous.left,g.previous.bottom-g.previous.top},g.previous);g.drawn=false;}
}
void CursorCompositor::paint(Generation& g,const Frame& frame){
    // Validate before restoring so an invalid frame leaves all pixels unchanged.
    if(frame.visible && (frame.atlas.right-frame.atlas.left>g.cellWidth || frame.atlas.bottom-frame.atlas.top>g.cellHeight))throw std::invalid_argument("Cursor frame exceeds saved background");
    restore(g);g.frame=frame;
    if(!frame.visible)return;
    const auto left=std::max<std::int64_t>(0,frame.x),top=std::max<std::int64_t>(0,frame.y);
    const auto right=std::min<std::int64_t>(g.width,std::int64_t(frame.x)+frame.atlas.right-frame.atlas.left),bottom=std::min<std::int64_t>(g.height,std::int64_t(frame.y)+frame.atlas.bottom-frame.atlas.top);
    if(left>=right || top>=bottom)return;
    Rect destination{int(left),int(top),int(right),int(bottom)};
    Rect source{int(frame.atlas.left+left-frame.x),int(frame.atlas.top+top-frame.y),int(frame.atlas.left+right-frame.x),int(frame.atlas.top+bottom-frame.y)};
    checkedCopy(*backend_,g.canvas,g.saved,destination,{0,0,destination.right-destination.left,destination.bottom-destination.top});
    // Publish restoration metadata before the possibly throwing draw. A failed
    // draw can always be undone by the next frame or background refresh.
    g.previous=destination;g.drawn=true;
    checkedCopy(*backend_,g.atlas,g.canvas,source,destination,true);
}
std::size_t CursorCompositor::drain(){
    graphics();std::size_t count=0;std::uint64_t cutoff;
    {std::lock_guard<std::mutex> lock(shared_->mutex);cutoff=shared_->nextSequence-1;}
    // A bounded snapshot by publication sequence. New publications remain for
    // the next drain; stop can still cancel every job not yet executing.
    for(unsigned priority=0;priority<3;++priority)for(;;){
        Shared::Job job;
        {std::lock_guard<std::mutex> lock(shared_->mutex);Shared::Job* selected=nullptr;
            for(auto& queued:shared_->queue)if(queued.token && queued.priority==priority && queued.sequence<=cutoff && (!selected || queued.sequence<selected->sequence))selected=&queued;
            if(!selected)break;
            job=*selected;selected->token=0;}
        checkpointCurrent_=false;
        try {paint(generation(job.token),job.frame);}catch(...){std::lock_guard<std::mutex> lock(shared_->mutex);--shared_->generations.at(job.token).pending;throw;}
        {std::lock_guard<std::mutex> lock(shared_->mutex);--shared_->generations.at(job.token).pending;}
        ++count;
    }
    return count;
}
Image CursorCompositor::snapshot(Token token){graphics();return backend_->read(generation(token).canvas);}
void CursorCompositor::background(Token token,const Image& image){
    graphics();auto& g=generation(token);
    if(image.width!=g.width || image.height!=g.height || image.pixels.size()!=std::size_t(g.width)*g.height)throw std::invalid_argument("Cursor background dimensions changed");
    checkpointCurrent_=false;const auto frame=g.frame;restore(g);backend_->update(g.canvas,0,0,image);paint(g,frame);
}
void CursorCompositor::checkpointGraphics(){
    graphics();checkpointCurrent_=false;
    // Commit validity only after every read succeeds; a partial checkpoint
    // never authorizes destroying the live context.
    for(auto& item:generations_){auto& g=*item.second;
        g.recoveryCanvas=backend_->read(g.canvas);g.recoveryAtlas=backend_->read(g.atlas);g.recoverySaved=backend_->read(g.saved);
    }
    checkpointCurrent_=true;
}
void CursorCompositor::suspendGraphics(){
    graphics();if(!checkpointCurrent_)throw std::runtime_error("Cursor checkpoint absent or stale");
    // SurfaceBackend owns all current allocations. Zero per-generation handles
    // before actually destroying it; no later destructor can call the old owner.
    for(auto& item:generations_){auto& g=*item.second;g.canvas=g.atlas=g.saved=0;g.backend=nullptr;}
    backend_.reset();
}
void CursorCompositor::recoverGraphics(){
    owner();if(backend_ || !checkpointCurrent_)throw std::runtime_error("Cursor recovery state invalid");
    auto fresh=std::make_unique<SurfaceBackend>();
    std::unordered_map<Token,std::unique_ptr<Generation>> staged;
    // All allocations are staged against a live fresh backend. Failure leaves
    // both queued metadata and the immutable CPU checkpoint available to retry.
    for(auto& item:generations_){const auto& old=*item.second;auto g=std::make_unique<Generation>();g->backend=fresh.get();
        g->canvas=fresh->create(old.recoveryCanvas,cursorFormat);g->atlas=fresh->create(old.recoveryAtlas,cursorFormat);
        fresh->setSourceKey(g->atlas,old.key);g->saved=fresh->create(old.recoverySaved,cursorFormat);
        staged.emplace(item.first,std::move(g));
    }
    for(auto& item:generations_){auto& g=*item.second;auto& replacement=*staged.at(item.first);
        g.canvas=replacement.canvas;g.atlas=replacement.atlas;g.saved=replacement.saved;g.backend=fresh.get();
        replacement.canvas=replacement.atlas=replacement.saved=0;
    }
    backend_=std::move(fresh);
}
bool CursorCompositor::graphicsSuspended() const {owner();return !backend_;}
void CursorCompositor::shutdown(){
    owner();Publisher(shared_).stop();
    for(auto& item:generations_)release(*item.second);
    generations_.clear();checkpointCurrent_=false;std::lock_guard<std::mutex> lock(shared_->mutex);shared_->generations.clear();
}
std::size_t CursorCompositor::surfaces() const {owner();return backend_?backend_->stats().surfaces:0;}
}
