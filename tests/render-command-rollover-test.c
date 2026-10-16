/* Actual producer queue and mapped reader/writer. All expected bytes originate
 * in this fixture, not copied from a transport output. No game media required. */
#define QUEUE_TEST_MAIN legacy_queue_controls
#include "render-command-queue-test.c"

/* Use the production emitter with archive/log fakes. Transport below is the
 * actual producer queue/ring, not a mocked success return. */
static u32 session_active,session_sequence,session_bytes,session_archive_sequence,session_archive_bytes;
static u32 game_session_continuous=1,emission_gap,archive_gaps;
static HANDLE session_file;
static u32 session_cleanup_bytes(void){return 1180;}
static u32 session_cleanup_records(void){return 66;}
static void session_archive_close(void){session_file=0;}
static void session_archive_gap(u32 reason){check(reason==2);++archive_gaps;session_file=0;}
static void game_session_gap(u32 reason){emission_gap=reason;session_active=0;command_channel_fail(MNM_RENDER_COMMANDS_V2_REASON_GAP);}
static int command_record(HANDLE file,u32* sequence,u32 op,const void* fields,u32 fl,const void* pixels,u32 bytes){(void)file;(void)op;(void)fields;(void)fl;(void)pixels;(void)bytes;++*sequence;return 1;}
#define lock_diagnostic(...) ((void)0)
#include "../runtime/render/session_emission.h"
static void emission_reset(u32 continuous){
    game_session_continuous=continuous;session_active=1;session_sequence=0;session_bytes=16;
    session_archive_sequence=session_archive_bytes=emission_gap=archive_gaps=0;session_file=0;
}
static void emission_guards(void){
    u8 data[8]={1,3,5,7,9,11,13,15},out[32];u32 id=1;struct mnm_ring_reader reader;
    ring_reset_version(3);emission_reset(1);session_bytes=0xfffffff8u;
    check(session_record(5,&id,4,data,8) && session_active && session_bytes==0xfffffff8u);
    check(session_emit(5,&id,4,data,8,1) && session_sequence==2 && session_bytes==0xfffffff8u);
    check(mnm_ring_reader_bind(&reader,storage,mapped_size));command_channel_pump();
    check(mnm_ring_read(&reader,out,24)==24 && !memcmp(out+16,data,8));
    check(mnm_ring_read(&reader,out,24)==24 && !memcmp(out+16,data,8));command_channel_close();
    /* Diagnostic archive exhaustion must not consume continuous live policy. */
    ring_reset_version(3);emission_reset(1);session_file=storage;session_archive_bytes=64*1024*1024-1180;
    check(session_record(5,&id,4,data,8) && archive_gaps==1 && session_active && !session_file);command_channel_close();
    /* Bounded bytes/cleanup, finite sequence IDs and overflow-safe lengths stay. */
    for(u32 which=0;which<5;++which){
        ring_reset_version(3);emission_reset(which>=2);
        if(which==0)session_bytes=64*1024*1024-1180;
        if(which==1)session_sequence=4096-66;
        if(which==2)session_sequence=0xffffffffu-66;
        if(which==3)session_sequence=0xffffffffu;
        check(!(which==3?session_emit(5,&id,4,data,8,1):which==4?session_record(5,&id,0xfffffff0u,data,8):session_record(5,&id,4,data,8)) && emission_gap==2 && !storage[5]);
        command_channel_close();
    }
}
static void seed(u32 at,struct mnm_ring_reader* r){
    command_queue_written=command_queue_read=at;
    command_ring.published=command_ring.acknowledged=at;
    storage[5]=storage[9]=at;r->published=r->consumed=at;
}
static void rollover_guard_cases(void){
    u8 bytes[16];for(u32 i=0;i<sizeof(bytes);++i)bytes[i]=(u8)(i+41);
    struct mnm_ring_reader r;u8 out[16];
    /* Header version/magic cannot be changed after either peer binds. */
    ring_reset_version(3);check(mnm_ring_reader_bind(&r,storage,mapped_size));
    memcpy(storage,MNM_RENDER_COMMANDS_V2_MAGIC,8);storage[2]=2;
    check(!mnm_ring_writer_valid(&command_ring));check(mnm_ring_read(&r,out,1)==-1 && !storage[8]);
    memcpy(storage,MNM_RENDER_COMMANDS_V3_MAGIC,8);storage[2]=3;command_channel_close();
    /* Physical copy splits at rollover. A partial ACK across zero frees only
     * those bytes; an unacknowledged suffix remains owned and unchanged. */
    ring_reset_version(3);check(mnm_ring_reader_bind(&r,storage,mapped_size));seed(0xfffffff8u,&r);
    check(command_channel_append(bytes,3,bytes+3,7,bytes+10,6));command_channel_pump();
    check(command_queue_read==8 && command_queue_written==8 && command_ring.published==8);
    check(mnm_ring_read(&r,out,10)==10 && !memcmp(bytes,out,10));
    check(command_channel_append(bytes,16,0,0,0,0));command_channel_pump();
    check(mnm_ring_read(&r,out,6)==6 && !memcmp(bytes+10,out,6));
    check(mnm_ring_read(&r,out,16)==16 && !memcmp(bytes,out,16));
    command_channel_end();command_channel_pump();check(mnm_ring_read(&r,out,1)==0 && r.state==2);command_channel_close();
    /* Across zero, full still means no overwrite and cancellation remains sticky. */
    ring_reset_version(3);check(mnm_ring_reader_bind(&r,storage,mapped_size));seed(0xffff0000u,&r);
    u8* block=malloc(MNM_RENDER_COMMANDS_V3_CAPACITY);check(block!=0);memset(block,0x69,MNM_RENDER_COMMANDS_V3_CAPACITY);
    check(mnm_ring_write(&command_ring,block,MNM_RENDER_COMMANDS_V3_CAPACITY)==1);
    const u32 pub=storage[5];check(mnm_ring_write(&command_ring,bytes,1)==0 && storage[5]==pub);
    check(mnm_ring_read(&r,out,16)==16);for(u32 i=0;i<16;++i)check(out[i]==0x69);
    mnm_ring_cancel(&r);check(mnm_ring_write(&command_ring,bytes,1)==-1 && storage[7]==3);command_channel_close();free(block);
    /* Both backward and ahead ACK/publication are rejected at wrap. */
    for(u32 i=0;i<4;++i){
        ring_reset_version(3);check(mnm_ring_reader_bind(&r,storage,mapped_size));seed(0xfffffff8u,&r);
        check(mnm_ring_write(&command_ring,bytes,16)==1);
        if(i<2){storage[9]=i?9:0xfffffff7u;check(!mnm_ring_writer_valid(&command_ring) && storage[7]==5);}
        else{storage[5]=i==2?0xfffffff7u:MNM_RENDER_COMMANDS_V3_CAPACITY;check(mnm_ring_read(&r,out,1)==-1 && storage[8]==1);}
        command_channel_close();
    }
}
int main(int argc,char** argv){
    (void)argv;legacy_queue_controls();rollover_guard_cases();emission_guards();
    /* Real byte traffic, bounded 32MiB private/1MiB wire/64KiB read storage.
     * Sanitized controls use the accelerated boundary; --long also traverses
     * the entire 4GiB distance with actual queue appends and ACKs. */
    const uint64_t target=argc>1?((uint64_t)1<<32)+2*MNM_RENDER_COMMANDS_V3_CAPACITY:3*MNM_RENDER_COMMANDS_V3_CAPACITY;
    ring_reset_version(3);struct mnm_ring_reader r;check(mnm_ring_reader_bind(&r,storage,mapped_size));
    if(argc==1)seed(0xffff0000u,&r);
    const u32 record_bytes=MNM_RENDER_COMMANDS_V3_CAPACITY,payload_bytes=record_bytes-16;
    u8* input=malloc(payload_bytes);check(input!=0);u8 out[65536];uint64_t sent=0,received=0,payload_sent=0;u32 seq=0;
    emission_reset(1);
    while(sent<target){
        for(u32 i=0;i<payload_bytes;++i)input[i]=(u8)((payload_sent+i)*13u+((payload_sent+i)>>24)+7u);
        u32 id=1;check(session_record(5,&id,4,input,payload_bytes));++seq;
        const u32 expected[4]={5,seq,payload_bytes+4,1};sent+=record_bytes;memset(input,0xa5,payload_bytes);command_channel_pump();
        while(received<sent){
            int k=mnm_ring_read(&r,out,sizeof(out));check(k>0);
            for(int i=0;i<k;++i){
                const u32 at=(u32)((received+(u32)i)%record_bytes);
                const u8 value=at<16?((const u8*)expected)[at]:(u8)((payload_sent+at-16)*13u+((payload_sent+at-16)>>24)+7u);
                check(out[i]==value);
            }
            received+=(u32)k;
        }
        payload_sent+=payload_bytes;
    }
    check(session_bytes==16 && session_sequence==seq && !emission_gap && session_active);
    command_channel_end();command_channel_pump();check(mnm_ring_read(&r,out,1)==0 && r.state==2 && received==target && !command_queue_failure);
    check(storage[5]==storage[9] && command_queue_written==command_queue_read);command_channel_close();free(input);
    printf("{\"success\":true,\"version\":3,\"rollover_guard_cases\":8,\"emission_guard_cases\":8,\"queue_and_ring_actual_bytes\":%llu,\"long\":%s}\n",(unsigned long long)target,argc>1?"true":"false");return 0;
}
