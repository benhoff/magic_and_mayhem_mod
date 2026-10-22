
void __fastcall FUN_004d8250(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1fdb;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_FUN_005c6eb0;
  local_4 = 0;
  param_1[2] = 0;
  if (param_1[3] != 0) {
    FUN_00597870(param_1[3]);
  }
  if (param_1[4] != 0) {
    FUN_00597870(param_1[4]);
  }
  if (param_1[5] != 0) {
    FUN_00597870(param_1[5]);
  }
  *(undefined4 *)((int)param_1 + 0x166) = 0;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  FUN_0058a070();
  ExceptionList = local_c;
  return;
}

