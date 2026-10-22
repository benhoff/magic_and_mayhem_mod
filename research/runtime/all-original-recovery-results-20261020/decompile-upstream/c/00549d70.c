
/* WARNING: Removing unreachable block (ram,0x00549e00) */

undefined4 * __thiscall
FUN_00549d70(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9,undefined4 param_10)

{
  int iVar1;
  uint uVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2978;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00548650(param_4,param_5,param_6,param_7,param_8,param_9);
  local_4 = 0;
  *(undefined4 *)((int)param_1 + 0x5a) = 0;
  iVar1 = DAT_005e1424 / 10;
  *(undefined4 *)((int)param_1 + 0x72) = param_10;
  *(int *)((int)param_1 + 0x5e) = iVar1;
  *(undefined4 *)((int)param_1 + 0x76) = param_2;
  *(undefined4 *)((int)param_1 + 0x7a) = param_3;
  uVar2 = FUN_0054e200();
  *(uint *)((int)param_1 + 0x7e) = uVar2 & 7;
  *param_1 = &PTR_FUN_005c752c;
  FUN_0054a5f0();
  ExceptionList = local_c;
  return param_1;
}

