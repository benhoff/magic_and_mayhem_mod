// Run reconstructed scheduler decisions and, in the i386 build, unchanged PE
// bodies against the same authored inputs. Only clock/COM boundaries are fake.
#include "voice_scheduler.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifdef MNM_SCHEDULER_REFERENCE
#include <fstream>
#include <iterator>
#include <sys/mman.h>
#endif
namespace r = mnm::reconstruction::audio;
namespace {
void check(bool b, const char* reason) { if (!b) throw std::runtime_error(reason); }
struct Event {
    unsigned kind, value;
    bool operator==(const Event& e) const { return kind == e.kind && value == e.value; }
};
struct Backend : r::SchedulerBackend {
    unsigned now=10, flags=1;
    int statusError=0, stopError=0, resetError=0;
    bool buffers=true;
    std::vector<Event> trace;
    unsigned tickCount() override { trace.push_back({0,now}); return now; }
    unsigned bufferForVoice(unsigned id) override { return buffers ? id : 0; }
    r::Status getStatus(unsigned id,unsigned& out) override { trace.push_back({1,id}); out=flags; return statusError; }
    r::Status stop(unsigned id) override { trace.push_back({2,id}); return stopError; }
    r::Status position(unsigned id,unsigned p) override { check(p==0,"reset byte"); trace.push_back({3,id}); return resetError; }
    r::Status volume(unsigned,int) override { throw std::runtime_error("unexpected volume"); }
    r::Status pan(unsigned,int) override { throw std::runtime_error("unexpected pan"); }
    r::Status play(unsigned,unsigned,unsigned,unsigned) override { throw std::runtime_error("unexpected play"); }
    void volumeRecord(r::VoiceContract&,int) override { throw std::runtime_error("unexpected notification"); }
};
struct Pool {
    std::array<r::ScheduleNode,5> nodes{};
    std::array<unsigned,5> slots{};
    r::ScheduleNode* head;
    unsigned count;
    Pool(unsigned n,unsigned first,unsigned states,unsigned scores,unsigned now,bool outputs):head(&nodes[first]),count(n) {
        for(unsigned i=0;i<n;++i) {
            auto& v=nodes[i]; const unsigned state=states%4; states/=4;
            const int volume=int(scores%3)*5000-5000; scores/=3;
            slots[i]=0xabcdef00+i;
            v.next=&nodes[(i+1)%n]; v.previous=&nodes[(i+n-1)%n]; v.output=&slots[i];
            v.record={0,0,state==1?now:state==2?now+11:0xffffffffu,volume,
                      state?i+1:0,outputs?i+1:0,int(i*7),-int(i*9)-1};
        }
    }
    unsigned index(const r::ScheduleNode* p) const {
        if(!p)return 0xffffffffu;
        for(unsigned i=0;i<count;++i)if(p==&nodes[i])return i;
        throw std::runtime_error("foreign native ring node");
    }
    void validate() const {
        auto* p=head;unsigned visited=0;
        do { check(p->next->previous==p && p->previous->next==p,"native ring links");
             ++visited;check(visited<=count,"bounded native ring");p=p->next;
        } while(p!=head);
        check(visited==count,"native membership");
    }
};
enum Operation { Select, Evict, Retire, Reorder, Clear, Assign };
struct Input {
    unsigned n=3,head=0,states=63,scores=13,target=0,now=10,duration=32;
    int volume=0,x=-1,y=-1,statusError=0,stopError=0,resetError=0;
    unsigned flags=1,gates=3;
    bool outputs=true,buffers=true,loop=false;
};
Input lastInput;
Operation lastOperation=Select;
std::array<std::uint64_t,6> counts{};
std::uint64_t events=0,guardBytes=0;
#ifdef MNM_SCHEDULER_REFERENCE
using Bytes=std::vector<unsigned char>;
unsigned word(const Bytes& b,unsigned at) { check(at<=b.size() && b.size()-at>=4,"PE extent");unsigned v;std::memcpy(&v,b.data()+at,4);return v; }
struct Guarded {
    std::array<unsigned,4> before; std::array<unsigned,10> data; std::array<unsigned,4> after;
    bool operator==(const Guarded& other) const { return before==other.before && data==other.data && after==other.after; }
};
static std::vector<std::pair<unsigned,Bytes>> originalBodies;
void mapImage(const char* name) {
    std::ifstream file(name,std::ios::binary);check(bool(file),"PE open");
    Bytes bytes(std::istreambuf_iterator<char>(file),{});
    const auto pe=word(bytes,60),opt=pe+24,length=word(bytes,opt+56);
    check(word(bytes,pe)==0x4550 && word(bytes,opt+28)==0x400000,"PE identity/base");
    check(length>0x2fa79c && length<16*1024*1024,"PE image bound");
    auto* memory=static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000),length,
        PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
    check(memory!=MAP_FAILED,"PE map");
    const auto sections=opt+(word(bytes,pe+20)&0xffff),count=word(bytes,pe+4)>>16;
    check(count<=32,"PE section count");
    for(unsigned i=0;i<count;++i) {
        const auto at=sections+40*i,rva=word(bytes,at+12),size=word(bytes,at+16),offset=word(bytes,at+20);
        check(offset<=bytes.size() && size<=bytes.size()-offset && rva<=length && size<=length-rva,"PE section bound");
        std::memcpy(memory+rva,bytes.data()+offset,size);
    }
    for(auto range:std::vector<std::pair<unsigned,unsigned>>{{0x571ab0,0x571bd1},{0x571be0,0x571c2e},
            {0x571c30,0x571d16},{0x571d20,0x571d7d},{0x571d80,0x571eb1},{0x571ec0,0x571f7a}})
        originalBodies.push_back({range.first,Bytes(reinterpret_cast<unsigned char*>(range.first),reinterpret_cast<unsigned char*>(range.second))});
}
unsigned address(const void* p) { return unsigned(reinterpret_cast<std::uintptr_t>(p)); }
Backend* script;
unsigned __attribute__((stdcall)) ticks() { return script->tickCount(); }
struct Com { unsigned table,token; };
int __attribute__((stdcall)) status(Com* self,unsigned* out) { return script->getStatus(self->token,*out); }
int __attribute__((stdcall)) stop(Com* self) { return script->stop(self->token); }
int __attribute__((stdcall)) position(Com* self,unsigned byte) { return script->position(self->token,byte); }
struct Raw {
    std::array<Guarded,5> records,voices,slots;
    Guarded context,manager;
    std::array<unsigned,20> vtable{};
    std::array<Com,5> buffers{};
    explicit Raw(const Input& in,const Pool& p) {
        for(auto* list:{&records,&voices,&slots})for(auto& v:*list)std::memset(&v,0xa5,sizeof(v));
        std::memset(&context,0xa5,sizeof(context));std::memset(&manager,0xa5,sizeof(manager));
        vtable[9]=address(reinterpret_cast<void*>(&status));vtable[18]=address(reinterpret_cast<void*>(&stop));
        vtable[13]=address(reinterpret_cast<void*>(&position));
        for(unsigned i=0;i<in.n;++i) {
            buffers[i]={address(vtable.data()),i+1};voices[i].data[0]=in.buffers?address(&buffers[i]):0;
            voices[i].data[5]=unsigned(in.volume);voices[i].data[9]=in.duration;
        }
        manager.data[6]=(in.gates&1)?0x80000000u:0;manager.data[7]=(in.gates&2)?0xa5a5a5a5u:0;
        expected(p,*this);
    }
    unsigned record(unsigned i)const{return address(records[i].data.data());}
    unsigned voice(unsigned i)const{return address(voices[i].data.data());}
    unsigned slot(unsigned i)const{return address(slots[i].data.data());}
    // Addresses belong to this Raw fixture even when writing an expected copy.
    void expected(const Pool& p,Raw& out) const {
        out.context.data[1]=record(p.index(p.head));
        for(unsigned i=0;i<p.count;++i) {
            const auto& n=p.nodes[i];const auto& v=n.record;auto& d=out.records[i].data;
            d[0]=record(p.index(n.next));d[1]=record(p.index(n.previous));d[2]=v.deadline;d[3]=unsigned(v.volume);
            d[4]=v.voiceAddress?voice(v.voiceAddress-1):0;d[5]=v.outputAddress?slot(i):0;
            d[6]=unsigned(v.x);d[7]=unsigned(v.y);out.slots[i].data[0]=p.slots[i];
        }
    }
};
#endif
void runCase(Operation op,const Input& in) {
    lastInput=in;lastOperation=op;
    Pool p(in.n,in.head,in.states,in.scores,in.now,in.outputs);
    Backend b;b.now=in.now;b.flags=in.flags;b.statusError=in.statusError;
    b.stopError=in.stopError;b.resetError=in.resetError;b.buffers=in.buffers;
    const auto initial=b;
#ifdef MNM_SCHEDULER_REFERENCE
    Raw raw(in,p);const auto before=raw;
#endif
    unsigned selected=0xffffffffu;
    switch(op) {
    case Select:selected=p.index(r::selectSchedule(b,p.head,in.volume,in.gates==3));break;
    case Evict:selected=p.index(&r::evictSchedule(b,p.head,p.nodes[in.target],in.gates==3));break;
    case Retire:r::retireSchedule(p.head,p.nodes[in.target]);break;
    case Reorder:r::updateScheduleVolume(p.head,p.nodes[in.target],in.volume);break;
    case Clear:r::clearSchedules(b,p.head,in.gates==3);break;
    case Assign:{r::VoiceWrapper v;v.identity=in.target+1;v.duration=in.duration;v.cachedVolume=in.volume;
        r::assignSchedule(b,p.nodes[in.target],v,in.outputs?in.target+1:0,&p.slots[in.target],in.loop,in.x,in.y);break;}
    }
    p.validate();events+=b.trace.size();++counts[op];
#ifdef MNM_SCHEDULER_REFERENCE
    auto expected=before;raw.expected(p,expected);static Backend observed;observed=initial;script=&observed;
    *reinterpret_cast<unsigned*>(0x5c5164)=address(reinterpret_cast<void*>(&ticks));
    *reinterpret_cast<unsigned*>(0x6f437c)=address(raw.manager.data.data());
    using One=unsigned(__attribute__((thiscall))*)(void*,unsigned);
    using Two=void(__attribute__((thiscall))*)(void*,unsigned,int);
    using None=void(__attribute__((thiscall))*)(void*);
    using Six=void(__attribute__((thiscall))*)(void*,unsigned,unsigned,unsigned,unsigned,int,int);
    auto* self=raw.context.data.data();unsigned result=0;
    switch(op) {
    case Select:result=reinterpret_cast<One>(0x571ab0)(self,unsigned(in.volume));break;
    case Evict:result=reinterpret_cast<One>(0x571c30)(self,raw.record(in.target));break;
    case Retire:reinterpret_cast<One>(0x571d20)(self,raw.record(in.target));break;
    case Reorder:reinterpret_cast<Two>(0x571d80)(self,raw.voice(in.target),in.volume);break;
    case Clear:reinterpret_cast<None>(0x571ec0)(self);break;
    case Assign:reinterpret_cast<Six>(0x571be0)(self,raw.record(in.target),raw.voice(in.target),in.outputs?raw.slot(in.target):0,in.loop,in.x,in.y);break;
    }
    script=nullptr;
    if(op==Select || op==Evict)check(result==(selected==0xffffffffu?0:raw.record(selected)),"original selected record");
    check(observed.trace==b.trace,"original clock/COM event order");
    check(raw.records==expected.records,"original complete record/link/guard bytes");
    check(raw.voices==expected.voices,"original wrapper/guard bytes");
    check(raw.slots==expected.slots,"original output slot/guard bytes");
    check(!std::memcmp(&raw.context,&expected.context,sizeof(Guarded)),"original context/guard bytes");
    check(!std::memcmp(&raw.manager,&expected.manager,sizeof(Guarded)),"original manager gate/guard bytes");
    check(raw.vtable==expected.vtable && !std::memcmp(raw.buffers.data(),expected.buffers.data(),sizeof(raw.buffers)),"original COM fixture unchanged");
    guardBytes+=17*sizeof(Guarded);
#else
    (void)initial;(void)selected;
#endif
}
unsigned power(unsigned base,unsigned n) {unsigned x=1;while(n--)x*=base;return x;}
}
int main(int argc,char** argv)try {
#ifdef MNM_SCHEDULER_REFERENCE
    check(argc==2,"expected pinned PE input");mapImage(argv[1]);
#else
    (void)argc;(void)argv;
#endif
    Input in;
    for(unsigned n=2;n<=4;++n)for(unsigned states=0;states<power(4,n);++states)
    for(unsigned scores=0;scores<(n==4?12:power(3,n));++scores)
    for(unsigned head=0;head<n;++head)for(int volume:{-10000,-5000,0,5000})
    for(unsigned now:{0u,10u,0xfffffff0u}) {
        in=Input{};in.n=n;in.states=states;in.scores=scores;in.head=head;in.volume=volume;in.now=now;
        runCase(Select,in);
    }
    for(unsigned n=2;n<=4;++n)for(unsigned head=0;head<n;++head)for(unsigned target=0;target<n;++target)
    for(unsigned gates=0;gates<4;++gates)for(unsigned flags:{0u,1u,5u})for(int query:{0,-17})
    for(int stopError:{0,-19})for(int resetError:{0,-23})for(bool buffers:{false,true})
    for(bool outputs:{false,true})for(unsigned states:{0u,85u,170u,255u}) {
        in=Input{};in.n=n;in.head=head;in.target=target;in.gates=gates;in.flags=flags;
        in.statusError=query;in.stopError=stopError;in.resetError=resetError;in.buffers=buffers;
        in.outputs=outputs;in.states=states;runCase(Evict,in);runCase(Clear,in);runCase(Retire,in);
    }
    for(unsigned n=2;n<=5;++n)for(unsigned head=0;head<n;++head)for(unsigned target=0;target<n;++target)
    for(unsigned scores=0;scores<power(3,n);++scores)for(int volume:{-10000,-5000,0,5000,10000}) {
        in=Input{};in.n=n;in.head=head;in.target=target;in.scores=scores;in.volume=volume;
        in.states=power(4,n)-1;runCase(Reorder,in);
    }
    for(unsigned n=2;n<=5;++n)for(unsigned head=0;head<n;++head)for(unsigned target=0;target<n;++target)
    for(unsigned now:{0u,10u,0xfffffff0u,0xffffffffu})for(unsigned duration:{0u,32u,0xffffffffu})
    for(bool outputs:{false,true})for(bool loop:{false,true})for(int volume:{-10000,0,10000}) {
        in=Input{};in.n=n;in.head=head;in.target=target;in.now=now;in.duration=duration;
        in.outputs=outputs;in.loop=loop;in.volume=volume;in.x=std::numeric_limits<int>::min();
        in.y=std::numeric_limits<int>::max();runCase(Assign,in);
    }
#ifdef MNM_SCHEDULER_REFERENCE
    for(const auto& body:originalBodies)check(!std::memcmp(reinterpret_cast<void*>(body.first),body.second.data(),body.second.size()),"original scheduler instructions changed");
#endif
    std::cout<<"{\"success\":true,\"operations\":[";
    std::uint64_t total=0;for(unsigned i=0;i<counts.size();++i){if(i)std::cout<<',';std::cout<<counts[i];total+=counts[i];}
    std::cout<<"],\"cases\":"<<total<<",\"events\":"<<events<<",\"guard_bytes\":"<<guardBytes<<"}\n";
    return 0;
}catch(const std::exception& e){
    std::cerr<<e.what()<<"; op="<<lastOperation<<" n="<<lastInput.n<<" head="<<lastInput.head
             <<" target="<<lastInput.target<<" states="<<lastInput.states<<" scores="<<lastInput.scores
             <<" volume="<<lastInput.volume<<" now="<<lastInput.now<<" gates="<<lastInput.gates
             <<" buffers="<<lastInput.buffers<<" outputs="<<lastInput.outputs
             <<" flags="<<lastInput.flags<<" status="<<lastInput.statusError
             <<" stop="<<lastInput.stopError<<" reset="<<lastInput.resetError<<'\n';return 1;
}
