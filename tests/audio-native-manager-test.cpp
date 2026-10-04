#include "native_manager_backend.hpp"
#include <QTemporaryDir>
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;namespace a=mnm::assets;
void check(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
std::vector<std::uint8_t> bytes(const std::string& s){return {s.begin(),s.end()};}
void write(const std::filesystem::path& p,const std::vector<std::uint8_t>& data){std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(data.data()),std::streamsize(data.size()));check(bool(f),"write synthetic fixture");}
std::vector<std::uint8_t> wav(int sample){
    std::vector<std::uint8_t> v(44+256,0);auto tag=[&](unsigned at,const char* s){for(unsigned i=0;i<4;++i)v[at+i]=std::uint8_t(s[i]);};
    auto word=[&](unsigned at,unsigned n,unsigned count){for(unsigned i=0;i<count;++i)v[at+i]=std::uint8_t(n>>(i*8));};
    tag(0,"RIFF");word(4,292,4);tag(8,"WAVE");tag(12,"fmt ");word(16,16,4);word(20,1,2);word(22,1,2);word(24,48000,4);word(28,96000,4);word(32,2,2);word(34,16,2);tag(36,"data");word(40,256,4);
    for(unsigned at=44;at<v.size();at+=2)word(at,unsigned(sample),2);
    return v;
}
a::AssetStore store(const std::filesystem::path& path){auto made=a::AssetStore::create(path,{"C:\\Mnm\\Sounds"});check(std::holds_alternative<a::AssetStore>(made),"fixture asset root");return std::get<a::AssetStore>(std::move(made));}
void profileContracts(){
    auto p=a::ProfileSnapshot::parse(bytes(";comment\r\n[Sounds]\r\n10 = 'Tone'\r\n20=\" other \"\r\n30=empty ; literal inline\r\n[7 Load Permanent]\r\n10\r\n[Optimisation]\r\nMaxSimultaneousSounds=4\r\n"));
    check(p.value("sOuNdS","10",260)=="Tone" && p.value("Sounds","20",260)==" other ","case-insensitive keys and surrounding quotes");
    check(p.value("Sounds","30",260)=="empty ; literal inline","inline semicolon retained");
    check(p.value("Sounds","missing",4,"abcdef")=="abc" && p.value("Sounds","10",1).empty(),"default and string truncation capacities");
    check(p.value("missing","key",128,"  fallback  ")=="  fallback" && p.value("missing","key",128,"fallback\t")=="fallback\t","Wine fallback trims trailing spaces only");
    auto map=a::ProfileSnapshot::parse(bytes("[Map] ; installed-style header comment\n10\n20\n"));
    check(map.section("Map",7).returned==5 && map.section("Map",7).entries.empty() && map.section("Map",8).returned==6 && map.section("Map",8).entries.size()==2,"Wine exact section boundary and native partial-content policy");
    auto list=p.section("Sounds",16384);check(list.entries==std::vector<std::string>({"10='Tone'","20=\" other \"","30=empty ; literal inline"}),"section quotes preserved and whitespace normalized");
    check(p.section("Sounds",4).returned==2 && p.section("Sounds",4).entries.empty(),"section truncation marker");
    check(p.section("7 Load Permanent",16384).entries==std::vector<std::string>({"10"}),"bare per-map entries");
    check(p.integer("optimisation","maxsimultaneoussounds",16)==4 && p.integer("missing","key",16)==16,"integer and missing fallback");
    for(const auto& text:{"[A]\nx=1\nX=2\n","[A]\nx=1\n[a]\ny=2\n","orphan=1\n","[broken\nx=1\n","[A] junk\nx=1\n"}){
        bool rejected=false;try{a::ProfileSnapshot::parse(bytes(text));}catch(const std::invalid_argument&){rejected=true;}check(rejected,"malformed/duplicate profile native rejection");
    }
    std::string many="[Many]\n";for(unsigned i=0;i<10000;++i)many+="k"+std::to_string(i)+"=value\n";
    auto large=a::ProfileSnapshot::parse(bytes(many));check(large.section("many",16384).returned==16382 && large.value("many","k9999",260)=="value","large bounded section truncation and indexed duplicate validation");
    bool rejected=false;try{a::ProfileSnapshot::parse({0xef,0xbb,0xbf});}catch(const std::invalid_argument&){rejected=true;}check(rejected,"non-ASCII encoding rejection");
    for(const auto& integer:{"0x10","-1","3junk","4294967296"}){auto bad=a::ProfileSnapshot::parse(bytes(std::string("[A]\nx=")+integer));bool badInt=false;try{bad.integer("a","x",9);}catch(const std::invalid_argument&){badInt=true;}check(badInt,"native integer domain explicit");}
}
void endToEnd(){
    QTemporaryDir dir;check(dir.isValid(),"temporary profile/WAV directory");const auto root=std::filesystem::path(dir.path().toStdString());
    const auto tone=wav(4000),other=wav(1000);write(root/"Tone.wav",tone);write(root/"Other.wav",other);
    write(root/"Sounds.ini",bytes("[Sounds]\n10='tone'\n20=other\n30=missing\n40=broken\n[Randomised]\n90=10,20\n[Optimisation]\nMaxSimultaneousSounds=4\n[7 Load Permanent]\n10\n[7 Load Temporary]\n20\n"));write(root/"Broken.wav",bytes("invalid WAV"));
    r::AudioManager m;r::ManagerGlobals g{0,1,16};unsigned clock=100;
    r::NativeManagerBackend backend(store(root),m,[&]{return clock;},[]{return 0u;});
    check(!r::initializeManager(backend.services(),m,g,0,0,"c:\\mNm\\sOuNdS"),"Qt file/profile to native manager startup");
    check(m.cache.manager.active && backend.device().primaryState().status()==5 && g.simultaneousLimit==4 && m.schedules.nodes.size()==4,"native primary and scheduler initialized");
    check(backend.device().primaryState().format && backend.device().primaryState().format->rate==22050 && backend.device().outputRate()==48000,"requested primary format metadata independent of native output clock");
    check(!r::initializeSourcePool(backend,m.cache,m.sources,7) && m.cache.classes==std::vector<unsigned>({0,2,1,1}) && m.sources.pinnedBytes==tone.size(),"profile classes/stat and permanent WAV preload");
    check(m.sources.head->sourceIndex==10 && backend.bufferSamples(m.sources.head->buffer)==std::vector<std::uint8_t>(tone.begin()+44,tone.end()),"exact committed file PCM");
    unsigned first=0,second=0; r::AdmissionRequest request;request.sound=90;request.volume=0;request.looping=true;request.outputAddress=1;request.output=&first;
    check(!r::admitVoice(backend,m.cache.manager,m.cache.catalog,m.sources.head,m.schedules.head,0,request) && first && m.sources.head->sourceIndex==10,"group RNG to pinned source start");
    request.outputAddress=2;request.output=&second;
    check(!r::admitVoice(backend,m.cache.manager,m.cache.catalog,m.sources.head,m.schedules.head,0,request) && second>=65537 && second!=first,"overlap duplicate token and scheduled publication");
    std::vector<std::int16_t> pcm;check(backend.device().mixStereo(1,pcm)==mnm::audio::Error::ok && pcm==std::vector<std::int16_t>({8000,8000}),"exact overlapped PCM through aggregate startup/admission");
    check(!backend.volume(m.cache.manager.primary,-2000),"native primary attenuation");backend.device().mixStereo(1,pcm);check(pcm==std::vector<std::int16_t>({800,800}),"master gain after summed voices");
    // A stopped root with retained duplicates rotates a backend-owned child into the ring.
    check(!backend.stop(backend.bufferForVoice(first)),"stop source for recovered duplicate rotation");
    request.outputAddress=3;unsigned rotated=0;request.output=&rotated;
    check(!r::admitVoice(backend,m.cache.manager,m.cache.catalog,m.sources.head,m.schedules.head,0,request) && rotated==first && m.sources.head->identity==second && m.sources.head->duplicate->identity==first,"rotation retains stable manager-owned duplicate storage");
    backend.freeWrapper(second);bool referenced=false;try{backend.collectDisposed();}catch(const std::logic_error&){referenced=true;}check(referenced,"cannot collect a still-linked duplicate");
    check(!r::shutdownManager(backend.services(),m,g) && !first && !second && !rotated && backend.device().count()==1 && !m.sources.head && m.sources.ownedDuplicates.empty(),"all root/duplicate buffers and caller slots cleaned; primary retained");
    backend.collectDisposed();backend.device().mixStereo(1,pcm);check(pcm==std::vector<std::int16_t>({0,0}) && backend.device().primaryState().volume==0,"silence and saved master restored");
    check(!r::initializeManager(backend.services(),m,g,0,0,"C:\\Mnm\\Sounds") && !r::initializeSourcePool(backend,m.cache,m.sources,8),"restart/map with defaults");
    unsigned oneShot=0;request.sound=20;request.looping=false;request.outputAddress=4;request.output=&oneShot;
    check(!r::admitVoice(backend,m.cache.manager,m.cache.catalog,m.sources.head,m.schedules.head,0,request) && oneShot,"temporary WAV native upload");
    backend.device().mixStereo(128,pcm);check(pcm.size()==256 && pcm.front()==1000 && pcm.back()==1000,"exact complete one-shot PCM");
    unsigned flags=1;check(!backend.getStatus(backend.bufferForVoice(oneShot),flags) && !flags,"native one-shot completion");
    clock+=3;unsigned unknown=77;request.sound=30;request.outputAddress=5;request.output=&unknown;
    check(r::admitVoice(backend,m.cache.manager,m.cache.catalog,m.sources.head,m.schedules.head,0,request)==r::SourceLoadFailure && backend.assetError() && backend.assetError()->code==a::ErrorCode::notFound,"missing WAV returns recovered failure with Qt diagnostic");
    request.sound=40;check(r::admitVoice(backend,m.cache.manager,m.cache.catalog,m.sources.head,m.schedules.head,0,request)==r::SourceLoadFailure && !backend.diagnostic().empty(),"strict malformed WAV rejection");
    r::destroyManager(backend.services(),m,g);backend.collectDisposed();check(m.destroyed && m.schedules.nodes.empty() && backend.device().count()==1,"destructor retains reused native primary; host device owns final cleanup");
}
void missingAndBoundedInputs(){
    QTemporaryDir dir;check(dir.isValid(),"error fixture directory");const auto root=std::filesystem::path(dir.path().toStdString());
    r::AudioManager m;r::ManagerGlobals g; r::NativeManagerBackend b(store(root),m,[]{return 0u;},[]{return 0u;});
    check(r::initializeManager(b.services(),m,g,0,0,"C:\\Mnm\\Sounds")==r::SourceLoadFailure && b.assetError() && b.device().count()==0,"missing profile without native device work");
    write(root/"Sounds.ini",bytes("[Sounds]\n10=tone\n"));check(b.openProfile("C:\\Mnm\\Sounds\\Sounds.ini"),"bound profile snapshot");
    write(root/"Sounds.ini",bytes("[Sounds]\n20=other\n"));check(b.section("Sounds",16384).entries==std::vector<std::string>({"10=tone"}),"profile snapshot owns bytes across disk edits");
    check(b.openProfile("C:\\Mnm\\Sounds\\Sounds.ini") && b.section("Sounds",16384).entries==std::vector<std::string>({"20=other"}),"explicit reopen refreshes snapshot");
    write(root/"Sounds.ini",bytes("[Sounds]\n10=tone\n10=other\n"));check(!b.openProfile("C:\\Mnm\\Sounds\\Sounds.ini") && b.assetError()->code==a::ErrorCode::invalidArgument,"profile parse errors retain structured diagnostics");
    write(root/"Sounds.ini",bytes("[Sounds]\n10=tone\n"));
    auto opened=store(root).open("Sounds.ini");auto& file=*std::get<std::unique_ptr<a::AssetFile>>(opened);file.seek(4);auto loaded=a::loadProfile(file);
    check(std::holds_alternative<a::ProfileSnapshot>(loaded) && std::get<a::ProfileSnapshot>(loaded).value("sounds","10",260)=="tone","Qt profile loader rewinds before parsing");
    write(root/"Huge.ini",std::vector<std::uint8_t>(a::ProfileInputLimit+1,'x'));check(!b.openProfile("C:\\Mnm\\Sounds\\Huge.ini") && b.assetError()->code==a::ErrorCode::limitExceeded,"bounded Qt profile read");
}
void sourceFilenamePolicy(){
    QTemporaryDir dir;check(dir.isValid(),"filename policy directory");const auto root=std::filesystem::path(dir.path().toStdString());
    const auto tone=wav(4000),literal=wav(-2000);write(root/"Tone.wav",tone);
    write(root/"Sounds.ini",bytes("[Sounds]\n10='Tone' ; installed-style comment\n"));
    r::AudioManager strictManager,compatibleManager;r::ManagerGlobals strictGlobals,compatibleGlobals;
    r::NativeManagerBackend strict(store(root),strictManager,[]{return 100u;},[]{return 0u;});
    r::NativeManagerBackend compatible(store(root),compatibleManager,[]{return 100u;},[]{return 0u;},48000,r::NativeSourcePathPolicy::dequoteMissingLeaf);
    auto startup=[](r::NativeManagerBackend& backend,r::AudioManager& manager,r::ManagerGlobals& globals){
        check(!r::initializeManager(backend.services(),manager,globals,0,0,"C:\\Mnm\\Sounds") && !r::initializeSourcePool(backend,manager.cache,manager.sources,1),"filename policy startup");
    };
    startup(strict,strictManager,strictGlobals);startup(compatible,compatibleManager,compatibleGlobals);
    r::VoiceWrapper* voice=nullptr;
    check(strict.loadSource(10,voice)==r::SourceLoadFailure && strict.assetError()->code==a::ErrorCode::notFound,"literal baseline remains strict");
    check(!compatible.loadSource(10,voice) && voice && compatible.bufferSamples(voice->buffer)==std::vector<std::uint8_t>(tone.begin()+44,tone.end()),"explicit compatibility loads commented source PCM");
    unsigned size=0;check(compatible.fileSize("C:\\Mnm\\Sounds\\'Tone'.WaV",size) && size==tone.size(),"stat and WAV open share missing-leaf adaptation");
    write(root/"'Tone'.wav",literal);check(!r::initializeSourcePool(compatible,compatibleManager.cache,compatibleManager.sources,1),"clear cache before literal precedence");
    voice=nullptr;check(!compatible.loadSource(10,voice) && compatible.bufferSamples(voice->buffer)==std::vector<std::uint8_t>(literal.begin()+44,literal.end()),"existing literal quoted file wins over unquoted alternate");
    for(const auto& path:{"C:\\Mnm\\Sounds\\'Missing'.wav","C:\\Mnm\\Sounds\\'Tone.wav","C:\\Mnm\\Sounds\\'Tone'.ogg","C:\\Mnm\\Sounds\\../'Tone'.wav"})check(!compatible.openWave(path),"no missing-name, unbalanced-quote, extension or traversal repair");
    std::filesystem::remove(root/"'Tone'.wav");write(root/"TONE.WAV",tone);
    check(!compatible.openWave("C:\\Mnm\\Sounds\\'Tone'.wav") && compatible.assetError()->code==a::ErrorCode::ambiguousPath,"dequoted lookup preserves resolver ambiguity rejection");
    r::destroyManager(strict.services(),strictManager,strictGlobals);r::destroyManager(compatible.services(),compatibleManager,compatibleGlobals);
}
int main(){try{sourceFilenamePolicy();profileContracts();endToEnd();missingAndBoundedInputs();std::cout<<"Qt profile/WAV to aggregate manager, duplicate PCM, rotation and teardown passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
