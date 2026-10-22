
void __fastcall FUN_004d4260(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005c1eb3;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FUN_005c6e4c;
  local_4 = 1;
  if (*(int *)((int)param_1 + 0x82) != 0) {
    FUN_0058b3e0();
  }
  if (*(int **)((int)param_1 + 0xb1) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0xb1) + 4))(1);
    *(undefined4 *)((int)param_1 + 0xb1) = 0;
  }
  local_4 = local_4 & 0xffffff00;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x55) = 0;
  *param_1 = &PTR_LAB_005c5f24;
  if (*(int *)((int)param_1 + 0x45) != 0) {
    FUN_00597870(*(int *)((int)param_1 + 0x45));
  }
  *param_1 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  param_1[1] = 0xffffffff;
  ExceptionList = pvStack_c;
  return;
}

