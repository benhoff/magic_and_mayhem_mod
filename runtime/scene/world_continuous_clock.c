/* Included by continuous source adapter. Real clock policy shared with the
 * validated native command route, admitted independently for this channel. */
API void *WIN GetProcAddress(HANDLE, const char *);
#define MNM_PACER_POLICY_TEST
#define PACER_FIRST_YIELD(caller) ((void)(caller))
#define same(a, b, n) bypass_equal(a, b, n)
#include "../render/pacer_yield.h"
#undef same
#undef PACER_FIRST_YIELD
#undef MNM_PACER_POLICY_TEST
static u32 continuous_pacer_start,continuous_pacer_end,continuous_pacer_mask;
static u32 WIN continuous_clock_read(void) {
  u32 caller=(u32)__builtin_return_address(0),tick=pacer_read(0,0);
  /* Record the existing wait's actual entry/commit calls; the returned clock
   * and original comparison target are unchanged. No extra clock call. */
  if(caller==0x4e3f59){continuous_pacer_start=tick;continuous_pacer_mask=1;}
  else if(caller==0x4e3f97&&continuous_pacer_mask==1){continuous_pacer_end=tick;continuous_pacer_mask=3;}
  return tick;
}
static int continuous_clock_install(void) {
  char option[2];
  u32 n = GetEnvironmentVariableA("MNM_CONTINUOUS_PRECISE_CLOCK", option, 2);
  if (!n)
    return 1;
  if (n != 1 || option[0] != '1')
    return 0;
  u32 *slot = (u32 *)0x5c5164;
  HANDLE kernel = GetModuleHandleA("kernel32.dll");
  PacerTickFn original = (PacerTickFn)GetProcAddress(kernel, "GetTickCount");
  HANDLE winmm = GetModuleHandleA("winmm.dll");
  PacerTickFn fine =
      winmm ? (PacerTickFn)GetProcAddress(winmm, "timeGetTime") : 0;
  if (!continuous_map || continuous_load(continuous_map, 8) != 1 ||
      continuous_load(continuous_map, 9) != 1 || !readable(0x4e3e5b, 6) ||
      !readable(0x4e3f89, 12) || !readable((u32)slot, 4) ||
      !pacer_entry_matches((void *)0x4e3e5b, (void *)0x4e3f89, *slot,
                           (u32)original, (u32)GetModuleHandleA(0)) ||
      !fine || !precise_clock_epoch_matches(original(), fine()))
    return 0;
  const u8 first[8]={0xff,0xd6,0x2b,0x05,0x00,0x81,0x6e,0x00};
  const u8 last[7]={0xff,0xd6,0xa3,0x00,0x81,0x6e,0x00};
  if(!readable(0x4e3f57,8)||!readable(0x4e3f95,7)||
     !bypass_equal((void*)0x4e3f57,first,8)||!bypass_equal((void*)0x4e3f95,last,7))return 0;
  u32 protection, ignored;
  if (!VirtualProtect(slot, 4, 4, &protection))
    return 0;
  pacer_original_tick = original;
  pacer_precise_tick = fine;
  pacer_enabled = 0;
  __atomic_store_n(slot, (u32)&continuous_clock_read, __ATOMIC_RELEASE);
  return VirtualProtect(slot, 4, protection, &ignored) != 0;
}
