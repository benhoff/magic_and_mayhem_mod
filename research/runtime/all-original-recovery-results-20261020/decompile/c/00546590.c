
undefined4 * __thiscall
FUN_00546590(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  bool bVar1;
  
  FUN_00545590(param_2,param_3,param_4);
  param_1[4] = param_5;
  param_1[5] = param_6;
  param_1[6] = param_7;
  *param_1 = &PTR_FUN_005c73d8;
  param_1[7] = 0x80;
  param_1[8] = 0x80;
  param_1[9] = 0x20;
  param_1[10] = 0x4b;
  param_1[0xb] = DAT_005e1464;
  param_1[0xc] = 2;
  param_1[0xd] = 2;
  param_1[0xe] = 0x10;
  bVar1 = DAT_006e1f88 != 0;
  param_1[0x10] = 4;
  param_1[0xf] = (int)(0x80 / (ulonglong)(longlong)(int)((-(uint)bVar1 & 4) + 4));
  *(undefined1 *)(param_1 + 0x11) = 0;
  *param_1 = &PTR_FUN_005c7420;
  return param_1;
}

