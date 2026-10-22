
void __fastcall FUN_004c1010(int param_1)

{
  int local_4;
  
  local_4 = param_1;
  FUN_0042f330(&local_4,**(undefined4 **)(param_1 + 4),*(undefined4 **)(param_1 + 4));
  FUN_00597870(*(undefined4 *)(param_1 + 4));
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  DAT_006e204c = DAT_006e204c + -1;
  if (DAT_006e204c == 0) {
    FUN_00597870(DAT_005faf4c);
    DAT_005faf4c = 0;
  }
  return;
}

