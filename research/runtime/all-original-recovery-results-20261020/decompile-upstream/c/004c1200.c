
void __fastcall FUN_004c1200(undefined4 *param_1)

{
  int *piVar1;
  int local_14;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1741;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_LAB_005c6a60;
  local_4 = 10;
  if (*(int *)((int)param_1 + 0x12) != 0) {
    FUN_00550370();
  }
  local_4._0_1_ = 9;
  FUN_0056d930();
  local_4._0_1_ = 8;
  FUN_004cab70();
  local_4._0_1_ = 7;
  FUN_004cb560();
  local_4._0_1_ = 6;
  FUN_004cb560();
  local_4._0_1_ = 5;
  FUN_004cb560();
  local_4._0_1_ = 4;
  FUN_004cb560();
  local_4._0_1_ = 3;
  FUN_00464480();
  local_4 = CONCAT31(local_4._1_3_,2);
  if (*(int *)((int)param_1 + 0x9f) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x9f));
    *(undefined4 *)((int)param_1 + 0x9f) = 0;
    *(undefined4 *)((int)param_1 + 0xa3) = 0;
    *(undefined4 *)((int)param_1 + 0xa7) = 0;
    *(undefined4 *)((int)param_1 + 0xab) = 0;
  }
  piVar1 = (int *)((int)param_1 + 0xaf);
  local_14 = 4;
  do {
    if (*piVar1 != 0) {
      FUN_0057d0d0(*piVar1);
      *piVar1 = 0;
    }
    piVar1 = piVar1 + 1;
    local_14 = local_14 + -1;
  } while (local_14 != 0);
  if (*(int *)((int)param_1 + 0xbf) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0xbf));
    *(undefined4 *)((int)param_1 + 0xbf) = 0;
  }
  local_4 = CONCAT31(local_4._1_3_,1);
  FUN_0058b3e0();
  if (*(int *)((int)param_1 + 0x59) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0x59));
    *(undefined4 *)((int)param_1 + 0x59) = 0;
    *(undefined4 *)((int)param_1 + 0x5d) = 0;
  }
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = local_c;
  return;
}

