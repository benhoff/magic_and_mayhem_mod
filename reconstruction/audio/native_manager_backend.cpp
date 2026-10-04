#include "native_manager_backend.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
constexpr Status Invalid=-2147024809,Unsupported=-2147467263,Limit=-2147024882,Busy=-2005401430;
}
NativeManagerBackend::NativeManagerBackend(mnm::assets::AssetStore assets,AudioManager& manager,
    std::function<std::uint32_t()> ticks,std::function<std::uint32_t()> random,std::uint32_t rate,NativeSourcePathPolicy sourcePaths)
    :sourcePaths_(sourcePaths),assets_(std::move(assets)),manager_(manager),device_(16*1024*1024,128,rate),ticks_(std::move(ticks)),random_(std::move(random)){
    if(!ticks_ || !random_)throw std::invalid_argument("Native manager requires explicit clock and RNG policies");
}
NativeManagerBackend::~NativeManagerBackend(){device_.stopPrimary();} // Host device/session RAII, not original primary release evidence.
Status NativeManagerBackend::status(mnm::audio::Error e){
    using E=mnm::audio::Error;
    switch(e){case E::ok:return 0;case E::unsupported:return Unsupported;case E::busy:return Busy;case E::limit:return Limit;default:return Invalid;}
}
Status NativeManagerBackend::bind(mnm::audio::Error e,mnm::audio::BufferId id,std::uint32_t& out){
    out=0;if(e!=mnm::audio::Error::ok)return status(e);
    if(!nextBuffer_){device_.release(id);return Limit;}
    out=nextBuffer_++;buffers_.emplace(out,id);return 0;
}
mnm::audio::BufferId NativeManagerBackend::native(std::uint32_t id) const{auto found=buffers_.find(id);return found==buffers_.end()?0:found->second;}
bool NativeManagerBackend::primary(std::uint32_t id) const{auto info=device_.info(native(id));return info && info->primary;}
const mnm::assets::ProfileSnapshot& NativeManagerBackend::profile() const{if(!profile_)throw std::logic_error("No bound profile snapshot");return *profile_;}
const mnm::audio::Wave& NativeManagerBackend::wave() const{if(!wave_)throw std::logic_error("No owned WAV input");return *wave_;}
void NativeManagerBackend::failure(const mnm::assets::Error& error){assetError_=error;diagnostic_=error.operation+": "+error.requestedPath+": "+error.detail;}
bool NativeManagerBackend::openProfile(const std::string& path){
    profile_.reset();diagnostic_.clear();assetError_.reset();auto opened=assets_.open(path);
    if(auto* e=std::get_if<mnm::assets::Error>(&opened)){failure(*e);return false;}
    auto parsed=mnm::assets::loadProfile(*std::get<std::unique_ptr<mnm::assets::AssetFile>>(opened));
    if(auto* e=std::get_if<mnm::assets::Error>(&parsed)){if(e->requestedPath.empty())e->requestedPath=path;failure(*e);return false;}
    profile_=std::get<mnm::assets::ProfileSnapshot>(std::move(parsed));return true;
}
ProfileSection NativeManagerBackend::section(const std::string& name,std::uint32_t capacity){auto decoded=profile().section(name,capacity);return {decoded.returned,std::move(decoded.entries)};}
std::uint32_t NativeManagerBackend::profileValue(std::int32_t sound,std::string& out){out=profile().value("Sounds",std::to_string(sound),260);return std::uint32_t(out.size());}
std::uint32_t NativeManagerBackend::groupValue(std::int32_t group,std::string& out){out=profile().value("Randomised",std::to_string(group),256);return std::uint32_t(out.size());}
std::uint32_t NativeManagerBackend::profileInteger(const std::string& s,const std::string& k,std::uint32_t fallback){return profile().integer(s,k,fallback);}
mnm::assets::Result<std::unique_ptr<mnm::assets::AssetFile>> NativeManagerBackend::openSourceFile(const std::string& path){
    auto opened=assets_.open(path);
    const auto* error=std::get_if<mnm::assets::Error>(&opened);
    if(sourcePaths_!=NativeSourcePathPolicy::dequoteMissingLeaf || !error || error->code!=mnm::assets::ErrorCode::notFound)return opened;
    const auto separator=path.find_last_of("/\\");const auto first=separator==path.npos?0:separator+1;
    if(path.size()<first+7 || path[first]!='\'' || path[path.size()-5]!='\'')return opened;
    auto suffix=path.substr(path.size()-4);for(auto& c:suffix)if(c>='A' && c<='Z')c=char(c-'A'+'a');
    if(suffix!=".wav")return opened;
    auto normalized=path;normalized.erase(normalized.size()-5,1);normalized.erase(first,1);
    return assets_.open(normalized); // Same trusted-root resolver; no directory/alias/fallback substitution.
}
bool NativeManagerBackend::fileSize(const std::string& path,std::uint32_t& bytes){
    auto opened=openSourceFile(path);if(auto* e=std::get_if<mnm::assets::Error>(&opened)){failure(*e);return false;}
    auto size=std::get<std::unique_ptr<mnm::assets::AssetFile>>(opened)->size();
    if(auto* e=std::get_if<mnm::assets::Error>(&size)){failure(*e);return false;}
    const auto n=std::get<std::int64_t>(size);if(n<0 || std::uint64_t(n)>0xffffffffu){diagnostic_="File size exceeds DWORD adapter domain";return false;}
    bytes=std::uint32_t(n);return true;
}
Status NativeManagerBackend::createDevice(const DeviceRequest& request,std::uint32_t& out){
    out=0;if(activeDevice_ || !request.defaultDevice || !request.noAggregation)return Invalid;
    activeDevice_=true;out=1;return 0;
}
Status NativeManagerBackend::cooperativeLevel(std::uint32_t id,std::uint32_t,std::uint32_t level){return activeDevice_ && id==1 && level==2?0:Invalid;}
void NativeManagerBackend::freeAllocation(ManagerAllocation,std::size_t){} // Controller owns host tables/schedules.
Status NativeManagerBackend::create(const Descriptor32& d){
    if(!activeDevice_ || d.size!=20 || d.flags!=0x81 || d.bytes || d.reserved || d.formatAddress){primary_=0;return Invalid;}
    // Native Device owns a singleton primary. Reuse its retained storage on
    // controller restart; this is native policy, not a COM reference-count claim.
    if(primary(primary_))return 0;
    primary_=0;
    mnm::audio::BufferId id=0;const auto error=device_.createPrimary(d.flags,id);return bind(error,id,primary_);
}
Status NativeManagerBackend::deviceCaps(std::uint32_t& flags){if(!activeDevice_)return Invalid;flags=mnm::audio::Device::capabilities;return 0;}
Status NativeManagerBackend::setFormat(const mnm::audio::PcmFormat& f){
    auto e=device_.setPrimaryFormat(native(primary_),f);
    if(e==mnm::audio::Error::ok)e=device_.setPrimaryOutputFormat(f);
    return status(e);
}
Status NativeManagerBackend::bufferBytes(std::uint32_t& bytes){auto info=device_.info(native(primary_));if(!info)return Invalid;bytes=std::uint32_t(info->bytes);return 0;}
Status NativeManagerBackend::compact(){return activeDevice_?0:Invalid;} // Native storage has no hardware compaction.
Status NativeManagerBackend::getVolume(std::uint32_t id,std::int32_t& value){if(!primary(id))return Invalid;value=device_.primaryState().volume;return 0;}
Status NativeManagerBackend::getStatus(std::uint32_t id,std::uint32_t& flags){
    if(primary(id)){flags=device_.primaryState().status();return 0;}
    const auto v=device_.voice(native(id));if(!v)return Invalid;flags=v->status();return 0;
}
Status NativeManagerBackend::stop(std::uint32_t id){if(primary(id)){device_.stopPrimary();return 0;}return status(device_.stop(native(id)));}
Status NativeManagerBackend::position(std::uint32_t id,std::uint32_t byte){return byte?Unsupported:status(device_.resetPosition(native(id)));}
Status NativeManagerBackend::volume(std::uint32_t id,std::int32_t value){return status(primary(id)?device_.setPrimaryVolume(value):device_.setVolume(native(id),value));}
Status NativeManagerBackend::pan(std::uint32_t id,std::int32_t value){return status(device_.setPan(native(id),value));}
Status NativeManagerBackend::play(std::uint32_t id,std::uint32_t a,std::uint32_t b,std::uint32_t flags){if(a || b)return Invalid;return status(primary(id)?device_.playPrimary(flags):device_.play(native(id),flags));}
std::uint32_t NativeManagerBackend::tickCount(){return ticks_();}
std::uint32_t NativeManagerBackend::randomWord(){return random_();}
VoiceWrapper* NativeManagerBackend::wrapper(std::uint32_t id) const{
    auto* head=manager_.sources.head;
    if(head){auto* root=head;do{
        for(auto* v=root;v;v=v->duplicate)if(v->identity==id)return v;
        root=root->next;
    }while(root!=head);}
    if(manager_.ownsDisabled && manager_.sources.disabled.identity==id)return &manager_.sources.disabled;
    return nullptr;
}
std::uint32_t NativeManagerBackend::bufferForVoice(std::uint32_t id){const auto* v=wrapper(id);return v?v->buffer:0;}
void NativeManagerBackend::retireVoice(std::uint32_t id){
    if(auto* v=wrapper(id))mnm::reconstruction::audio::retireVoice(static_cast<ConfigurationBackend&>(*this),*v,false,false,manager_.cache.manager.initialized && manager_.cache.manager.active);
}
void NativeManagerBackend::clearVoiceSchedule(std::uint32_t id){
    auto* head=manager_.schedules.head;if(!head)return;auto* n=head;
    do{if(n->record.voiceAddress==id){retireSchedule(manager_.schedules.head,*n);return;}n=n->next;}while(n!=head);
}
void NativeManagerBackend::volumeRecord(VoiceContract& v,std::int32_t value){
    auto* head=manager_.schedules.head;if(!head)return;auto* n=head;
    do{if(n->record.voiceAddress && bufferForVoice(n->record.voiceAddress)==v.buffer){updateScheduleVolume(manager_.schedules.head,*n,value);return;}n=n->next;}while(n!=head);
}
void NativeManagerBackend::releaseVoice(std::uint32_t id){releaseBuffer(id);}
void NativeManagerBackend::releaseDevice(std::uint32_t id){if(id==1)activeDevice_=false;}
void NativeManagerBackend::releaseBuffer(std::uint32_t id){
    const auto value=native(id);if(value)device_.release(value);buffers_.erase(id);locks_.erase(id);if(activeLock_==id)activeLock_=0;
}
void NativeManagerBackend::freeWrapper(std::uint32_t id){if(id>=65537)disposed_.insert(id);} // Stable until explicit collection.
void NativeManagerBackend::collectDisposed(){
    for(auto id:disposed_)if(wrapper(id))throw std::logic_error("Cannot collect a duplicate still linked into the manager");
    auto& children=manager_.sources.ownedDuplicates;
    children.erase(std::remove_if(children.begin(),children.end(),[&](const auto& p){return disposed_.count(p->identity)!=0;}),children.end());disposed_.clear();
}
Status NativeManagerBackend::loadSource(std::int32_t sound,VoiceWrapper*& out){return loadSourceCache(*this,manager_.cache,manager_.sources.head,&manager_.sources.disabled,sound,&out);}
Status NativeManagerBackend::duplicateBuffer(std::uint32_t source,std::uint32_t& out){mnm::audio::BufferId id=0;const auto e=device_.duplicate(native(source),id);return bind(e,id,out);}
VoiceWrapper& NativeManagerBackend::allocateWrapper(){
    if(!nextWrapper_)throw std::overflow_error("Native wrapper token domain exhausted");
    auto v=std::make_unique<VoiceWrapper>();v->identity=nextWrapper_++;manager_.sources.ownedDuplicates.push_back(std::move(v));return *manager_.sources.ownedDuplicates.back();
}
bool NativeManagerBackend::openWave(const std::string& path){
    wave_.reset();diagnostic_.clear();assetError_.reset();auto opened=openSourceFile(path);
    if(auto* e=std::get_if<mnm::assets::Error>(&opened)){failure(*e);return false;}
    try{wave_=mnm::audio::loadWave(*std::get<std::unique_ptr<mnm::assets::AssetFile>>(opened));return true;}
    catch(const mnm::audio::AssetInputError& e){failure(e.error());}
    catch(const std::runtime_error& e){diagnostic_=path+": "+e.what();}
    return false;
}
void NativeManagerBackend::closeWave(){wave_.reset();}
std::uint32_t NativeManagerBackend::waveBytes(){return std::uint32_t(wave().samples.size());}
mnm::audio::PcmFormat NativeManagerBackend::waveFormat(){return wave().format;}
Status NativeManagerBackend::createSource(const Descriptor32& d,const mnm::audio::PcmFormat& f,std::uint32_t& out){
    out=0;if(!activeDevice_ || d.size!=20 || d.flags!=0xea || d.reserved)return Invalid;
    mnm::audio::BufferId id=0;const auto e=device_.createStatic(d.flags,f,d.bytes,id);return bind(e,id,out);
}
Status NativeManagerBackend::lockSource(std::uint32_t id,std::uint32_t bytes,std::uint8_t*& first){
    if(activeLock_)return Busy;
    mnm::audio::WriteLock lock;auto e=device_.lock(native(id),0,bytes,0,lock);if(e!=mnm::audio::Error::ok)return status(e);
    locks_[id]=lock;activeLock_=id;first=lock.first.data;return lock.second.size?Unsupported:0;
}
std::uint32_t NativeManagerBackend::readSource(std::uint8_t*& first,std::uint32_t bytes){
    auto found=locks_.find(activeLock_);
    if(found==locks_.end() || first!=found->second.first.data || bytes!=wave().samples.size() || bytes>found->second.first.size)throw std::logic_error("Invalid native whole-WAV upload domain");
    std::copy(wave().samples.begin(),wave().samples.end(),first);return bytes;
}
Status NativeManagerBackend::unlockSource(std::uint32_t id,std::uint8_t* first,std::uint32_t bytes){
    auto found=locks_.find(id);if(found==locks_.end() || id!=activeLock_ || first!=found->second.first.data)return Invalid;
    const auto e=device_.unlock(found->second,bytes,0);if(e==mnm::audio::Error::ok){locks_.erase(found);activeLock_=0;}return status(e);
}
std::uint32_t NativeManagerBackend::waveDuration(){return wavDuration(std::uint32_t(wave().samples.size()),wave().format.bytesPerSecond);}
}
