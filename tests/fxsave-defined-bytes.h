#pragma once
#include <stdint.h>
/* PUSHAD/flags plus a48-byte capture prefix and32-bit FXSAVE. Empty x87
 * payloads and reserved bytes have no preservation contract. Test the saved
 * before tag map; changed tags, live x87 values, control/status, FIP/FDP,
 * MXCSR and all eight XMM registers still compare exactly. */
static inline int fixture_state_byte_defined(const uint8_t before[560],unsigned offset){
 if(offset<48)return 1;
 const unsigned i=offset-48;
 if(i>=288||i==5||i==14||i==15||i==22||i==23)return 0;
 if(i>=32&&i<160){
  const unsigned reg=(i-32)/16,byte=(i-32)%16,top=(before[51]>>3)&7;
  return byte<10&&(before[52]&(1u<<((top+reg)&7)));
 }
 return 1;
}
