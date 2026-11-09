#pragma once
#include <cmath>
namespace mnm::reconstruction {
// Positive finite binary64 operands of 0059bcc0/005a05e0, retaining the x87
// result and the caller's precision/rounding context until its original spill.
// Palette dark-wing factors use this path; host powl has different rounding.
// Native non-x86 fallback remains a policy, not original numerical equivalence.
inline long double paletteDarkPower(double base,double power){
#if defined(__i386__) || defined(__x86_64__)
 long double result;
 asm volatile("fldl %2; fldl %1; fyl2x; fld %%st(0); frndint; fxch; "
              "fsub %%st(1), %%st(0); f2xm1; fld1; faddp; fscale; fstp %%st(1); fstpt %0"
              : "=m"(result) : "m"(base), "m"(power)
              : "st", "st(1)", "st(2)");
 return result;
#else
 return std::pow(static_cast<long double>(base),static_cast<long double>(power));
#endif
}
}
