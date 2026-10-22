
void __fastcall FUN_004d9240(undefined4 *param_1)

{
  int *piVar1;
  int iVar2;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005c2045;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FUN_005c6ec0;
  local_4 = 2;
  piVar1 = (int *)((int)param_1 + 0x5f7);
  iVar2 = 10;
  do {
    if ((int *)*piVar1 != (int *)0x0) {
      (**(code **)(*(int *)*piVar1 + 4))(1);
      *piVar1 = 0;
    }
    piVar1 = piVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  if (*(int **)((int)param_1 + 0x61f) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x61f) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x61f) = 0;
  }
  if (param_1[0x169] != 0) {
    FUN_0058b3e0();
  }
  local_4 = CONCAT31(local_4._1_3_,1);
  if (*(int *)((int)param_1 + 0x5d2) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x5d2));
    *(undefined4 *)((int)param_1 + 0x5d2) = 0;
    *(undefined4 *)((int)param_1 + 0x5d6) = 0;
    *(undefined4 *)((int)param_1 + 0x5da) = 0;
    *(undefined4 *)((int)param_1 + 0x5de) = 0;
  }
  piVar1 = (int *)((int)param_1 + 0x5e2);
  iVar2 = 4;
  do {
    if (*piVar1 != 0) {
      FUN_0057d0d0(*piVar1);
      *piVar1 = 0;
    }
    piVar1 = piVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  if (*(int *)((int)param_1 + 0x5f2) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x5f2));
    *(undefined4 *)((int)param_1 + 0x5f2) = 0;
  }
  local_4 = local_4 & 0xffffff00;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  FUN_0059bf07(param_1 + 2,0x8a,10,FUN_004d9200);
  ExceptionList = pvStack_c;
  return;
}

