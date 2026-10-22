#define MNM_SCENE_SELFTEST 1
#include "../runtime/scene/canvas_producers.c"
static void check(int okay,u32 code){if(!okay)ExitProcess(code);}
void start(void){
 check(producer_trace_configure(),201);enabled=1;stopped=failures=sequence=queue=0;
 u32 wrapper[3]={0,0x1111,0x2222},out[32];
 surfaces[0].object=(u32)wrapper;surfaces[0].id=77;surfaces[0].width=4;surfaces[0].height=3;surfaces[0].stride=6;
 SetLastError(0xbeef);check(SceneDrawingSourcePairV1((void*)0x1111,(void*)0x2222,out,128)&&GetLastError()==0xbeef&&out[2]==19&&out[6]==77&&bypass_equal(out,out+16,64),202);
 check(SceneDrawingSourcePointV1((void*)0x3333,out,64)&&out[2]==3&&!out[6],203);
 surfaces[1]=surfaces[0];surfaces[1].id=78;check(SceneDrawingSourcePointV1((void*)0x1111,out,64)&&out[2]==11&&!out[6],204);surfaces[1].object=0;
 stopped=1;check(SceneDrawingSourcePointV1((void*)0x1111,out,64)&&out[2]==23,205);
 u8* split=VirtualAlloc(0,8192,0x3000,4);check(split!=0,206);for(u32 i=0;i<8192;++i)split[i]=0x6b;u32 old,unused;
 check(VirtualProtect(split+4096,4096,2,&old)&&!SceneDrawingSourcePairV1(0,0,(u32*)(split+4096-64),128),207);
 for(u32 i=0;i<8192;++i)check(split[i]==0x6b,208);
 check(VirtualProtect(split+4096,4096,old,&unused),209);
 check(!SceneDrawingSourcePointV1(0,0,64)&&!SceneDrawingSourcePairV1(0,0,0,128)&&!SceneDrawingSourcePairV1(0,0,out,127),210);ExitProcess(0);
}
