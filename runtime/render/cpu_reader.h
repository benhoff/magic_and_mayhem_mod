/* Explicit reader boundary, never entered from an exception handler. Existing
 * FIFO/copy admission stays held while the publication worker services CPU_READ.
 * The tracker is released only for the bounded reply wait. */
static u32 game_cpu_reader_thread;
__declspec(dllexport) i32 WIN RenderRequireCpuHistoryV1(void* object,const void* pixels,u32 family,u32 route,u32 caller){
 u32 error=GetLastError();i32 ok=0;
 if(!game_resident_world || !game_copy_order_enabled || !game_native_cpu_lease_transfer || !game_native_cpu_lease_snapshot || !game_native_cpu_lease_mask)return 0;
 struct CommandLease callback __attribute__((cleanup(command_gate_leave)))=command_gate_enter();
 if(!callback.observe)return 0;
 struct GameCopyLease admission __attribute__((cleanup(game_copy_leave)))={0,1,0};
 if(__atomic_load_n(&game_copy_owner,__ATOMIC_ACQUIRE)!=GetCurrentThreadId())admission=game_copy_bind_enter();
 if(!admission.observe || game_cpu_reader_thread || !game_tracker_acquire()){SetLastError(error);return 0;}
 game_cpu_reader_thread=GetCurrentThreadId();game_surface_sync();
 struct GameLock* lock=0;
 for(u32 i=0;i<32;++i)if(game_locks[i].active && game_alias_same(game_locks[i].object,object)){
  if(lock)goto reject;lock=game_locks+i;
 }
 if(!lock){
  game_tracker_release();game_materialize_reader(object,3);game_cpu_reader_thread=0;SetLastError(error);return 1;
 }
 if(lock->owner!=GetCurrentThreadId() || lock->epoch!=game_lock_epoch ||
    !(lock->resident || lock->gpu_history) || !lock->base.data)goto reject;
 struct SessionSurface* surface=session_find(object);
 u32 identity[12];i32 query=game_native_cpu_lease_transfer(object,identity,0,0);
 if(query==1){ok=1;goto reject;}
 if(query || !surface || !session_active || game_cpu_read_ticket==0xffffffffu ||
    identity[0]!=(u32)pixels || identity[1]!=lock->base.width || identity[2]!=lock->base.height ||
    identity[3]!=lock->desc[4] || lock->base.bits!=16 || lock->base.length!=identity[1]*identity[2]*2)goto reject;
 u32 offset=lock->rectangle?(u32)lock->region.top*identity[3]+(u32)lock->region.left*2:0;
 if(lock->desc[9]!=(u32)pixels+offset)goto reject;
 struct Snapshot result,pending;
 if(!game_surface_blank(&pending,&lock->base))goto reject;
 if(!game_surface_blank(&result,&lock->base)){
  __atomic_sub_fetch(&lock_capture_reserved,pending.length,__ATOMIC_RELAXED);free_snapshot(&pending);goto reject;
 }
 u32 layout[4]={identity[0],identity[1],identity[2],identity[3]};
 if(game_native_cpu_lease_snapshot(object,layout,(u16*)pending.data,pending.length))game_cpu_read_refuse(10,object,0);
 u8* mask=game_cpu_lease_mask(lock,result.length);
 if(!mask){
  u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
  if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes || result.length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved)game_cpu_read_refuse(17,object,0);
  mask=HeapAlloc(GetProcessHeap(),8,result.length);if(!mask)game_cpu_read_refuse(18,object,0);
  __atomic_add_fetch(&lock_capture_reserved,result.length,__ATOMIC_RELAXED);
 }
 if(lock->rectangle)for(u32 y=0;y<identity[2];++y)for(u32 x=0;x<identity[1];++x){
  if(x>=(u32)lock->region.left && x<(u32)lock->region.right && y>=(u32)lock->region.top && y<(u32)lock->region.bottom)continue;
  u32 at=(y*identity[1]+x)*2;if(mask[at] || mask[at+1])game_cpu_read_refuse(15,object,at);
 }
 const u32 generation=lock->generation,epoch=lock->epoch,ticket=++game_cpu_read_ticket,surface_id=surface->id;
 u32 fields[10]={surface_id,ticket,0,command_channel_session,identity[1],identity[2],identity[4],identity[5],identity[6],identity[7]};
 __atomic_store_n(&game_cpu_transfer_wait,1,__ATOMIC_RELEASE);
 if(!session_record(MNM_RENDER_STREAM_V5_OPERATION_CPU_READ,fields,40,0,0))game_cpu_read_refuse(11,object,0);
 game_tracker_release();
 char path[544];u32 n=GetEnvironmentVariableA("MNM_RENDER_COMMAND_CHANNEL",path,512);
 if(!n || n>=512)game_cpu_read_refuse(12,object,n);copy(path+n,".cpu-reply",11);
 u32 start=GetTickCount(),ready=0;
 while(GetTickCount()-start<MNM_NATIVE_CPU_READ_V1_TIMEOUT_MS){
  if(!command_channel_identity() || command_channel_refused ||
     __atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)>=2 || __atomic_load_n(command_channel+8,__ATOMIC_ACQUIRE))break;
  HANDLE file=CreateFileA(path,0x80000000,7,0,3,0x80,0);
  if(file!=(HANDLE)-1){u32 h[16],read=0,file_bytes=GetFileSize(file,0);
   if(ReadFile(file,h,64,&read,0) && read==64 && mnm_native_cpu_read_reply(h,file_bytes,fields) &&
      ReadFile(file,result.data,result.length,&read,0) && read==result.length)ready=1;
   CloseHandle(file);if(ready)break;
  }
  Sleep(1);
 }
 if(!ready || !game_tracker_acquire())game_cpu_read_refuse(13,object,GetTickCount()-start);
 if(!lock->active || lock->generation!=generation || lock->epoch!=epoch || epoch!=game_lock_epoch ||
    lock->owner!=GetCurrentThreadId() || !game_alias_same(lock->object,object) ||
    !session_find(object) || session_find(object)->id!=surface_id)game_cpu_read_refuse(14,object,0);
 /* A partial-byte store cannot upload an entire stale WORD. Merge exactly the
  * bytes written by this lease into the native snapshot, then publish those
  * complete WORDs before exposing current pixels to the CPU operation. */
 for(u32 i=0;i<result.length;++i)if(mask[i])result.data[i]=pending.data[i];
 if(!session_resident_unlock(surface,&result,&result,mask) ||
    game_native_cpu_lease_transfer(object,identity,(const u16*)result.data,result.length))game_cpu_read_refuse(14,object,0);
 copy(lock->base.data,result.data,result.length);
 __atomic_sub_fetch(&lock_capture_reserved,pending.length+result.length,__ATOMIC_RELAXED);free_snapshot(&pending);HeapFree(GetProcessHeap(),0,mask);
 __atomic_sub_fetch(&lock_capture_reserved,result.length,__ATOMIC_RELAXED);free_snapshot(&result);
 /* Keep reader admission through any enclosing copy; its final leave clears
  * the marker after publication/current-byte work, before releasing ownership. */
 {u32 values[19]={ticket,surface_id,family,route,caller,identity[9]};lock_diagnostic_values("resident_lock_cpu_materialized",values);}
 game_cpu_read_completed(!family && !route && !caller?2:1);
 ok=1;
 reject:
 game_cpu_reader_thread=0;game_tracker_release();SetLastError(error);return ok;
}
