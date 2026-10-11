/* Included after world_raster_batch.c. Pointer-free fixed storage shared with
 * the native owner. The original destination stays NOACCESS while waiting. */
#include "../../protocols/include/mnm/world_continuous_v3.h"
API HANDLE WIN CreateFileMappingA(HANDLE, void *, u32, u32, u32, const char *);
API void *WIN MapViewOfFile(HANDLE, u32, u32, u32, u32);
API i32 WIN UnmapViewOfFile(void *);
static HANDLE continuous_file, continuous_mapping;
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
static int continuous_wait(u8 *p, u32 n, u32 value) {
  u32 start = GetTickCount();
  while (continuous_load(p, n) != value) {
    if (continuous_load(continuous_map, 10) ||
        (u32)(GetTickCount() - start) >= MNM_WORLD_BYPASS_TIMEOUT)
      return 0;
    Sleep(0);
  }
  return !continuous_load(continuous_map, 10);
}
static void continuous_cancel(void) {
  if (continuous_map)
    continuous_store(continuous_map, 10, 1);
}
static int continuous_next(void) {
  u8 *slot = continuous_slot();
  if (!continuous_wait(slot, 0, MNM_CONT_FREE))
    return 0;
  if (!__sync_bool_compare_and_swap((u32 *)slot, MNM_CONT_FREE,
                                    MNM_CONT_WRITING))
    return 0;
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
  return 1;
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
  if (!bypass_equal(h, MNM_CONT_MAGIC, 8) || get(h + 8) != MNM_CONT_VERSION ||
      get(h + 12) != MNM_CONT_HEADER || get(h + 16) != MNM_CONT_BYTES ||
      get(h + 20) || (!get(h + 24) && !get(h + 28)) ||
      continuous_load(h, 8) != 1 || continuous_load(h, 9) ||
      continuous_load(h, 10) || continuous_load(h, 11))
    return 0;
  for (u32 i = 14; i < 32; ++i)
    if (get(h + i * 4))
      return 0;
  if (!__sync_bool_compare_and_swap((u32 *)(h + 36), 0, 1))
    return 0;
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
      get(slot + 40) != MNM_CONT_REPLY_BYTES)
    return 0;
  u8 *reply = wire + MNM_CONT_REQUEST_BYTES;
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
