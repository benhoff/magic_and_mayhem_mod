
void __fastcall FUN_004d34d0(undefined4 *param_1)

{
  int iVar1;
  int *piVar2;
  int iStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005c1e75;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LAB_005c6e38;
  *(undefined ***)((int)param_1 + 0x21) = &PTR_FUN_005c6e28;
  local_4 = 5;
  if ((int *)param_1[0x7d] != (int *)0x0) {
    (**(code **)(*(int *)param_1[0x7d] + 4))(1);
  }
  param_1[0x7d] = 0;
  local_4._0_1_ = 4;
  FUN_0058a070();
  local_4 = CONCAT31(local_4._1_3_,3);
  if (*(int *)((int)param_1 + 0xa3) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0xa3));
    *(undefined4 *)((int)param_1 + 0xa3) = 0;
    *(undefined4 *)((int)param_1 + 0xa7) = 0;
    *(undefined4 *)((int)param_1 + 0xab) = 0;
    *(undefined4 *)((int)param_1 + 0xaf) = 0;
  }
  piVar2 = (int *)((int)param_1 + 0xb3);
  iStack_14 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iStack_14 = iStack_14 + -1;
  } while (iStack_14 != 0);
  if (*(int *)((int)param_1 + 0xc3) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0xc3));
    *(undefined4 *)((int)param_1 + 0xc3) = 0;
  }
  local_4 = CONCAT31(local_4._1_3_,2);
  if (*(int *)((int)param_1 + 0x7f) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x7f));
    *(undefined4 *)((int)param_1 + 0x7f) = 0;
    *(undefined4 *)((int)param_1 + 0x83) = 0;
    *(undefined4 *)((int)param_1 + 0x87) = 0;
    *(undefined4 *)((int)param_1 + 0x8b) = 0;
  }
  piVar2 = (int *)((int)param_1 + 0x8f);
  iVar1 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (*(int *)((int)param_1 + 0x9f) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x9f));
    *(undefined4 *)((int)param_1 + 0x9f) = 0;
  }
  local_4 = CONCAT31(local_4._1_3_,1);
  if (*(int *)((int)param_1 + 0x5b) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x5b));
    *(undefined4 *)((int)param_1 + 0x5b) = 0;
    *(undefined4 *)((int)param_1 + 0x5f) = 0;
    *(undefined4 *)((int)param_1 + 99) = 0;
    *(undefined4 *)((int)param_1 + 0x67) = 0;
  }
  piVar2 = (int *)((int)param_1 + 0x6b);
  iVar1 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (*(int *)((int)param_1 + 0x7b) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x7b));
    *(undefined4 *)((int)param_1 + 0x7b) = 0;
  }
  local_4 = local_4 & 0xffffff00;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  *param_1 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  param_1[1] = 0xffffffff;
  ExceptionList = pvStack_c;
  return;
}

