#include "source_cache.hpp"
#include "wave_loader.hpp"
#include <QTemporaryDir>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
struct Backend:r::SourceCacheBackend {
    std::string calls,path;std::map<unsigned,std::pair<int,unsigned>> status;
    std::vector<unsigned> released,freed;std::vector<int> profiles;
    int createFault=0,lockFault=0,unlockFault=0;bool opens=true,nullCreate=false;
    unsigned profileCount=4,reportedBytes=32;std::string entry="tone";
    std::vector<std::uint8_t> memory=std::vector<std::uint8_t>(32,0);
    r::Status getStatus(unsigned id,unsigned& flags)override{calls+='Q';auto [error,value]=status[id];flags=value;return error;}
    r::Status stop(unsigned)override{throw std::runtime_error("loader must not Stop");}
    r::Status position(unsigned,unsigned)override{throw std::runtime_error("loader must not reset position");}
    r::Status volume(unsigned,int)override{throw std::runtime_error("loader must not set volume");}
    r::Status pan(unsigned,int)override{throw std::runtime_error("loader must not set pan");}
    r::Status play(unsigned,unsigned,unsigned,unsigned)override{throw std::runtime_error("loader must not Play");}
    void volumeRecord(r::VoiceContract&,int)override{throw std::runtime_error("loader must not update scheduler volume");}
    void clearVoiceSchedule(unsigned)override{throw std::runtime_error("loader must not clear scheduler");}
    void releaseBuffer(unsigned id)override{calls+='E';released.push_back(id);}
    void freeWrapper(unsigned id)override{calls+='F';freed.push_back(id);}
    unsigned profileValue(int id,std::string& out)override{calls+='I';profiles.push_back(id);out=entry;return profileCount;}
    bool openWave(const std::string& p)override{calls+='O';path=p;return opens;}
    void closeWave()override{calls+='C';}
    unsigned waveBytes()override{calls+='B';return 32;}
    mnm::audio::PcmFormat waveFormat()override{calls+='M';return {1,1,22050,44100,2,16,0};}
    r::Status createSource(const r::Descriptor32& d,const mnm::audio::PcmFormat&,unsigned& out)override{
        calls+='A';check(d.size==20 && d.flags==0xea && d.bytes==32 && !d.reserved,"secondary descriptor");out=nullCreate?0:101;return createFault;
    }
    r::Status lockSource(unsigned id,unsigned bytes,std::uint8_t*& first)override{calls+='K';check(id==101 && bytes==32,"lock whole source");first=memory.data();return lockFault;}
    unsigned readSource(std::uint8_t*& first,unsigned bytes)override{calls+='R';check(first==memory.data() && bytes==32,"first-region read");return reportedBytes;}
    r::Status unlockSource(unsigned id,std::uint8_t* first,unsigned bytes)override{calls+='U';check(id==101 && first==memory.data() && bytes==reportedBytes,"unlock helper-reported bytes");return unlockFault;}
    unsigned waveDuration()override{calls+='D';return 321;}
};
struct Ring {
    std::vector<r::VoiceWrapper> voices;r::VoiceWrapper* head;
    explicit Ring(unsigned count=3):voices(count),head(&voices[0]){
        for(unsigned i=0;i<count;++i){auto& v=voices[i];v.identity=i+1;v.sourceIndex=int(100+i);v.buffer=11+i;v.field0c=1;
            v.previous=&voices[(i+count-1)%count];v.next=&voices[(i+1)%count];}
    }
    void valid(){auto* v=head;for(unsigned i=0;i<voices.size();++i){check(v->next->previous==v && v->previous->next==v,"bidirectional source ring");v=v->next;}check(v==head,"source ring bounded");}
};
r::SourceCacheState state(){r::SourceCacheState s;s.manager.initialized=s.manager.active=true;
    s.catalog.sourceIds={10,20,30};s.classes={0,1,2};s.field234=4;s.assetRoot="Sounds";s.pathInfix="\\";s.pathSuffix=".wav";return s;}
void scoreAndSelection(){
    unsigned cases=0;
    for(unsigned denominator:{1u,2u,4u,64u,0xffffffffu})for(unsigned pattern=0;pattern<125;++pattern){
        Ring ring;Backend b;unsigned code=pattern;
        for(auto& v:ring.voices){unsigned kind=code%5;code/=5;
            v.field0c=kind==0?0:kind==1?2:1;v.requestedVolume=-int(v.identity)*111;
            v.field08=kind==4?0xffffffffu:v.identity*100;
            b.status[v.buffer]={kind==3?7:0,kind==2?1u:0u};
        }
        // Independent index walk and arithmetic oracle, including wrapping score.
        int winner=-1,best=999999;
        for(unsigned rank=1;rank<=3;++rank){int i=3-int(rank);const auto& v=ring.voices[unsigned(i)];
            if(!v.field0c || b.status[v.buffer].first || (b.status[v.buffer].second&1))continue;
            if(v.field0c==2){winner=i;break;}
            std::uint64_t raw=(std::uint64_t(5000)-std::uint32_t(v.requestedVolume))*18+
                std::uint64_t(rank)*(240000u/denominator)+v.field08;
            raw&=0xffffffffu;const auto score=int(raw<0x80000000u?std::int64_t(raw):std::int64_t(raw)-0x100000000ll);
            check(r::sourceCacheScore(v,rank,240000u/denominator)==score,"score wrap oracle");
            if(score<best){best=score;winner=i;}
        }
        auto* chosen=r::selectSourceCache(b,ring.head,0,denominator);
        check(chosen==(winner<0?nullptr:&ring.voices[unsigned(winner)]),"independent backward selection oracle");ring.valid();++cases;
    }
    check(cases==625,"selection matrix size");
    Ring tie;Backend b;for(auto& v:tie.voices){v.requestedVolume=0;v.field08=0;}
    check(r::selectSourceCache(b,tie.head,0,0xffffffffu)==&tie.voices[2],"score ties retain first backward candidate");
    for(auto& v:tie.voices)v.field08=2000000;
    check(!r::selectSourceCache(b,tie.head,0,0xffffffffu),"initial score ceiling can reject all idle sources");
    r::VoiceWrapper child;child.buffer=44;tie.voices[2].field0c=2;tie.voices[2].duplicate=&child;b.status[44]={0,1};
    check(!r::selectSourceCache(b,tie.head,0,4),"busy child blocks parent recycling");
    b.status[44]={8,0};check(!r::selectSourceCache(b,tie.head,0,4),"child query failure counts as busy");
    tie.voices[2].buffer=0;check(r::selectSourceCache(b,tie.head,0,4)==&tie.voices[2],"bufferless root skips even busy descendants");
    bool rejected=false;try{r::selectSourceCache(b,tie.head,4,4);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"zero divisor rejected");
}
void destructiveUpload(){
    for(int fault=0;fault<5;++fault){
        Ring ring;Backend b;r::VoiceWrapper child,last;child.identity=7;child.buffer=17;last.identity=8;last.buffer=18;child.duplicate=&last;
        auto& v=ring.voices[2];v.duplicate=&child;v.duration=99;v.requestedVolume=-200;v.cachedVolume=-200;
        b.opens=fault!=1;b.createFault=fault==2?12:0;b.lockFault=fault==3?13:0;b.unlockFault=fault==4?14:0;
        b.reportedBytes=16;
        check(r::recycleSource(b,v,"Sounds\\tone.wav",20,1)==(fault==1?r::SourceLoadFailure:fault?10+fault:0),"upload error result");
        const std::string prefix="EFEFEOBM";
        const std::string expected=fault==1?"EFEFEOC":prefix+(fault==2?"AEC":fault==3?"AKEC":fault==4?"AKRUEC":"AKRUDC");
        check(b.calls==expected,"destructive cleanup and upload call order");
        check(b.released==std::vector<unsigned>(fault>=2?std::initializer_list<unsigned>{18,17,13,101}:std::initializer_list<unsigned>{18,17,13}),"descendants before source before failed new buffer");
        check(b.freed==std::vector<unsigned>({8,7}) && !v.duplicate,"duplicate ownership disposal");
        check(v.previous==&ring.voices[1] && v.next==&ring.voices[0],"recycling preserves links");
        if(fault)check(!v.buffer && v.sourceIndex==-1 && v.duration==0 && v.cachedVolume==99,"failed replacement stays reset");
        else check(v.buffer==101 && v.sourceIndex==20 && v.field08==16 && v.field0c==1 && v.duration==321,"success metadata uses ID and helper bytes");
    }
    Ring empty;Backend missing;empty.voices[0].buffer=0;empty.voices[0].duration=87;missing.opens=false;
    check(r::recycleSource(missing,empty.voices[0],"missing",10,1)==r::SourceLoadFailure && missing.calls=="OC" && empty.voices[0].sourceIndex==100 && empty.voices[0].duration==87,"bufferless source skips root reset on failed replacement");
    Ring null;Backend bad;bad.createFault=12;bad.nullCreate=true;bool rejected=false;
    try{r::recycleSource(bad,null.voices[0],"x",10,1);}catch(const std::domain_error&){rejected=true;}
    check(rejected && bad.calls=="EOBMA","null create cleanup remains unsupported original fault domain");
}
void loaderDecisions(){
    for(bool initialized:{false,true})for(bool active:{false,true}){
        if(initialized && active)continue;
        Ring ring;Backend b;auto s=state();s.manager.initialized=initialized;s.manager.active=active;
        auto* output=&ring.voices[1];check(!r::loadSourceCache(b,s,ring.head,&ring.voices[2],10,&output) && output==&ring.voices[2] && b.calls.empty(),"disabled loader publishes sentinel");
    }
    Ring cached;Backend b;auto s=state();cached.voices[1].sourceIndex=20;b.status[12]={0,5};auto* output=&cached.voices[2];
    check(!r::loadSourceCache(b,s,cached.head,nullptr,20,&output) && b.calls=="Q" && output==&cached.voices[2] && cached.head==&cached.voices[0],"healthy cached loop returns without output/head write");
    b.calls.clear();check(r::loadSourceCache(b,s,cached.head,nullptr,999,&output)==r::SourceLoadFailure && b.calls.empty(),"unknown ID fails, no admission fallback here");
    for(bool profile:{false,true})for(bool opens:{false,true}){
        Ring ring;Backend backend;ring.voices[2].field0c=2;backend.profileCount=profile?4:0;backend.opens=opens;auto* published=&ring.voices[0];
        auto result=r::loadSourceCache(backend,s,ring.head,nullptr,20,&published);
        if(!profile)check(result==r::SourceLoadFailure && backend.calls=="QI" && ring.voices[2].buffer==13,"missing profile does not destroy candidate");
        else if(!opens)check(result==r::SourceLoadFailure && backend.calls=="QIEOC" && !ring.voices[2].buffer && published==&ring.voices[0],"open failure destroys candidate without publishing");
        else check(!result && backend.calls=="QIEOBMAKRUDC" && ring.head==&ring.voices[2] && published==ring.head && ring.head->sourceIndex==20 && ring.head->field0c==1 && backend.path=="Sounds\\tone.wav","full load and metadata ordinal independent of sound ID");
        ring.valid();
    }
    Ring lost;Backend reloader;lost.voices[1].sourceIndex=20;reloader.status[12]={0,2};lost.voices[2].field0c=0;
    check(!r::loadSourceCache(reloader,s,lost.head,nullptr,20) && lost.head==&lost.voices[1] && reloader.calls=="QQIEOBMAKRUDC","lost cached buffer marks class 2 then recycles");
    Ring busy;Backend statuses;for(auto& v:busy.voices)statuses.status[v.buffer]={0,1};
    check(r::loadSourceCache(statuses,s,busy.head,nullptr,20)==r::SourceLoadFailure && statuses.calls=="QQQ","no reusable slot does not read profile");
}
void groupsAndNames(){
    Ring ring;Backend b;auto s=state();s.catalog.groups={{5,{10,20}},{6,{}}};
    for(auto& v:ring.voices){v.buffer=0;v.field0c=2;}
    auto* output=&ring.voices[0];check(!r::loadSourceCache(b,s,ring.head,nullptr,5,&output) && b.profiles==std::vector<int>({10,20}) && output==&ring.voices[0],"group loads all members without parent output write");
    b.calls.clear();check(!r::loadSourceCache(b,s,ring.head,nullptr,6,&output) && b.calls.empty(),"empty preload group succeeds");
    struct Partial:Backend {
        unsigned profileValue(int id,std::string& out)override{if(id==20)profileCount=0;return Backend::profileValue(id,out);}
    } partial;
    Ring committed;auto* oldOutput=&committed.voices[0];
    check(r::loadSourceCache(partial,s,committed.head,nullptr,5,&oldOutput)==r::SourceLoadFailure && partial.profiles==std::vector<int>({10,20}) && committed.head->sourceIndex==10 && committed.head->buffer==101 && oldOutput==&committed.voices[0],"group failure preserves previously committed member");
    committed.valid();
    Ring failed;Backend fault;fault.opens=false;
    check(r::loadSourceCache(fault,s,failed.head,nullptr,5)==r::SourceLoadFailure && fault.profiles==std::vector<int>({10}),"group failure stops remaining members");
    s.catalog.groups={{5,{6}},{6,{5}}};bool rejected=false;
    try{r::loadSourceCache(b,s,ring.head,nullptr,5);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"group recursion cycle rejected");
    check(r::sourceEntryName("tone")=="tone" && r::sourceEntryName("tone'  ; comment")=="tone'","original backward apostrophe comment scan");
    for(const auto& name:{std::string("; bad"),std::string("tone; bad"),std::string(260,'x')}){
        rejected=false;try{r::sourceEntryName(name);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"unsafe profile domain rejected");
    }
}
struct Native:Backend {
    mnm::audio::Device device{4096,16,22050};mnm::assets::AssetStore store;
    mnm::audio::Wave wave;mnm::audio::WriteLock lock;
    explicit Native(mnm::assets::AssetStore assets):store(std::move(assets)){}
    r::Status getStatus(unsigned id,unsigned& flags)override{auto voice=device.voice(id);if(!voice)return 1;flags=voice->status();return 0;}
    void releaseBuffer(unsigned id)override{check(device.release(id)==mnm::audio::Error::ok,"native released source buffer");}
    bool openWave(const std::string& path)override{
        auto opened=store.open(path);if(!std::holds_alternative<std::unique_ptr<mnm::assets::AssetFile>>(opened))return false;
        auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));
        try{wave=mnm::audio::loadWave(*file);}catch(const std::runtime_error&){return false;}
        return true;
    }
    void closeWave()override{wave={};}
    unsigned waveBytes()override{return unsigned(wave.samples.size());}
    mnm::audio::PcmFormat waveFormat()override{return wave.format;}
    r::Status createSource(const r::Descriptor32& descriptor,const mnm::audio::PcmFormat& format,unsigned& out)override{
        mnm::audio::BufferId id=0;auto error=device.createStatic(descriptor.flags,format,descriptor.bytes,id);out=unsigned(id);return int(error);
    }
    r::Status lockSource(unsigned id,unsigned bytes,std::uint8_t*& first)override{
        auto error=device.lock(id,0,bytes,0,lock);first=lock.first.data;return int(error);
    }
    unsigned readSource(std::uint8_t*& first,unsigned bytes)override{
        check(first==lock.first.data && bytes==wave.samples.size(),"native exact sample copy");
        std::copy(wave.samples.begin(),wave.samples.end(),first);return bytes;
    }
    r::Status unlockSource(unsigned,std::uint8_t*,unsigned bytes)override{return int(device.unlock(lock,bytes,0));}
    unsigned waveDuration()override{return r::wavDuration(unsigned(wave.samples.size()),wave.format.bytesPerSecond);}
};
struct Admission:r::AdmissionBackend {
    Native& native;r::SourceCacheState& state;r::VoiceWrapper*& head;
    std::vector<std::unique_ptr<r::VoiceWrapper>> children;
    Admission(Native& b,r::SourceCacheState& s,r::VoiceWrapper*& h):native(b),state(s),head(h){}
    unsigned tickCount()override{return 100;}
    unsigned randomWord()override{throw std::runtime_error("no random group in fixture");}
    unsigned bufferForVoice(unsigned id)override{
        auto* v=head;do{if(v->identity==id)return v->buffer;v=v->next;}while(v!=head);
        for(auto& child:children)if(child->identity==id)return child->buffer;
        throw std::runtime_error("unknown scheduled wrapper");
    }
    r::Status loadSource(int sound,r::VoiceWrapper*& out)override{return r::loadSourceCache(native,state,head,nullptr,sound,&out);}
    r::Status getStatus(unsigned id,unsigned& flags)override{return native.getStatus(id,flags);}
    r::Status duplicateBuffer(unsigned source,unsigned& out)override{mnm::audio::BufferId id=0;auto result=native.device.duplicate(source,id);out=unsigned(id);return int(result);}
    r::VoiceWrapper& allocateWrapper()override{auto child=std::make_unique<r::VoiceWrapper>();child->identity=100+unsigned(children.size());children.push_back(std::move(child));return *children.back();}
    r::Status stop(unsigned id)override{return int(native.device.stop(id));}
    r::Status position(unsigned id,unsigned byte)override{check(!byte,"native zero reset");return int(native.device.resetPosition(id));}
    r::Status volume(unsigned id,int volume)override{return int(native.device.setVolume(id,volume));}
    r::Status pan(unsigned id,int pan)override{return int(native.device.setPan(id,pan));}
    r::Status play(unsigned id,unsigned a,unsigned b,unsigned flags)override{check(!a && !b,"reserved play args");return int(native.device.play(id,flags));}
    void volumeRecord(r::VoiceContract&,int)override{throw std::runtime_error("no notification during admission");}
};
void nativeFileAndAdmission(){
    QTemporaryDir directory;check(directory.isValid(),"fixture directory");
    std::vector<std::uint8_t> bytes(44+256,0);
    auto text=[&](unsigned at,const char* value){for(unsigned i=0;i<4;++i)bytes[at+i]=std::uint8_t(value[i]);};
    auto word=[&](unsigned at,unsigned value,unsigned size){for(unsigned i=0;i<size;++i)bytes[at+i]=std::uint8_t(value>>(i*8));};
    text(0,"RIFF");word(4,292,4);text(8,"WAVE");text(12,"fmt ");word(16,16,4);
    word(20,1,2);word(22,1,2);word(24,22050,4);word(28,44100,4);word(32,2,2);word(34,16,2);
    text(36,"data");word(40,256,4);for(unsigned i=44;i<bytes.size();i+=2){bytes[i]=0xa0;bytes[i+1]=0x0f;}
    const auto root=std::filesystem::path(directory.path().toStdString());
    {std::ofstream file(root/"Tone.wav",std::ios::binary);file.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));check(bool(file),"write synthetic WAV");}
    auto created=mnm::assets::AssetStore::create(root,{"C:\\Mnm\\Sounds"});check(std::holds_alternative<mnm::assets::AssetStore>(created),"native asset store");
    Native native(std::get<mnm::assets::AssetStore>(std::move(created)));native.entry="tone";
    Ring ring;for(auto& v:ring.voices){v.buffer=0;v.field0c=2;}
    auto cache=state();cache.assetRoot="c:\\mNm\\sOuNdS";
    Admission adapter(native,cache,ring.head);
    r::ScheduleNode first,second;first.next=first.previous=&second;second.next=second.previous=&first;auto* schedules=&first;
    unsigned output=0;r::AdmissionRequest request;request.sound=20;request.outputAddress=1;request.output=&output;request.looping=true;
    check(!r::admitVoice(adapter,cache.manager,cache.catalog,ring.head,schedules,0,request) && output==3 && ring.head->sourceIndex==20 && ring.head->field0c==1,"Qt file load through cache to admission with nonordinal sound ID");
    const auto source=ring.head->buffer;check(native.device.samples(source)==std::vector<std::uint8_t>(bytes.begin()+44,bytes.end()),"original synthetic PCM byte agreement");
    check(!r::admitVoice(adapter,cache.manager,cache.catalog,ring.head,schedules,0,request) && output==100 && second.record.voiceAddress==100,"overlap duplicates loaded source");
    std::vector<std::int16_t> pcm;check(native.device.mixStereo(1,pcm)==mnm::audio::Error::ok && pcm==std::vector<std::int16_t>({8000,8000}),"file to cache to admission to exact stereo PCM");
    const auto duplicate=adapter.children[0]->buffer;
    check(native.device.stop(source)==mnm::audio::Error::ok,"stop source for recycling check");
    auto* savedHead=ring.head;
    for(auto& v:ring.voices)if(&v!=savedHead)v.field0c=0;
    check(!r::selectSourceCache(native,ring.head,0,4),"playing duplicate prevents recycling its otherwise idle parent");
    ring.valid();
    check(native.device.stop(duplicate)==mnm::audio::Error::ok,"stop child for cleanup");
    check(r::selectSourceCache(native,ring.head,0,4)==savedHead,"stopped duplicate permits parent selection");
    r::releaseSourceContents(native,*savedHead);check(native.device.count()==0 && !savedHead->buffer && !savedHead->duplicate,"loaded source and duplicate retire with owned samples");
}
int main(){scoreAndSelection();destructiveUpload();loaderDecisions();groupsAndNames();nativeFileAndAdmission();std::cout<<"625 cache selection cases, destructive upload, groups, profile paths and Qt file to admission PCM passed\n";}
