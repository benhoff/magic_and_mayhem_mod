
undefined4 * __thiscall
FUN_00548ac0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  uint uVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c28d8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00545590(param_2,param_3,param_4);
  param_1[8] = param_6;
  local_4 = 0;
  param_1[4] = 10;
  param_1[5] = 0x14;
  param_1[6] = 0xfffffffe;
  param_1[7] = param_5;
  param_1[9] = param_7;
  uVar1 = FUN_0054e200();
  param_1[0xb] = 0xe0;
  param_1[0xc] = 0x20;
  param_1[0xd] = 0;
  param_1[0xe] = 0x40;
  param_1[0xf] = 0;
  *(undefined1 *)(param_1 + 0x10) = 1;
  *param_1 = &PTR_FUN_005c74cc;
  param_1[10] = (uVar1 & 0x7fffffff) % 6 + 3;
  ExceptionList = local_c;
  return param_1;
}

