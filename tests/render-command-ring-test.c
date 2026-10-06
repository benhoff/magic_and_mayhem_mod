#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include "../protocols/include/mnm/render_command_ring.h"
static unsigned cases;
static void check(int value){if(!value){fprintf(stderr,"ring check failed case%u\n",cases);exit(1);}}
static uint32_t* memory;
static unsigned char input[MNM_RENDER_COMMANDS_V2_CAPACITY],output[MNM_RENDER_COMMANDS_V2_POLL_BYTES];
static struct mnm_ring_writer writer;static struct mnm_ring_reader reader;
static void reset(void){++cases;memset(memory,0,MNM_RENDER_COMMANDS_V2_SIZE);memcpy(memory,MNM_RENDER_COMMANDS_V2_MAGIC,8);memory[2]=2;memory[3]=MNM_RENDER_COMMANDS_V2_SIZE;memory[4]=123;check(mnm_ring_reader_bind(&reader,memory,MNM_RENDER_COMMANDS_V2_SIZE));check(mnm_ring_writer_bind(&writer,memory,MNM_RENDER_COMMANDS_V2_SIZE));}
static void drain(uint32_t size,uint32_t base){uint32_t count=0;while(count<size){uint32_t budget=size-count;if(budget>37)budget=37;int n=mnm_ring_read(&reader,output,budget);check(n>0);for(int i=0;i<n;++i)check(output[i]==(unsigned char)(base+count+(uint32_t)i));count+=(uint32_t)n;}check(count==size);}
int main(void){
    memory=mmap(0,MNM_RENDER_COMMANDS_V2_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);check(memory!=MAP_FAILED);
    for(uint32_t i=0;i<sizeof(input);++i)input[i]=(unsigned char)i;
    reset();check(mnm_ring_write(&writer,input,sizeof(input))==1);uint32_t published=memory[5];check(mnm_ring_write(&writer,input,1)==0 && memory[5]==published && memory[6]==1);check(!memcmp((unsigned char*)memory+64,input,sizeof(input)));
    check(mnm_ring_read(&reader,output,sizeof(output))==(int)sizeof(output));check(!memcmp(output,input,sizeof(output)));check(memory[9]==sizeof(output));
    memset(input,0xa5,sizeof(output));check(mnm_ring_write(&writer,input,sizeof(output))==1);
    for(uint32_t i=0;i<sizeof(output);++i)check(output[i]==(unsigned char)i); /* Owned output survives recycled slots. */
    drain(sizeof(input)-sizeof(output),sizeof(output));check(mnm_ring_read(&reader,output,sizeof(output))==(int)sizeof(output));
    for(uint32_t i=0;i<sizeof(output);++i)check(output[i]==0xa5);
    for(uint32_t i=0;i<sizeof(input);++i)input[i]=(unsigned char)i;
    check(reader.consumed==writer.published && mnm_ring_end(&writer));check(mnm_ring_read(&reader,output,1)==0 && reader.state==2);
    reset();for(unsigned i=0;i<40;++i){check(mnm_ring_write(&writer,input,65531)==1);drain(65531,0);}check(writer.published>2*MNM_RENDER_COMMANDS_V2_CAPACITY);check(mnm_ring_end(&writer));check(mnm_ring_write(&writer,input,1)==-1 && memory[6]==2);
    reset();check(mnm_ring_write(&writer,input,100)==1);check(mnm_ring_end(&writer));drain(100,0);check(reader.state==2 && reader.consumed==100);
    reset();mnm_ring_cancel(&reader);check(mnm_ring_write(&writer,input,1)==-1 && memory[6]==3 && memory[7]==3 && memory[5]==0);
    reset();memory[9]=1;check(mnm_ring_write(&writer,input,1)==-1 && memory[7]==5);
    reset();check(mnm_ring_write(&writer,input,3)==1);check(mnm_ring_read(&reader,output,1)==1);check(mnm_ring_write(&writer,input,1)==1);memory[9]=0;check(mnm_ring_write(&writer,input,1)==-1 && memory[7]==5);
    reset();writer.published=writer.acknowledged=UINT32_MAX-1;memory[5]=memory[9]=UINT32_MAX-1;check(mnm_ring_write(&writer,input,2)==-1 && memory[7]==1 && memory[5]==UINT32_MAX-1);
    reset();memory[5]=MNM_RENDER_COMMANDS_V2_CAPACITY+1;check(mnm_ring_read(&reader,output,1)==-1 && memory[9]==0 && memory[8]==1);memory[5]=0;check(mnm_ring_read(&reader,output,1)==-1);
    reset();memory[4]=124;check(mnm_ring_write(&writer,input,1)==-1 && memory[5]==0 && memory[6]==1);check(mnm_ring_read(&reader,output,1)==-1 && memory[9]==0 && !memory[8]);
    reset();memory[10]=1;check(mnm_ring_write(&writer,input,1)==-1);check(mnm_ring_read(&reader,output,1)==-1);
    reset();check(mnm_ring_write(&writer,input,2)==1 && mnm_ring_read(&reader,output,1)==1);memory[5]=0;check(mnm_ring_read(&reader,output,1)==-1 && memory[9]==1);
    reset();check(mnm_ring_end(&writer) && mnm_ring_read(&reader,output,1)==0);memory[6]=1;check(mnm_ring_read(&reader,output,1)==-1);
    reset();check(mnm_ring_end(&writer) && mnm_ring_read(&reader,output,1)==0);memory[5]=1;check(mnm_ring_read(&reader,output,1)==-1);
    reset();check(mnm_ring_read(&reader,output,0)==-1 && memory[9]==0);
    reset();struct mnm_ring_writer second;check(!mnm_ring_writer_bind(&second,memory,MNM_RENDER_COMMANDS_V2_SIZE));check(!mnm_ring_writer_bind(&second,(unsigned char*)memory+1,MNM_RENDER_COMMANDS_V2_SIZE));
    /* Actual independent processes share only byte mapping/counters. Deterministic
     * expected stream crosses64MiB without overwriting any unacknowledged byte. */
    reset();pid_t child=fork();check(child>=0);const uint32_t total=70*1024*1024;
    if(child==0){uint32_t count=0;while(count<total){int n=mnm_ring_read(&reader,output,sizeof(output));if(n<0)_exit(2);if(!n){sched_yield();continue;}for(int i=0;i<n;++i)if(output[i]!=(unsigned char)((count+(uint32_t)i)*13u+7u))_exit(3);count+=(uint32_t)n;}while(reader.state!=2){if(mnm_ring_read(&reader,output,1)<0)_exit(4);sched_yield();}_exit(reader.consumed==total?0:5);}
    uint32_t count=0;while(count<total){uint32_t length=65531;if(length>total-count)length=total-count;for(uint32_t i=0;i<length;++i)input[i]=(unsigned char)((count+i)*13u+7u);int result=mnm_ring_write(&writer,input,length);check(result>=0);if(!result){sched_yield();continue;}count+=length;}check(mnm_ring_end(&writer));int status;check(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);check(memory[9]==total);
    check(munmap(memory,MNM_RENDER_COMMANDS_V2_SIZE)==0);printf("{\"success\":true,\"cases\":%u,\"cross_process_bytes\":%u}\n",cases,total);return 0;
}
