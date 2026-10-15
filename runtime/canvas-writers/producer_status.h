/* One process-local resolved callback per optional producer DLL. */
API void* WIN GetProcAddress(void*,const char*);
typedef void (WIN *ProducerCount)(u32,u32,u32);
static ProducerCount producer_count;
static u32 producer_family;
static int producer_start(u32 family,u32 routes){
 producer_family=family;void* module=GetModuleHandleA("MnmDraw.dll");
 if(!module)return 1; /* Original-reference fixtures have no facade. */
 producer_count=(ProducerCount)GetProcAddress(module,"NativeDrawProducerCountV1");
 if(!producer_count)return 0;
 for(u32 i=0;i<routes;++i)producer_count(family,i,0);return 1;
}
static void producer_notice(unsigned route,int native){if(producer_count)producer_count(producer_family,route,native?1:2);}
