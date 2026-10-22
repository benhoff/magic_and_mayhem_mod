
undefined4 __thiscall FUN_0058d1a0(int param_1,undefined4 param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2e68;
  pvStack_c = ExceptionList;
  uVar3 = 0;
  ExceptionList = &pvStack_c;
  FUN_00486950();
  local_4 = 0;
  iVar2 = FUN_00486ae0(param_2);
  if (iVar2 != 0) {
    piVar1 = *(int **)(param_1 + 8);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x44))(piVar1,&param_2);
      FUN_00486a80(&local_4,0,0);
      (**(code **)(**(int **)(param_1 + 8) + 0x68))(*(int **)(param_1 + 8),local_4);
    }
    uVar3 = 1;
  }
  local_4 = 0xffffffff;
  FUN_00486960();
  ExceptionList = pvStack_c;
  return uVar3;
}

