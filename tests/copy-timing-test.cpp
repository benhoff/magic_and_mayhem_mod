#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <thread>
using u8=unsigned char;using u32=unsigned;using i32=int;using HANDLE=void*;
#define API
#define WIN
static u32 tick,mode,ioCalls,misses,wakes,qpcCalls;
static thread_local u32 threadId=1,lastError=0x77;
static const char* option="1";
static std::vector<u8> output;
static u32 GetLastError(){return lastError;}
static void SetLastError(u32 n){lastError=n;}
static u32 GetTickCount(){return tick;}
static u32 GetCurrentThreadId(){return threadId;}
static u32 GetEnvironmentVariableA(const char*,char* out,u32 size){auto n=std::strlen(option);if(n<size)std::memcpy(out,option,n+1);return u32(n);}
static void copy(void* dst,const void* src,u32 n){std::memcpy(dst,src,n);}
static void zero(void* dst,u32 n){std::memset(dst,0,n);}
static HANDLE CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE){++ioCalls;return reinterpret_cast<HANDLE>(1);}
static int CloseHandle(HANDLE){++ioCalls;return 1;}
static int write_all(HANDLE,const void* p,u32 n){++ioCalls;const auto* bytes=static_cast<const u8*>(p);output.insert(output.end(),bytes,bytes+n);return 1;}
#include "../runtime/render/copy_timing.h"
int QueryPerformanceCounter(unsigned long long* out){__atomic_add_fetch(&qpcCalls,1,__ATOMIC_RELAXED);*out=1000000+static_cast<unsigned long long>(tick)*1000;lastError=0xaa;return 1;}
int QueryPerformanceFrequency(unsigned long long* out){*out=1000000;lastError=0xaa;return 1;}
static u32 game_copy_order_enabled=1,game_lock_epoch,historyInvalid,command_channel_session=1516015097;
struct GameCopyLease {u32 held,observe,deferred;};
static u32* history_current(){return &historyInvalid;}
#define history_invalid historyInvalid
static void game_pixel_missed(void*){++misses;}
static void game_metadata_invalidate(){}
static u32 game_alias_reset_pending;
static void lock_diagnostic(const char*,void*,u32,u32,u32,i32,u32,const u32*){}
static void lock_diagnostic_values(const char*,const u32*){}
static i32 WaitOnAddress(volatile void*,void*,u32,u32);
static void WakeByAddressAll(void*){++wakes;lastError=0xbb;}
static void Sleep(u32){++tick;}
#include "../runtime/render/copy_order.h"
static void require(bool ok,const char* why){if(!ok){std::fprintf(stderr,"Copy timing: %s\n",why);std::exit(1);}}
static u32 deferStarts,deferEnds;
static void deferredStatus(u32 begin){
    if(begin){require(game_copy_owner==threadId,"status deferral begins under admission");++deferStarts;}
    else{require(!game_copy_owner&&!copy_trace_owner_ready,"status flush follows diagnostic close and admission release");++deferEnds;}
    lastError=0xaa;
}
static void reset(bool enabled){
    tick=mode=ioCalls=misses=wakes=qpcCalls=game_copy_owner=game_copy_started=game_copy_site=game_copy_phase=0;
    game_lock_epoch=historyInvalid=game_alias_reset_pending=0;threadId=1;lastError=0x77;
    copy_trace_enabled=copy_trace_written=copy_trace_read=copy_trace_dropped=copy_trace_wait_id=copy_trace_armed=0;
    copy_trace_owner_sequence=copy_trace_owner_ready=copy_trace_acquisitions=0;copy_trace_file=nullptr;
    zero(copy_trace_slots,sizeof(copy_trace_slots));zero(copy_trace_owner,sizeof(copy_trace_owner));output.clear();
    option=enabled?"1":"0";copy_trace_init("capture",7);require(lastError==0x77,"init LastError");
    game_copy_native_defer=nullptr;deferStarts=deferEnds=0;
}
static void deferredRelease(){
    for(bool enabled:{false,true}){
        reset(enabled);game_copy_native_defer=deferredStatus;
        auto lease=game_copy_enter_at(nullptr,17);
        require(lease.held&&lease.deferred&&deferStarts==1&&lastError==0x77,"deferred admission preserves LastError");
        lastError=0x88;game_copy_leave(&lease);
        require(!lease.held&&!lease.deferred&&deferEnds==1&&wakes==1&&lastError==0x88,"deferred release preserves LastError");
        game_copy_leave(&lease);require(deferEnds==1&&wakes==1,"release and status flush are idempotent");
    }
}
static void holder(u32 at){
    tick=at;game_copy_owner=2;game_copy_started=at;game_copy_site=56;game_copy_phase=0;
    copy_trace_owner_open(reinterpret_cast<void*>(0x9000),56,2,at);copy_trace_owner_phase(4);
}
static i32 WaitOnAddress(volatile void*,void*,u32,u32 milliseconds){
    require(milliseconds<=50,"unchanged wait budget");
    if(mode==1){tick+=65;return 0;}
    if(mode==2){for(u32 at:{25u,40u,49u}){copy_trace_owner_close();game_copy_owner=0;holder(at);}tick=65;return 1;}
    tick+=10;copy_trace_owner_close();game_copy_owner=0;return 1;
}
static void admission(){
    for(bool enabled:{false,true}){
        reset(enabled);auto lease=game_copy_enter_at(reinterpret_cast<void*>(0x1000),17);
        require(lease.held&&lease.observe&&lastError==0x77,"immediate admission unchanged");
        game_copy_stage(3);lastError=0x88;game_copy_leave(&lease);require(lastError==0x88&&!game_copy_owner&&wakes==1,"release unchanged");
        require(!copy_trace_written&&!ioCalls,"no success I/O or event storm");
        if(!enabled)require(!qpcCalls,"disabled has no high-resolution calls");
        reset(enabled);holder(0);mode=1;
        lease=game_copy_enter_at(reinterpret_cast<void*>(0x1000),17);
        require(!lease.held&&!lease.observe&&tick==65&&game_lock_epoch==1&&misses==1&&historyInvalid==1&&lastError==0x77,"timeout forwarding/invalidation policy unchanged");
        require(!ioCalls,"timeout producer does no I/O");
        if(!enabled)require(!copy_trace_written&&!qpcCalls,"disabled timeout silent");
        else {
            require(copy_trace_written==3,"three bounded timeout records");
            auto* v=copy_trace_slots[0].words+6;require(v[2]==65&&v[6]==1&&v[7]==1&&v[12]==1,"single holder summary");
            copy_trace_drain();require(output.size()==32+3*100&&std::memcmp(output.data(),"MNMCOPY1",8)==0,"worker file contract");
        }
        reset(enabled);holder(0);mode=0;lease=game_copy_enter_at(reinterpret_cast<void*>(0x1000),17);
        require(lease.held&&lease.observe&&tick==10&&lastError==0x77&&!historyInvalid,"released holder admission");game_copy_leave(&lease);
        reset(enabled);game_copy_owner=1;lease=game_copy_enter_at(nullptr,9);require(!lease.held&&lease.observe,"original reentry unchanged");
        lease=game_copy_bind_enter();require(!lease.held&&!lease.observe&&!copy_trace_written,"bind reentry is not timeout");
        reset(enabled);holder(0);mode=1;lease=game_copy_observation_enter_at(nullptr,19);
        require(!lease.held&&!lease.observe&&tick==65&&!game_lock_epoch&&!misses&&!historyInvalid&&lastError==0x77,"read-only observation timeout preserves drawing history");
        command_callback_missed();require(game_alias_reset_pending==1,"callback invalidation unchanged");
        reset(enabled);lease=game_copy_shutdown_enter();require(lease.held&&lease.observe,"shutdown admission unchanged");game_copy_leave(&lease);
    }
}
static void reacquisition(){
    reset(true);holder(0);mode=2;auto lease=game_copy_enter_at(reinterpret_cast<void*>(0x1000),17);
    require(!lease.observe&&copy_trace_written==3,"reacquisition timeout");
    auto* v=copy_trace_slots[0].words+6;require(v[6]==1&&v[7]==4&&v[5]==1&&v[11]==65,"three intervening published acquisitions and delayed wake");
    auto* owner=copy_trace_slots[2].words+6;require(owner[2]==1&&owner[3]==2&&owner[4]==4&&owner[5]==49&&65-owner[5]==16,"65ms wait with coherent16ms final owner age");
    require(owner[18]==v[18]&&copy_trace_slots[1].words[24]==v[18],"triplet correlation");
    require(!ioCalls,"reacquisition no I/O in drawing thread");
    u32 fields[8];copy_trace_owner_close();require(!copy_trace_sample(&game_copy_owner,fields)&&fields[0]==0,"unpublished holder rejected");
    copy_trace_owner_ready=1;copy_trace_owner_sequence|=1;require(!copy_trace_sample(&game_copy_owner,fields),"torn publication rejected");
    copy_trace_owner_sequence+=1;game_copy_owner=3;require(!copy_trace_sample(&game_copy_owner,fields),"different holder rejected");
}
static void boundedRecords(){
    reset(true);u32 values[19]={};
    values[0]=5;copy_trace_record(COPY_TRACE_DROP,7,values);
    require(!copy_trace_written,"ordinary startup cannot consume fault capacity");
    copy_trace_record(COPY_TRACE_RECOVERY,7,values);values[0]=0;copy_trace_record(COPY_TRACE_DROP,7,values);
    require(copy_trace_written==1,"complete replacements excluded");
    values[0]=5;for(u32 i=0;i<400;++i)copy_trace_record(COPY_TRACE_DROP,7,values);
    require(copy_trace_written==256&&copy_trace_dropped==145,"discard provenance bounded to half capacity");
    copy_trace_record(COPY_TRACE_CLOCK,7,values);require(copy_trace_written==257,"fault timing retains reserved capacity");
    reset(true);
    std::vector<std::thread> writers;for(u32 t=0;t<4;++t)writers.emplace_back([t]{threadId=t+10;u32 v[19]={t};for(u32 i=0;i<200;++i){v[1]=i;copy_trace_record(COPY_TRACE_CLOCK,7,v);}});
    for(auto& writer:writers)writer.join();
    require(copy_trace_written==512&&copy_trace_dropped==288&&!ioCalls,"bounded concurrent producers");
    for(u32 i=0;i<512;++i)require(copy_trace_slots[i].ready&&copy_trace_slots[i].words[0]==COPY_TRACE_CLOCK,"immutable complete slots");
    copy_trace_slots[0].ready=0;copy_trace_drain();require(copy_trace_read==0&&!ioCalls,"worker never waits on unpublished slot");
    copy_trace_slots[0].ready=1;copy_trace_drain();require(copy_trace_read==512&&output.size()==32+512*100,"finite worker drain");
    reset(true);copy_trace_record(COPY_TRACE_RECOVERY,7,values);copy_trace_record(COPY_TRACE_CHECKPOINT,7,values);copy_trace_drain();
    require(output.size()==232,"recovery records same bounded contract");
    reset(false);option="yes";copy_trace_init("capture",7);require(!copy_trace_enabled,"strict opt in");
}
int main(int argc,char** argv){admission();deferredRelease();reacquisition();copy_trace_drain();
    if(argc==2){auto* file=std::fopen(argv[1],"wb");require(file&&std::fwrite(output.data(),1,output.size(),file)==output.size(),"fixture log write");std::fclose(file);}
    boundedRecords();std::puts("{\"success\":true,\"admission_modes\":2,\"reacquisition_case\":true,\"bounded_records\":512,\"dropped_records\":288,\"live_validated\":false}");}
