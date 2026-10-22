
void __fastcall FUN_00463270(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_005bf39e;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LAB_005c5c2c;
  local_4 = 2;
  FUN_0058b3e0();
  FUN_0058b3e0();
  if (*(int **)((int)param_1 + 0x93) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x93) + 4))(1);
  }
  *(undefined4 *)((int)param_1 + 0x93) = 0;
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x85));
  *(undefined4 *)((int)param_1 + 0x85) = 0;
  local_4._0_1_ = 1;
  FUN_0058b3e0();
  local_4 = (uint)local_4._1_3_ << 8;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  *param_1 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  param_1[1] = 0xffffffff;
  ExceptionList = pvStack_c;
  return;
}

