
uint __fastcall FUN_00555e20(uint param_1,uint param_2,uint param_3)

{
  if (0xff < (ushort)param_1) {
    param_1 = 0xff;
  }
  if (0xff < (ushort)param_2) {
    param_2 = 0xff;
  }
  if (0xff < (ushort)param_3) {
    param_3 = 0xff;
  }
  if (DAT_006e1f88 != 0) {
    return ((param_1 & 0xf8) << 5 | param_2 & 0xf8) << 2 | param_3 >> 3 & 0x1f;
  }
  return ((param_1 & 0xf8) << 5 | param_2 & 0xfc) << 3 | param_3 >> 3 & 0x1f;
}

