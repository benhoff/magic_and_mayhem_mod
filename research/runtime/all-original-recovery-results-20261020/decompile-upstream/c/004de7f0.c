
void __fastcall FUN_004de7f0(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c21bb;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FUN_005c6f80;
  local_4 = 0;
  if (*(int **)((int)param_1 + 0x42) != (int *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x42) + 4))(1);
    *(undefined4 *)((int)param_1 + 0x42) = 0;
  }
  if (param_1[5] != 0) {
    FUN_0058b3e0();
  }
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return;
}

