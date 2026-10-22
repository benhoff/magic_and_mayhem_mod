
void __fastcall FUN_004daba0(undefined4 *param_1)

{
  uint uVar1;
  int *piVar2;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005c20f3;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LAB_005c6f10;
  *(undefined ***)((int)param_1 + 0x21) = &PTR_FUN_005c6f00;
  uVar1 = 0;
  local_4 = 1;
  if (*(int *)((int)param_1 + 0x67) != 0) {
    piVar2 = (int *)((int)param_1 + 0x5f);
    do {
      if ((int *)*piVar2 != (int *)0x0) {
        (**(code **)(*(int *)*piVar2 + 4))(1);
        *piVar2 = 0;
      }
      uVar1 = uVar1 + 1;
      piVar2 = piVar2 + 1;
    } while (uVar1 < *(uint *)((int)param_1 + 0x67));
  }
  if (*(int *)((int)param_1 + 0x20b) != 0) {
    FUN_00597870(*(int *)((int)param_1 + 0x20b));
  }
  if (*(int *)((int)param_1 + 0x2d) != 0) {
    FUN_0058b3e0();
  }
  if (*(int **)((int)param_1 + 0x217) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x217) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x217) = 0;
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

