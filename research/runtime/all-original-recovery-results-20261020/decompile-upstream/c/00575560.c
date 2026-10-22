
void __fastcall FUN_00575560(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_005c2d09;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_LAB_005c775c;
  local_4 = 1;
  if (param_1[2] != 0) {
    FUN_005782a0();
  }
  FUN_005759c0();
  local_4 = local_4 & 0xffffff00;
  FUN_00534b60();
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = local_c;
  return;
}

