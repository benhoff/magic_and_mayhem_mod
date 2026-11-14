#ifndef MNM_CONT_CRC32C_H
#define MNM_CONT_CRC32C_H
#if defined(__aarch64__) && defined(__linux__) && \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#include <arm_acle.h>
#include <asm/hwcap.h>
#include <sys/auxv.h>
/* Keep CRC instructions in this function; the baseline caller must also run
 * on ARM64 CPUs without the optional CRC extension. Wide loads use memcpy to
 * support unaligned buffers without aliasing or alignment assumptions. */
__attribute__((target("+crc")))
static inline unsigned mnm_cont_crc32c_arm64(const unsigned char *p, unsigned n) {
  unsigned h = ~0u;
  while (n >= 8) {
    unsigned long long w;
    __builtin_memcpy(&w, p, sizeof(w));
    h = __crc32cd(h, w);
    p += 8;
    n -= 8;
  }
  while (n--)
    h = __crc32cb(h, *p++);
  return ~h;
}
#endif
/* Standard reflected CRC-32C (Castagnoli), init/final xor0xffffffff. This is
 * corruption detection, not authentication. Byte order is fixed regardless of
 * host alignment. The portable implementation is also the independent oracle.
 */
static inline unsigned mnm_cont_crc32c_portable(const unsigned char *p,
                                                unsigned n) {
  unsigned h = ~0u;
  while (n--) {
    h ^= *p++;
    for (unsigned k = 0; k < 8; ++k)
      h = (h >> 1) ^ (0x82f63b78u & (0u - (h & 1u)));
  }
  return ~h;
}
#if defined(__i386__) || defined(__x86_64__)
static inline unsigned mnm_cont_crc32c_hardware(const unsigned char *p,
                                                unsigned n) {
  unsigned h = ~0u;
#if defined(__x86_64__)
  unsigned long long wide = h;
  while (n >= 8) {
    unsigned long long w = 0;
    __builtin_memcpy(&w, p, sizeof(w));
    __asm__("crc32q %1, %0" : "+r"(wide) : "r"(w));
    p += 8;
    n -= 8;
  }
  h = (unsigned)wide;
#endif
  while (n >= 4) {
    const unsigned w = p[0] | (unsigned)p[1] << 8 | (unsigned)p[2] << 16 |
                       (unsigned)p[3] << 24;
    __asm__("crc32l %1, %0" : "+r"(h) : "r"(w));
    p += 4;
    n -= 4;
  }
  while (n--)
    __asm__("crc32b %1, %0" : "+r"(h) : "r"(*p++));
  return ~h;
}
#endif
static inline unsigned mnm_cont_crc32c(const unsigned char *p, unsigned n) {
#if defined(__i386__) || defined(__x86_64__)
  static unsigned capability;
  unsigned ready = __atomic_load_n(&capability, __ATOMIC_ACQUIRE);
  if (!ready) {
    unsigned a = 1, b, c, d;
    __asm__("cpuid" : "+a"(a), "=b"(b), "=c"(c), "=d"(d));
    ready = c & (1u << 20) ? 2u : 1u;
    __atomic_store_n(&capability, ready, __ATOMIC_RELEASE);
  }
  if (ready == 2)
    return mnm_cont_crc32c_hardware(p, n);
#elif defined(__aarch64__) && defined(__linux__) && \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  static unsigned capability;
  unsigned ready = __atomic_load_n(&capability, __ATOMIC_ACQUIRE);
  if (!ready) {
    ready = getauxval(AT_HWCAP) & HWCAP_CRC32 ? 2u : 1u;
    __atomic_store_n(&capability, ready, __ATOMIC_RELEASE);
  }
  if (ready == 2)
    return mnm_cont_crc32c_arm64(p, n);
#endif
  return mnm_cont_crc32c_portable(p, n);
}
#endif
