/* Private bridge storage only. Checked-out buffers retain the existing 64 MiB
 * accounting; at most four 64 KiB..1 MiB blocks remain idle. Every budgeted
 * allocation trims idle storage against the same remaining budget before reuse
 * or allocation. No snapshot contents, authority or object identities are cached.
 * Allocations are tracker-serialized; release may follow a contended callback.
 * This small independent lock never acquires the tracker or calls game code. */
struct CaptureStorage {u8* data;u32 bytes;};
static struct CaptureStorage capture_storage[4];
static u32 capture_storage_enabled,capture_storage_bytes;
static volatile i32 capture_storage_busy;
static void capture_storage_enter(void){
 while(!__sync_bool_compare_and_swap(&capture_storage_busy,0,1))__asm__ volatile("pause");
}
static void capture_storage_leave(void){__sync_lock_release(&capture_storage_busy);}
static void capture_storage_clear(u32 stop){
 capture_storage_enter();if(stop)capture_storage_enabled=0;
 for(u32 i=0;i<4;++i)if(capture_storage[i].data){HeapFree(GetProcessHeap(),0,capture_storage[i].data);capture_storage[i].data=0;capture_storage[i].bytes=0;}
 capture_storage_bytes=0;capture_storage_leave();
}
static u8* capture_storage_allocate(u32 flags,u32 bytes,u32 available){
 if(bytes>available)return 0;
 u8* out=0;capture_storage_enter();
 if(capture_storage_enabled){
  for(u32 i=0;i<4;++i)if(capture_storage[i].data && capture_storage[i].bytes==bytes){
   out=capture_storage[i].data;capture_storage[i].data=0;capture_storage[i].bytes=0;capture_storage_bytes-=bytes;break;
  }
 }
 /* Idle buffers never displace an otherwise admissible active allocation. */
 for(u32 i=0;i<4 && capture_storage_bytes>available-bytes;++i)if(capture_storage[i].data){
  capture_storage_bytes-=capture_storage[i].bytes;HeapFree(GetProcessHeap(),0,capture_storage[i].data);capture_storage[i].data=0;capture_storage[i].bytes=0;
 }
 capture_storage_leave();
 if(!out)return HeapAlloc(GetProcessHeap(),flags,bytes);
 if(flags&8)zero(out,bytes);
 return out;
}
static void capture_storage_release(u8* data,u32 bytes){
 if(!data)return;
 capture_storage_enter();
 if(capture_storage_enabled && bytes>=65536 && bytes<=1048576){
  for(u32 i=0;i<4;++i)if(!capture_storage[i].data){
   capture_storage[i].data=data;capture_storage[i].bytes=bytes;capture_storage_bytes+=bytes;data=0;break;
  }
 }
 capture_storage_leave();if(data)HeapFree(GetProcessHeap(),0,data);
}
static void game_snapshot_free(struct Snapshot* s){capture_storage_release(s->data,s->length);s->data=0;}
