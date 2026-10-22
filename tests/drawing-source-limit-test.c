#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
API u32 WIN SetEnvironmentVariableA(const char*,const char*);
static void check(int okay,u32 code){if(!okay)ExitProcess(code);}
void start(void){
 static const struct{const char* text;u32 maximum,okay,value;} cases[]={
  {0,8192,1,42},{"",8192,0,42},{"0",8192,0,42},{"01",8192,0,42},
  {"+1",8192,0,42},{"-1",8192,0,42},{"1x",8192,0,42},{" 1",8192,0,42},
  {"1",8192,1,1},{"8192",8192,1,8192},{"8193",8192,0,42},
  {"2147483648",0x80000000u,1,0x80000000u},{"2147483649",0x80000000u,0,42},
  {"4294967295",0x80000000u,0,42},{"999999999999999",0x80000000u,0,42}
 };
 for(u32 i=0;i<sizeof(cases)/sizeof(cases[0]);++i){
  u32 value=42;check(SetEnvironmentVariableA("MNM_FIXTURE_MAP_LIMIT",cases[i].text),171);
  check(wine_source_map_limit("MNM_FIXTURE_MAP_LIMIT",cases[i].maximum,&value)==(int)cases[i].okay&&value==cases[i].value,172+i);
 }
 check(SetEnvironmentVariableA("MNM_FIXTURE_MAP_LIMIT",0),190);ExitProcess(0);
}
