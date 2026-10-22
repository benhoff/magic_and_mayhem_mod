
void __fastcall FUN_004d5c70(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_005c1fbc;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FUN_005c6e88;
  uVar2 = 0;
  local_4 = 6;
  if (param_1[0x3e] != 0) {
    piVar3 = param_1 + 2;
    do {
      if ((int *)*piVar3 != (int *)0x0) {
        (**(code **)(*(int *)*piVar3 + 4))(1);
        *piVar3 = 0;
      }
      uVar2 = uVar2 + 1;
      piVar3 = piVar3 + 1;
    } while (uVar2 < (uint)param_1[0x3e]);
  }
  local_4 = CONCAT31(local_4._1_3_,5);
  if (param_1[0x632] != 0) {
    FUN_005979b0(param_1[0x632]);
    param_1[0x632] = 0;
    param_1[0x633] = 0;
    param_1[0x634] = 0;
    param_1[0x635] = 0;
  }
  piVar3 = param_1 + 0x636;
  iVar1 = 4;
  do {
    if (*piVar3 != 0) {
      FUN_0057d0d0(*piVar3);
      *piVar3 = 0;
    }
    piVar3 = piVar3 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (param_1[0x63a] != 0) {
    FUN_005979b0(param_1[0x63a]);
    param_1[0x63a] = 0;
  }
  local_4 = CONCAT31(local_4._1_3_,4);
  if (param_1[0x629] != 0) {
    FUN_005979b0(param_1[0x629]);
    param_1[0x629] = 0;
    param_1[0x62a] = 0;
    param_1[0x62b] = 0;
    param_1[0x62c] = 0;
  }
  piVar3 = param_1 + 0x62d;
  iVar1 = 4;
  do {
    if (*piVar3 != 0) {
      FUN_0057d0d0(*piVar3);
      *piVar3 = 0;
    }
    piVar3 = piVar3 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (param_1[0x631] != 0) {
    FUN_005979b0(param_1[0x631]);
    param_1[0x631] = 0;
  }
  local_4._0_1_ = 3;
  FUN_0058b3e0();
  local_4._0_1_ = 2;
  FUN_0058b3e0();
  local_4._0_1_ = 1;
  FUN_0059bf07((int)param_1 + 0x1596,0xc6,3,FUN_004d5bc0);
  local_4 = (uint)local_4._1_3_ << 8;
  *(undefined4 *)((int)param_1 + 0x1571) = 0;
  param_1[0x547] = &PTR_LAB_005c5f24;
  if (*(int *)((int)param_1 + 0x1561) != 0) {
    FUN_00597870(*(int *)((int)param_1 + 0x1561));
  }
  param_1[0x547] = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  param_1[0x548] = 0xffffffff;
  local_4 = 0xffffffff;
  FUN_0059bf07(param_1 + 0x3f,0xc6,0x1a,FUN_004d5bc0);
  ExceptionList = pvStack_c;
  return;
}

