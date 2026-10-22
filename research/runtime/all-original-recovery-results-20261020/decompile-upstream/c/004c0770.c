
void __fastcall FUN_004c0770(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1508;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  if (DAT_005faf50 == (undefined4 *)0x0) {
    ExceptionList = &local_c;
    DAT_005faf50 = (undefined4 *)FUN_00597850(0x18);
    DAT_005faf50[1] = 0;
    DAT_005faf50[5] = 1;
    *DAT_005faf50 = 0;
    DAT_005faf50[2] = 0;
  }
  puVar1 = DAT_005faf50;
  DAT_006e2040 = DAT_006e2040 + 1;
  iVar2 = FUN_00597850(0x18);
  *(undefined4 **)(iVar2 + 4) = puVar1;
  *(undefined4 *)(iVar2 + 0x14) = 0;
  *(int *)(param_1 + 4) = iVar2;
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(int *)iVar2 = iVar2;
  *(int *)(*(int *)(param_1 + 4) + 8) = *(int *)(param_1 + 4);
  ExceptionList = local_c;
  return;
}

