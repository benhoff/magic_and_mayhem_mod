
void __fastcall FUN_004c0cf0(int param_1)

{
  int local_4;
  
  local_4 = param_1;
  FUN_0042b6f0(&local_4,**(undefined4 **)(param_1 + 4),*(undefined4 **)(param_1 + 4));
  FUN_00597870(*(undefined4 *)(param_1 + 4));
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  DAT_006e2040 = DAT_006e2040 + -1;
  if (DAT_006e2040 == 0) {
    FUN_00597870(DAT_005faf50);
    DAT_005faf50 = 0;
  }
  return;
}

