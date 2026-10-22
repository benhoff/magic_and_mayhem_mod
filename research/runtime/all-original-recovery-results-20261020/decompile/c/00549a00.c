
undefined4 * __thiscall
FUN_00549a00(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            int param_5,int param_6)

{
  int iVar1;
  uint uVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2958;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00545590(param_2,param_3,param_4);
  iVar1 = DAT_006dbe99;
  local_4 = 0;
  uVar2 = FUN_0054e200();
  *param_1 = &PTR_FUN_005c7514;
  param_1[5] = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar1) + param_6;
  param_1[4] = param_5;
  param_1[6] = (*(int *)(*(int *)(param_5 + 0xac) + 0x198) - param_1[1]) + *(int *)(param_5 + 0x14);
  param_1[7] = (*(int *)(*(int *)(param_5 + 0xac) + 0x198) - param_1[2]) + *(int *)(param_5 + 0x18);
  param_1[8] = (*(int *)(*(int *)(param_5 + 0xac) + 0x198) - param_1[3]) + *(int *)(param_5 + 0x1c);
  ExceptionList = local_c;
  return param_1;
}

