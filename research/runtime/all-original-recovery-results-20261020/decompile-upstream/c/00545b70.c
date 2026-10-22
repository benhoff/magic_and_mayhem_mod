
undefined4 * __thiscall
FUN_00545b70(undefined4 *param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5,
            int param_6)

{
  *param_1 = param_2;
  param_1[1] = param_3;
  if (param_4 < 0x100) {
    if (param_4 < 0) {
      param_4 = 0;
    }
  }
  else {
    param_4 = 0xff;
  }
  param_1[2] = (int)(param_4 + (param_4 >> 0x1f & 7U)) >> 3;
  if (param_5 < 0x100) {
    if (param_5 < 0) {
      param_5 = 0;
    }
  }
  else {
    param_5 = 0xff;
  }
  param_1[3] = param_5 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
  if (param_6 < 0x100) {
    if (param_6 < 0) {
      param_1[4] = 0;
      return param_1;
    }
  }
  else {
    param_6 = 0xff;
  }
  param_1[4] = (int)(param_6 + (param_6 >> 0x1f & 7U)) >> 3;
  return param_1;
}

