#include "manager_lifecycle.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
r::ProfileSection section(std::vector<std::string> entries){r::ProfileSection s;for(auto& e:entries)s.returned+=unsigned(e.size()+1);s.entries=std::move(entries);return s;}
struct Backend:r::ConfigurationBackend,r::ManagerBackend,r::PrimaryBackend,r::LifecycleBackend {
    r::AudioManager& manager;r::ManagerGlobals& globals;
    std::vector<std::string> calls;std::string fault;int error=71;
    unsigned primary=99,device=88,limit=4;bool accessible=true;
    std::map<std::string,r::ProfileSection> sections;
    std::vector<unsigned> retired;std::uint8_t sample=0;unsigned buffer=100;
    Backend(r::AudioManager& m,r::ManagerGlobals& g):manager(m),globals(g){
        sections["Sounds"]=::section({"10=tone","20=other"});sections["Randomised"]=::section({"30=10,20","40=20"});
    }
    r::ManagerServices services(){return {*this,*this,*this,*this};}
    int call(const std::string& n){calls.push_back(n);return n==fault?error:0;}
    bool openProfile(const std::string& p)override{check(p=="root\\Sounds.ini","profile path");call("profile");return accessible;}
    int createDevice(const r::DeviceRequest& d,unsigned& out)override{check(d.defaultDevice && d.noAggregation && d.cooperativeLevel==2,"device descriptor");int e=call("device");out=e?0:device;return e;}
    int cooperativeLevel(unsigned d,unsigned w,unsigned l)override{check(d==88 && w==22 && l==2 && manager.cache.manager.initialized,"initialized before cooperative call");return call("cooperative");}
    unsigned primaryIdentity()override{return primary;}
    void freeAllocation(r::ManagerAllocation k,std::size_t i)override{
        const char* names[]={"classes","ids","groupIds","member","groups","schedules"};call(std::string("free:")+names[unsigned(k)]+(k==r::ManagerAllocation::groupMembers?std::to_string(i):""));
    }
    r::ProfileSection section(const std::string& n,unsigned cap)override{check(cap==16384,"section capacity");call(n);return sections[n];}
    unsigned groupValue(int id,std::string& value)override{call("group:"+std::to_string(id));value=id==30?"10,20":"20";return unsigned(value.size());}
    unsigned profileInteger(const std::string& s,const std::string& key,unsigned fallback)override{
        check(s=="Optimisation" && key=="MaxSimultaneousSounds" && fallback==globals.simultaneousLimit,"profile limit contract");
        check(globals.device==88 && manager.cache.manager.active,"device published after successful Play");call("limit");return limit;
    }
    int create(const r::Descriptor32& d)override{check(d.flags==0x81 && d.size==20,"primary descriptor");int e=call("primary");if(e)primary=0;return e;}
    int deviceCaps(unsigned& flags)override{flags=10;return call("caps");}
    int setFormat(const mnm::audio::PcmFormat& f)override{check(f.channels==2 && f.bits==16 && f.alignment==4 && f.rate==22050,"primary format");return call("format");}
    int bufferBytes(unsigned& bytes)override{bytes=4096;return call("bytes");}
    int compact()override{return call("compact");}
    int getVolume(unsigned id,int& v)override{check(id==99,"primary GetVolume");v=-700;return call("getVolume");}
    int volume(unsigned id,int v)override{check(id==99 && v==manager.cache.manager.savedVolume,"restore saved volume");return call("restore");}
    int play(unsigned id,unsigned a,unsigned b,unsigned flags)override{check(id==99 && !a && !b && flags==1 && !manager.cache.manager.active,"primary looping before active publication");return call("play");}
    int stop(unsigned id)override{check(id==99 && !manager.cache.manager.active && globals.lastPrimaryStop==0,"inactive/error cleared before Stop");return call("stop");}
    unsigned tickCount()override{call("clock");return 10;}
    void retireVoice(unsigned id)override{retired.push_back(id);call("retire:"+std::to_string(id));}
    void releaseDevice(unsigned id)override{check(id==88 && !globals.device && !globals.manager,"globals cleared before device release");call("releaseDevice");}
    void releaseVoice(unsigned)override{throw std::runtime_error("unexpected generic release");}
    int getStatus(unsigned,unsigned& flags)override{flags=0;return 0;}
    int position(unsigned,unsigned)override{throw std::runtime_error("unexpected seek");}
    int pan(unsigned,int)override{throw std::runtime_error("unexpected pan");}
    void volumeRecord(r::VoiceContract&,int)override{throw std::runtime_error("unexpected scheduler volume");}
    void clearVoiceSchedule(unsigned)override{throw std::runtime_error("unexpected scheduler clearing");}
    void releaseBuffer(unsigned id)override{call("release:"+std::to_string(id));}
    void freeWrapper(unsigned id)override{call("wrapper:"+std::to_string(id));}
    unsigned profileValue(int id,std::string& value)override{value=std::to_string(id);return unsigned(value.size());}
    bool fileSize(const std::string&,unsigned& out)override{out=0;return false;}
    bool openWave(const std::string&)override{return true;}
    void closeWave()override{}
    unsigned waveBytes()override{return 1;}
    mnm::audio::PcmFormat waveFormat()override{return {1,1,22050,22050,1,8,0};}
    int createSource(const r::Descriptor32&,const mnm::audio::PcmFormat&,unsigned& out)override{out=++buffer;return 0;}
    int lockSource(unsigned,unsigned,std::uint8_t*& out)override{out=&sample;return 0;}
    unsigned readSource(std::uint8_t*&,unsigned bytes)override{return bytes;}
    int unlockSource(unsigned,std::uint8_t*,unsigned)override{return 0;}
    unsigned waveDuration()override{return 1;}
};
const std::vector<std::string> startup={"profile","Sounds","Randomised","group:30","group:40","device","cooperative","primary","caps","format","bytes","compact","getVolume","play","limit"};
void startupFailures(){
    for(auto stage:{"device","caps","format","compact","getVolume","play"})for(int error:{71,-71}){
        r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.fault=stage;b.error=error;
        check(r::initializeManager(b.services(),m,g,11,22,"root")==error,"original exact startup error preserved");
        if(b.fault=="device"){
            auto expected=std::vector<std::string>(startup.begin(),startup.begin()+6);expected.insert(expected.end(),{"free:classes","free:ids"});check(b.calls==expected,"device failure cleanup order");
            check(!m.cache.manager.initialized && m.ownsGroupDescriptors && m.cache.catalog.groups.size()==2 && g.manager==1,"device failure retains groups and constructor global");
            b.calls.clear();r::destroyManager(b.services(),m,g);check(b.calls==std::vector<std::string>({"wrapper:0"}) && m.ownsGroupDescriptors,"uninitialized destructor skips leaked original groups");
        }else{
            auto at=std::find(startup.begin(),startup.end(),stage);std::vector<std::string> expected(startup.begin(),at+1);
            expected.insert(expected.end(),{"restore","releaseDevice","free:classes","free:ids","free:groupIds","free:member1","free:member0","free:groups","free:classes","free:ids"});
            check(b.calls==expected,"post-device shutdown and outer free(NULL) sequence");
            check(!m.cache.manager.initialized && !m.cache.manager.active && m.cache.manager.primary==99 && !m.cache.manager.device && !g.manager && !g.device,"retained primary after failure cleanup");
            check(!m.ownsSourceIds && !m.ownsGroupDescriptors && m.sourceCount==2 && m.groupCount==2,"freed pointers retain counts");
        }
    }
    {r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);m.cache.manager.primary=99;m.cache.manager.savedVolume=-123;b.fault="cooperative";
        check(r::initializeManager(b.services(),m,g,11,22,"root")==71 && !m.cache.manager.initialized && m.cache.manager.primary==99,"cooperative failure with existing primary has valid shutdown path");}
    {r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.sections["Randomised"]={};
        check(!r::initializeManager(b.services(),m,g,11,22,"root") && !m.ownsGroupIds && !m.ownsGroupDescriptors && !m.groupCount,"empty groups allocate no group tables");
        b.calls.clear();r::shutdownManager(b.services(),m,g);check(std::find(b.calls.begin(),b.calls.end(),"free:groups")==b.calls.end(),"empty-group shutdown skips group free");}
    for(auto stage:{"cooperative","primary"}){
        r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.fault=stage;
        bool rejected=false;try{r::initializeManager(b.services(),m,g,11,22,"root");}catch(const std::domain_error&){rejected=true;}
        check(rejected && m.cache.manager.initialized && m.cache.manager.device==88 && m.ownsSourceIds,"absent-primary cleanup fault preserves pre-fault state");
        check(std::find(b.calls.begin(),b.calls.end(),"restore")==b.calls.end(),"no fake null-primary unwind");
    }
    {r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.accessible=false;check(r::initializeManager(b.services(),m,g,11,22,"root")==r::SourceLoadFailure && b.calls==std::vector<std::string>({"profile"}),"profile access early exit");}
    {r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.sections["Randomised"]={0x3ffe,{}};
        check(r::initializeManager(b.services(),m,g,11,22,"root")==r::SourceLoadFailure && m.ownsClasses && m.ownsSourceIds && !m.cache.manager.initialized,"early group parse failure retains allocated source tables");}
    {r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.fault="bytes";
        check(!r::initializeManager(b.services(),m,g,11,22,"root") && !m.primaryBytes && b.calls==startup,"ignored caps query failure yields unknown bytes but successful startup");}
    {r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);b.limit=1;
        bool rejected=false;try{r::initializeManager(b.services(),m,g,11,22,"root");}catch(const std::invalid_argument&){rejected=true;}
        check(rejected && g.device==88 && g.simultaneousLimit==1 && m.cache.manager.active && m.schedules.nodes.empty(),"unsupported schedule allocation preserves publication");}
}
void inactiveAndStraySlots(){
    r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);
    check(!r::initializeManager(b.services(),m,g,11,22,"root"),"inactive test startup");
    auto* node=m.schedules.head;unsigned slot=42;node->record.voiceAddress=7;node->record.outputAddress=8;node->output=&slot;
    m.cache.manager.active=false;b.calls.clear();b.fault="restore";
    check(!r::shutdownManager(b.services(),m,g) && slot==42 && node->record.voiceAddress==7,"inactive shutdown skips schedule retirement and ignores restore error");
    check(b.calls[0]=="restore" && b.calls[1]=="releaseDevice" && std::find(b.calls.begin(),b.calls.end(),"stop")==b.calls.end(),"inactive shutdown ordering");
    b.fault.clear();check(!r::initializeManager(b.services(),m,g,11,22,"root") && slot==42,"restart frees old schedules without touching old caller slot");
    node=m.schedules.head;node->record.outputAddress=8;node->output=&slot;
    r::shutdownManager(b.services(),m,g);check(slot==42 && !node->record.outputAddress,"voice-less stray slot not zeroed, record cleared");
}
void lifecycle(){
    r::AudioManager m;r::ManagerGlobals g{0,1,16};Backend b(m,g);
    check(!r::initializeManager(b.services(),m,g,11,22,"root") && b.calls==startup,"complete startup order");
    check(m.application==11 && m.window==22 && m.primaryBytes==4096 && m.schedules.nodes.size()==4,"startup outputs");
    check(!r::initializeSourcePool(b,m.cache,m.sources,7),"map source pool integration");
    m.sources.head->buffer=501;r::VoiceWrapper child;child.identity=77;child.buffer=502;m.sources.head->duplicate=&child;
    auto* n=m.schedules.head;unsigned slots[]={1,2,3,4};
    for(unsigned i=0;i<4;++i){n->record.voiceAddress=100+i;n->record.outputAddress=200+i;n->output=&slots[i];n->record.deadline=i==0?9:i==1?10:i==2?11:0xffffffffu;n=n->next;}
    b.calls.clear();b.fault="stop";
    check(!r::shutdownManager(b.services(),m,g) && b.retired==std::vector<unsigned>({102,103}),"shutdown expires at equality and retires future/looping only");
    check(b.calls[0]=="restore" && b.calls[1]=="clock" && b.calls[2]=="clock" && b.calls[3]=="clock" && b.calls[4]=="retire:102" && b.calls[5]=="retire:103" && b.calls[6]=="stop","restore/selection/stop order");
    for(auto slot:slots)check(!slot,"shutdown clears selected and expired voice slots");
    check(g.lastPrimaryStop==71 && m.cache.manager.lastPrimaryStop==71 && !m.cache.manager.initialized && m.cache.manager.primary==99 && m.schedules.nodes.size()==4 && m.sources.nodes.empty(),"shutdown keeps primary/schedule allocation, clears sources");
    auto release=std::find(b.calls.begin(),b.calls.end(),"release:502");check(release!=b.calls.end() && *(release+1)=="wrapper:77" && *(release+2)=="release:501","duplicate before root ownership release");
    b.calls.clear();check(!r::shutdownManager(b.services(),m,g) && b.calls.empty(),"repeated shutdown gate");
    b.fault.clear();check(!r::initializeManager(b.services(),m,g,11,22,"root"),"restart after shutdown");
    m.sources.disabled.identity=999;m.sources.disabled.buffer=601;child.buffer=602;child.duplicate=nullptr;m.sources.disabled.duplicate=&child;
    b.calls.clear();r::destroyManager(b.services(),m,g);
    check(b.calls[0]=="release:602" && b.calls[1]=="wrapper:77" && b.calls[2]=="release:601" && b.calls[3]=="wrapper:999" && b.calls[4]=="restore" && b.calls.back()=="free:schedules","disabled destruction before shutdown and scheduler free after");
    check(m.destroyed && !m.ownsDisabled && m.schedules.nodes.empty(),"destructor final ownership");
    b.calls.clear();r::destroyManager(b.services(),m,g);check(b.calls.empty(),"host destructor idempotence guard");
}
int main(){try{startupFailures();inactiveAndStraySlots();lifecycle();std::cout<<"Manager startup/failure/shutdown/destructor checks passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
