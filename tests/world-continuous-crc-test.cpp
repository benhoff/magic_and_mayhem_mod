/* Standalone C/C++ checksum check, including dispatch without ARM CRC support. */
#if defined(__aarch64__) && defined(__linux__) && \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define TEST_ARM64_CRC 1
#include <sys/auxv.h>
#include <asm/hwcap.h>
static unsigned long test_caps;
static unsigned test_queries;
static unsigned long test_getauxval(unsigned long type) {
  (void)type;
  ++test_queries;
  return test_caps;
}
#define getauxval test_getauxval
#endif
#include "../protocols/include/mnm/world_continuous_checksum.h"
#if defined(TEST_ARM64_CRC)
#undef getauxval
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
#if defined(TEST_ARM64_CRC)
  const unsigned long host_caps = getauxval(AT_HWCAP);
  if (argc > 1 && !(host_caps & HWCAP_CRC32))
    return 77;
  test_caps = argc > 1 ? HWCAP_CRC32 : 0;
#else
  (void)argc;
#endif
  (void)argv;
  assert(mnm_cont_crc32c((const unsigned char *)"123456789", 9) == 0xe3069283u);
  assert(mnm_cont_crc32c(0, 0) == 0);
  unsigned char data[8208];
  for (unsigned i = 0; i < sizeof(data); ++i)
    data[i] = (unsigned char)(i * 37u + (i >> 3));
  for (unsigned offset = 0; offset < 16; ++offset) {
    for (unsigned n = 0; n <= 128; ++n)
      assert(mnm_cont_crc32c(data + offset, n) ==
             mnm_cont_crc32c_portable(data + offset, n));
    for (unsigned n = 129; n <= 8192; n += 127)
      assert(mnm_cont_crc32c(data + offset, n) ==
             mnm_cont_crc32c_portable(data + offset, n));
  }
#if defined(TEST_ARM64_CRC)
  assert(test_queries == 1);
#endif
  puts("CRC dispatch, known vector, empty input, alignments and tails passed");
}
