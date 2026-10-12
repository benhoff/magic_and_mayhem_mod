typedef unsigned int u32;
#define WIN __attribute__((stdcall))
#define API __declspec(dllimport)
API void* WIN CensusProxyForTest(u32,void*);
API u32 WIN CensusCountForTest(u32,u32);
API void WIN CensusSeedForTest(u32,u32,u32);
API u32 WIN GetLastError(void);
API void WIN SetLastError(u32);
API void WIN ExitProcess(u32);
static u32 WIN standard(u32 a,u32 b){SetLastError(0x88);return a*3+b;}
static u32 plain(u32 a,u32 b){return a*5+b;}
static u32 __fastcall fast(u32 a,u32 b,u32 c){return a*7+b+c;}
static volatile u32 __attribute__((used)) flags_seen,ecx_seen,edx_seen;
static u32 __attribute__((naked)) inspect(void){
    __asm__ __volatile__("pushfl\n popl _flags_seen\n movl %ecx,_ecx_seen\n movl %edx,_edx_seen\n movl $0x98765432,%eax\n ret\n");
}
static u32 __attribute__((naked)) call_inspect(void* call __attribute__((unused))){
    __asm__ __volatile__("pushfl\n pushal\n movl 40(%esp),%eax\n movl $0x11223344,%ecx\n movl $0x55667788,%edx\n pushl $0x8d7\n popfl\n call *%eax\n movl %eax,28(%esp)\n popal\n popfl\n ret\n");
}
void start(void){
    void* p=CensusProxyForTest(0,(void*)&standard);if(!p||CensusProxyForTest(4,(void*)&standard)||CensusProxyForTest(1,0))ExitProcess(1);
    SetLastError(0x77);if(((u32(WIN*)(u32,u32))p)(11,22)!=55||GetLastError()!=0x88)ExitProcess(2);
    p=CensusProxyForTest(1,(void*)&plain);if(!p||((u32(*)(u32,u32))p)(11,22)!=77)ExitProcess(3);
    p=CensusProxyForTest(2,(void*)&fast);if(!p||((u32(__fastcall*)(u32,u32,u32))p)(11,22,33)!=132)ExitProcess(4);
    p=CensusProxyForTest(3,(void*)&inspect);SetLastError(0x77);
    if(!p||call_inspect(p)!=0x98765432||GetLastError()!=0x77||ecx_seen!=0x11223344||edx_seen!=0x55667788||(flags_seen&0x8d5)!=0x8d5)ExitProcess(5);
    for(u32 i=0;i<4;++i)if(CensusCountForTest(i,0)!=1||CensusCountForTest(i,1))ExitProcess(6);
    CensusSeedForTest(3,0xffffffffu,7);
    if(call_inspect(p)!=0x98765432||CensusCountForTest(3,0)||CensusCountForTest(3,1)!=8)ExitProcess(7);
    for(u32 i=0;i<10000;++i)if(call_inspect(p)!=0x98765432)ExitProcess(8);
    if(CensusCountForTest(3,0)!=10000||CensusCountForTest(3,1)!=8)ExitProcess(9);
    ExitProcess(0);
}
