
void __thiscall FUN_004075b0(int *param_1,int param_2)

{
  undefined8 uVar1;
  
  if (DAT_006e1f64 == 0) {
    *param_1 = param_2;
    param_1[2] = param_2;
  }
  else if (DAT_006e1f64 == 1) {
    *param_1 = param_2;
    param_1[2] = param_2 / 0xfa;
    if (DAT_006a4910 != 0) {
      uVar1 = rdtsc();
      param_1[1] = (int)uVar1;
      return;
    }
  }
  return;
}

