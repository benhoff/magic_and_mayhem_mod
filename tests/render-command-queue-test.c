#define main legacy_writer_main
#include "render-command-writer-test.c"
#undef main
static void ring_reset(void){
    check(!command_queue);reset();command_channel_refused=0;env=1;mapped_size=MNM_RENDER_COMMANDS_V2_SIZE;
    memcpy(storage,MNM_RENDER_COMMANDS_V2_MAGIC,8);storage[2]=2;storage[3]=mapped_size;
    command_queue_written=command_queue_read=command_queue_end=command_queue_failure=0;command_queue_draining=0;
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
    free(input);puts("{\"success\":true,\"queue_cases\":7,\"bytes\":2097275,\"queue_wrap_bytes\":35653675}");return 0;
}
