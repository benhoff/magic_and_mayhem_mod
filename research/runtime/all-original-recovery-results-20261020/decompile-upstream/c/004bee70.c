
void __fastcall FUN_004bee70(int param_1)

{
  int local_4;
  
  local_4 = param_1;
  FUN_0047cce0(&local_4,**(undefined4 **)(param_1 + 4),*(undefined4 **)(param_1 + 4));
  FUN_00597870(*(undefined4 *)(param_1 + 4));
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  DAT_006e203c = DAT_006e203c + -1;
  if (DAT_006e203c == 0) {
    FUN_00597870(DAT_005fce98);
    DAT_005fce98 = 0;
  }
  return;
}

