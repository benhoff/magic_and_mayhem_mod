#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 mode;
static void check(int okay,u32 code){if(!okay)ExitProcess(code);}
static u32 WIN point(void* dst,void* src,u32* out,u32 size){
 check(dst==(void*)2&&src==(void*)3&&size==128,201);
 for(u32 at=0;at<32;at+=16){for(u32 i=0;i<16;++i)out[at+i]=0;out[at]=64;out[at+1]=1;out[at+2]=19;out[at+3]=4;out[at+6]=77;out[at+7]=4;out[at+8]=3;out[at+9]=4;out[at+14]=GetCurrentThreadId();}
 if(mode==1)out[16+3]=5;if(mode==2)out[16+14]^=1;if(mode==3)out[16+15]=1;
 if(mode==4)out[16+2]=27;if(mode==5)out[16+6]=0;if(mode==6){out[16+2]=3;out[16+6]=1;}if(mode==7)out[16+12]=129;
 return mode!=8;
}
void start(void){
 check(initialize(),200);producer_point=&point;char value[2]={0};GetEnvironmentVariableA("MNM_PAIR_FIXTURE_CASE",value,2);mode=value[0]>='1'&&value[0]<='9'?value[0]-'0':0;
 struct Context c={0};c.observed=1;c.r[1]=1;c.r[2]=5;c.r[6]=GetCurrentThreadId();c.r[29]=1;c.args[1]=3;
 if(mode==9){producer_points_capture(&c,(void*)2,0);ExitProcess(202);}
 for(u32 phase=1;phase<=3;++phase)producer_points_capture(&c,(void*)2,phase);
 check(producer_point_records==3,203);producer_points_close(1);ExitProcess(0);
}
