
void __fastcall FUN_004da130(undefined4 *param_1)

{
  undefined4 *puVar1;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_005c20b0;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FUN_005c6ed8;
  param_1[2] = &PTR_LAB_005c6ee8;
  local_4 = 2;
  if (*(int **)((int)param_1 + 0x1bf) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x1bf) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x1bf) = 0;
  }
  if (*(int *)((int)param_1 + 0x16d) != 0) {
    FUN_0058b3e0();
  }
  FUN_0058a0a0();
  FUN_004a4f70();
  param_1[3] = 0xffffffff;
  local_4._0_1_ = 1;
  FUN_0058b3e0();
  local_4 = (uint)local_4._1_3_ << 8;
  FUN_0058a070();
  local_4 = 0xffffffff;
  puVar1 = (undefined4 *)(-(uint)(param_1 != (undefined4 *)0x0) & (uint)(param_1 + 2));
  *puVar1 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  puVar1[1] = 0xffffffff;
  ExceptionList = pvStack_c;
  return;
}

