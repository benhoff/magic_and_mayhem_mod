/* Included after world_raster_batch.c. Pointer-free fixed storage shared with
 * the native owner. The original destination stays NOACCESS while waiting. */
#include "../../protocols/include/mnm/world_continuous_v6.h"
API HANDLE WIN CreateFileMappingA(HANDLE, void *, u32, u32, u32, const char *);
API void *WIN MapViewOfFile(HANDLE, u32, u32, u32, u32);
API i32 WIN UnmapViewOfFile(void *);
static HANDLE continuous_file, continuous_mapping;
static u32 continuous_prefixes, continuous_source_low=1, continuous_source_high;
static u32 continuous_snapshot, continuous_nonce_low, continuous_nonce_high;
API void* WIN GetProcAddress(HANDLE,const char*);
API u32 WIN GetCurrentThreadId(void);
typedef int (WIN *ResidentWorldBind)(u32,u32,u32,u32,u32,u32,u32,u32);
static ResidentWorldBind continuous_public_bind;
static u32 continuous_written;
static u32 continuous_generation_low = 1, continuous_generation_high,
           continuous_slot_index;
static u32 continuous_load(u8 *p, u32 n) {
  return __atomic_load_n((u32 *)(p + n * 4), __ATOMIC_ACQUIRE);
}
static void continuous_store(u8 *p, u32 n, u32 v) {
  __atomic_store_n((u32 *)(p + n * 4), v, __ATOMIC_RELEASE);
}
static u8 *continuous_slot(void) {
  return continuous_map + MNM_CONT_HEADER +
         continuous_slot_index * MNM_CONT_SLOT_BYTES;
}
/* Preserve a bounded terminal diagnostic even when the slot is no longer
 * WRITING and therefore cannot accept a producer failure record. */
static void continuous_failure(u32 reason,u32 detail) {
  if(!continuous_map)return;
  u32 diagnostic[12]={0};copy(diagnostic,"MNMCTERM",8);
  diagnostic[2]=reason;diagnostic[3]=detail;
  diagnostic[4]=continuous_generation_low;diagnostic[5]=continuous_generation_high;
  diagnostic[6]=continuous_load(continuous_slot(),0);diagnostic[7]=sequence;
  diagnostic[8]=bytes;diagnostic[9]=GetTickCount();diagnostic[10]=GetCurrentThreadId();
  char name[260];path(name,"world-terminal-",reason,".bin");
  HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
  if(f!=(HANDLE)-1){write(f,diagnostic,sizeof(diagnostic));CloseHandle(f);}
}
static int continuous_wait(u8 *p, u32 n, u32 value) {
  u32 start = GetTickCount();
  while (continuous_load(p, n) != value) {
    if (continuous_load(continuous_map, 10) ||
        (u32)(GetTickCount() - start) >= MNM_WORLD_BYPASS_TIMEOUT){
      continuous_failure(3,start);return 0;
    }
    Sleep(0);
  }
  return !continuous_load(continuous_map, 10);
}
static void continuous_cancel(void) {
  if (continuous_map)
    continuous_store(continuous_map, 10, 1);
}
/* Background offscreen bitmap draws may overlap the immutable World lease.
 * Capture only independently addressed admitted secondary-surface records;
 * preserve their owned bytes as the next slot's prefix. World access still
 * faults under the mandatory page guard. Storage is bounded independently. */
#include "world_deferred.h"
#define CONTINUOUS_DEFERRED_BYTES MNM_WORLD_DEFERRED_BYTES
static u8* continuous_deferred;
static u32 continuous_deferred_bytes,continuous_deferred_count;
static int continuous_defer(u32* r,const void* payload,u32 n) {
  if(continuous_load(continuous_slot(),0)==MNM_CONT_WRITING &&
     (!continuous_public_bind || !raster_world_open || GetCurrentThreadId()==batch_owner))return 0;
  if(!mnm_world_defer_admitted(r,n,batch_surface?batch_surface->id:0,continuous_deferred_count,continuous_deferred_bytes)){continuous_failure(4,r[2]);
    char name[260];path(name,"world-refused-record-",r[2],".bin");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
    if(f!=(HANDLE)-1){write(f,r,96);CloseHandle(f);}
    u32 bounds[8]={0x444d4e4d,0x31464552,1,batch_surface?batch_surface->id:0,continuous_deferred_count,continuous_deferred_bytes,n,r[2]};
    path(name,"world-deferred-bounds-",r[2],".bin");f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
    if(f!=(HANDLE)-1){write(f,bounds,sizeof(bounds));CloseHandle(f);}return -1;}
  if(!continuous_deferred)continuous_deferred=HeapAlloc(producer_heap(),0,CONTINUOUS_DEFERRED_BYTES);
  if(!continuous_deferred){continuous_failure(5,r[2]);return -1;}
  return mnm_world_defer_append(continuous_deferred,&continuous_deferred_bytes,&continuous_deferred_count,r,payload,n,batch_surface?batch_surface->id:0)?1:-1;
}
static int continuous_next(void) {
  u8 *slot = continuous_slot();
  if (!continuous_wait(slot, 0, MNM_CONT_FREE))
    return 0;
  while(__sync_lock_test_and_set(&continuous_emit_lock,1))Sleep(0);
  if (!__sync_bool_compare_and_swap((u32 *)slot, MNM_CONT_FREE,
                                    MNM_CONT_WRITING)){
    __sync_lock_release(&continuous_emit_lock);return 0;
  }
  raster_world_open=0;
  zero(slot + 4, 124);
  put(slot + 8, continuous_generation_low);
  put(slot + 12, continuous_generation_high);
  zero(source_packets, sizeof(source_packets));
  zero(continuous_source_heads, sizeof(continuous_source_heads));
  source_packet_count = source_packet_bytes = 0;
  sequence = oracle = oracle_bytes = queue = bypass_completed =
      batch_transfers = raster_preparations = 0;
  bytes = 64;
  continuous_written = 64;
  journal_used = 0;
  u32 h[16] = {0};
  copy(h, MNM_PRODUCER_V3_MAGIC, 8);
  h[2] = 3;
  h[3] = 64;
  h[4] = MNM_PRODUCER_BUILD;
  h[5] = MNM_PRODUCER_V3_MAX_RECORDS;
  h[6] = 32;
  copy(slot + MNM_CONT_SLOT_HEADER, h, 64);
  for(u32 at=0;at<continuous_deferred_bytes;){u32* r=(u32*)(continuous_deferred+at);u32 n=r[18];emit_raw(r,(u8*)r+96,n);at+=96+n;}
  continuous_deferred_bytes=continuous_deferred_count=0;
  __sync_lock_release(&continuous_emit_lock);
  return !stopped;
}
#include "world_continuous_clock.c"
static u32 continuous_time(void) { return pacer_precise_tick ? continuous_clock_read() : GetTickCount(); }
static int continuous_init(void) {
  char name[260];
  u32 n = GetEnvironmentVariableA("MNM_WORLD_CONTINUOUS", name, sizeof(name));
  if (!n)
    return 1;
  if (n >= sizeof(name) || !batch_enabled || limit != 32 || raster_first != 1 ||
      raster_last != 32)
    return 0;
  continuous_file = CreateFileA(name, 0xc0000000, 3, 0, 3, 0x80, 0);
  if (continuous_file == (HANDLE)-1) {
    continuous_file = 0;
    return 0;
  }
  u32 high = 0;
  if (GetFileSize(continuous_file, &high) != MNM_CONT_BYTES || high)
    return 0;
  continuous_mapping = CreateFileMappingA(continuous_file, 0, 4, 0, 0, 0);
  if (!continuous_mapping)
    return 0;
  continuous_map =
      MapViewOfFile(continuous_mapping, 2 | 4, 0, 0, MNM_CONT_BYTES);
  if (!continuous_map)
    return 0;
  u8 *h = continuous_map;
  continuous_prefixes = bypass_equal(h, MNM_CONT_PREFIX_MAGIC, 8);
  continuous_snapshot = bypass_equal(h, MNM_CONT_SNAPSHOT_MAGIC, 8);
  continuous_resident = continuous_prefixes || continuous_snapshot || bypass_equal(h, MNM_CONT_RESIDENT_MAGIC, 8);
  if (!bypass_equal(h, continuous_prefixes ? MNM_CONT_PREFIX_MAGIC : continuous_snapshot ? MNM_CONT_SNAPSHOT_MAGIC : continuous_resident ? MNM_CONT_RESIDENT_MAGIC : MNM_CONT_MAGIC, 8) ||
      get(h + 8) != (continuous_prefixes ? MNM_CONT_PREFIX_VERSION : continuous_snapshot ? MNM_CONT_SNAPSHOT_VERSION : continuous_resident ? MNM_CONT_RESIDENT_VERSION : MNM_CONT_VERSION) ||
      get(h + 12) != MNM_CONT_HEADER || get(h + 16) != MNM_CONT_BYTES ||
      get(h + 20) != (continuous_prefixes ? MNM_CONT_PREFIX_MODE : continuous_snapshot ? MNM_CONT_SNAPSHOT_MODE : continuous_resident ? MNM_CONT_RESIDENT_MODE : 0) || (!get(h + 24) && !get(h + 28)) ||
      continuous_load(h, 8) != 1 || continuous_load(h, 9) ||
      continuous_load(h, 10) || continuous_load(h, 11))
    return 0;
  for (u32 i = 14; i < 32; ++i)
    if (get(h + i * 4))
      return 0;
  continuous_nonce_low=get(h+24);continuous_nonce_high=get(h+28);
  if (!__sync_bool_compare_and_swap((u32 *)(h + 36), 0, 1))
    return 0;
  char public_mode[2];u32 public_n=GetEnvironmentVariableA("MNM_PUBLIC_RESIDENT_WORLD",public_mode,2);
  if(public_n){
    if(public_n!=1 || public_mode[0]!='1' || !continuous_resident)return 0;
    HANDLE module=GetModuleHandleA("MnmRender.dll");
    if(!module || !(continuous_public_bind=(ResidentWorldBind)GetProcAddress(module,"RenderResidentWorldV1")))return 0;
  }
  if(continuous_prefixes && !continuous_public_bind)return 0;
  return continuous_clock_install() && continuous_next();
}
static int continuous_write(const void *p, u32 n) {
  u8 *s = continuous_slot();
  if (continuous_load(s, 0) != MNM_CONT_WRITING ||
      continuous_written > MNM_CONT_SOURCE_BYTES ||
      n > MNM_CONT_SOURCE_BYTES - continuous_written)
    return 0;
  copy(s + MNM_CONT_SLOT_HEADER + continuous_written, p, n);
  continuous_written += n;
  return 1;
}
static u8* continuous_payload(u32 n) {
  u8 *slot = continuous_slot();
  if (continuous_load(slot, 0) != MNM_CONT_WRITING || !n ||
      continuous_written > MNM_CONT_SOURCE_BYTES ||
      continuous_written < 64 + 96 || n > continuous_written - 64 - 96)
    return 0;
  return slot + MNM_CONT_SLOT_HEADER + continuous_written - n;
}
static int continuous_finish(struct Surface *s, const u32 *request) {
  u8 *slot = continuous_slot();
  if (continuous_load(slot, 0) != MNM_CONT_WRITING || !batch_identity(s) ||
      !batch_guard_open)
    return 0;
  const u32 request_n = 64 + request[11];
  if (bytes > MNM_CONT_SOURCE_BYTES - 192 || request_n > MNM_CONT_REQUEST_BYTES)
    return 0;
  u8 *source = slot + MNM_CONT_SLOT_HEADER,
     *wire = source + MNM_CONT_SOURCE_BYTES;
  copy(wire, request, 64);
  copy(wire + 64, batch_rows, request[11]);
  put(slot + 16, bytes);
  put(slot + 20, request_n);
  put(slot + 24, mnm_cont_crc32c(source, bytes));
  put(slot + 28, mnm_cont_crc32c(wire, request_n));
  put(slot+84,continuous_region_queries);
  put(slot + 52, continuous_time());
  continuous_store(slot, 0, MNM_CONT_REQUEST);
  if (!continuous_wait(slot, 0, MNM_CONT_REPLY) ||
      get(slot + 8) != continuous_generation_low ||
      get(slot + 12) != continuous_generation_high ||
      get(slot + 40) != (continuous_resident ? MNM_CONT_ACK_BYTES : MNM_CONT_REPLY_BYTES))
    return 0;
  u8 *reply = wire + MNM_CONT_REQUEST_BYTES;
  if (continuous_resident) {
    if (!bypass_equal(continuous_map, continuous_prefixes ? MNM_CONT_PREFIX_MAGIC : continuous_snapshot ? MNM_CONT_SNAPSHOT_MAGIC : MNM_CONT_RESIDENT_MAGIC, 8) ||
        get(continuous_map + 8) != (continuous_prefixes ? MNM_CONT_PREFIX_VERSION : continuous_snapshot ? MNM_CONT_SNAPSHOT_VERSION : MNM_CONT_RESIDENT_VERSION) ||
        get(continuous_map + 20) != (continuous_prefixes ? MNM_CONT_PREFIX_MODE : continuous_snapshot ? MNM_CONT_SNAPSHOT_MODE : MNM_CONT_RESIDENT_MODE) ||
        get(continuous_map + 24) != continuous_nonce_low ||
        get(continuous_map + 28) != continuous_nonce_high ||
        mnm_cont_crc32c(reply, MNM_CONT_ACK_BYTES) != get(slot + 44) ||
        !(continuous_snapshot ? mnm_snapshot_ack_valid : mnm_ack_valid)(reply, get(slot + 40), (const u8*)request,
                       continuous_generation_low, continuous_generation_high,
                       continuous_nonce_low, continuous_nonce_high) ||
        !batch_identity(s) || !batch_restore())return 0;
    if(continuous_public_bind && !continuous_public_bind(s->pixels,s->width,s->height,s->stride,
       continuous_generation_low,continuous_generation_high,continuous_nonce_low,continuous_nonce_high))return 0;
    /* Completion count remains the local batch ordinal. No pixel copy occurs. */
    ++batch_transfers;
    put(slot + 56, continuous_time());
    continuous_store(slot, 0, MNM_CONT_WRITING);
    return 1;
  }
  u32 h[16];
  copy(h, reply, 64);
  if (mnm_cont_crc32c(reply, MNM_CONT_REPLY_BYTES) != get(slot + 44) ||
      !bypass_equal(h, MNM_WORLD_BATCH_REPLY, 8) || h[2] != MNM_CONT_BATCH_VERSION || h[3] != 64 ||
      h[11] != batch_pixel_bytes || h[13] != 1)
    return 0;
  for (u32 i = 4; i <= 10; ++i)
    if (h[i] != request[i])
      return 0;
  for (u32 i = 14; i <= 15; ++i)
    if (h[i] != request[i])
      return 0;
  if (mnm_cont_crc32c(reply + 64, batch_pixel_bytes) != h[12] ||
      !batch_identity(s) || !batch_restore() ||
      !bypass_writable(s->pixels, batch_pixel_bytes))
    return 0;
  copy((void *)s->pixels, reply + 64, batch_pixel_bytes);
  ++batch_transfers;
  put(slot + 56, continuous_time());
  /* Return records may now be appended, but the source slot stays exclusively
   * original-owned until it publishes the actual return. */
  continuous_store(slot, 0, MNM_CONT_WRITING);
  return 1;
}
static int continuous_return(void) {
  u8 *slot = continuous_slot();
  if (continuous_load(slot, 0) != MNM_CONT_WRITING ||
      bytes != get(slot + 16) + 192 || failures)
    return 0;
  put(slot + 32, bytes);
  put(slot + 60, continuous_time());
  continuous_store(slot, 0, MNM_CONT_RETURNED);
  const u32 low = get(continuous_map + 48), high = get(continuous_map + 52);
  if ((low || high) && low == continuous_generation_low &&
      high == continuous_generation_high) {
    if (!continuous_wait(slot, 0, MNM_CONT_FREE))
      return 0;
    continuous_store(continuous_map, 11, 1);
    stopped = 1;
    return 1;
  }
  if (++continuous_generation_low == 0 && ++continuous_generation_high == 0)
    return 0;
  continuous_slot_index ^= 1;
  return continuous_next();
}

static u32 continuous_screen_start,continuous_screen_ms,continuous_screen_calls;
static void continuous_mark_screen(int begin) {
  if(!continuous_map)return;
  if(begin)continuous_screen_start=continuous_time();
  else {continuous_screen_ms+=continuous_time()-continuous_screen_start;++continuous_screen_calls;}
}
static void continuous_mark_entry(void) {
  if (continuous_map){
    batch_owner=GetCurrentThreadId();
    u8* slot=continuous_slot();
    put(slot+48,continuous_time());
    if(continuous_pacer_mask==3){put(slot+64,continuous_pacer_start);put(slot+68,continuous_pacer_end);put(slot+72,3);}
    put(slot+76,continuous_screen_ms);put(slot+80,continuous_screen_calls);
    continuous_pacer_start=continuous_pacer_end=continuous_pacer_mask=0;
    continuous_screen_ms=continuous_screen_calls=0;
    continuous_region_queries=0;
  }
}

static int continuous_equal(const u8 *a, const u8 *b, u32 n) {
  while (n >= 16) {
    if (get(a) != get(b) || get(a + 4) != get(b + 4) ||
        get(a + 8) != get(b + 8) || get(a + 12) != get(b + 12))
      return 0;
    a += 16;
    b += 16;
    n -= 16;
  }
  while (n--) {
    if (*a++ != *b++)
      return 0;
  }
  return 1;
}

/* Called under the emission lock before source cache lookup/definition+use.
 * Never split a World or reset a borrowed source definition in mid-operation.
 * The original callback may wait, but native draining never calls into it. */
static int continuous_drain_prefix(void) {
  if(!continuous_prefixes || raster_world_open ||
     (sequence<MNM_CONT_PREFIX_RECORDS && bytes<MNM_CONT_PREFIX_BYTES))return 1;
  u8* slot=continuous_slot();
  if(continuous_load(slot,0)!=MNM_CONT_WRITING || !sequence || failures ||
     continuous_written!=bytes || (continuous_source_low==0xffffffffu && continuous_source_high==0xffffffffu))return 0;
  const u32 n=bytes,digest=mnm_cont_crc32c(slot+MNM_CONT_SLOT_HEADER,n);
  // Source-only header excludes leftover World timing, requests and replies.
  zero(slot+16,112);put(slot+16,n);put(slot+24,digest);
  put(slot+88,continuous_source_low);put(slot+92,continuous_source_high);
  continuous_store(slot,0,MNM_CONT_SOURCE_REQUEST);
  if(!continuous_wait(slot,0,MNM_CONT_SOURCE_ACK) ||
     !bypass_equal(continuous_map,MNM_CONT_PREFIX_MAGIC,8) ||
     get(continuous_map+8)!=MNM_CONT_PREFIX_VERSION || get(continuous_map+20)!=MNM_CONT_PREFIX_MODE ||
     get(continuous_map+24)!=continuous_nonce_low || get(continuous_map+28)!=continuous_nonce_high ||
     get(slot+8)!=continuous_generation_low || get(slot+12)!=continuous_generation_high ||
     get(slot+16)!=n || get(slot+24)!=digest ||
     get(slot+88)!=continuous_source_low || get(slot+92)!=continuous_source_high)return 0;
  for(u32 i=0;i<32;++i)
    if(i!=0 && i!=2 && i!=3 && i!=4 && i!=6 && i!=22 && i!=23 && get(slot+i*4))return 0;
  if(++continuous_source_low==0)++continuous_source_high;
  // Cache pointers point into the slot. Retire them only after native applied
  // every owned operation, then reuse storage without changing World identity.
  zero(source_packets,sizeof(source_packets));zero(continuous_source_heads,sizeof(continuous_source_heads));
  source_packet_count=source_packet_bytes=sequence=oracle=oracle_bytes=0;bytes=continuous_written=64;
  zero(slot+16,112);continuous_store(slot,0,MNM_CONT_WRITING);
  return 1;
}
