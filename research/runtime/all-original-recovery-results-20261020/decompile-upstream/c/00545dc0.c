
undefined4 * __thiscall
FUN_00545dc0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  int iVar1;
  
  FUN_00545590(param_2,param_3,param_4);
  param_1[4] = param_5;
  param_1[5] = param_6;
  param_1[6] = param_7;
  *param_1 = &PTR_FUN_005c73c0;
  param_1[7] = 0x80;
  param_1[8] = 0x80;
  param_1[9] = 0x20;
  param_1[10] = 100;
  param_1[0xb] = 5;
  param_1[0xc] = 0;
  param_1[0xd] = 2;
  param_1[0xe] = 2;
  param_1[0xf] = 0x10;
  iVar1 = DAT_006e1f88;
  param_1[0x11] = 4;
  param_1[0x10] = (int)(0x80 / (ulonglong)(longlong)(int)((-(uint)(iVar1 != 0) & 4) + 4));
  *param_1 = &PTR_FUN_005c73f0;
  return param_1;
}

