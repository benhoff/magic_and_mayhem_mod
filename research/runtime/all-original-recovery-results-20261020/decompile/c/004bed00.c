
void __fastcall FUN_004bed00(int param_1)

{
  int local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1228;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  local_10 = param_1;
  FUN_0059bf07(param_1 + 0x10,0x1d,100,FUN_00477d00);
  local_4 = 0xffffffff;
  FUN_0047cce0(&local_10,**(undefined4 **)(param_1 + 4),*(undefined4 **)(param_1 + 4));
  FUN_00597870(*(undefined4 *)(param_1 + 4));
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  DAT_006e203c = DAT_006e203c + -1;
  if (DAT_006e203c == 0) {
    FUN_00597870(DAT_005fce98);
    DAT_005fce98 = 0;
  }
  ExceptionList = local_c;
  return;
}

