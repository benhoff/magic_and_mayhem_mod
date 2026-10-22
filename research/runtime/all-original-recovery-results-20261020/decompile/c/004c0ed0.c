
void __fastcall FUN_004c0ed0(int param_1)

{
  int local_4;
  
  local_4 = param_1;
  FUN_004302d0(&local_4,**(undefined4 **)(param_1 + 4),*(undefined4 **)(param_1 + 4));
  FUN_00597870(*(undefined4 *)(param_1 + 4));
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  DAT_006e2044 = DAT_006e2044 + -1;
  if (DAT_006e2044 == 0) {
    FUN_00597870(DAT_005faf54);
    DAT_005faf54 = 0;
  }
  return;
}

