#define main legacy_writer_main
#include "render-command-writer-test.c"
#undef main
static void ring_reset(void){
    check(!command_queue);reset();command_channel_refused=0;env=1;mapped_size=MNM_RENDER_COMMANDS_V2_SIZE;
    memcpy(storage,MNM_RENDER_COMMANDS_V2_MAGIC,8);storage[2]=2;storage[3]=mapped_size;
    command_queue_written=command_queue_read=command_queue_end=command_queue_failure=0;command_queue_draining=0;
    command_queue_timeout=command_queue_armed=command_queue_since=command_queue_ack=command_queue_timed_out=0;
    command_queue_peak=command_queue_full=command_queue_blocked=clock_ms=0;
    command_channel_init();check(command_channel==storage && command_queue && storage[6]==1);
}
int main(void){
    legacy_writer_main();ring_reset();u32 length=2*1024*1024+123;
    u8* input=malloc(COMMAND_QUEUE_CAPACITY);check(input!=0);
    for(u32 i=0;i<length;++i)input[i]=(u8)(i*13+31);
    check(command_channel_append(input,17,input+17,length-17,0,0));memset(input,0xa5,length);command_channel_end();
    last_error=0x88;command_channel_pump();check(last_error==0x88 && storage[5]==MNM_RENDER_COMMANDS_V2_CAPACITY && storage[6]==1);
    command_channel_pump();check(storage[5]==MNM_RENDER_COMMANDS_V2_CAPACITY && storage[6]==1);
    struct mnm_ring_reader reader;check(mnm_ring_reader_bind(&reader,storage,mapped_size));u8 bytes[65536];u32 count=0;
    while(count<length){int n=mnm_ring_read(&reader,bytes,sizeof(bytes));check(n>=0);for(int i=0;i<n;++i)check(bytes[i]==(u8)((count+(u32)i)*13+31));count+=(u32)n;command_channel_pump();}
    check(mnm_ring_read(&reader,bytes,1)==0 && reader.state==2 && storage[5]==length && storage[9]==length);
    command_channel_close();check(!command_queue && !command_channel && storage[6]==2);
    ring_reset();check(command_channel_append(input,COMMAND_QUEUE_CAPACITY,0,0,0,0));check(!command_channel_append(input,1,0,0,0,0));
    command_channel_fail(MNM_RENDER_COMMANDS_V2_REASON_INVALID);command_channel_pump();check(storage[6]==3 && storage[7]==1 && storage[5]==0);command_channel_close();
    ring_reset();check(command_channel_append(input,length,0,0,0,0));storage[8]=1;command_channel_pump();check(storage[6]==3 && storage[7]==3 && storage[5]==0);command_channel_close();
    ring_reset();check(command_channel_append(input,length,0,0,0,0));command_channel_end();command_channel_close();check(storage[6]==3 && storage[7]==4 && !command_queue);
    ring_reset();command_queue_draining=1;check(command_channel_append(input,123,0,0,0,0));command_channel_pump();check(storage[5]==0);command_queue_draining=0;command_channel_pump();check(storage[5]==123);command_channel_close();
    ring_reset();check(mnm_ring_reader_bind(&reader,storage,mapped_size));count=0;
    for(u32 batch=0;batch<17;++batch){
        u32 target=count+length;for(u32 i=0;i<length;++i)input[i]=(u8)((count+i)*13+31);
        check(command_channel_append(input,length,0,0,0,0));memset(input,0xa5,length);command_channel_pump();
        while(count<target){int n=mnm_ring_read(&reader,bytes,sizeof(bytes));check(n>=0);for(int i=0;i<n;++i)check(bytes[i]==(u8)((count+(u32)i)*13+31));count+=(u32)n;command_channel_pump();}
    }
    command_channel_end();command_channel_pump();check(mnm_ring_read(&reader,bytes,1)==0 && reader.state==2 && count>COMMAND_QUEUE_CAPACITY);command_channel_close();
    reset();command_channel_refused=0;memcpy(storage,MNM_RENDER_COMMANDS_V2_MAGIC,8);storage[2]=2;storage[3]=MNM_RENDER_COMMANDS_V2_SIZE;fail_alloc=1;
    command_channel_init();check(!command_queue && storage[6]==3 && storage[7]==1 && !command_channel_append(input,1,0,0,0,0));command_channel_close();fail_alloc=0;
    /* Filesystem identity is required before a v2 claim; failed/zero IDs
     * cannot create a recoverable transport or mutate the ready ring. */
    for(u32 mode=1;mode<=4;++mode){
        reset();command_channel_refused=0;memcpy(storage,MNM_RENDER_COMMANDS_V2_MAGIC,8);
        storage[2]=2;storage[3]=MNM_RENDER_COMMANDS_V2_SIZE;identity_mode=mode;
        command_channel_init();check(!command_queue && !command_channel && command_channel_refused && storage[6]==0 && !storage[5]);
    }
    identity_mode=0;
    /* Empty private queue still watches outstanding bytes and cancellation. */
    ring_reset();command_queue_timeout=100;check(command_channel_append(input,64,0,0,0,0));command_channel_pump();
    check(command_queue_read==command_queue_written && command_queue_peak==64);clock_ms=99;command_channel_pump();check(storage[6]==1);
    clock_ms=100;last_error=0x77;command_channel_pump();check(last_error==0x77 && storage[6]==3 && storage[7]==4 && command_queue_timed_out);command_channel_close();
    /* Actual ACK progress resets the deadline; more producer traffic does not. */
    ring_reset();command_queue_timeout=100;check(command_channel_append(input,64,0,0,0,0));command_channel_pump();
    clock_ms=90;storage[9]=32;command_channel_pump();clock_ms=180;check(command_channel_append(input,64,0,0,0,0));command_channel_pump();check(storage[6]==1);
    clock_ms=190;command_channel_pump();check(storage[6]==3 && storage[7]==4);command_channel_close();
    ring_reset();command_queue_timeout=100;check(command_channel_append(input,64,0,0,0,0));command_channel_pump();
    storage[9]=64;clock_ms=1000;command_channel_pump();check(storage[6]==1 && !command_queue_armed);clock_ms=4000;command_channel_pump();check(storage[6]==1);command_channel_end();command_channel_pump();command_channel_close();check(storage[6]==2);
    ring_reset();check(command_channel_append(input,64,0,0,0,0));command_channel_pump();storage[8]=1;command_channel_pump();check(storage[6]==3 && storage[7]==3 && command_queue_failure==3);command_channel_close();
    ring_reset();check(command_channel_append(input,64,0,0,0,0));command_channel_pump();storage[9]=65;command_channel_pump();check(storage[6]==3 && storage[7]==5);command_channel_close();
    ring_reset();command_queue_timeout=100;clock_ms=0xffffffd0u;check(command_channel_append(input,64,0,0,0,0));command_channel_pump();clock_ms=0x34;command_channel_pump();check(storage[6]==3 && storage[7]==4);command_channel_close();
    /* Bounded/default v2 does not acquire the continuous stall deadline. */
    ring_reset();check(command_channel_append(input,64,0,0,0,0));command_channel_pump();clock_ms=9000;command_channel_pump();check(storage[6]==1);command_channel_close();
    continuous_env="1";
    const char* settings[]={"10","5000","0","5001","oops","999999999999999999","000150"};
    u32 expected[]={10,5000,5000,5000,5000,5000,150};
    for(u32 i=0;i<7;++i){stall_env=settings[i];ring_reset();check(command_queue_timeout==expected[i]);command_channel_close();}
    continuous_env=stall_env=0;
    free(input);puts("{\"success\":true,\"queue_cases\":25,\"bytes\":2097275,\"queue_wrap_bytes\":35653675}");return 0;
}
