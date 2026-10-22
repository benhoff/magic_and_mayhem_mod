
void __fastcall FUN_004c0c90(int param_1)

{
  int local_4;
  
  local_4 = param_1;
  FUN_00430800(&local_4,**(undefined4 **)(param_1 + 4),*(undefined4 **)(param_1 + 4));
  FUN_00597870(*(undefined4 *)(param_1 + 4));
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  DAT_006e2048 = DAT_006e2048 + -1;
  if (DAT_006e2048 == 0) {
    FUN_00597870(DAT_005faf58);
    DAT_005faf58 = 0;
  }
  return;
}

