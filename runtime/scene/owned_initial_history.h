/* Native-owned allocation policy only; never samples original-driver planes. */
static u32 owned_initial_rows[512][4],owned_initial_count,owned_initial_closed;
static volatile u32 owned_initial_busy;
static void producer_initial_history(struct Surface* s){
 if(!continuous_public_bind||!continuous_resident||__atomic_load_n(&owned_initial_count,__ATOMIC_ACQUIRE)>=512||
    !s->object||!readable(s->object,12))return;
 HANDLE module=GetModuleHandleA("MnmRender.dll");
 u32 (WIN *query)(void*,u32,u32)=module?(u32 (WIN *)(void*,u32,u32))GetProcAddress(module,"RenderOwnedInitialZeroV1"):0;
 if(!query)return;
 u32 known=0;
 for(u32 offset=8;offset>=4;offset-=4){
  u32 object=get((void*)(s->object+offset));
  if(object&&query((void*)object,s->width,s->height)==1){known=1;break;}
 }
 if(!known)return;
 u32 r[24]={0};r[2]=MNM_PRODUCER_FILL;extent(r,s);
 r[12]=s->width;r[13]=s->height;r[22]=0x4e495a31; /* NIZ1 policy marker. */
 while(__sync_lock_test_and_set(&owned_initial_busy,1))Sleep(0);
 u32 index=__atomic_load_n(&owned_initial_count,__ATOMIC_RELAXED);
 if(index>=512){__sync_lock_release(&owned_initial_busy);return;}
 u32* row=owned_initial_rows[index];
 row[0]=s->id;row[1]=s->width;row[2]=s->height;row[3]=r[22];
 __atomic_store_n(&owned_initial_count,index+1,__ATOMIC_RELEASE);
 __sync_lock_release(&owned_initial_busy);emit(r,0,0);
}
static void producer_initial_history_close(void){
 u32 count=__atomic_load_n(&owned_initial_count,__ATOMIC_ACQUIRE);
 if(!count||!__sync_bool_compare_and_swap(&owned_initial_closed,0,1))return;
 char name[260];path(name,"owned-initial-history-",0,".bin");
 HANDLE out=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(out==(HANDLE)-1)return;
 u32 header[4]={0x494d4e4d,0x3154494e,1,count};
 write(out,header,sizeof(header));write(out,owned_initial_rows,count*16);CloseHandle(out);
}
