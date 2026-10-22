
void __fastcall FUN_004dc410(int param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005c2186;
  pvStack_c = ExceptionList;
  local_4 = 1;
  ExceptionList = &pvStack_c;
  FUN_0058b3e0();
  FUN_0058b3e0();
  if (*(int **)(param_1 + 0x80) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x80) + 4))(1);
    *(undefined4 *)(param_1 + 0x80) = 0;
  }
  local_4 = local_4 & 0xffffff00;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return;
}

