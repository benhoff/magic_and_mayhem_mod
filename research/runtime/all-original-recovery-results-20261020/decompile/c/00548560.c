
undefined4 * __thiscall
FUN_00548560(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2898;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00545590(param_2,param_3,param_4);
  param_1[4] = param_5;
  param_1[5] = param_6;
  param_1[6] = param_7;
  *param_1 = &PTR_FUN_005c73d8;
  local_4 = 0;
  param_1[7] = 2;
  param_1[8] = 2;
  param_1[9] = 2;
  *(undefined4 *)((int)param_1 + 0x2e) = 0;
  FUN_00547890(0xc0,0,0);
  param_1[7] = 2;
  *param_1 = &PTR_FUN_005c7494;
  ExceptionList = local_c;
  return param_1;
}

