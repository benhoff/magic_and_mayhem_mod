
void __fastcall FUN_00572400(int param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c2c1b;
  pvStack_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &pvStack_c;
  if (*(int **)(param_1 + 0x42) != (int *)0x0) {
    ExceptionList = &pvStack_c;
    (**(code **)(**(int **)(param_1 + 0x42) + 4))(1);
    *(undefined4 *)(param_1 + 0x42) = 0;
  }
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return;
}

