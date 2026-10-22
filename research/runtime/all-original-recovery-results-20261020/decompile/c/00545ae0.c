
void __thiscall FUN_00545ae0(int param_1,int param_2,int param_3,int param_4)

{
  if (param_2 < 0x100) {
    if (param_2 < 0) {
      param_2 = 0;
    }
  }
  else {
    param_2 = 0xff;
  }
  *(int *)(param_1 + 8) = (int)(param_2 + (param_2 >> 0x1f & 7U)) >> 3;
  if (param_3 < 0x100) {
    if (param_3 < 0) {
      param_3 = 0;
    }
  }
  else {
    param_3 = 0xff;
  }
  *(int *)(param_1 + 0xc) = param_3 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
  if (param_4 < 0x100) {
    if (param_4 < 0) {
      *(undefined4 *)(param_1 + 0x10) = 0;
      return;
    }
  }
  else {
    param_4 = 0xff;
  }
  *(int *)(param_1 + 0x10) = (int)(param_4 + (param_4 >> 0x1f & 7U)) >> 3;
  return;
}

