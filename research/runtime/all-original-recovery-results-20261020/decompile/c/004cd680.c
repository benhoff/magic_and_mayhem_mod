
void __fastcall FUN_004cd680(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  undefined **local_30;
  undefined4 local_2c;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c1bb9;
  local_c = ExceptionList;
  iVar5 = 2;
  iVar1 = *(int *)(*(int *)(param_1 + 0x31) + 0x20);
  iVar2 = *(int *)(*(int *)(param_1 + 0x31) + 0x24);
  local_40 = *(int *)(param_1 + 0x11) + *(int *)(*DAT_006e2070 + 4 + DAT_006e2074) + iVar1;
  ExceptionList = &local_c;
  *(int *)(param_1 + 0x6d) = local_40;
  if (*(uint *)(DAT_006e2068 + 0xc) < 2) {
    iVar5 = 0;
  }
  iVar6 = 5;
  if (*(uint *)(DAT_006e2068 + 0xc) < 5) {
    iVar6 = 0;
  }
  *(int *)(param_1 + 0x71) =
       ((iVar1 - *(int *)(DAT_006e2070[iVar6] + 4 + DAT_006e2074)) -
       *(int *)(DAT_006e2070[iVar5] + 4 + DAT_006e2074)) + -2 + *(int *)(param_1 + 0x19);
  iVar5 = 2;
  if (*(uint *)(DAT_006e2068 + 0xc) < 2) {
    iVar5 = 0;
  }
  iVar6 = 2;
  if (*(uint *)(DAT_006e2068 + 0xc) < 2) {
    iVar6 = 0;
  }
  local_34 = *(int *)(DAT_006e2070[iVar6] + 8 + DAT_006e2074);
  local_3c = (((iVar2 * 2 - *(int *)(param_1 + 0x15)) - local_34) + *(int *)(param_1 + 0x1d)) / 2 +
             *(int *)(param_1 + 0x15) + iVar2;
  local_38 = *(int *)(DAT_006e2070[iVar5] + 4 + DAT_006e2074) + local_40;
  local_34 = local_34 + local_3c;
  puVar4 = (undefined4 *)FUN_00597850(0x8a);
  local_4 = 0;
  if (puVar4 == (undefined4 *)0x0) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    FUN_004cb4b0();
    *puVar4 = &PTR_LAB_005c6cd4;
    *(undefined4 *)((int)puVar4 + 0x7a) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x82) = 0;
    *(undefined4 *)((int)puVar4 + 0x7e) = 0;
    *(undefined4 *)((int)puVar4 + 0x86) = 1;
  }
  *(undefined4 **)(param_1 + 0x41) = puVar4;
  *(undefined **)((int)puVar4 + 0x31) = &DAT_006a49e0;
  local_4 = 0xffffffff;
  *(int **)(*(int *)(param_1 + 0x41) + 0x55) = &DAT_006e2068;
  iVar5 = *(int *)(param_1 + 0x41);
  *(undefined4 *)(iVar5 + 0x59) = 5;
  *(undefined4 *)(iVar5 + 0x5d) = 5;
  *(undefined4 *)(iVar5 + 0x61) = 0xffffffff;
  iVar5 = *(int *)(param_1 + 0x41);
  *(undefined4 *)(iVar5 + 0x65) = 0xffffffff;
  *(undefined4 *)(iVar5 + 0x69) = 6;
  *(undefined4 *)(iVar5 + 0x6d) = 0xffffffff;
  iVar5 = *(int *)(param_1 + 0x41);
  uVar3 = *(undefined4 *)(param_1 + 0x71);
  *(undefined4 *)(iVar5 + 0x7e) = *(undefined4 *)(param_1 + 0x6d);
  *(undefined4 *)(iVar5 + 0x82) = uVar3;
  iVar5 = *(int *)(param_1 + 0x41);
  uVar3 = *(undefined4 *)(param_1 + 0x7e);
  *(undefined4 *)(iVar5 + 0x29) = *(undefined4 *)(param_1 + 0x82);
  *(undefined4 *)(iVar5 + 0x25) = uVar3;
  *(undefined4 *)(iVar5 + 0x2d) = 0;
  FUN_004a4e20(&local_40,1);
  *(undefined4 *)(iVar5 + 0x39) = 0;
  *(undefined4 *)(iVar5 + 0x3d) = 0;
  local_34 = *(int *)(DAT_006e2074 + *DAT_006e2070 + 8);
  local_40 = *(int *)(param_1 + 0x11) + iVar1;
  local_3c = (((iVar2 * 2 - *(int *)(param_1 + 0x15)) - local_34) + *(int *)(param_1 + 0x1d)) / 2 +
             *(int *)(param_1 + 0x15) + iVar2;
  local_38 = *(int *)(DAT_006e2074 + 4 + *DAT_006e2070) + local_40;
  local_34 = local_34 + local_3c;
  puVar4 = (undefined4 *)FUN_00597850(0x7a);
  local_4 = 1;
  if (puVar4 == (undefined4 *)0x0) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    FUN_004cafa0();
    *puVar4 = &PTR_LAB_005c6bd0;
    *(undefined4 *)((int)puVar4 + 0x55) = 0;
    *(undefined4 *)((int)puVar4 + 0x61) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x5d) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x59) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x6d) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x69) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x65) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x71) = 0;
    *(undefined4 *)((int)puVar4 + 0x75) = 0;
    *(undefined1 *)((int)puVar4 + 0x79) = 1;
  }
  *(undefined4 **)(param_1 + 0x45) = puVar4;
  *(undefined **)((int)puVar4 + 0x31) = &DAT_006a49e0;
  local_4 = 0xffffffff;
  *(int **)(*(int *)(param_1 + 0x45) + 0x55) = &DAT_006e2068;
  iVar5 = *(int *)(param_1 + 0x45);
  *(undefined4 *)(iVar5 + 0x59) = 0;
  *(undefined4 *)(iVar5 + 0x5d) = 1;
  *(undefined4 *)(iVar5 + 0x61) = 0xffffffff;
  iVar5 = *(int *)(param_1 + 0x45);
  *(undefined4 *)(iVar5 + 0x25) = *(undefined4 *)(param_1 + 0x7a);
  *(undefined4 *)(iVar5 + 0x29) = 0;
  *(undefined4 *)(iVar5 + 0x2d) = 0;
  FUN_004a4e20(&local_40,1);
  *(undefined4 *)(iVar5 + 0x39) = 0;
  *(undefined4 *)(iVar5 + 0x3d) = 0;
  iVar5 = 2;
  if (*(uint *)(DAT_006e2068 + 0xc) < 2) {
    iVar5 = 0;
  }
  local_38 = *(int *)(DAT_006e2070[iVar5] + 4 + DAT_006e2074);
  iVar5 = 2;
  if (*(uint *)(DAT_006e2068 + 0xc) < 2) {
    iVar5 = 0;
  }
  local_34 = *(int *)(DAT_006e2070[iVar5] + 8 + DAT_006e2074);
  local_40 = (iVar1 - local_38) + *(int *)(param_1 + 0x19);
  local_3c = (((iVar2 * 2 - *(int *)(param_1 + 0x15)) - local_34) + *(int *)(param_1 + 0x1d)) / 2 +
             *(int *)(param_1 + 0x15) + iVar2;
  local_38 = local_38 + local_40;
  local_34 = local_34 + local_3c;
  puVar4 = (undefined4 *)FUN_00597850(0x7a);
  local_4 = 2;
  if (puVar4 == (undefined4 *)0x0) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    FUN_004ca8e0();
    local_4 = CONCAT31(local_4._1_3_,3);
    *puVar4 = &PTR_LAB_005c5f24;
    FUN_004a4df0();
    local_30 = &PTR_LAB_005c5c20;
    FUN_004a4f70();
    local_2c = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x39) = 0;
    *(undefined4 *)((int)puVar4 + 0x3d) = 0;
    *(undefined4 *)((int)puVar4 + 0x45) = 0;
    *(undefined4 *)((int)puVar4 + 0x49) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x4d) = 0;
    *(undefined4 *)((int)puVar4 + 0x51) = 0x321;
    *puVar4 = &PTR_LAB_005c6bd0;
    *(undefined4 *)((int)puVar4 + 0x55) = 0;
    *(undefined4 *)((int)puVar4 + 0x61) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x5d) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x59) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x6d) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x69) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x65) = 0xffffffff;
    *(undefined4 *)((int)puVar4 + 0x71) = 0;
    *(undefined4 *)((int)puVar4 + 0x75) = 0;
    *(undefined1 *)((int)puVar4 + 0x79) = 1;
  }
  *(undefined4 **)(param_1 + 0x49) = puVar4;
  *(undefined **)((int)puVar4 + 0x31) = &DAT_006a49e0;
  *(int **)(*(int *)(param_1 + 0x49) + 0x55) = &DAT_006e2068;
  iVar1 = *(int *)(param_1 + 0x49);
  local_4 = 0xffffffff;
  *(undefined4 *)(iVar1 + 0x59) = 2;
  *(undefined4 *)(iVar1 + 0x5d) = 3;
  *(undefined4 *)(iVar1 + 0x61) = 0xffffffff;
  iVar1 = *(int *)(param_1 + 0x49);
  *(undefined4 *)(iVar1 + 0x25) = *(undefined4 *)(param_1 + 0x7a);
  *(undefined4 *)(iVar1 + 0x29) = 0;
  *(undefined4 *)(iVar1 + 0x2d) = 1;
  FUN_004a4e20(&local_40,1);
  *(undefined4 *)(iVar1 + 0x39) = 0;
  *(undefined4 *)(iVar1 + 0x3d) = 0;
  ExceptionList = local_c;
  return;
}

