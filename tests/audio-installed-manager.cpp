#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "native_manager_backend.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
namespace r=mnm::reconstruction::audio;namespace a=mnm::assets;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
QJsonArray ids(const std::vector<std::int32_t>& values){QJsonArray out;for(auto i:values)out.append(i);return out;}
std::string hash(const std::vector<std::uint8_t>& bytes){return QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(bytes.data()),bytes.size()),QCryptographicHash::Sha256).toHex().toStdString();}
int main(int argc,char** argv){try{
    if(argc!=3 && (argc!=4 || std::string(argv[3])!="--dequote-source-leaf"))throw std::runtime_error("Usage: audio-installed-manager SOUNDS_ROOT EVIDENCE_ROOT [--dequote-source-leaf]");
    const auto policy=argc==4?r::NativeSourcePathPolicy::dequoteMissingLeaf:r::NativeSourcePathPolicy::literal;
    const auto evidence=std::filesystem::path(argv[2]);std::filesystem::create_directories(evidence/"pcm");
    auto made=a::AssetStore::create(argv[1],{"C:\\Mnm\\Sounds"});check(std::holds_alternative<a::AssetStore>(made),"trusted sounds root");
    r::AudioManager manager;r::ManagerGlobals globals;unsigned clock=100,random=0,slot=0,second=0;
    r::NativeManagerBackend backend(std::get<a::AssetStore>(std::move(made)),manager,[&]{return clock;},[&]{return random;},48000,policy);
    auto startup=[&](unsigned map){check(!r::initializeManager(backend.services(),manager,globals,0,0,"C:\\Mnm\\Sounds"),"installed startup");check(!r::initializeSourcePool(backend,manager.cache,manager.sources,map),"installed source pool");};
    startup(1);check(globals.simultaneousLimit==12 && manager.cache.manager.active,"installed active manager/limit");
    const auto sources=manager.cache.catalog.sourceIds;const auto groups=manager.cache.catalog.groups;
    QJsonObject report{{"game_launched",false},{"audio_device_opened",false},{"sourceIds",ids(sources)},{"outputRate",int(backend.device().outputRate())}};QJsonArray groupReport,cases;
    for(const auto& group:groups)groupReport.append(QJsonObject{{"id",group.id},{"members",ids(group.members)}});
    report.insert("sourcePathPolicy",argc==4?"dequoteMissingLeaf":"literal");report.insert("groups",groupReport);report.insert("poolNodes",int(manager.sources.nodes.size()));
    auto snapshot=[&](QJsonObject& row,r::VoiceWrapper& voice){
        const auto samples=backend.bufferSamples(voice.buffer);row.insert("pcm_sha256",QString::fromStdString(hash(samples)));row.insert("pcm_bytes",int(samples.size()));row.insert("source",voice.sourceIndex);row.insert("duration",int(voice.duration));
    };
    auto mix=[&](QJsonObject& row,const std::string& key){
        std::vector<std::int16_t> pcm;check(backend.device().mixStereo(256,pcm)==mnm::audio::Error::ok,"mix installed PCM");
        const auto file="pcm/"+key+".s16";std::ofstream output(evidence/file,std::ios::binary);for(auto sample:pcm){const auto word=std::uint16_t(sample);const char bytes[2]={char(word&255),char(word>>8)};output.write(bytes,2);}check(bool(output),"write PCM evidence");row.insert("mix",QString::fromStdString(file));
    };
    // Direct source upload intentionally bypasses logical group resolution.
    for(auto id:sources){
        std::string value;backend.profileValue(id,value);const auto name=r::sourceEntryName(value);const auto path="C:\\Mnm\\Sounds\\"+name+".wav";
        auto& voice=*manager.sources.head;const auto status=r::recycleSource(backend,voice,path,id,1);
        QJsonObject row{{"stage","upload"},{"requested",id},{"name",QString::fromStdString(name)},{"status",status}};
        if(!status){snapshot(row,voice);check(!backend.volume(voice.buffer,0) && !backend.pan(voice.buffer,0) && !backend.play(voice.buffer,0,0,0),"one-shot start");mix(row,"upload-"+std::to_string(id));
            // Drain a bounded number of blocks until native one-shot completion.
            unsigned flags=1;std::vector<std::int16_t> pcm;unsigned blocks=0;
            while(flags&1){check(!backend.getStatus(voice.buffer,flags),"one-shot status");if(flags&1){check(++blocks<=512,"bounded installed duration");check(backend.device().mixStereo(65536,pcm)==mnm::audio::Error::ok,"completion drain");}}
            check(backend.device().mixStereo(16,pcm)==mnm::audio::Error::ok && std::all_of(pcm.begin(),pcm.end(),[](auto n){return n==0;}),"one-shot completion silence");row.insert("completed",true);
        }else{row.insert("diagnostic",QString::fromStdString(backend.diagnostic()));check(!voice.buffer,"failed recycle releases prior samples");}
        cases.append(row);
    }
    check(!r::initializeSourcePool(backend,manager.cache,manager.sources,1),"reset after direct upload sweep");
    auto admit=[&](int requested,unsigned rng,const std::string& stage){
        r::clearSchedules(backend,manager.schedules.head,true);check(!slot && !second,"retired caller slots");random=rng;
        r::AdmissionRequest request;request.sound=requested;request.looping=true;request.outputAddress=1;request.output=&slot;
        const auto status=r::admitVoice(backend,manager.cache.manager,manager.cache.catalog,manager.sources.head,manager.schedules.head,0,request);
        QJsonObject row{{"stage",QString::fromStdString(stage)},{"requested",requested},{"rng",int(rng)},{"status",status}};
        if(!status){check(slot!=0,"published admitted voice");snapshot(row,*manager.sources.head);mix(row,stage+"-"+std::to_string(requested)+"-"+std::to_string(rng));}
        else{check(!slot,"failed admission leaves caller slot clear");row.insert("diagnostic",QString::fromStdString(backend.diagnostic()));}
        cases.append(row);return status;
    };
    for(auto id:sources)admit(id,0,"admission");
    for(const auto& group:groups)for(unsigned i=0;i<group.members.size();++i)admit(group.id,i,"group");
    // Select an installed source known to have loaded, then overlap two loops.
    int chosen=0;for(const auto& value:cases){const auto row=value.toObject();if(row["stage"]=="admission" && row["status"].toInt()==0){chosen=row["requested"].toInt();break;}}
    check(chosen>0 && !admit(chosen,0,"overlap-base"),"installed overlap source");
    r::AdmissionRequest request;request.sound=chosen;request.looping=true;request.outputAddress=2;request.output=&second;
    check(!r::admitVoice(backend,manager.cache.manager,manager.cache.catalog,manager.sources.head,manager.schedules.head,0,request) && second && second!=slot,"installed duplicate admission");
    // Rewind the first loop so independent comparison starts both at frame zero.
    check(!backend.stop(backend.bufferForVoice(slot)) && !backend.position(backend.bufferForVoice(slot),0) && !backend.play(backend.bufferForVoice(slot),0,0,1),"rewind overlap root");
    QJsonObject overlap{{"stage","overlap"},{"requested",chosen},{"status",0}};snapshot(overlap,*manager.sources.head);mix(overlap,"overlap");cases.append(overlap);
    check(!backend.volume(manager.cache.manager.primary,-2000),"installed master gain");
    for(auto identity:{slot,second})check(!backend.stop(backend.bufferForVoice(identity)) && !backend.position(backend.bufferForVoice(identity),0) && !backend.play(backend.bufferForVoice(identity),0,0,1),"rewind gained voices");
    QJsonObject gained{{"stage","master-gain"},{"requested",chosen},{"status",0}};snapshot(gained,*manager.sources.head);mix(gained,"master-gain");cases.append(gained);
    check(!r::shutdownManager(backend.services(),manager,globals) && !slot && !second && backend.device().count()==1,"installed cleanup retains primary only");backend.collectDisposed();
    std::vector<std::int16_t> silence;check(backend.device().mixStereo(16,silence)==mnm::audio::Error::ok && std::all_of(silence.begin(),silence.end(),[](auto n){return n==0;}),"shutdown silence");
    startup(2);check(backend.device().count()==1 && manager.sources.pinnedBytes==0,"installed restart/map 2");r::destroyManager(backend.services(),manager,globals);backend.collectDisposed();check(manager.destroyed && manager.schedules.nodes.empty() && backend.device().count()==1,"installed destructor cleanup");
    report.insert("cases",cases);report.insert("cleanup",true);report.insert("restart",true);report.insert("retainedBuffers",int(backend.device().count()));report.insert("simultaneousLimit",int(globals.simultaneousLimit));
    std::ofstream output(evidence/"native.json");output<<QJsonDocument(report).toJson().constData();check(bool(output),"write installed report");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
