
void __thiscall FUN_00547890(int param_1,uint param_2,uint param_3,uint param_4)

{
  if (((param_2 != *(ushort *)(param_1 + 0xc)) || (param_3 != *(ushort *)(param_1 + 0xe))) ||
     (param_4 != *(ushort *)(param_1 + 0x10))) {
    if ((int)param_2 < 0x100) {
      if ((int)param_2 < 0) {
        param_2 = 0;
      }
    }
    else {
      param_2 = 0xff;
    }
    *(short *)(param_1 + 0xc) = (short)((int)(param_2 + ((int)param_2 >> 0x1f & 7U)) >> 3);
    if ((int)param_3 < 0x100) {
      if ((int)param_3 < 0) {
        param_3 = 0;
      }
    }
    else {
      param_3 = 0xff;
    }
    *(short *)(param_1 + 0xe) = (short)((int)param_3 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4));
    if ((int)param_4 < 0x100) {
      if ((int)param_4 < 0) {
        param_4 = 0;
      }
    }
    else {
      param_4 = 0xff;
    }
    *(short *)(param_1 + 0x10) = (short)((int)(param_4 + ((int)param_4 >> 0x1f & 7U)) >> 3);
    if (*(int *)(param_1 + 0x12) != 0) {
      FUN_00547b70();
    }
  }
  return;
}

