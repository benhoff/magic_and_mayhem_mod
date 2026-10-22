#include "../runtime/engine-cadence/observer.c"
static void require(int okay,u32 code){if(!okay)ExitProcess(code);}
static void refused(void* q,u32 pc,u32* out,u32 size){SetLastError(0xbeef);require(!EngineCadenceQueuePointV1(q,pc,out,size)&&GetLastError()==0xbeef,171);}
void start(void){
 u32 old;require(VirtualProtect((void*)0x6c0000,0x40000,4,&old),170);put((void*)0x6c4830,10);put((void*)(0x6cbb78+0xfd62),20);*(u8*)(0x6cbb78+0xfd5a)=0;put((void*)0x6f34e0,0x6cbb78);
 enabled=1;owner=GetCurrentThreadId();frequency[0]=1000;count=1;u32* r=rows[0];r[0]=1;r[1]=8;r[2]=owner;r[3]=1234;r[5]=10;r[6]=20;r[12]=0x12345678;r[13]=0x4fd4b0;
 u32 out[16]={0};SetLastError(0xbeef);require(EngineCadenceQueuePointV1((void*)r[12],r[13],out,64)&&GetLastError()==0xbeef,172);require(out[0]==64&&out[1]==1&&out[2]==1&&out[3]==10&&out[4]==20&&out[6]==owner&&out[7]==1&&out[8]==1234&&out[10]==1000&&out[12]==8&&out[13]==1&&!out[14]&&!out[15],173);
 refused((void*)r[12],r[13],0,64);refused((void*)r[12],r[13],out,63);refused((void*)1,r[13],out,64);refused((void*)r[12],1,out,64);
 r[1]=7;refused((void*)r[12],r[13],out,64);r[1]=8;put((void*)0x6c4830,11);refused((void*)r[12],r[13],out,64);put((void*)0x6c4830,10);put((void*)(0x6cbb78+0xfd62),21);refused((void*)r[12],r[13],out,64);put((void*)(0x6cbb78+0xfd62),20);*(u8*)(0x6cbb78+0xfd5a)=1;refused((void*)r[12],r[13],out,64);*(u8*)(0x6cbb78+0xfd5a)=0;
 ++owner;refused((void*)r[12],r[13],out,64);--owner;frequency[0]=0;refused((void*)r[12],r[13],out,64);frequency[0]=1000;enabled=0;refused((void*)r[12],r[13],out,64);enabled=1;
 void* read_only=VirtualAlloc(0,4096,0x3000,2);require(read_only!=0,174);refused((void*)r[12],r[13],read_only,64);
 u32 saved_queue=r[12];r[12]=0;refused(0,r[13],out,64);r[12]=saved_queue;
 r[13]=0x4fd559;require(EngineCadenceQueuePointV1((void*)r[12],r[13],out,64)&&out[7]==2,175);ExitProcess(0);
}
