/* Actual PE32 observation thunks versus independent fixed original stand-ins.
 * Tests machine-state/one-forward preservation, not original game equivalence. */
#include "../runtime/engine-cadence/observer.c"
extern void fixture_invoke(void*,void*),fixture_original(void);
extern void fixture_sequence(void),fixture_tick(void);
extern void fixture_suppression(void),fixture_transition(void),fixture_target(void),fixture_target_zero(void),fixture_observed_target_zero(void);
static u8 baseline[576] __attribute__((aligned(16)));
static u8 observed[576] __attribute__((aligned(16)));
static u32 original_calls;
void fixture_error(void){++original_calls;SetLastError(0xabc123);}
static void require(int condition,u32 error){if(!condition)ExitProcess(error);}
static void branch(u32 at,u32 target){*(u8*)at=0xe9;put((void*)(at+1),target-at-5);}
void start(void){
 const u32 pages[]={0x460000,0x4e0000,0x4f0000,0x500000,0x520000,0x550000,0x6c0000,0x6d0000,0x6e0000,0x6f0000};
 /* This fixture's large owned .bss covers these authored addresses. Keeping the
  * executable at400000 avoids reserving the game's range over Wine's heap. */
 for(u32 i=0;i<10;++i){u32 old;require(VirtualProtect((void*)pages[i],0x10000,0x40,&old),30+i);}
 const u32 targets[]={0x4e3e40,0x464ec0,0x52a780,0x509e50,0x5002a0};
 void (*entries[])(void)={cadence_pacer,cadence_animation,cadence_scheduler,cadence_maintenance,cadence_queue};
 for(u32 i=0;i<5;++i)branch(targets[i],(u32)&fixture_original);
 enabled=1;
 for(u32 i=0;i<5;++i){
  SetLastError(0x77);fixture_invoke((void*)targets[i],baseline);
  require(GetLastError()==0xabc123,2);
  SetLastError(0x77);fixture_invoke(entries[i],observed);
  require(GetLastError()==0xabc123&&same((u32)baseline,observed,36)&&same((u32)baseline+48,observed+48,512),3+i);
 }
 require(original_calls==10&&count==9&&!foreign&&!dropped,9);
 *(u8*)0x46b465=0xc3;*(u8*)0x46b49b=0xc3;
 void (*markers[])(void)={cadence_sequence,cadence_tick};
 void (*references[])(void)={fixture_sequence,fixture_tick};
 for(u32 i=0;i<2;++i){
  SetLastError(0x77);fixture_invoke(references[i],baseline);
  SetLastError(0x77);fixture_invoke(markers[i],observed);
  require(GetLastError()==0x77&&same((u32)baseline,observed,36)&&same((u32)baseline+48,observed+48,512),10+i);
 }
 require(get(0x6cbb78+0xfd62)==2&&get(0x6c4830)==5&&count==11,12);
 /* Exact static ANI pointer derivation survives volatile ECX at return. */
 u32 registers[10]={0};registers[6]=0x6deffc;registers[9]=0x46b754;
 put((void*)0x6deffc,0x1234);cadence_record(registers,5);
 registers[6]=0xdeadbeef;put((void*)0x6deffc,0x5678);cadence_record(registers,15);
 require(rows[count-2][15]==0x1234&&rows[count-1][15]==0x5678,13);
 /* Every pinned byte must match before admission: all single-byte mutations. */
 for(u32 i=0;i<sizeof(patches)/sizeof(*patches);++i){
  struct Patch* p=patches+i;for(u32 k=0;k<p->length;++k)*(u8*)(p->site+k)=p->bytes[k];
  require(same(p->site,p->bytes,p->length),14);
  for(u32 k=0;k<p->length;++k){*(u8*)(p->site+k)^=1;require(!same(p->site,p->bytes,p->length),15);*(u8*)(p->site+k)^=1;}
 }
 /* Optional route thunks replay the exact comparisons/branches and preserve
  * the original call-site return PC on both tail-forwards. */
 *(u8*)0x46bba3=0xc3;*(u8*)0x46bbab=0xc3;
 const u8 available[]={0xbf,1,0,0,0,0xc3},missing[]={0xbf,2,0,0,0,0xc3};
 for(u32 i=0;i<6;++i){*(u8*)(0x4fd53c+i)=available[i];*(u8*)(0x4fd5ad+i)=missing[i];}
 branch(0x4fd280,(u32)&fixture_original);branch(0x557040,(u32)&fixture_original);
 void (*route_originals[])(void)={fixture_suppression,fixture_transition,(void*)0x4fd280,(void*)0x557040,fixture_target,fixture_target_zero};
 void (*route_entries[])(void)={cadence_draw_suppression,cadence_draw_transition,cadence_scene,cadence_results,cadence_target,fixture_observed_target_zero};
 u32 prior=count;
 for(u32 i=0;i<6;++i){
  SetLastError(0x77);fixture_invoke(route_originals[i],baseline);u32 expected_error=GetLastError();
  SetLastError(0x77);fixture_invoke(route_entries[i],observed);
  require(GetLastError()==expected_error&&same((u32)baseline,observed,36)&&same((u32)baseline+48,observed+48,512),50+i);
 }
 require(original_calls==14&&count==prior+6&&rows[count-2][19]==1&&rows[count-1][19]==0,56);
 for(u32 i=0;i<sizeof(route_patches)/sizeof(*route_patches);++i){
  struct Patch* p=route_patches+i;for(u32 k=0;k<p->length;++k)*(u8*)(p->site+k)=p->bytes[k];
  require(same(p->site,p->bytes,p->length),57);
  for(u32 k=0;k<p->length;++k){*(u8*)(p->site+k)^=1;require(!same(p->site,p->bytes,p->length),58);*(u8*)(p->site+k)^=1;}
 }
 count=LIMIT;cadence_record(registers,5);require(count==LIMIT&&dropped==1,16);
 ExitProcess(0);
}
