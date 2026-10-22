
void FUN_00473900(void)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005bf5bb;
  local_c = ExceptionList;
  if (DAT_006f98dc == (undefined4 *)0x0) {
    ExceptionList = &local_c;
    puVar1 = (undefined4 *)FUN_00597850(0x18);
    local_4 = 0;
    if (puVar1 == (undefined4 *)0x0) {
      DAT_006f98dc = (undefined4 *)0x0;
    }
    else {
      *puVar1 = 0;
      uVar2 = FUN_00597850(DAT_005f1a90 * 4);
      *puVar1 = uVar2;
      FUN_0058f3c0(0);
      FUN_0058f6c0();
      DAT_006f98dc = puVar1;
    }
  }
  ExceptionList = local_c;
  return;
}

