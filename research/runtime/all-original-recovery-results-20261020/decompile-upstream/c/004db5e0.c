
void __fastcall FUN_004db5e0(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c215b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_FUN_005c6f24;
  local_4 = 0;
  param_1[2] = 0;
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  FUN_0058a070();
  ExceptionList = local_c;
  return;
}

