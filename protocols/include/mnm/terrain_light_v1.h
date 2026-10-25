#pragma once
/* Little-endian owned byte records. No game/host pointers or C++ object layouts.
 * Input:128-byte header,128 relation bytes,11u32 states,creatures*28,
 * objects*32,five byte buffers. Header word offsets are authoritative.
 * Result:changed u32,11u32 states,five buffers. Reply32-byte header binds source.
 */
#define MNM_LIGHT_V1_MAGIC "MNMLIT01"
#define MNM_LIGHT_V1_REPLY "MNMLRP01"
#define MNM_LIGHT_V1_HEADER 128u
#define MNM_LIGHT_V1_MAX_BYTES (8u*1024u*1024u)
#define MNM_LIGHT_V1_MAX_SOURCES 65536u
#define MNM_LIGHT_V1_BUILD 0x40209ca7u
static unsigned int mnm_light_checksum(const unsigned char* p,unsigned int n){unsigned int h=2166136261u;for(unsigned int i=0;i<n;++i){unsigned char v=(i>=120 && i<124)?0:p[i];h=(h^v)*16777619u;}return h;}
