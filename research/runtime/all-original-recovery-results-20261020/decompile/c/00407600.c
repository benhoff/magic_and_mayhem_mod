
uint __fastcall FUN_00407600(int *param_1)

{
  undefined8 uVar1;
  uint uVar2;
  
  if (DAT_006e1f64 != 0) {
    if (DAT_006e1f64 != 1) {
      return 0;
    }
    if (DAT_006a4910 != 0) {
      uVar1 = rdtsc();
      uVar2 = *param_1 - (uint)((int)uVar1 - param_1[1]) / DAT_006a4914;
      return ((int)uVar2 < 1) - 1 & uVar2;
    }
  }
  return param_1[2];
}

