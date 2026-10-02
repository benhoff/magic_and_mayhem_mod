/* Ghidra inferred pseudocode; not original or buildable source.
 * Entry VA: 0040e290; see manifest.json for executable hash. */

void __thiscall FUN_0040e290(uint *param_1,uint param_2)

{
  if ((int)param_2 < 0) {
    do {
      param_2 = param_2 + DAT_006c5494;
    } while ((int)param_2 < 0);
    *param_1 = param_2;
    return;
  }
  for (; DAT_006c5494 <= param_2; param_2 = param_2 - DAT_006c5494) {
  }
  *param_1 = param_2;
  return;
}

