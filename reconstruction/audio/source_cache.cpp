#include "source_cache.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
std::int32_t signedWord(std::uint32_t value){return std::int32_t(value<=0x7fffffffu?std::int64_t(value):std::int64_t(value)-0x100000000ll);}
bool busy(SourceCacheBackend& b,const VoiceWrapper& v){
    if(!v.buffer)return false;
    std::uint32_t flags=0;return b.getStatus(v.buffer,flags)!=0 || (flags&1);
}
void promoteSource(VoiceWrapper*& head,VoiceWrapper& v){
    if(head==&v)return;
    if(head->sourceIndex!=v.sourceIndex){
        v.previous->next=v.next;v.next->previous=v.previous;
        v.previous=head->previous;v.next=head;head->previous->next=&v;head->previous=&v;
    }
    head=&v;
}
}
std::int32_t sourceCacheScore(const VoiceWrapper& v,std::uint32_t rank,std::uint32_t step){
    return signedWord((5000u-std::uint32_t(v.requestedVolume))*18u+rank*step+v.field08);
}
VoiceWrapper* selectSourceCache(SourceCacheBackend& b,VoiceWrapper* head,std::uint32_t low,std::uint32_t high){
    const auto divisor=high-low;if(!divisor)throw std::invalid_argument("Original source-cache divisor would be zero");
    const auto step=240000u/divisor;auto* start=head->previous;auto* v=start;
    VoiceWrapper* winner=nullptr;std::int32_t score=999999;std::uint32_t rank=1;
    do{
        if(v->field0c){
            bool available=true;
            if(v->buffer){
                available=!busy(b,*v);
                if(available)for(auto* child=v->duplicate;child;child=child->duplicate)
                    if(busy(b,*child)){available=false;break;}
            }
            if(available){
                if(v->field0c==2)return v;
                const auto candidate=sourceCacheScore(*v,rank,step);
                if(candidate<score){score=candidate;winner=v;}
            }
        }
        v=v->previous;++rank;
    }while(v!=start);
    return winner;
}
std::string sourceEntryName(std::string value){
    if(value.size()>=260 || value.find('\0')!=std::string::npos)
        throw std::invalid_argument("Outside inspected profile string domain");
    const auto comment=value.find(';');
    if(comment!=std::string::npos){
        if(!comment)throw std::invalid_argument("Original comment scan would underflow");
        const auto quote=value.rfind('\'',comment-1);
        if(quote==std::string::npos)throw std::invalid_argument("Original comment scan would underflow");
        value.resize(quote+1);
    }
    return value;
}
Status recycleSource(SourceCacheBackend& b,VoiceWrapper& v,const std::string& path,int source,std::uint32_t sourceClass){
    releaseSourceContents(b,v); // Destructive even if subsequent file open fails.
    if(!b.openWave(path)){b.closeWave();return SourceLoadFailure;}
    auto bytes=b.waveBytes();const auto format=b.waveFormat();
    auto result=b.createSource(secondaryDescriptor(bytes,0),format,v.buffer);
    std::uint8_t* first=nullptr;
    if(!result)result=b.lockSource(v.buffer,bytes,first);
    if(!result){bytes=b.readSource(first,bytes);result=b.unlockSource(v.buffer,first,bytes);}
    if(result){
        // Original cleanup dereferences this buffer unconditionally. Do not claim
        // a safe original unwind when a failed create supplied no COM object.
        if(!v.buffer)throw std::domain_error("Original upload cleanup would dereference a null buffer");
        b.releaseBuffer(v.buffer);v.buffer=0;b.closeWave();return result;
    }
    v.duration=b.waveDuration();v.field08=bytes;v.sourceIndex=source;v.field0c=sourceClass;
    b.closeWave();return 0;
}
namespace {
Status load(SourceCacheBackend& b,const SourceCacheState& state,VoiceWrapper*& head,
            VoiceWrapper* disabled,int sound,VoiceWrapper** output,std::vector<int>& stack){
    if(!state.manager.initialized || !state.manager.active){if(output)*output=disabled;return 0;}
    if(std::find(stack.begin(),stack.end(),sound)!=stack.end())throw std::invalid_argument("Cyclic source group");
    const auto& catalog=state.catalog;
    auto group=std::lower_bound(catalog.groups.begin(),catalog.groups.end(),sound,[](const SoundGroup& g,int id){return g.id<id;});
    if(group!=catalog.groups.end() && group->id==sound){
        stack.push_back(sound);
        for(auto member:group->members)if(load(b,state,head,disabled,member,nullptr,stack)){stack.pop_back();return SourceLoadFailure;}
        stack.pop_back();return 0; // Empty groups also succeed without publication.
    }
    const auto source=std::lower_bound(catalog.sourceIds.begin(),catalog.sourceIds.end(),sound);
    if(source==catalog.sourceIds.end() || *source!=sound)return SourceLoadFailure;
    const auto index=std::size_t(source-catalog.sourceIds.begin());
    auto* cursor=head;
    do{
        if(cursor->sourceIndex==sound){ // Loader compares caller ID, not table ordinal.
            std::uint32_t flags=0;
            if(!b.getStatus(cursor->buffer,flags)){
                if(flags&2)cursor->field0c=2;else return 0;
            }
            break;
        }
        cursor=cursor->next;
    }while(cursor!=head);
    auto* victim=selectSourceCache(b,head,state.field22c,state.field234);
    if(!victim)return SourceLoadFailure;
    std::string entry;if(!b.profileValue(sound,entry))return SourceLoadFailure;
    const auto path=state.assetRoot+state.pathInfix+sourceEntryName(std::move(entry))+state.pathSuffix;
    if(path.size()>=260)throw std::invalid_argument("Original path scratch buffer would overflow");
    if(index>=state.classes.size())throw std::out_of_range("Missing decoded source class");
    const auto result=recycleSource(b,*victim,path,sound,state.classes[index]);
    if(result)return result;
    promoteSource(head,*victim);if(output)*output=victim;return 0;
}
}
Status loadSourceCache(SourceCacheBackend& b,const SourceCacheState& state,VoiceWrapper*& head,
                       VoiceWrapper* disabled,int sound,VoiceWrapper** output){
    std::vector<int> stack;return load(b,state,head,disabled,sound,output,stack);
}
}
