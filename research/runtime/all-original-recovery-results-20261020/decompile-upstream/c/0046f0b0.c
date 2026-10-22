
void __fastcall FUN_0046f0b0(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 uStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_005bf4e1;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LAB_005c5dd8;
  iVar2 = *(int *)((int)param_1 + 0xfc22);
  local_4 = 5;
  if (iVar2 != 0) {
    FUN_0059bf07(iVar2,0x10,*(undefined4 *)(iVar2 + -4),&LAB_0046f2e0);
    FUN_00597870(iVar2 + -4);
  }
  iVar2 = *(int *)((int)param_1 + 0x100b3);
  if (iVar2 != 0) {
    FUN_004cb560();
    FUN_00597870(iVar2);
    *(undefined4 *)((int)param_1 + 0x100b3) = 0;
  }
  if (*(int **)((int)param_1 + 0x100b7) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x100b7) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x100b7) = 0;
  }
  local_4._0_1_ = 4;
  FUN_00534b60();
  local_4 = CONCAT31(local_4._1_3_,3);
  if (*(int *)((int)param_1 + 0x100bb) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x100bb));
    *(undefined4 *)((int)param_1 + 0x100bb) = 0;
    *(undefined4 *)((int)param_1 + 0x100bf) = 0;
    *(undefined4 *)((int)param_1 + 0x100c3) = 0;
    *(undefined4 *)((int)param_1 + 0x100c7) = 0;
  }
  piVar3 = (int *)((int)param_1 + 0x100cb);
  iVar2 = 4;
  do {
    if (*piVar3 != 0) {
      FUN_0057d0d0(*piVar3);
      *piVar3 = 0;
    }
    piVar3 = piVar3 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  if (*(int *)((int)param_1 + 0x100db) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x100db));
    *(undefined4 *)((int)param_1 + 0x100db) = 0;
  }
  local_4._0_1_ = 2;
  FUN_00473b00();
  local_4._0_1_ = 1;
  FUN_004dc410();
  iVar2 = *(int *)((int)param_1 + 0xfc52);
  local_4 = (uint)local_4._1_3_ << 8;
  while (iVar2 != 0) {
    iVar4 = *(int *)((int)param_1 + 0xfc32) + 4;
    *(int *)((int)param_1 + 0xfc32) = iVar4;
    *(int *)((int)param_1 + 0xfc52) = iVar2 + -1;
    if ((iVar2 + -1 == 0) || (iVar4 == *(int *)((int)param_1 + 0xfc2e))) {
      puVar1 = *(undefined4 **)((int)param_1 + 0xfc36);
      *(undefined4 **)((int)param_1 + 0xfc36) = puVar1 + 1;
      FUN_00597870(*puVar1);
      if (*(int *)((int)param_1 + 0xfc52) == 0) {
        FUN_00473270();
        *(undefined4 *)((int)param_1 + 0xfc2a) = uStack_1c;
        *(undefined4 *)((int)param_1 + 0xfc2e) = uStack_18;
        *(undefined4 *)((int)param_1 + 0xfc32) = uStack_14;
        *(undefined4 *)((int)param_1 + 0xfc36) = uStack_10;
        *(undefined4 *)((int)param_1 + 0xfc3a) = uStack_1c;
        *(undefined4 *)((int)param_1 + 0xfc3e) = uStack_18;
        *(undefined4 *)((int)param_1 + 0xfc42) = uStack_14;
        *(undefined4 *)((int)param_1 + 0xfc46) = uStack_10;
        FUN_00597870(*(undefined4 *)((int)param_1 + 0xfc4a));
      }
      else {
        iVar2 = **(int **)((int)param_1 + 0xfc36);
        *(int *)((int)param_1 + 0xfc2a) = iVar2;
        *(int *)((int)param_1 + 0xfc2e) = iVar2 + 0x1000;
        *(int *)((int)param_1 + 0xfc32) = iVar2;
        *(int **)((int)param_1 + 0xfc36) = *(int **)((int)param_1 + 0xfc36);
      }
    }
    iVar2 = *(int *)((int)param_1 + 0xfc52);
  }
  local_4 = 0xffffffff;
  FUN_005458b0();
  ExceptionList = pvStack_c;
  return;
}

