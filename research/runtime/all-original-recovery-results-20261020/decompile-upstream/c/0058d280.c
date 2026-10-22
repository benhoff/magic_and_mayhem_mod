
bool __thiscall FUN_0058d280(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 unaff_retaddr;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2e88;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  FUN_00486950();
  local_4 = 0;
  iVar1 = FUN_00486ae0(param_2);
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 8) + 0x44))(*(int **)(param_1 + 8),&param_2);
    FUN_00486a80(&local_4,unaff_retaddr,param_2);
    (**(code **)(**(int **)(param_1 + 8) + 0x68))(*(int **)(param_1 + 8),local_4);
  }
  local_4 = 0xffffffff;
  FUN_00486960();
  ExceptionList = pvStack_c;
  return iVar1 != 0;
}

