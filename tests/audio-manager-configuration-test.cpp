#include "manager_configuration.hpp"
#include <iostream>
#include <map>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
r::ProfileSection section(std::vector<std::string> entries){r::ProfileSection s;for(auto& e:entries)s.returned+=unsigned(e.size()+1);s.entries=std::move(entries);return s;}
struct Backend:r::ConfigurationBackend {
    std::map<std::string,r::ProfileSection> sections;std::map<int,std::string> groups;
    std::vector<unsigned> freed,released;std::vector<int> opens;std::vector<std::string> paths;
    unsigned limit=9;
    unsigned profileInteger(const std::string& section,const std::string& key,unsigned fallback)override{
        check(section=="Optimisation" && key=="MaxSimultaneousSounds" && fallback==16,"profile integer contract");return limit;
    }
    unsigned buffer=100;int current=0,failSound=-1;unsigned bytes=46080;
    std::uint8_t sample=0;
    r::ProfileSection section(const std::string& n,unsigned capacity)override{check(capacity==16384,"section capacity");return sections[n];}
    unsigned groupValue(int id,std::string& out)override{out=groups[id];return unsigned(out.size());}
    unsigned profileValue(int id,std::string& out)override{current=id;out=std::to_string(id);return unsigned(out.size());}
    bool fileSize(const std::string& p,unsigned& out)override{paths.push_back(p);out=bytes;return p!="root\\30.wav";}
    bool openWave(const std::string&)override{opens.push_back(current);return current!=failSound;}
    void closeWave()override{}
    unsigned waveBytes()override{return 1;}
    mnm::audio::PcmFormat waveFormat()override{return {1,1,22050,22050,1,8,0};}
    int createSource(const r::Descriptor32&,const mnm::audio::PcmFormat&,unsigned& out)override{out=++buffer;return 0;}
    int lockSource(unsigned,unsigned,std::uint8_t*& out)override{out=&sample;return 0;}
    unsigned readSource(std::uint8_t*&,unsigned n)override{return n;}
    int unlockSource(unsigned,std::uint8_t*,unsigned)override{return 0;}
    unsigned waveDuration()override{return 1;}
    int getStatus(unsigned,unsigned& flags)override{flags=0;return 0;}
    int stop(unsigned)override{throw std::runtime_error("unexpected Stop");}
    int position(unsigned,unsigned)override{throw std::runtime_error("unexpected position");}
    int volume(unsigned,int)override{throw std::runtime_error("unexpected volume");}
    int pan(unsigned,int)override{throw std::runtime_error("unexpected pan");}
    int play(unsigned,unsigned,unsigned,unsigned)override{throw std::runtime_error("unexpected Play");}
    void volumeRecord(r::VoiceContract&,int)override{throw std::runtime_error("unexpected scheduler");}
    void clearVoiceSchedule(unsigned)override{throw std::runtime_error("unexpected scheduler clear");}
    void releaseBuffer(unsigned id)override{released.push_back(id);}
    void freeWrapper(unsigned id)override{freed.push_back(id);}
};
void tables(){
    check(r::soundTable(section({";ignored","30=c","10=a","30=b","0=x","-1=y","20junk=z"}))==std::vector<int>({0,10,20,30}),"positive atoi, sorted duplicate slack");
    for(auto entry:{"missing","=bad","1234567=bad"}){bool threw=false;try{r::soundTable(section({entry}));}catch(const std::invalid_argument&){threw=true;}check(threw,"invalid key rejected");}
    check(r::groupMembers("10 20, +30,0,-1,30junk,,40")==std::vector<int>({10,30,30,40}),"comma-only strtok and atoi preserve duplicates/order");
    check(r::managerProfilePath("root")=="root\\Sounds.ini","manager profile path");
    bool overflow=false;try{r::managerProfilePath(std::string(250,'x'));}catch(const std::invalid_argument&){overflow=true;}check(overflow,"bounded profile path");
    Backend b;b.sections["Sounds"]=section({"30=c","10=a"});b.sections["Randomised"]=section({"99=x","99=y","88=z"});b.groups[99]="30,10,30";
    r::AdmissionCatalog catalog;check(!r::loadManagerCatalog(b,catalog),"catalog load");
    check(catalog.groups.size()==3 && catalog.groups[0].id==0 && catalog.groups[1].id==88 && catalog.groups[2].members==std::vector<int>({30,10,30}),"group zero slack and lookup ordering");
    b.sections["Randomised"]={0x3ffe,{}};check(r::loadManagerCatalog(b,catalog)==r::SourceLoadFailure && catalog.sourceIds==std::vector<int>({10,30}),"group truncation retains published source table");
    b.sections["Sounds"]={};check(r::loadManagerCatalog(b,catalog)==r::SourceLoadFailure,"empty source section fails");
}
void ringsAndStartup(){
    r::SourceCacheState state;state.catalog.sourceIds={10,20,30};state.assetRoot="root";
    Backend b;r::SourcePool pool;check(!r::initializeSourcePool(b,state,pool,7) && pool.nodes.empty(),"uninitialized gate");
    state.manager.initialized=state.manager.active=true;
    b.sections["7 Load Permanent"]=section({"30","10","10","20","999"});
    b.sections["7 Load Temporary"]=section({"20"});
    check(!r::initializeSourcePool(b,state,pool,7),"source startup");
    check(state.classes==std::vector<unsigned>({0,2,0}) && state.field22c==4,"duplicate permanent count and temporary override");
    check(pool.pinnedBytes==46080 && state.field234==25 && pool.nodes.size()==25,"stat-success-only budget / 46080");
    check(b.paths==std::vector<std::string>({"root\\10.wav","root\\30.wav"}) && b.opens==std::vector<int>({10,30}),"sorted pinned lookup/preload");
    auto* node=pool.head;for(unsigned i=0;i<25;++i){check(node->next->previous==node && node->previous->next==node,"source bidirectional links");node=node->next;}check(node==pool.head,"circular ring");
    check(pool.disabled.buffer==0 && pool.disabled.sourceIndex==-1 && pool.disabled.requestedVolume==99,"disabled wrapper defaults");
    std::vector<unsigned> disposal;node=pool.head->previous;for(unsigned i=0;i<25;++i){disposal.push_back(node->identity);node=node->previous;}
    b.failSound=30;check(r::initializeSourcePool(b,state,pool,7)==r::SourceLoadFailure,"preload exact error");
    check(b.freed.size()==25 && b.released.size()==2 && pool.head && pool.nodes.size()==25,"old ring released, failed new ring retained");
    check(b.freed==disposal,"backward old-pool teardown order");
    bool loaded=false;for(auto& v:pool.nodes)loaded|=v->sourceIndex==10;check(loaded,"earlier preload retained");
    b.sections["8 Load Permanent"]={0x3ffe,{}};check(r::initializeSourcePool(b,state,pool,8)==r::SourceLoadFailure && !pool.head && pool.nodes.empty(),"old ring disposal before profile failure");
    r::SchedulePool schedules;r::initializeSchedulePool(schedules,2);unsigned external=99;schedules.head->output=&external;schedules.head->record.voiceAddress=88;
    unsigned limit=16;r::initializeConfiguredSchedules(b,schedules,limit);check(limit==9,"global limit updated");check(external==99,"rebuild does not clear external slot");
    auto* n=schedules.head;for(unsigned i=0;i<9;++i){check(n->next->previous==n && n->record.volume==-5000 && n->record.x==-1 && !n->record.voiceAddress,"schedule defaults and links");n=n->next;}check(n==schedules.head,"schedule circle");
    b.limit=1;limit=16;bool badLimit=false;try{r::initializeConfiguredSchedules(b,schedules,limit);}catch(const std::invalid_argument&){badLimit=true;}check(badLimit && limit==1,"profile value published before rejected host allocation");
    for(unsigned count:{0u,1u,65537u}){bool threw=false;try{r::initializeSchedulePool(schedules,count);}catch(const std::invalid_argument&){threw=true;}check(threw && schedules.nodes.size()==9,"invalid count explicit guard preserves host pool");}
}
int main(){try{
    tables();ringsAndStartup();
    // Independent division oracle across wrap domain and threshold boundaries.
    for(std::uint64_t n=0;n<=0xffffffffull;n+=7919)check(r::dynamicSourceCapacity(0x100000u-unsigned(n))==n/46080,"unsigned budget division oracle");
    for(unsigned n:{0u,46079u,46080u,46081u,0xffffffffu})check(r::dynamicSourceCapacity(0x100000u-n)==n/46080,"division edge");
    std::cout<<"Manager catalog, source/schedule pool and budget checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
