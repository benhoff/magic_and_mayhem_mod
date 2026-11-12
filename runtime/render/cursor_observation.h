/* Optional completed native GetCursorPos observation. Not a new poll, not a
 * Drawing API journal prefix. Each slot is read and written only by its owner. */
static u32 cursor_observation_enabled,cursor_observation_slots[16][16];
static void cursor_observation_invalidate(void){
 if(!cursor_observation_enabled)return;
 u32 owner=GetCurrentThreadId();
 for(u32 i=0;i<16;++i)if(__atomic_load_n(cursor_observation_slots[i]+4,__ATOMIC_ACQUIRE)==owner){cursor_observation_slots[i][3]=0;break;}
}
static void cursor_observation_complete(u32 sequence,const i32* point,u32 caller){
 if(!cursor_observation_enabled)return;
 u32 owner=GetCurrentThreadId(),*r=0;
 for(u32 i=0;i<16;++i)if(__atomic_load_n(cursor_observation_slots[i]+4,__ATOMIC_ACQUIRE)==owner){r=cursor_observation_slots[i];break;}
 if(!r)for(u32 i=0;i<16;++i)if(__sync_bool_compare_and_swap(cursor_observation_slots[i]+4,0,owner)){r=cursor_observation_slots[i];break;}
 if(!r)return;
 r[0]=1;r[1]=64;r[2]=0x40209ca7;r[3]=1;++r[5];r[6]=sequence;
 r[7]=GetTickCount();r[8]=(u32)point[0];r[9]=(u32)point[1];
 r[10]=(u32)input_desktop_coordinates;r[11]=1;r[12]=caller;
}
__declspec(dllexport) u32 WIN RenderCursorObservationV1(u32 owner,u32* out,u32 bytes){
 if(!cursor_observation_enabled||!owner||owner!=GetCurrentThreadId()||!out||bytes!=64)return 0;
 for(u32 i=0;i<16;++i)if(__atomic_load_n(cursor_observation_slots[i]+4,__ATOMIC_ACQUIRE)==owner){
  u32* r=cursor_observation_slots[i];if(!r[5]||r[3]!=1)return 0;
  for(u32 j=0;j<16;++j)out[j]=r[j];
  return 1;
 }
 return 0;
}
