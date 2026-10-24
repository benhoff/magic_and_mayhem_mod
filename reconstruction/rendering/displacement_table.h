#ifndef MNM_DISPLACEMENT_TABLE_H
#define MNM_DISPLACEMENT_TABLE_H
#include <stddef.h>
#include <stdint.h>

/* NoCD 57dbd0, including the truncating 59bee0 conversion. The approximate
 * binary64 pi/8 and negative amplitude are the original constants. Native
 * owned output; no original image or observed table is consumed.
 *
 * Admitted domain: x86 x87, all exceptions masked, empty register stack,
 * precision 24/53/64 and any rounding direction. Reject reserved precision,
 * unmasked exceptions, occupied stack or short output before mutation.
 * Control word and empty stack are preserved. Arithmetic exception flags are
 * retained/raised as by the original. FIP/FDP and empty register bytes are not
 * a contract. This is not an installation into the running original process.
 */
static inline int mnm_initialize_displacement_table(int32_t* table, size_t words) {
#if defined(__i386__) || defined(__x86_64__)
  struct { unsigned char bytes[512]; } __attribute__((aligned(16))) state;
  uint16_t control;
  __asm__ volatile("fnstcw %0; fxsave %1" : "=m"(control), "=m"(state));
  if (!table || words < 16 || (control & 63) != 63 ||
      (control & 0x300) == 0x100 || state.bytes[4] != 0) return 0;
  const double angle = 0x1.921fb52713c02p-2, amplitude = -0x1p+3;
  const uint16_t truncate = (uint16_t)(control | 0xc00);
  for (int32_t i = 0; i < 16; ++i) {
    int64_t value;
    __asm__ volatile("fildl %1; fmull %2; fsin; fmull %3; "
                     "fldcw %4; fistpq %0; fldcw %5"
                     : "=m"(value)
                     : "m"(i), "m"(angle), "m"(amplitude),
                       "m"(truncate), "m"(control) : "st");
    table[i] = 8 - (int32_t)value;
  }
  return 1;
#else
  (void)table; (void)words;
  return 0;
#endif
}
#endif
