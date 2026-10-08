#include "world_preparation.hpp"
#include "../../protocols/include/mnm/world_frame_v1.h"
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace mnm::legacy {
namespace {
using Clock=std::chrono::steady_clock;
std::uint64_t milliseconds(Clock::time_point start){return std::uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-start).count());}
struct Candidate {BoundFrame binding;QByteArray visual;};
struct FileIndex {
    assets::ResourceId id;
    std::size_t frames=0;
    std::map<QByteArray,std::vector<Candidate>> entries;
    std::shared_ptr<const assets::VisualResource> decoded;
};
FileIndex indexResource(const assets::PreparedResource& prepared,const std::function<void()>& check){
    const auto& input=prepared.sourceImage();
    const QByteArray bytes(reinterpret_cast<const char*>(input.data()),qsizetype(input.size()));
    const auto* sprite=std::get_if<assets::Sprite>(&prepared.resource().image);
    if(!sprite||sprite->version!=4)throw std::runtime_error("World preparation requires v4 SPR");
    FileIndex result{prepared.id(),sprite->frames.size(),{},std::make_shared<const assets::VisualResource>(prepared.resource())};
    for(std::size_t i=0;i<sprite->frames.size();++i){
        check();const auto& frame=sprite->frames[i];
        result.entries[frameIdentity(bytes.mid(frame.sourceOffset,frame.encodedSize),sprite->storage==assets::SpriteStorage::indexed8)]
            .push_back({{prepared.id(),i},spriteVisualIdentity(*sprite,frame)});
    }
    return result;
}
BoundFrame resolve(const SnapshotFrame& frame,const std::vector<FileIndex>& indexes){
    const auto identity=frameIdentity(frame.encoded,frame.indexed);
    const Candidate* first=nullptr;
    for(const auto& index:indexes){const auto it=index.entries.find(identity);if(it==index.entries.end())continue;
        for(const auto& candidate:it->second){
            if(!first)first=&candidate;
            if(candidate.visual!=first->visual&&!frame.indexed)
                throw std::runtime_error("Ambiguous observed frame with different native palette pixels");
        }
    }
    if(!first)throw std::runtime_error("Unmapped complete prepared World frame");
    return first->binding;
}
}
struct WorldPreparation::State {
    struct Job {bool admission=false;QByteArray inputs;std::vector<assets::ResourcePreparation> requests;std::uint64_t bytes=0;std::size_t resources=0;std::optional<render::SceneUploadNeed> upload;};
    explicit State(assets::AssetStore input,WorldPreparationCheckpoint hook):store(std::move(input)),checkpoint(std::move(hook)){}
    assets::AssetStore store;
    WorldPreparationCheckpoint checkpoint;
    mutable std::mutex mutex;
    std::condition_variable ready;
    std::atomic<bool> cancelled{false},finished{false};
    bool busy=false;
    std::optional<Job> job;
    std::optional<WorldPreparationReply> reply;
    WorldPreparationStats stats;
};
WorldPreparation::WorldPreparation(assets::AssetStore store,WorldPreparationCheckpoint hook):state_(std::make_shared<State>(std::move(store),std::move(hook))){
    std::thread([state=state_]{run(state);}).detach();
}
WorldPreparation::~WorldPreparation(){cancel();}
void WorldPreparation::plan(QByteArray inputs){
    if(inputs.size()<MNM_WORLD_HEADER||inputs.size()>MNM_WORLD_MAX_BYTES)throw std::invalid_argument("World preparation input extent");
    std::lock_guard<std::mutex> lock(state_->mutex);
    if(state_->cancelled||state_->finished||state_->busy)throw std::runtime_error("World preparation is closed or busy");
    state_->busy=true;state_->job=State::Job{false,std::move(inputs),{},0,0,{}};state_->ready.notify_one();
}
void WorldPreparation::prepare(std::vector<assets::ResourcePreparation> requests,std::uint64_t bytes,std::size_t resources){
    if(requests.size()>64||resources>64||bytes>128ULL*1024*1024)throw std::invalid_argument("World preparation completion limits");
    std::lock_guard<std::mutex> lock(state_->mutex);
    if(state_->cancelled||state_->finished||state_->busy)throw std::runtime_error("World preparation is closed or busy");
    state_->busy=true;state_->job=State::Job{true,{},std::move(requests),bytes,resources,{}};state_->ready.notify_one();
}
void WorldPreparation::upload(render::SceneUploadNeed need){
    std::lock_guard<std::mutex> lock(state_->mutex);
    if(state_->cancelled||state_->finished||state_->busy)throw std::runtime_error("World preparation is closed or busy");
    state_->busy=true;state_->job=State::Job{true,{},{},0,0,std::move(need)};state_->ready.notify_one();
}
std::optional<WorldPreparationReply> WorldPreparation::take(){
    std::lock_guard<std::mutex> lock(state_->mutex);
    if(state_->cancelled||!state_->reply)return {};
    auto reply=std::move(state_->reply);state_->reply.reset();state_->busy=false;return reply;
}
WorldPreparationStats WorldPreparation::stats() const{std::lock_guard<std::mutex> lock(state_->mutex);return state_->stats;}
void WorldPreparation::cancel(){state_->cancelled=true;state_->ready.notify_one();}
bool WorldPreparation::stopped() const{return state_->finished;}
void WorldPreparation::run(const std::shared_ptr<State>& state){
    const auto check=[&]{if(state->cancelled)throw std::runtime_error("World preparation cancelled");};
    const auto publish=[&](WorldPreparationReply reply){std::lock_guard<std::mutex> lock(state->mutex);if(!state->cancelled)state->reply=std::move(reply);};
    try{
        if(state->checkpoint)state->checkpoint(state->cancelled);
        check();const auto start=Clock::now();WorldCatalogue catalogue(state->store,check);
        {std::lock_guard<std::mutex> lock(state->mutex);state->stats.catalogueMs=milliseconds(start);}
        std::optional<WorldPreparationPlan> plan;
        std::vector<FileIndex> indexes;std::size_t indexedFrames=0;std::uint64_t retainedBytes=0;
        std::shared_ptr<const WorldFrame> uploadFrame;std::vector<render::SceneDraw> uploadDraws;
        while(!state->cancelled){
            State::Job job;
            {std::unique_lock<std::mutex> lock(state->mutex);state->ready.wait(lock,[&]{return state->cancelled||state->job.has_value();});
                if(state->cancelled)break;
                job=std::move(*state->job);state->job.reset();}
            check();
            if(job.upload){
                const auto need=*job.upload;const auto& req=need.request;
                if(!uploadFrame||need.draw>=uploadDraws.size())throw std::runtime_error("Upload without prepared World draw");
                const auto& draw=uploadDraws[need.draw];
                if(!(draw.resource==req.id)||draw.frame!=req.frame||(!req.shadow&&draw.colours!=req.colours))throw std::runtime_error("World upload identity differs");
                const auto source=std::find_if(indexes.begin(),indexes.end(),[&](const auto& item){return item.id==req.id;});
                if(source==indexes.end())throw std::runtime_error("World upload source is absent");
                if(state->checkpoint)state->checkpoint(state->cancelled);
                check();const auto uploadStart=Clock::now();
                auto planes=render::prepareResourceUpload(*source->decoded,req,check);
                {std::lock_guard<std::mutex> lock(state->mutex);state->stats.uploadPrepareMs+=milliseconds(uploadStart);++state->stats.uploadsPrepared;}
                check();publish(PreparedWorldUpload{uploadFrame,need,std::move(planes)});continue;
            }
            if(!job.admission){
                plan=WorldPreparationPlan{std::make_shared<const WorldFrame>(decodeWorldFrame(job.inputs)),{}};
                plan->files=catalogue.needed(*plan->frame);check();publish(*plan);continue;
            }
            if(!plan)throw std::runtime_error("World assets submitted without a complete plan");
            if(job.requests.size()>job.resources)throw std::runtime_error("Decoded resource count budget exceeded");
            const auto prepareStart=Clock::now();PreparedWorld result{plan->frame,{},{}};
            std::uint64_t bytes=0;
            std::set<assets::ResourceId> requested;
            for(const auto& request:job.requests){
                check();if(!requested.insert(request.id()).second)throw std::runtime_error("Duplicate World preparation request");
                const auto file=std::find_if(plan->files.begin(),plan->files.end(),[&](const auto& f){return f.id==request.id();});
                if(file==plan->files.end())throw std::runtime_error("Unrequested World resource preparation");
                if(state->checkpoint)state->checkpoint(state->cancelled);
                auto prepared=assets::prepareResource(request,check,file->sha.toStdString());
                if(prepared.resource().decodedBytes>job.bytes-bytes)throw std::runtime_error("Decoded resource byte budget exceeded");
                bytes+=prepared.resource().decodedBytes;
                auto index=indexResource(prepared,check);
                const auto old=std::find_if(indexes.begin(),indexes.end(),[&](const auto& item){return item.id==request.id();});
                const auto oldBytes=old==indexes.end()?0:old->decoded->decodedBytes;
                if(indexes.size()+(old==indexes.end()?1:0)>64||retainedBytes-oldBytes+prepared.resource().decodedBytes>128ULL*1024*1024)
                    throw std::runtime_error("World retained upload source budget exceeded");
                retainedBytes=retainedBytes-oldBytes+prepared.resource().decodedBytes;
                const auto oldFrames=old==indexes.end()?0:old->frames;
                if(indexedFrames-oldFrames+index.frames>65536)throw std::runtime_error("World indexed frame budget exceeded");
                indexedFrames=indexedFrames-oldFrames+index.frames;
                if(old==indexes.end())indexes.push_back(std::move(index));else *old=std::move(index);
                prepared.discardSource();result.resources.push_back(std::move(prepared));
            }
            result.draws.reserve(plan->frame->draws.size());
            for(const auto& draw:plan->frame->draws){check();if((draw.additive||draw.colourRectangle)){result.draws.push_back(worldPrimitiveDraw(draw));continue;}const auto bound=resolve(draw.frame,indexes);
                result.draws.push_back({bound.resource,bound.frame,draw.x,draw.y,true,true,draw.colours,draw.clip,draw.composite});}
            {std::lock_guard<std::mutex> lock(state->mutex);state->stats.prepareMs+=milliseconds(prepareStart);state->stats.resourcesPrepared+=result.resources.size();state->stats.retainedDecodedBytes=retainedBytes;}
            uploadFrame=result.frame;uploadDraws=result.draws;check();publish(std::move(result));plan.reset();
        }
    }catch(const std::exception& error){publish(std::string(error.what()));}
    catch(...){publish(std::string("Unknown World preparation failure"));}
    state->finished=true;
}
}
